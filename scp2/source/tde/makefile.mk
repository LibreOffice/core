#*************************************************************************
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
# Timothy Pearson <kb9vqf@pearsoncomputing.net> (C) 2012, All Rights Reserved.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#
#*************************************************************************

PRJ=..$/..

PRJPCH=

PRJNAME=scp2
TARGET=tde
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

SCP_PRODUCT_TYPE=osl

.IF "$(ENABLE_TDE)" == "TRUE"

.IF "$(ENABLE_TDE)" != ""
SCPDEFS+=-DENABLE_TDE
.ENDIF

PARFILES =                   \
        module_tde.par     \
        file_tde.par

ULFFILES= \
        module_tde.ulf
.ENDIF

# --- File ---------------------------------------------------------

.INCLUDE :  target.mk
