#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: mba $ $Date: 2000-11-20 12:59:43 $
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

PRJPCH=

PRJNAME=sot
TARGET=sot

# so2.hid generieren
#GEN_HID=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

.IF "$(header)" == ""

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1ARCHIV= $(LB)$/lib$(TARGET)$(UPD)$(DLLPOSTFIX).a
LIB1FILES=	$(SLB)$/base.lib				\
            $(SLB)$/sdstor.lib

SHL1TARGET= $(TARGET)$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB= $(TARGET)
SHL1LIBS=	$(SLB)$/$(TARGET).lib

SHL1STDLIBS=$(TOOLSLIB) $(RTLLIB) $(SALLIB) $(UNOTOOLSLIB) $(CPPUHELPERLIB) $(UCBHELPERLIB) $(UNOLIB) $(CPPULIB)

.IF "$(COM)"=="ICC" || "$(COM)"=="WTC"
SHL1OBJS= $(SLO)$/object.obj
.ENDIF

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

.IF "$(linkinc)" != ""
SHL11FILE=	$(MISC)$/base.slo
SHL12FILE=	$(MISC)$/sdstor.slo
.ENDIF

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt \
         $(PRJ)$/inc$/absdev.hxx	\
         $(PRJ)$/inc$/agg.hxx       \
         $(PRJ)$/inc$/dtrans.hxx    \
         $(PRJ)$/inc$/exchange.hxx  \
         $(PRJ)$/inc$/factory.hxx   \
         $(PRJ)$/inc$/object.hxx    \
         $(PRJ)$/inc$/sotdata.hxx   \
         $(PRJ)$/inc$/sotref.hxx    \
         $(PRJ)$/inc$/stg.hxx       \
         $(PRJ)$/inc$/storage.hxx   \
         $(PRJ)$/inc$/storinfo.hxx
DEFLIB1NAME =$(TARGET)
DEF1DES 	=StarObjectsTools

.ENDIF


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @+echo ------------------------------
    @+echo Making: $@
    @+echo WEP>$@
    @+echo LIBMAIN>>$@
    @+echo LibMain>>$@
.IF "$(GUI)"!="UNX"
    @+echo CT??_R0?AV>>$@
    @+echo CTA2?AV>>$@
.ENDIF
.IF "$(GUI)"=="OS2"
    @+echo __alloc>>$@
    @+echo __malloc>>$@
.ENDIF
#	@+echo AVbad>>$@
#	@+echo AVexception>>$@

.INCLUDE :  target.pmk
