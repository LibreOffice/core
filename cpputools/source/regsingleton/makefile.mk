#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 09:38:15 $
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
PRJ=..$/..

PRJNAME=cpputools
TARGET=regsingleton
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb 
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

UNOUCROUT = $(OUT)$/inc$/$(TARGET)
INCPRE += $(UNOUCROUT)
CPPUMAKERFLAGS+= -C

UNOTYPES= \
     com.sun.star.uno.TypeClass \
     com.sun.star.uno.XAggregation \
     com.sun.star.uno.XWeak \
     com.sun.star.uno.XComponentContext \
     com.sun.star.lang.XTypeProvider \
     com.sun.star.lang.XComponent \
     com.sun.star.lang.XSingleServiceFactory \
     com.sun.star.lang.XSingleComponentFactory \
     com.sun.star.lang.XMultiServiceFactory \
     com.sun.star.lang.XMultiComponentFactory \
     com.sun.star.container.XHierarchicalNameAccess \
    com.sun.star.registry.XSimpleRegistry \
    com.sun.star.registry.XRegistryKey

# --- Files --------------------------------------------------------

DEPOBJFILES= \
    $(OBJ)$/regsingleton.obj 

APP1TARGET=$(TARGET)
APP1OBJS=$(DEPOBJFILES)  

APP1STDLIBS= \
    $(SALLIB)		\
    $(CPPULIB)		\
    $(CPPUHELPERLIB)

.IF "$(debug)" != ""
.IF "$(COM)" == "MSC"
CFLAGS += -Ob0
.ENDIF
.ENDIF

.INCLUDE :  target.mk

