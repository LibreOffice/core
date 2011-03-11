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

PRJ=..

PRJNAME=vcl
TARGET=svdem
LIBTARGET=NO
TARGETTYPE=GUI

ENABLE_EXCEPTIONS=TRUE

my_components = i18npool i18nsearch

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/svdem.obj \
    $(OBJ)$/vcldemo.obj \
    $(OBJ)$/outdevgrind.obj

# --- Targets ------------------------------------------------------

# svdem

APP1NOSAL=		TRUE
APP1TARGET= 	$(TARGET)
APP1OBJS= \
    $(OBJ)$/svdem.obj

APP1STDLIBS=	$(CPPULIB)			\
                $(UNOTOOLSLIB)	\
                $(CPPUHELPERLIB)	\
                $(COMPHELPERLIB)	\
                $(TOOLSLIB) 		\
                $(SALLIB)			\
                $(SOTLIB)			\
                $(VCLLIB)

# outdevgrind

APP2TARGET= outdevgrind
APP2OBJS=	\
    $(OBJ)$/outdevgrind.obj $(OBJ)$/salmain.obj

APP2NOSAL=		TRUE
APP2STDLIBS=$(TOOLSLIB) 		\
            $(COMPHELPERLIB)	\
            $(CPPULIB)			\
            $(CPPUHELPERLIB)	\
            $(UCBHELPERLIB)		\
            $(SALLIB)			\
            $(VCLLIB)

# vcldemo

APP3NOSAL=TRUE
APP3TARGET=vcldemo
APP3OBJS= \
    $(OBJ)$/vcldemo.obj

APP3STDLIBS=	$(CPPULIB)			\
                $(UNOTOOLSLIB)	\
                $(CPPUHELPERLIB)	\
                $(COMPHELPERLIB)	\
                $(TOOLSLIB) 		\
                $(SALLIB)			\
                $(SOTLIB)			\
                $(VCLLIB)


# --- Targets ------------------------------------------------------
.IF "$(GUIBASE)" == "unx"

APP4NOSAL=              TRUE
APP4TARGET=     svptest
APP4OBJS=               $(OBJ)$/svptest.obj

APP4STDLIBS=    $(CPPULIB)                      \
                                $(CPPUHELPERLIB)        \
                                $(COMPHELPERLIB)        \
                                $(VCLLIB)                       \
                                $(TOOLSLIB)             \
                                $(SALLIB)                       \
                                $(SOTLIB)                       \
                                $(VCLLIB)

APP5NOSAL=              TRUE
APP5TARGET=     svpclient
APP5OBJS=               $(OBJ)$/svpclient.obj

APP5STDLIBS=    $(CPPULIB)                      \
                                $(CPPUHELPERLIB)        \
                                $(COMPHELPERLIB)        \
                                $(UCBHELPERLIB)        \
                                $(VCLLIB)                       \
                                $(TOOLSLIB)             \
                                $(SALLIB)                       \
                                $(SOTLIB)                       \
                                $(VCLLIB)

.IF "$(OS)" == "SOLARIS"
APP5STDLIBS+=-lsocket
.ENDIF

.ENDIF

.INCLUDE :	target.mk

ALLTAR : $(BIN)/applicat.rdb $(BIN)/types.rdb

$(BIN)/applicat.rdb .ERRREMOVE : $(SOLARENV)/bin/packcomponents.xslt \
        $(MISC)/applicat.input $(my_components:^"$(SOLARXMLDIR)/":+".component")
    $(XSLTPROC) --nonet --stringparam prefix $(SOLARXMLDIR)/ -o $@ \
        $(SOLARENV)/bin/packcomponents.xslt $(MISC)/applicat.input

$(MISC)/applicat.input :
    echo \
        '<list>$(my_components:^"<filename>":+".component</filename>")</list>' \
        > $@

$(BIN)/types.rdb : $(SOLARBINDIR)/types.rdb
    $(COPY) $< $@
