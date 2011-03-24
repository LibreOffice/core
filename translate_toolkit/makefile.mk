#*************************************************************************
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
# Petr Mladek <pmladek@suse.cz>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s): 
# Ted <ted@bear.com>
# Portions created by the Ted are Copyright (C) 2010 Ted. All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#
#*************************************************************************

PRJ=.

PRJNAME=trt
TARGET=trt

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_TRANSLATE_TOOLKIT)" == "YES"
@all:
    @echo "Using system translate toolkit..."
.ENDIF

# --- Python paths --------------------------------------------------

.IF "$(SYSTEM_PYTHON)"!="YES"
PYTHON=$(AUGMENT_LIBRARY_PATH) $(WRAPCMD) $(SOLARBINDIR)/python
.ELSE                   # "$(SYSTEM_PYTHON)"!="YES"
PYTHON=$(AUGMENT_LIBRARY_PATH) $(WRAPCMD) python
.ENDIF                  # "$(SYSTEM_PYTHON)"!="YES"

# --- Files --------------------------------------------------------

TARFILE_NAME=translate-toolkit-1.8.1
TARFILE_MD5=b4cae0700aa1c2aef7eb7f345365e6f1
BUILD_ACTION=$(PYTHON) setup.py build
BUILD_DIR=

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk
