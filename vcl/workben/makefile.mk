#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
                $(VOSLIB)			\
                $(SOTLIB)			\
                $(VCLLIB)

# outdevgrind

APP2TARGET= outdevgrind
APP2OBJS=	\
    $(OBJ)$/outdevgrind.obj 

APP2NOSAL=		TRUE
APP2STDLIBS=$(TOOLSLIB) 		\
            vclmain.lib \
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
                $(VOSLIB)			\
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
                                $(VOSLIB)                       \
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
                                $(VOSLIB)                       \
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
