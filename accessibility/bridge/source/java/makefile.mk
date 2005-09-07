#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 15:45:16 $
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
PRJNAME=accessibility
TARGET=accessbridge
LIBTARGET=NO
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE
VERSIONOBJ=

USE_JAVAVER:=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(GUI)"=="WNT"
.IF "$(JAVANUMVER:s/.//)" >= "000100040000"

SLOFILES= $(SLO)$/WindowsAccessBridgeAdapter.obj

SHL1TARGET=java_uno_accessbridge
SHL1IMPLIB=i$(SHL1TARGET)
SHL1STDLIBS=$(VCLLIB) $(TOOLSLIB) $(JVMACCESSLIB) $(CPPULIB) $(SALLIB)
SHL1OBJS=$(SLOFILES) 
SHL1VERSIONOBJ=
SHL1DESCRIPTIONOBJ=

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

SHL1HEADER=$(OUT)$/inc$/WindowsAccessBridgeAdapter.h

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(JAVANUMVER:s/.//)" >= "000100040000"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.IF "$(GUI)"=="WNT"
.IF "$(JAVANUMVER:s/.//)" >= "000100040000"

$(SLO)$/WindowsAccessBridgeAdapter.obj : $(SHL1HEADER)

$(SHL1HEADER) :
    + javah -classpath $(OUT)$/class -o $(SHL1HEADER) org.openoffice.accessibility.WindowsAccessBridgeAdapter

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(JAVANUMVER:s/.//)" >= "000100040000"
