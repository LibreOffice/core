#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: hr $ $Date: 2003-03-19 17:23:42 $
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

PRJNAME=cppuhelper
TARGET=testhelper
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE
USE_DEFFILE=TRUE

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

APP1TARGET=$(TARGET)
APP1OBJS=$(OBJFILES) 

APP1STDLIBS+=	\
        $(CPPULIB)	\
        $(CPPUHELPERLIB) \
        $(SALLIB)

APP1DEF=	$(MISC)$/$(APP1TARGET).def

#############################################

APP2OBJS = $(OBJ)$/testdefaultbootstrapping.obj
APP2STDLIBS += $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB)
APP2TARGET = testdefaultbootstrapping

#############################################

SLOFILES= \
        $(SLO)$/cfg_test.obj
LIB1TARGET=$(SLB)$/cfg_test.lib
LIB1OBJFILES=$(SLOFILES)


APP3OBJS = $(OBJ)$/cfg_test.obj
APP3STDLIBS += $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB)
APP3TARGET = test_cfg

#############################################

ALLIDLFILES:=	helpertest.idl

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL: $(BIN)$/cpputest.rdb				\
     unoheader 					\
     $(BIN)$/testrc 				\
     $(BIN)$/testdefaultbootstrapping.pl		\
    ALLTAR 						\
     $(BIN)$/$(APP2TARGET).bin  			\
     $(BIN)$/$(APP2TARGET).Bin  			\
     $(BIN)$/$(APP2TARGET).Exe

.IF "$(GUI)"=="UNX"
ALL:  $(BIN)$/$(APP2TARGET).exe

$(BIN)$/$(APP2TARGET).exe : $(APP2TARGETN)
    cp $(APP2TARGETN) $@

.ENDIF


$(BIN)$/$(APP2TARGET).bin : $(APP2TARGETN)
    cp $(APP2TARGETN) $@

$(BIN)$/$(APP2TARGET).Bin : $(APP2TARGETN)
    cp $(APP2TARGETN) $@

$(BIN)$/$(APP2TARGET).Exe : $(APP2TARGETN)
    cp $(APP2TARGETN) $@

.ELSE
ALL: 	ALLDEP
.ENDIF

.INCLUDE :  target.mk

CPPUMAKERFLAGS =
.IF "$(COM)" == "MSC"
CPPUMAKERFLAGS = -L
.ENDIF

TYPES:=test.A;test.BA;test.CA;test.DBA;test.E;test.FE;test.G;test.H;test.I
HELPERTYPES:=com.sun.star.uno.XReference;com.sun.star.uno.XAdapter;com.sun.star.uno.XAggregation;com.sun.star.uno.XWeak;com.sun.star.lang.XComponent;com.sun.star.lang.XTypeProvider;com.sun.star.lang.XEventListener;com.sun.star.lang.XSingleServiceFactory;com.sun.star.lang.XMultiServiceFactory;com.sun.star.registry.XRegistryKey;com.sun.star.lang.XInitialization;com.sun.star.lang.XServiceInfo;com.sun.star.loader.XImplementationLoader;com.sun.star.lang.IllegalAccessException;com.sun.star.beans.XVetoableChangeListener;com.sun.star.beans.XPropertySet;com.sun.star.uno.XComponentContext
FACTORYTYPES:=com.sun.star.lang.XComponent;com.sun.star.registry.XSimpleRegistry;com.sun.star.lang.XInitialization;com.sun.star.lang.XMultiServiceFactory;com.sun.star.loader.XImplementationLoader;com.sun.star.registry.XImplementationRegistration;com.sun.star.container.XSet;com.sun.star.lang.XSingleServiceFactory;com.sun.star.lang.XSingleComponentFactory;com.sun.star.lang.XMultiComponentFactory

$(BIN)$/cpputest.rdb: $(ALLIDLFILES)
    +idlc -I$(PRJ) -I$(SOLARIDLDIR) -O$(BIN) $?
    +regmerge $@ /UCR $(BIN)$/{$(?:f:s/.idl/.urd/)}
    +regmerge $@ / $(UNOUCRRDB)
    +regcomp -register -r $@ -c $(DLLPRE)corefl$(DLLPOST)
    touch $@

unoheader: $(BIN)$/cpputest.rdb
    +cppumaker $(CPPUMAKERFLAGS) -BUCR -O$(UNOUCROUT) -T"$(TYPES);$(HELPERTYPES)" $(BIN)$/cpputest.rdb
    +cppumaker $(CPPUMAKERFLAGS) -BUCR -O$(UNOUCROUT) -T"$(FACTORYTYPES)" $(BIN)$/cpputest.rdb


$(BIN)$/testdefaultbootstrapping.pl: testdefaultbootstrapping.pl
    cp testdefaultbootstrapping.pl $@

$(BIN)$/testrc: makefile.mk
.IF "$(GUI)"=="WNT"
    echo [Tests] > $@
    echo TestKey1=com.sun.star.script.Invocation.rdb >> $@
.ELSE
    echo '[Tests]' > $@
    echo 'TestKey1=file:///'$(PWD)/$(BIN)'/com.sun.star.script.Invocation.rdb' >> $@
.ENDIF



