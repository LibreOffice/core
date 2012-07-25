#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

PRJ=..$/..$/..$/..
PRJNAME=setup_native
TARGET=sn_msidb


# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

.IF "$(WINEGCC)"==""
@all:
    @echo "No winegcc present, not building msidb..."
.ELSE
@all: $(BIN)/msidb.exe $(BIN)/msidb.exe.so

$(BIN)/msidb.exe.so: $(BIN)/msidb.exe

$(BIN)/msidb.exe:
	$(WINEGCC) -o $(BIN)/msidb.exe msidb.c -m32 -municode -lmsi

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

# -------------------------------------------------------------------------
.ENDIF
