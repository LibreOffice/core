#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 16:16:52 $
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
PRJ=..$/..

PRJNAME=extensions
TARGET=res
LIBTARGET=NO
USE_LDUMP2=TRUE
USE_DEFFILE=TRUE
# NEW
NO_BSYMBOLIC=TRUE
# END NEW

# --- Settings -----------------------------------------------------
.INCLUDE :	$(PRJ)$/util$/makefile.pmk


# --- Files --------------------------------------------------------

SLOFILES=	$(SLO)$/resource.obj

EXCEPTIONSFILES=	$(SLO)$/resource.obj

LIB1TARGET= 	$(SLB)$/$(TARGET).lib
LIB1OBJFILES=	$(EXCEPTIONSFILES)

SHL1TARGET= $(TARGET)$(UPD)$(DLLPOSTFIX)

#		$(UNOLIB)	 \
# NEW				 \
#		$(CPPULIB) 	 \
#		$(CPPUHELPERLIB) 	 \
# END NEW			 \

SHL1STDLIBS= \
        $(CPPULIB) 	 \
        $(CPPUHELPERLIB) 	 \
        $(VOSLIB) 	 \
        $(SALLIB) 	 \
        $(VCLLIB)	\
        $(TOOLSLIB)

SHL1DEPN=
SHL1IMPLIB=		ires
SHL1LIBS=		$(LIB1TARGET)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

# NEW
UNOUCRDEP=	$(SOLARBINDIR)$/applicat.rdb
UNOUCRRDB=	$(SOLARBINDIR)$/applicat.rdb

UNOTYPES=\
    com.sun.star.uno.XWeak						\
    com.sun.star.uno.XAggregation				\
    com.sun.star.lang.XSingleServiceFactory		\
    com.sun.star.lang.XMultiServiceFactory		\
    com.sun.star.lang.XTypeProvider				\
    com.sun.star.lang.XServiceInfo				\
    com.sun.star.reflection.InvocationTargetException	\
    com.sun.star.registry.XRegistryKey			\
    com.sun.star.script.XInvocation				\
    com.sun.star.script.XTypeConverter			\
    com.sun.star.beans.XExactName
# END NEW

DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk

