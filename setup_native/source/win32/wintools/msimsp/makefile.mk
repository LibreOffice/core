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
@all:
    @echo "Not building msimsp yet, stay tuned..."
.ENDIF

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

# -------------------------------------------------------------------------
