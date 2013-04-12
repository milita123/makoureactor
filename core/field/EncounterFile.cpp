/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2012 Arzel J�r�me <myst6re@gmail.com>
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
#include "EncounterFile.h"
#include "Field.h"

EncounterFile::EncounterFile(Field *field) :
	FieldPart(field)
{
}

bool EncounterFile::open()
{
	return open(field()->sectionData(Field::Encounter));
}

bool EncounterFile::open(const QByteArray &data)
{
	if(sizeof(EncounterTable) != 24) {
		qWarning() << "Encounter invalid struct size" << sizeof(EncounterTable);
		return false;
	}

	if(data.size() != 48) {
		qWarning() << "Encounter invalid data size" << data.size();
		return false;
	}

	memcpy(tables, data.constData(), sizeof(EncounterTable) * 2);

	setOpen(true);

	return true;
}

QByteArray EncounterFile::save() const
{
	return QByteArray((char *)&tables, sizeof(EncounterTable) * 2);
}

void EncounterFile::clear()
{
	//TODO: clear
}

const EncounterTable &EncounterFile::encounterTable(Table tableID) const
{
	return tables[(int)tableID];
}

void EncounterFile::setEncounterTable(Table tableID, const EncounterTable &table)
{
	tables[(int)tableID] = table;
	setModified(true);
}
