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
#       [ Novell Inc. Cédric Bosdonnat <cbosdonnat@novell.com> ]
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Contributor(s):
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#
PRJ=..$/..

PRJNAME=editeng
TARGET=test-items
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

#building with stlport, but cppunit was not built with stlport
.IF "$(USE_SYSTEM_STL)"!="YES"
.IF "$(SYSTEM_CPPUNIT)"=="YES"
CFLAGSCXX+=-DADAPT_EXT_STL
.ENDIF
.ENDIF

CFLAGSCXX += $(CPPUNIT_CFLAGS)
DLLPRE = # no leading "lib" on .so files


# --- Targets ------------------------------------------------------

SHL1TARGET = borderline_test
SHL1OBJS = \
             $(SLO)$/borderline_test.obj

SHL1STDLIBS= \
             $(CPPUNITLIB) \
             $(XMLOFFLIB) \
             $(BASEGFXLIB) \
             $(LNGLIB) \
             $(SVTOOLLIB) \
             $(TKLIB) \
             $(VCLLIB) \
             $(SVLLIB) \
             $(SOTLIB) \
             $(UNOTOOLSLIB) \
             $(TOOLSLIB) \
             $(I18NISOLANGLIB) \
             $(I18NPAPERLIB) \
             $(COMPHELPERLIB) \
             $(UCBHELPERLIB) \
             $(CPPUHELPERLIB) \
             $(CPPULIB) \
             $(SALLIB) \
             $(SALHELPERLIB) \
             $(ICUUCLIB) \
             $(EDITENGLIB)

SHL1VERSIONMAP = version.map
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)


SLOFILES = $(SHL1OBJS)

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk
