#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: dbo $ $Date: 2000-11-15 12:28:29 $
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

PRJNAME=	testcppu
TARGET=		testcppu
LIBTARGET=	NO
TARGETTYPE=	CUI
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

UNOUCRDEP=	$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=	$(SOLARBINDIR)$/udkapi.rdb
UNOUCROUT=	$(OUT)$/inc$/test
INCPRE+=	$(OUT)$/inc$/test

OBJFILES=	\
        $(OBJ)$/testcppu.obj	\
        $(OBJ)$/test_di.obj


APP1TARGET=	testcppu
APP1OBJS=	$(OBJ)$/test_di.obj	\
        $(OBJ)$/testcppu.obj

APP1STDLIBS+=	\
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

APP1DEF=	$(MISC)$/$(APP1TARGET).def

ALLIDLFILES:=	cpputest.idl 		\
        language_binding.idl

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL : $(BIN)$/testcppu.rdb unoheader ALLTAR 
.ELSE
ALL: 	ALLDEP
.ENDIF

.INCLUDE :  target.mk

CPPUMAKERFLAGS = -C
.IF "$(COM)" == "MSC"
CPPUMAKERFLAGS = -L
.ENDIF

TYPES:=		-Ttest.XLanguageBindingTest \
        -Ttest.XSimpleInterface \
        -Ttest.Test1 \
        -Ttest.Test2 \
        -Ttest.TdTest1 \
        -Ttest.Test3 \
        -Ttest.Base \
        -Ttest.Base1 \
        -Ttest.Base2 \
        -Tcom.sun.star.lang.XMultiServiceFactory \
        -Tcom.sun.star.lang.XSingleServiceFactory \
        -Tcom.sun.star.lang.XInitialization \
        -Tcom.sun.star.lang.XServiceInfo \
        -Tcom.sun.star.lang.XEventListener \
        -Tcom.sun.star.lang.XTypeProvider \
        -Tcom.sun.star.registry.XSimpleRegistry \
        -Tcom.sun.star.registry.XRegistryKey \
        -Tcom.sun.star.loader.XImplementationLoader \
        -Tcom.sun.star.registry.XImplementationRegistration \
        -Tcom.sun.star.lang.XComponent \
        -Tcom.sun.star.container.XSet \
        -Tcom.sun.star.uno.XReference \
        -Tcom.sun.star.uno.XAdapter \
        -Tcom.sun.star.uno.XAggregation \
        -Tcom.sun.star.uno.XWeak \
        -Tcom.sun.star.reflection.XIdlClassProvider

$(BIN)$/testcppu.rdb: $(ALLIDLFILES)
    +unoidl -I$(PRJ) -I$(SOLARIDLDIR) -Burd -OH$(BIN) $?
    +regmerge $@ /UCR $(BIN)$/{$(?:f:s/.idl/.urd/)}
    +regmerge $@ / $(UNOUCRRDB)
    touch $@
    
unoheader: $(BIN)$/testcppu.rdb
    +cppumaker $(CPPUMAKERFLAGS) -BUCR -O$(UNOUCROUT) $(TYPES) $(BIN)$/testcppu.rdb

