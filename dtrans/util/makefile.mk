#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: vg $ $Date: 2003-04-15 14:06:11 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..

PRJNAME=dtrans
TARGET=dtrans
TARGET1=mcnttype
TARGET2=ftransl
TARGET3=sysdtrans
TARGET4=dnd

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk


# --- mcnttype dynlib ----------------------------------------------

SHL1TARGET=$(TARGET1)

SHL1LIBS=$(SLB)$/mcnttype.lib

SHL1STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB) 

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= \
        uwinapi.lib
.ENDIF



SHL1IMPLIB=i$(SHL1TARGET) 

#--- comment -----------------

SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp
SHL1VERSIONMAP=exports.map


.IF "$(GUI)"=="WNT"

# --- ftransl dll ---

SHL2TARGET=$(TARGET2)

SHL2LIBS=$(SLB)$/ftransl.lib\
         $(SLB)$/dtutils.lib

SHL2STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB)\
        uwinapi.lib\
        advapi32.lib\
        ole32.lib\
        gdi32.lib

SHL2IMPLIB=i$(SHL2TARGET) 

SHL2DEF=		$(MISC)$/$(SHL2TARGET).def
DEF2NAME=		$(SHL2TARGET)
DEF2EXPORTFILE=	exports.dxp


# --- sysdtrans dll ---

SHL3TARGET=$(TARGET3)

SHL3LIBS=$(SLB)$/sysdtrans.lib\
         $(SLB)$/dtutils.lib\
         $(SLB)$/dtobjfact.lib\
         $(SLB)$/mtaolecb.lib

SHL3STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB)\
        uwinapi.lib\
        advapi32.lib\
        ole32.lib\
        comsupp.lib\
        oleaut32.lib\
        gdi32.lib
        
SHL3IMPLIB=i$(SHL3TARGET) 

SHL3DEF=		$(MISC)$/$(SHL3TARGET).def
DEF3NAME=		$(SHL3TARGET)
DEF3EXPORTFILE=	exports.dxp


# --- dnd dll ---

SHL4TARGET=$(TARGET4)

SHL4LIBS=	\
            $(SLB)$/dnd.lib\
            $(SLB)$/dtobjfact.lib\
            $(SLB)$/dtutils.lib

SHL4STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB) \
        uwinapi.lib\
        advapi32.lib\
        ole32.lib\
        comsupp.lib\
        oleaut32.lib\
        gdi32.lib

SHL4DEPN=
SHL4IMPLIB=i$(SHL4TARGET) 

SHL4DEF=		$(MISC)$/$(SHL4TARGET).def

DEF4NAME=		$(SHL4TARGET)
DEF4EXPORTFILE=	exports.dxp

.ENDIF			# "$(GUI)"=="WNT"


.INCLUDE :  target.mk

