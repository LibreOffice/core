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
#               Novell, Inc.
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Contributor(s): Jan Holesovsky <kendy@novell.com>
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#
PRJ=..$/..
PRJNAME=desktop
TARGET=oosplash

NO_DEFAULT_STL=TRUE

.INCLUDE :  settings.mk

.IF "$(ENABLE_UNIX_QUICKSTARTER)"!="TRUE"

dummy:
    @echo "Unix quickstarter disabled"

.ELSE

STDLIB=

CFLAGS+=$(LIBPNG_CFLAGS)

OBJFILES= \
    $(OBJ)$/splashx.obj \
    $(OBJ)$/start.obj

APP1TARGET = $(TARGET)
APP1RPATH  = BRAND
APP1OBJS   = $(OBJFILES)
APP1LIBSALCPPRT=
APP1CODETYPE = C
APP1STDLIBS = $(SALLIB) -lX11 $(LIBPNG_LIBS)
.IF "$(OS)"=="SOLARIS"
APP1STDLIBS+= -lsocket
.ENDIF

.ENDIF # ENABLE_UNIX_QUICKSTARTER

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
