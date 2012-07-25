#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

PRJ=..$/..$/..$/..
PRJNAME=setup_native
TARGET=sn_makecab


# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

.IF "$(WINEGCC)"==""
@all:
    @echo "No winegcc present, not building makecab..."
.ELSE
@all: $(BIN)/makecab.exe $(BIN)/makecab.exe.so

$(BIN)/makecab.exe.so: $(BIN)/makecab.exe

$(BIN)/makecab.exe:
	$(WINEGCC) -o $(BIN)/makecab.exe makecab.c parseddf.c -m32 -mconsole -lmsi

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

.ENDIF
