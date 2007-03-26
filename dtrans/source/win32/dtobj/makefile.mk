#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: vg $ $Date: 2007-03-26 15:07:48 $
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

PRJNAME=dtrans
TARGET=dtobjfact
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

#-DUNICODE -D_UNICODE
.IF "$(COM)"!="GCC"
CFLAGS+=-GR -Ob0
.ENDIF

SLOFILES=$(SLO)$/DtObjFactory.obj\
         $(SLO)$/APNDataObject.obj\
         $(SLO)$/DOTransferable.obj\
         $(SLO)$/DTransHelper.obj\
         $(SLO)$/XTDataObject.obj\
         $(SLO)$/TxtCnvtHlp.obj\
         $(SLO)$/DataFmtTransl.obj\
         $(SLO)$/FmtFilter.obj\
         $(SLO)$/FetcList.obj\
         $(SLO)$/Fetc.obj\
         $(SLO)$/XNotifyingDataObject.obj

LIB1TARGET=$(SLB)$/$(TARGET).lib
.IF "$(COM)"!="GCC"
LIB1OBJFILES=$(SLOFILES)
.ELSE
LIB1OBJFILES=$(SLO)$/DtObjFactory.obj\
         $(SLO)$/APNDataObject.obj\
         $(SLO)$/DOTransferable.obj\
         $(SLO)$/DTransHelper.obj\
         $(SLO)$/XTDataObject.obj\
         $(SLO)$/TxtCnvtHlp.obj\
         $(SLO)$/DataFmtTransl.obj\
         $(SLO)$/FmtFilter.obj\
         $(SLO)$/FetcList.obj\
         $(SLO)$/Fetc.obj
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
