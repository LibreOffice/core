#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: pluby $ $Date: 2001-02-13 20:55:24 $
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

PRJNAME=	cppuhelper
TARGET=		testhelper
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
        $(OBJ)$/testhelper.obj		\
        $(OBJ)$/testpropshlp.obj 	\
        $(OBJ)$/testidlclass.obj 	\
        $(OBJ)$/testproptyphlp.obj	\
        $(OBJ)$/testimplhelper.obj \
        $(OBJ)$/testcontainer.obj

# gcc on Mac OS X optimizes out some temporary variables when optimization is
# turned on for compiling
.IF "$(OS)"=="MACOSX"
NOOPTFILES+=$(OBJFILES)
.IF "$(NOOPT_FLAG)"!=""
CFLAGSNOOPT=
.ENDIF
.ENDIF

# SCO and MACOSX: the linker does know about weak symbols, but we can't ignore multiple defined symbols
.IF "$(OS)"=="SCO" || "$(OS)$(COM)"=="OS2GCC" || "$(OS)"=="MACOSX"
OBJFILES+=$(OBJ)$/staticmbtest.obj
.ENDIF

APP1TARGET=	$(TARGET)
APP1OBJS=	$(OBJFILES) 

APP1STDLIBS+=	\
        $(CPPULIB)	\
        $(CPPUHELPERLIB) \
        $(SALLIB)

APP1DEF=	$(MISC)$/$(APP1TARGET).def

ALLIDLFILES:=	helpertest.idl

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL : 	$(BIN)$/cpputest.rdb	\
        unoheader 	\
        ALLTAR 

.ELSE
ALL: 	ALLDEP
.ENDIF

.INCLUDE :  target.mk

CPPUMAKERFLAGS =
.IF "$(COM)" == "MSC"
CPPUMAKERFLAGS = -L
.ENDIF

TYPES:=test.A;test.BA;test.CA;test.DBA;test.E;test.FE;test.G
HELPERTYPES:=com.sun.star.uno.XReference;com.sun.star.uno.XAdapter;com.sun.star.uno.XAggregation;com.sun.star.uno.XWeak;com.sun.star.lang.XComponent;com.sun.star.lang.XTypeProvider;com.sun.star.lang.XEventListener;com.sun.star.lang.XSingleServiceFactory;com.sun.star.lang.XMultiServiceFactory;com.sun.star.registry.XRegistryKey;com.sun.star.lang.XInitialization;com.sun.star.lang.XServiceInfo;com.sun.star.loader.XImplementationLoader;com.sun.star.lang.IllegalAccessException
FACTORYTYPES:=com.sun.star.lang.XComponent;com.sun.star.registry.XSimpleRegistry;com.sun.star.lang.XInitialization;com.sun.star.lang.XMultiServiceFactory;com.sun.star.loader.XImplementationLoader;com.sun.star.registry.XImplementationRegistration;com.sun.star.container.XSet;com.sun.star.lang.XSingleServiceFactory

$(BIN)$/cpputest.rdb: $(ALLIDLFILES)
    +unoidl -I$(PRJ) -I$(SOLARIDLDIR) -Burd -OH$(BIN) $?
    +regmerge $@ /UCR $(BIN)$/{$(?:f:s/.idl/.urd/)}
    +regmerge $@ / $(UNOUCRRDB)
    touch $@
    
unoheader: $(BIN)$/cpputest.rdb
    +cppumaker $(CPPUMAKERFLAGS) -BUCR -O$(UNOUCROUT) -T"$(TYPES);$(HELPERTYPES);$(FACTORYTYPES)" $(BIN)$/cpputest.rdb

