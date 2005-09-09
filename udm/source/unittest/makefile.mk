#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 14:39:40 $
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

PRJNAME=cosv
TARGET=udm_unittest
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true
PRJINC=$(PRJ)$/source

.INCLUDE :  settings.mk
.INCLUDE :  static.mk

.INCLUDE : $(PRJ)$/source$/mkinc$/fullcpp.mk




# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/test.obj




APP1TARGET=	$(TARGET)
APP1STACK=	1000000
APP1OBJS=   $(OBJ)$/test.obj

.IF "$(COMEX)"=="10"
APP1STDLIBS= $(STATIC_LIBS) cosv.lib
.ELSE
APP1STDLIBS= $(STATIC_LIBS) msvcirt.lib cosv.lib
.ENDIF


APP1LIBS=	$(LB)$/$(TARGET).lib


APP1DEPN=   $(LB)$/$(TARGET).lib $(L)$/cosv.lib


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



