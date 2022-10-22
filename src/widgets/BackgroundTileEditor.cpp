/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "BackgroundTileEditor.h"
#include "core/field/BackgroundFile.h"

BackgroundTileEditor::BackgroundTileEditor(QWidget *parent)
    : QWidget(parent)
{
	_formPage = new QWidget(this);

	_tileWidget = new ImageGridWidget(_formPage);
	_tileWidget->setCellSize(1);
	_tileWidget->setFixedHeight(32 * 4);

	_bgParamGroup = new QGroupBox(tr("Parameter"), _formPage);
	_bgParamGroup->setCheckable(true);
	
	_bgParamInput = new QSpinBox(_bgParamGroup);
	_bgParamInput->setRange(1, 255);
	_bgParamStateInput = new QSpinBox(_bgParamGroup);
	_bgParamStateInput->setRange(1, 8);

	QFormLayout *tileParameterEditorLayout = new QFormLayout(_bgParamGroup);
	tileParameterEditorLayout->addRow(tr("Param ID"), _bgParamInput);
	tileParameterEditorLayout->addRow(tr("State ID"), _bgParamStateInput);

	_blendTypeInput = new QComboBox(_formPage);
	_blendTypeInput->addItem(tr("None"), -1);
	_blendTypeInput->addItem(tr("Average"), 0);
	_blendTypeInput->addItem(tr("Plus"), 1);
	_blendTypeInput->addItem(tr("Minus"), 2);
	_blendTypeInput->addItem(tr("Source +25% destination"), 3);

	_paletteIdInput = new QSpinBox(_formPage);
	_paletteIdInput->setRange(0, 255);

	_tileEditorLayout = new QFormLayout();
	_tileEditorLayout->addRow(tr("Blend type"), _blendTypeInput);
	_tileEditorLayout->addRow(tr("Palette ID"), _paletteIdInput);

	QVBoxLayout *layout = new QVBoxLayout(_formPage);
	layout->addWidget(_tileWidget);
	layout->addLayout(_tileEditorLayout);
	layout->addWidget(_bgParamGroup);
	layout->setContentsMargins(QMargins());

	_createPage = new QWidget(this);

	_colorTypeInput = new QComboBox(_createPage);
	_colorTypeInput->addItem(tr("Paletted 8-bit"), 1);
	_colorTypeInput->addItem(tr("Direct color 16-bit"), 2);

	QPushButton *createButton = new QPushButton(tr("Create tile"), _createPage);

	_tileCreateLayout = new QFormLayout();
	_tileCreateLayout->addRow(tr("Color type"), _colorTypeInput);

	QVBoxLayout *createPageLayout = new QVBoxLayout(_createPage);
	createPageLayout->addLayout(_tileCreateLayout);
	createPageLayout->addWidget(createButton, 0, Qt::AlignRight);
	createPageLayout->addStretch(1);

	_stackedLayout = new QStackedLayout(this);
	_stackedLayout->addWidget(_formPage);
	_stackedLayout->addWidget(_createPage);
	
	connect(_blendTypeInput, &QComboBox::currentIndexChanged, this, &BackgroundTileEditor::updateBlendType);
	connect(createButton, &QPushButton::clicked, this, &BackgroundTileEditor::createTile);
	connect(_bgParamInput, &QSpinBox::valueChanged, this, &BackgroundTileEditor::updateBgParam);
	connect(_bgParamStateInput, &QSpinBox::valueChanged, this, &BackgroundTileEditor::updateBgState);
	connect(_bgParamGroup, &QGroupBox::toggled, this, &BackgroundTileEditor::updateBgParamEnabled);
}

void BackgroundTileEditor::setBackgroundFile(BackgroundFile *backgroundFile)
{
	_backgroundFile = backgroundFile;
}

void BackgroundTileEditor::clear()
{
	_backgroundFile = nullptr;
	setTiles(QList<Tile>());
}

