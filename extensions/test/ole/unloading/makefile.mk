#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 20:53:44 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
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
PRJ=..$/..$/..

PRJNAME=	extensions
TARGET=		unloadtest
TARGETTYPE=CUI
COMP1TYPELIST=$(TARGET1)

ENABLE_EXCEPTIONS=TRUE

USE_DEFFILE=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

CPPUMAKERFLAGS =

.IF "$(BOOTSTRAP_SERVICE)" == "TRUE"
UNOUCROUT=	$(OUT)$/inc$/bootstrap
INCPRE+=	$(OUT)$/inc$/bootstrap
.ELSE
.IF "$(COM)" == "MSC"
CPPUMAKERFLAGS = -L
.ENDIF
UNOUCROUT=	$(OUT)$/inc$/light
INCPRE+=	$(OUT)$/inc$/light
.ENDIF

# ------------------------------------------------------------------
UNOTYPES= com.sun.star.uno.XComponentContext \
      com.sun.star.lang.XSingleComponentFactory \
     com.sun.star.lang.XMultiComponentFactory \
    com.sun.star.lang.XServiceInfo \
    com.sun.star.lang.XComponent \
    com.sun.star.container.XHierarchicalNameAccess \
    com.sun.star.registry.XSimpleRegistry

APP1NOSAL=TRUE

APP1TARGET=	$(TARGET)

APP1OBJS=	$(OBJ)$/unloadTest.obj

APP1STDLIBS= \
    $(SALLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB)




LIBCIMT=MSVCPRTD.lib

# all:						\
# 	$(BINDIR)			\
# 	$(BINDIR)$/test.ini 		\
# 	ALLTAR

# $(BINDIR) :
# 	@+-$(MKDIR) $(BINDIR) >& $(NULLDEV)

# $(BINDIR)$/test.ini : .$/unloadtest.ini
# 	+-$(GNUCOPY) .$/unloadtest.ini $(BINDIR)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk


