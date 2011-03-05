#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************
PRJ=..$/..

PRJNAME=sw
TARGET=qa_unit

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGSCXX += $(CPPUNIT_CFLAGS)

SHL1OBJS=\
    $(SLO)$/Test-BigPtrArray.obj \
    $(SLO)$/bparr.obj

SHL1TARGET=$(TARGET)
SHL1STDLIBS=$(SALLIB) $(TOOLSLIB) $(CPPUNITLIB)

SHL1IMPLIB= i$(SHL1TARGET)

DEF1NAME    =$(SHL1TARGET)

SHL1VERSIONMAP = export.map

# 2nd test ------------------------------------------------------------------

SHL2OBJS=$(SLO)$/swdoc-test.obj
SHL2TARGET=swdoctest
SHL2STDLIBS+= \
    $(LNGLIB) \
    $(BASICLIB) \
    $(SFXLIB)           \
    $(SVTOOLLIB)        \
    $(SVLLIB)           \
    $(SVXCORELIB)               \
    $(EDITENGLIB)               \
    $(SVXLIB)           \
    $(BASEGFXLIB) \
    $(DRAWINGLAYERLIB) \
    $(VCLLIB)           \
    $(CPPULIB)          \
    $(CPPUHELPERLIB)    \
    $(COMPHELPERLIB)    \
    $(UCBHELPERLIB)     \
    $(TKLIB)            \
    $(VOSLIB)           \
    $(SALLIB)           \
    $(SALHELPERLIB)     \
    $(TOOLSLIB) \
    $(I18NISOLANGLIB) \
    $(UNOTOOLSLIB) \
    $(SOTLIB)           \
    $(XMLOFFLIB)        \
    $(ICUUCLIB) \
    $(I18NUTILLIB) \
    $(AVMEDIALIB) \
    $(CPPUNITLIB)

.IF "$(DBG_LEVEL)">="2"
SHL2STDLIBS += $(LIBXML2LIB)
.ENDIF

SHL2IMPLIB= i$(SHL2TARGET)
DEF2NAME    =$(SHL2TARGET)
SHL2VERSIONMAP = export.map

# linking statically against sw parts
SHL2LIBS=\
    $(SLB)$/swall.lib   \
    $(SLB)$/core1.lib	\
    $(SLB)$/core2.lib	\
    $(SLB)$/core3.lib	\
    $(SLB)$/core4.lib	\
    $(SLB)$/filter.lib	\
    $(SLB)$/ui1.lib	    \
    $(SLB)$/ui2.lib

SHL3OBJS=$(SLO)$/filters-test.obj
SHL3TARGET=swfilterstest
SHL3STDLIBS+= \
    $(ISWLIB) \
    $(LNGLIB) \
    $(BASICLIB) \
    $(SFXLIB)           \
    $(SVTOOLLIB)        \
    $(SVLLIB)           \
    $(SVXCORELIB)               \
    $(EDITENGLIB)               \
    $(SVXLIB)           \
    $(BASEGFXLIB) \
    $(DRAWINGLAYERLIB) \
    $(VCLLIB)           \
    $(CPPULIB)          \
    $(CPPUHELPERLIB)    \
    $(COMPHELPERLIB)    \
    $(UCBHELPERLIB)     \
    $(TKLIB)            \
    $(VOSLIB)           \
    $(SALLIB)           \
    $(SALHELPERLIB)     \
    $(TOOLSLIB) \
    $(I18NISOLANGLIB) \
    $(UNOTOOLSLIB) \
    $(SOTLIB)           \
    $(XMLOFFLIB)        \
    $(ICUUCLIB) \
    $(I18NUTILLIB) \
    $(AVMEDIALIB) \
    $(CPPUNITLIB)

.IF "$(DBG_LEVEL)">="2"
SHL3STDLIBS += $(LIBXML2LIB)
.ENDIF

SHL3IMPLIB= i$(SHL3TARGET)
DEF3NAME    =$(SHL3TARGET)
SHL3VERSIONMAP = export.map

# END ------------------------------------------------------------------

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
# include comphelper/version.mk for platform where it is not
# included through libs.mk
.IF ("$(GUI)"!="UNX" && "$(COM)"!="GCC") || "$(GUI)"=="OS2"
.INCLUDE :  comphelper/version.mk
.END

.IF "$(OS)" == "WNT"
my_file = file:///
.ELSE
my_file = file://
.END

ALLTAR: test

$(MISC)$/$(TARGET)$/types.rdb .ERRREMOVE : $(SOLARBINDIR)$/types.rdb
    $(MKDIRHIER) $(@:d)
    $(GNUCOPY) $? $@

$(MISC)/$(TARGET)/udkapi.rdb .ERRREMOVE : $(SOLARBINDIR)$/udkapi.rdb
    $(MKDIRHIER) $(@:d)
    $(GNUCOPY) $? $@

#Make a services.rdb with the services we know we need to get up and running
$(MISC)/$(TARGET)/services.rdb .ERRREMOVE : $(MISC)/$(TARGET)/udkapi.rdb makefile.mk
    $(MKDIRHIER) $(@:d)
    $(REGCOMP) -register -br $(MISC)/$(TARGET)/udkapi.rdb -r $@ -wop \
        -c $(DLLPRE)sw$(DLLPOSTFIX)$(DLLPOST) \
        -c $(DLLPRE)fileacc$(DLLPOST) \
        -c $(DLLPRE)fwk$(DLLPOSTFIX)$(DLLPOST) \
        -c $(DLLPRE)sfx$(DLLPOSTFIX)$(DLLPOST) \
        -c $(DLLPRE)ucb1$(DLLPOST) \
        -c $(DLLPRE)ucpfile1$(DLLPOST) \
        -c $(DLLPRE)unoxml$(DLLPOSTFIX)$(DLLPOST) \
        -c $(DLLPRE)comphelp$(COMPHLP_MAJOR)$(COMID)$(DLLPOST) \
        -c $(DLLPRE)xstor$(DLLPOST) \
        -c $(DLLPRE)package2$(DLLPOST) \
        -c $(DLLPRE)xof$(DLLPOSTFIX)$(DLLPOST) \
        -c $(DLLPRE)utl$(DLLPOSTFIX)$(DLLPOST) \
        -c stocservices.uno$(DLLPOST) \
        -c reflection.uno$(DLLPOST) \
        -c i18npool.uno$(DLLPOST) \
        -c sax.uno$(DLLPOST)

#Tweak things so that we use the .res files in the solver
STAR_RESOURCEPATH:=$(PWD)/$(BIN)$(PATH_SEPERATOR)$(SOLARBINDIR)
.EXPORT : STAR_RESOURCEPATH

test .PHONY: $(SHL1TARGETN) $(SHL2TARGETN) $(SHL3TARGETN) $(MISC)/$(TARGET)/services.rdb $(MISC)$/$(TARGET)$/types.rdb $(MISC)/$(TARGET)/udkapi.rdb
    echo "$(STAR_RESOURCEPATH)"
    @echo ----------------------------------------------------------
    @echo - start unit test \#1 on library $(SHL1TARGETN)
    @echo ----------------------------------------------------------
    $(CPPUNITTESTER) $(SHL1TARGETN)
    @echo ----------------------------------------------------------
    @echo - start unit test \#2 on library $(SHL2TARGETN)
    @echo ----------------------------------------------------------
    $(CPPUNITTESTER) $(SHL2TARGETN) -headless -invisible \
        -env:UNO_SERVICES=$(my_file)$(PWD)/$(MISC)/$(TARGET)/services.rdb \
        -env:UNO_TYPES="$(my_file)$(PWD)/$(MISC)/$(TARGET)/types.rdb $(my_file)$(PWD)/$(MISC)/$(TARGET)/udkapi.rdb" \
        -env:OOO_BASE_DIR="$(my_file)$(PWD)/$(MISC)/$(TARGET)" \
        -env:BRAND_BASE_DIR="$(my_file)$(PWD)/$(MISC)/$(TARGET)" \
        -env:UNO_USER_PACKAGES_CACHE="$(my_file)$(PWD)/$(MISC)/$(TARGET)"
    @echo ----------------------------------------------------------
    @echo - start unit test \#3 on library $(SHL3TARGETN)
    @echo ----------------------------------------------------------
    $(CPPUNITTESTER) $(SHL3TARGETN) -headless -invisible \
        -env:UNO_SERVICES=$(my_file)$(PWD)/$(MISC)/$(TARGET)/services.rdb \
        -env:UNO_TYPES="$(my_file)$(PWD)/$(MISC)/$(TARGET)/types.rdb $(my_file)$(PWD)/$(MISC)/$(TARGET)/udkapi.rdb" \
        -env:OOO_BASE_DIR="$(my_file)$(PWD)/$(MISC)/$(TARGET)" \
        -env:BRAND_BASE_DIR="$(my_file)$(PWD)/$(MISC)/$(TARGET)" \
        -env:UNO_USER_PACKAGES_CACHE="$(my_file)$(PWD)/$(MISC)/$(TARGET)"
