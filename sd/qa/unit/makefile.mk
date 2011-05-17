# -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
#       Novell, Inc.
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Contributor(s):  Michael Meeks <michael.meeks@novell.com>
#                  Caolan McNamara <caolanm@redhat.com>
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#

PRJ=..$/..
PRJNAME=sd
TARGET=qa_unit

ENABLE_EXCEPTIONS=TRUE

.INCLUDE : settings.mk

CFLAGSCXX += $(CPPUNIT_CFLAGS)

SHL1TARGET = $(TARGET)
SHL1OBJS = $(SLO)$/uimpress.obj
SHL1STDLIBS= \
    $(BASICLIB)	\
    $(SFXLIB)		\
    $(SVTOOLLIB)	\
    $(SVLLIB)		\
    $(SVXCORELIB)		\
    $(CANVASTOOLSLIB) \
    $(EDITENGLIB)		\
    $(SVXLIB)		\
    $(BASEGFXLIB) \
    $(DRAWINGLAYERLIB) \
    $(I18NISOLANGLIB) \
    $(VCLLIB)		\
    $(CPPULIB)		\
    $(CPPCANVASLIB)	\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)	\
    $(UCBHELPERLIB)	\
    $(SALLIB)		\
    $(SALHELPERLIB)	\
    $(TOOLSLIB)	\
    $(TKLIB)	\
    $(SOTLIB)	\
    $(UNOTOOLSLIB) \
    $(XMLOFFLIB)	\
    $(AVMEDIALIB)	\
    $(CPPUNITLIB)
SHL1IMPLIB = i$(SHL1TARGET)
SHL1LIBS=$(SLB)$/sdraw3.lib $(SLB)$/sdraw3_2.lib $(SLB)$/sdraw3_3.lib
DEF1NAME = $(SHL1TARGET)
SHL1VERSIONMAP=version.map

.INCLUDE: target.mk

.IF "$(OS)" == "WNT"
my_file = file:///
.ELSE
my_file = file://
.END

ALLTAR: test

test_components = \
    component/framework/util/fwk

#Make a services.rdb with the services we know we need to get up and running
$(MISC)/$(TARGET)/services.input : makefile.mk
    $(MKDIRHIER) $(@:d)
    echo \
        '<list>$(test_components:^"<filename>":+".component</filename>")</list>' \
        > $@

$(MISC)/$(TARGET)/services.rdb .ERRREMOVE : makefile.mk $(MISC)/$(TARGET)/services.input
    $(MKDIRHIER) $(@:d)
    $(XSLTPROC) --nonet --stringparam prefix $(SOLARXMLDIR)/ -o $@.tmp \
        $(SOLARENV)/bin/packcomponents.xslt $(MISC)/$(TARGET)/services.input
    cat $(MISC)/$@.tmp | sed 's|/program/|/|g' > $@

#Tweak things so that we use the .res files in the solver
STAR_RESOURCEPATH:=$(PWD)/$(BIN)$(PATH_SEPERATOR)$(SOLARBINDIR)
.EXPORT : STAR_RESOURCEPATH

.IF "$(OS)" != "DRAGONFLY"

test .PHONY: $(SHL1TARGETN) $(MISC)/$(TARGET)/services.rdb
    @echo ----------------------------------------------------------
    @echo - start unit test \#1 on library $(SHL1TARGETN)
    @echo ----------------------------------------------------------
    $(CPPUNITTESTER) $(SHL1TARGETN) --headless --invisible \
        '-env:UNO_TYPES=$(my_file)$(SOLARBINDIR)/udkapi.rdb $(my_file)$(SOLARBINDIR)$/types.rdb' \
        '-env:UNO_SERVICES=$(my_file)$(SOLARXMLDIR)/ure/services.rdb $(my_file)$(PWD)/$(MISC)/$(TARGET)/services.rdb'\
        -env:URE_INTERNAL_LIB_DIR="$(my_file)$(SOLARSHAREDBIN)" \
        -env:OOO_BASE_DIR="$(my_file)$(SOLARSHAREDBIN)" \
        -env:BRAND_BASE_DIR="$(my_file)$(SOLARSHAREDBIN)"

.ELSE

test .PHONY: $(SHL1TARGETN)
    @echo ----------------------------------------------------------
    @echo - WARNING!!, test disabled on your platform
    @echo - Please test manually, and enable if it works
    @echo ----------------------------------------------------------

.ENDIF

