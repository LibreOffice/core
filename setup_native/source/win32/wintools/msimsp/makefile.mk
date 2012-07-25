#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

PRJ=..$/..$/..$/..
PRJNAME=setup_native
TARGET=sn_msimsp


# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

.IF "$(WINEGCC)"==""
@all:
    @echo "No winegcc present, not building msimsp..."
.ELSE
@all: $(BIN)/msimsp.exe $(BIN)/msimsp.exe.so

$(BIN)/msimsp.exe.so: $(BIN)/msimsp.exe

$(BIN)/msimsp.exe:
	$(WINEGCC) -o $(BIN)/msimsp.exe msimsp.c -m32 -mconsole -lmsi

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

.ENDIF
