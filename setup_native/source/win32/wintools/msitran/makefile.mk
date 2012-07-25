#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

PRJ=..$/..$/..$/..
PRJNAME=setup_native
TARGET=sn_msitran


# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

.IF "$(WINEGCC)"==""
@all:
    @echo "No winegcc present, not building msitran..."
.ELSE
@all: $(BIN)/msitran.exe $(BIN)/msitran.exe.so

$(BIN)/msitran.exe.so: $(BIN)/msitran.exe

$(BIN)/msitran.exe:
	$(WINEGCC) -o $(BIN)/msitran.exe msitran.c -m32 -mconsole -lmsi

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

.ENDIF