void BackgroundTileEditor::setTiles(const QList<Tile> &tiles)
{
	_tiles = tiles;

	bool isInvalid = false;
	int layerID = 0;
	QSet<int> params, states, paletteIDs, blendings, typeTranss;

	for (const Tile &tile: tiles) {
		layerID = tile.layerID;
		if (tile.tileID == quint16(-1)) {
			isInvalid = true;
			continue;
		}
		params.insert(tile.param);
		states.insert(tile.state);
		if (tile.depth < 2) {
			paletteIDs.insert(tile.paletteID);
		}
		blendings.insert(tile.blending);
		typeTranss.insert(tile.typeTrans);
	}

	if (isInvalid) {
		_stackedLayout->setCurrentWidget(_createPage);

		_colorTypeInput->setCurrentIndex(_colorTypeInput->findData(2));
		_colorTypeInput->setEnabled(true);
		_tileCreateLayout->labelForField(_colorTypeInput)->setEnabled(true);
	} else {
		_stackedLayout->setCurrentWidget(_formPage);

		if (tiles.size() == 1 && _backgroundFile != nullptr) {
			_tileWidget->setPixmap(QPixmap::fromImage(_backgroundFile->textures()->toImage(tiles.first(), _backgroundFile->palettes())));
			_tileWidget->setEnabled(true);
		} else {
			_tileWidget->setPixmap(QPixmap());
			_tileWidget->setEnabled(false);
		}

		_bgParamGroup->blockSignals(true);
		_bgParamInput->blockSignals(true);
		_bgParamStateInput->blockSignals(true);
		if (params.size() == 1) {
			_bgParamGroup->setChecked(*params.begin() > 0);
			_bgParamGroup->setEnabled(layerID > 0);
			_bgParamInput->setValue(*params.begin());

			if (states.size() == 1) {
				_bgParamStateInput->setValue(*states.begin());
				_bgParamStateInput->setEnabled(true);
			} else {
				_bgParamStateInput->setValue(0);
				_bgParamStateInput->setEnabled(false);
			}
		} else {
			_bgParamGroup->setChecked(false);
			_bgParamGroup->setEnabled(false);
			_bgParamInput->setValue(1);
		}
		_bgParamGroup->blockSignals(false);
		_bgParamInput->blockSignals(false);
		_bgParamStateInput->blockSignals(false);

		_paletteIdInput->blockSignals(true);
		_paletteIdInput->setVisible(!paletteIDs.isEmpty());
		_tileEditorLayout->labelForField(_paletteIdInput)->setVisible(!paletteIDs.isEmpty());
		if (paletteIDs.size() == 1) {
			_paletteIdInput->setValue(*paletteIDs.begin());
			_paletteIdInput->setEnabled(true);
			_tileEditorLayout->labelForField(_paletteIdInput)->setEnabled(true);
		} else {
			_paletteIdInput->setValue(0);
			_paletteIdInput->setEnabled(false);
			_tileEditorLayout->labelForField(_paletteIdInput)->setEnabled(false);
		}
		_paletteIdInput->blockSignals(false);

		_blendTypeInput->blockSignals(true);
		if (blendings.size() == 1 && (typeTranss.size() == 1 || !*blendings.begin())) {
			_blendTypeInput->setCurrentIndex(*blendings.begin() ? *typeTranss.begin() + 1 : 0);
			_blendTypeInput->setEnabled(layerID > 0);
			_tileEditorLayout->labelForField(_blendTypeInput)->setEnabled(true);
		} else {
			_blendTypeInput->setCurrentIndex(-1);
			_blendTypeInput->setEnabled(false);
			_tileEditorLayout->labelForField(_blendTypeInput)->setEnabled(false);
		}
		_blendTypeInput->blockSignals(false);
	}
}

void BackgroundTileEditor::updateBlendType(int index)
{
	if (index < 0) {
		return;
	}

	quint8 blendType = quint8(_blendTypeInput->itemData(index).toInt());
	bool blending = true;

	if (blendType > 3) {
		blendType = 0;
		blending = false;
	}

	for (Tile &tile: _tiles) {
		tile.typeTrans = blendType;
		tile.blending = blending;
		
		if (!_backgroundFile->setTile(tile)) {
			qWarning() << "BackgroundTileEditor::updateBlendType tile not found" << tile.tileID;
		}
	}

	emit changed(_tiles);
}

void BackgroundTileEditor::createTile()
{
	QList<Tile> modifiedTiles;

	for (Tile &tile: _tiles) {
		if (tile.tileID == quint16(-1)) {
			tile.depth = quint8(_colorTypeInput->currentData().toInt());
			if (!_backgroundFile->addTile(tile)) {
				QMessageBox::warning(this, tr("No more space"), tr("No more space available in the file for a new Tile"));
			} else {
				modifiedTiles.append(tile);
			}
		}
	}

	setTiles(_tiles);

	emit changed(modifiedTiles);
}

void BackgroundTileEditor::updateBgParam(int value)
{
	quint8 bgParam = quint8(value);

	for (Tile &tile: _tiles) {
		tile.param = bgParam;

		if (!_backgroundFile->setTile(tile)) {
			qWarning() << "BackgroundTileEditor::updateBgParam tile not found" << tile.tileID;
		}
	}

	emit changed(_tiles);
}

void BackgroundTileEditor::updateBgState(int value)
{
	quint8 bgState = quint8(value);

	for (Tile &tile: _tiles) {
		tile.state = bgState;

		if (!_backgroundFile->setTile(tile)) {
			qWarning() << "BackgroundTileEditor::updateBgState tile not found" << tile.tileID;
		}
	}

	emit changed(_tiles);
}

void BackgroundTileEditor::updateBgParamEnabled(bool enabled)
{
	updateBgParam(enabled ? _bgParamInput->value() : 0);
	updateBgState(enabled ? _bgParamStateInput->value() : 0);
}
