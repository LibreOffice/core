#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 15:29:34 $
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

PRJNAME=stoc
TARGET=jen
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

# Kollision zwischen bool.h aus Java und bool.h aus der STL.
# Das Problem tritt fuer alle Plattformen auf, aber anscheinend stolpert nur der
# GCC darueber
.IF "$(COM)" == "GCC"
CDEFS += -D__SGI_STL_BOOL_H
.ENDIF

.INCLUDE :  ..$/cppumaker.mk

UNOTYPES= \
    com.sun.star.uno.Exception	                                    \
    com.sun.star.uno.XInterface		                                \
    com.sun.star.java.XJavaVM 	                                    \
    com.sun.star.uno.XWeak		                                    \
    com.sun.star.uno.TypeClass	                                    \
    com.sun.star.java.XJavaThreadRegister_11						\
    com.sun.star.lang.XServiceInfo		                            \
    com.sun.star.lang.XTypeProvider                                 \
    com.sun.star.lang.XMultiServiceFactory	                        \
    com.sun.star.container.XNameAccess		                        \
    com.sun.star.lang.XSingleServiceFactory	                        \
    com.sun.star.registry.XRegistryKey                              \
    com.sun.star.registry.XSimpleRegistry                           \
    com.sun.star.frame.XConfigManager

SLOFILES= \
        $(SLO)$/settings.obj	\
        $(SLO)$/javavm.obj		\
        $(SLO)$/jvmargs.obj		\
        $(SLO)$/jen_desc.obj		

SHL1TARGET= $(TARGET)

SHL1STDLIBS= \
        $(CPPUHELPERLIB) 	\
        $(CPPULIB)	    	\
        $(UNOLIB)	    	\
        $(VOSLIB) 	    	\
        $(SALLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS += advapi32.lib
.ENDIF

SHL1DEPN=		
SHL1IMPLIB=		i$(TARGET)
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL : 	$(BIN)$/jen.rdb	\
        $(MISC)$/jen_desc.cxx	\
        ALLTAR 
.ELSE
ALL: 		ALLDEP
.ENDIF

.INCLUDE :	target.mk

$(BIN)$/jen.rdb: $(SOLARBINDIR)$/applicat.rdb
    +rdbmaker -BUCR -O$(BIN)$/jen.rdb $(foreach,i,$(UNOTYPES) -T$i ) $(SOLARBINDIR)$/applicat.rdb

$(MISC)$/jen_desc.cxx: jen.xml
    +xml2cmp -func $(MISC)$/jen_desc.cxx jen.xml


