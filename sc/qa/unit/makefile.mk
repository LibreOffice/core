#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: ihi $ $Date: 2007/11/23 13:58:12 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2007 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..
PRJNAME=sc
TARGET=qa_unit

ENABLE_EXCEPTIONS=TRUE

.INCLUDE : settings.mk

#building with stlport, but cppunit was not built with stlport
.IF "$(USE_SYSTEM_STL)"!="YES"
.IF "$(SYSTEM_CPPUNIT)"=="YES"
CFLAGSCXX+=-DADAPT_EXT_STL
.ENDIF
.ENDIF

CFLAGSCXX += $(CPPUNIT_CFLAGS)

SHL1TARGET = $(TARGET)
SHL1OBJS = $(SLO)$/ucalc.obj
SHL1STDLIBS=       \
    $(BASICLIB)	\
    $(SFXLIB)		\
    $(SVTOOLLIB)	\
    $(SVLLIB)		\
    $(SVXCORELIB)		\
    $(EDITENGLIB)		\
    $(SVXLIB)		\
    $(BASEGFXLIB) \
    $(DRAWINGLAYERLIB) \
    $(VCLLIB)		\
    $(CPPULIB)		\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)	\
    $(UCBHELPERLIB)	\
    $(TKLIB)		\
    $(VOSLIB)		\
    $(SALLIB)		\
    $(SALHELPERLIB)	\
    $(TOOLSLIB)	\
    $(I18NISOLANGLIB) \
    $(UNOTOOLSLIB) \
    $(SOTLIB)		\
    $(XMLOFFLIB)	\
    $(AVMEDIALIB) \
    $(FORLIB) \
    $(FORUILIB) \
    $(CPPUNITLIB)
SHL1IMPLIB = i$(SHL1TARGET)
SHL1LIBS=$(SLB)$/scalc3.lib $(SLB)$/scalc3c.lib 
DEF1NAME = $(SHL1TARGET)
SHL1VERSIONMAP=version.map

.INCLUDE: target.mk

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
$(MISC)/$(TARGET)/services.rdb .ERRREMOVE : $(MISC)/$(TARGET)/udkapi.rdb
    $(MKDIRHIER) $(@:d)
    $(REGCOMP) -register -br $(MISC)/$(TARGET)/udkapi.rdb -r $@ -wop \
        -c $(DLLPRE)fwk$(DLLPOSTFIX)$(DLLPOST)

#Tweak things so that we use the .res files in the solver
STAR_RESOURCEPATH:=$(PWD)/$(BIN):$(SOLARBINDIR)
.EXPORT : STAR_RESOURCEPATH

.IF "$(OS)" == "LINUX"

test .PHONY: $(SHL1TARGETN) $(MISC)/$(TARGET)/services.rdb $(MISC)$/$(TARGET)$/types.rdb $(MISC)/$(TARGET)/udkapi.rdb
    @echo ----------------------------------------------------------
    @echo - start unit test \#1 on library $(SHL1TARGETN)
    @echo ----------------------------------------------------------
    $(CPPUNITTESTER) $(SHL1TARGETN) -headless -invisible \
        -env:UNO_SERVICES=$(my_file)$(PWD)/$(MISC)/$(TARGET)/services.rdb \
        -env:UNO_TYPES="$(my_file)$(PWD)/$(MISC)/$(TARGET)/types.rdb $(my_file)$(PWD)/$(MISC)/$(TARGET)/udkapi.rdb" \
        -env:OOO_BASE_DIR="$(my_file)$(PWD)/$(MISC)/$(TARGET)" \
        -env:BRAND_BASE_DIR="$(my_file)$(PWD)/$(MISC)/$(TARGET)" \
        -env:UNO_USER_PACKAGES_CACHE="$(my_file)$(PWD)/$(MISC)/$(TARGET)"

.ELSE

test .PHONY: $(SHL1TARGETN)
    @echo ----------------------------------------------------------
    @echo - WARNING!!, test disabled on your platform
    @echo - Please test manually, and enable if it works
    @echo ----------------------------------------------------------

.ENDIF
