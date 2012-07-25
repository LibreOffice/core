#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

PRJ=..$/..$/..$/..
PRJNAME=setup_native
TARGET=sn_msiinfo


# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

.IF "$(WINEGCC)"==""
@all:
    @echo "No winegcc present, not building msiinfo..."
.ELSE
@all: $(BIN)/msiinfo.exe $(BIN)/msiinfo.exe.so

$(BIN)/msiinfo.exe.so: $(BIN)/msiinfo.exe

$(BIN)/msiinfo.exe:
	$(WINEGCC) -o $(BIN)/msiinfo.exe msiinfo.c -m32 -mconsole -municode -lmsi

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

.ENDIF
