#**************************************************************************
#
#     $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/scaddins/source/datefunc/makefile.mk,v 1.4 2001-03-20 15:05:42 kz Exp $
#
#     $Date: 2001-03-20 15:05:42 $
#     $Author: kz $
#     $Revision: 1.4 $
#
#  The Contents of this file are made available subject to the terms of
#  either of the following licenses
#
#         - GNU Lesser General Public License Version 2.1
#         - Sun Industry Standards Source License Version 1.1
#
#  Sun Microsystems Inc., October, 2000
#
#  GNU Lesser General Public License Version 2.1
#  =============================================
#  Copyright 2000 by Sun Microsystems, Inc.
#  901 San Antonio Road, Palo Alto, CA 94303, USA
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License version 2.1, as published by the Free Software Foundation.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#  MA  02111-1307  USA
#
#
#  Sun Industry Standards Source License Version 1.1
#  =================================================
#  The contents of this file are subject to the Sun Industry Standards
#  Source License Version 1.1 (the "License"); You may not use this file
#  except in compliance with the License. You may obtain a copy of the
#  License at http://www.openoffice.org/license.html.
#
#  Software provided under this License is provided on an "AS IS" basis,
#  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#  See the License for the specific provisions governing your rights and
#  obligations concerning the Software.
#
#  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#  Copyright: 2000 by Sun Microsystems, Inc.
#
#  All Rights Reserved.
#
#  Contributor(s): _______________________________________
#
#
#**************************************************************************

PRJ=..$/..
PRJNAME=addin

TARGET=date


ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
LIBTARGET=NO

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Types -------------------------------------

UNOUCRRDB=$(SOLARBINDIR)$/applicat.rdb $(BIN)$/dateadd.rdb
UNOUCRDEP=$(UNOUCRRDB)

UNOUCROUT=$(OUT)$/inc$/$(PRJNAME)$/$(TARGET)
INCPRE+=$(UNOUCROUT)

# --- Types -------------------------------------

# comprehensive type info, so rdb needn't be installed
CPPUMAKERFLAGS*=-C

UNOTYPES=\
    com.sun.star.sheet.addin.XDateFunctions \
    com.sun.star.lang.XComponent \
    com.sun.star.lang.XMultiServiceFactory \
    com.sun.star.lang.XSingleServiceFactory \
    com.sun.star.uno.TypeClass \
    com.sun.star.uno.XInterface \
    com.sun.star.registry.XImplementationRegistration \
    com.sun.star.sheet.XAddIn \
    com.sun.star.lang.XServiceName \
    com.sun.star.lang.XServiceInfo \
    com.sun.star.lang.XTypeProvider \
    com.sun.star.uno.XWeak \
    com.sun.star.uno.XAggregation \
    com.sun.star.util.Date

# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/datefunc.obj

ALLIDLFILES=\
    dateadd.idl

# --- Library -----------------------------------

SHL1TARGET=$(TARGET)$(UPD)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS= \
        $(CPPUHELPERLIB)	\
        $(CPPULIB)			\
        $(VOSLIB)			\
        $(SALLIB) 

SHL1DEPN=makefile.mk
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# --- Targets ----------------------------------

.INCLUDE : target.mk

$(BIN)$/dateadd.rdb: $(ALLIDLFILES)
    +idlc -I$(PRJ) -I$(SOLARIDLDIR) -O$(BIN) $?
    +regmerge $@ /UCR $(BIN)$/{$(?:f:s/.idl/.urd/)}
    touch $@


