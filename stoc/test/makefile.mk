#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.20 $
#
#   last change: $Author: hr $ $Date: 2003-03-27 12:01:24 $
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

PRJNAME=stoc
TARGET=stoc
TARGET1=testloader
TARGET2=testregistry
TARGET3=testsmgr
TARGET4=testcorefl
TARGET5=testinvocation
TARGET6=testintrosp
TARGET7=testconv
TARGET8=testproxyfac
TARGET9=testsmgr2
TARGETTYPE=CUI
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Application 1 ------------------------------------------------
APP1TARGET= $(TARGET1)
APP1OBJS=   $(OBJ)$/testloader.obj 

APP1STDLIBS= \
        $(CPPULIB) 	\
        $(CPPUHELPERLIB) 	\
        $(SALHELPERLIB) 	\
        $(SALLIB)

.IF "$(GUI)"=="WNT"
APP1STDLIBS+= 
#		$(LIBCIMT) $(LIBCMT)
.ENDIF

# --- Application 2 ------------------------------------------------
APP2TARGET= $(TARGET2)
APP2OBJS=   $(OBJ)$/testregistry.obj $(OBJ)$/mergekeys.obj

APP2STDLIBS= \
        $(CPPULIB)	\
        $(CPPUHELPERLIB) 	\
        $(SALHELPERLIB) 	\
        $(SALLIB)

.IF "$(GUI)"=="WNT"
APP2STDLIBS+= 
#		$(LIBCIMT) $(LIBCMT)
.ENDIF


# --- Application 3 - testsmgr main ------------------------------------
APP3TARGET=	$(TARGET3)
APP3OBJS  =	$(OBJ)$/testsmgr.obj 
APP3STDLIBS= \
        $(CPPULIB)	\
        $(CPPUHELPERLIB) 	\
        $(SALHELPERLIB) 	\
        $(SALLIB) 	

.IF "$(GUI)"=="UNX"
APP3STDLIBS+= -lat
.ENDIF
.IF "$(GUI)"=="WNT"
APP3STDLIBS+=	$(LIBCIMT) \
        iat.lib
.ENDIF

# --- Application 4 - testcorefl main ------------------------------------
APP4TARGET=	$(TARGET4)
APP4OBJS  = 	$(OBJ)$/testcorefl.obj 
APP4STDLIBS= \
        $(CPPULIB) 		\
        $(CPPUHELPERLIB) 	\
        $(SALLIB)

.IF "$(GUI)"=="WNT"
APP4STDLIBS+=	$(LIBCIMT)
.ENDIF

# --- Application 5 - testinvocation main ------------------------------------
APP5TARGET= 	$(TARGET5)
APP5OBJS  = 	$(OBJ)$/testiadapter.obj 
APP5STDLIBS= \
        $(CPPULIB) 		\
        $(CPPUHELPERLIB) 	\
        $(SALLIB)

.IF "$(GUI)"=="WNT"
APP5STDLIBS+=	$(LIBCIMT)
.ENDIF

# --- Application 6 - testitrosp main ------------------------------------
APP6TARGET= 	$(TARGET6)
APP6OBJS  = 	$(OBJ)$/testintrosp.obj 
APP6STDLIBS= \
        $(CPPULIB) 		\
        $(CPPUHELPERLIB) 	\
        $(SALHELPERLIB) 	\
        $(SALLIB)

.IF "$(GUI)"=="WNT"
APP6STDLIBS+=	$(LIBCIMT)
.ENDIF

# --- Application 7 - testconv main ------------------------------------
APP7TARGET= 	$(TARGET7)
APP7OBJS  = 	$(OBJ)$/testconv.obj 
APP7STDLIBS= \
        $(CPPULIB) 		\
        $(CPPUHELPERLIB) 	\
        $(SALLIB)

.IF "$(GUI)"=="WNT"
APP7STDLIBS+=	$(LIBCIMT)
.ENDIF

# --- Application 8 - testproxyfac main ------------------------------------
APP8TARGET= 	$(TARGET8)
APP8OBJS  = 	$(OBJ)$/testproxyfac.obj 
APP8STDLIBS= \
        $(CPPULIB) 		\
        $(CPPUHELPERLIB) 	\
        $(SALLIB)

.IF "$(GUI)"=="WNT"
APP8STDLIBS+=	$(LIBCIMT)
.ENDIF

# --- Application 9 - testproxyfac main ------------------------------------
APP9TARGET= 	$(TARGET9)
APP9OBJS  = 	$(OBJ)$/testsmgr2.obj 
APP9STDLIBS= \
        $(CPPULIB) 		\
        $(CPPUHELPERLIB) 	\
        $(SALLIB)


ALLIDLFILES:=	testcorefl.idl language_binding.idl testintrosp.idl


# --- Target ------------------------------------------------

.IF "$(depend)" == ""
ALL : 	$(MISC)$/test_types_generated.flag \
    $(BIN)$/test1.rdb 		   \
    $(BIN)$/test2.rdb		   \
    ALLTAR
.ELSE
ALL: 		ALLDEP
.ENDIF

.INCLUDE :	target.mk

CPPUMAKERFLAGS =
.IF "$(COM)" == "MSC"
CPPUMAKERFLAGS = -L
.ENDIF

FACTORYTYPES:= 	-T com.sun.star.lang.XSingleComponentFactory \
        -T com.sun.star.uno.XComponentContext \
        -T com.sun.star.uno.XWeak \
        -T com.sun.star.container.XEnumeration \
        -T com.sun.star.lang.XComponent \
        -T com.sun.star.registry.XSimpleRegistry \
        -T com.sun.star.lang.XInitialization \
        -T com.sun.star.lang.XMultiServiceFactory\
        -T com.sun.star.loader.XImplementationLoader \
        -T com.sun.star.registry.XImplementationRegistration \
        -T com.sun.star.container.XSet \
        -T com.sun.star.lang.XSingleServiceFactory\
        -T com.sun.star.lang.XServiceInfo \
        -T com.sun.star.container.XContentEnumerationAccess \
        -T com.sun.star.container.XEnumeration

.IF "$(GUI)"=="WNT"
MY_DLLPOSTFIX=.dll
MY_DLLPREFIX=
.ELSE
MY_DLLPOSTFIX=.so
MY_DLLPREFIX=lib
.ENDIF
TESTCOREFL:=ModuleC;ModuleC.XInterfaceA;ModuleC.XInterfaceB;ModuleA.XInterface1;com.sun.star.reflection.XIdlReflection;com.sun.star.reflection.XIdlField;com.sun.star.reflection.XIdlArray;com.sun.star.reflection.XIdlMethod;com.sun.star.reflection.XIdlClass;com.sun.star.beans.XPropertySet;com.sun.star.lang.XComponent;com.sun.star.container.XHierarchicalNameAccess;com.sun.star.reflection.XIdlField2;com.sun.star.lang.DisposedException
TESTIADAPTER:=com.sun.star.beans.XIntrospection;com.sun.star.beans.MethodConcept;com.sun.star.beans.XExactName;com.sun.star.lang.XTypeProvider;com.sun.star.uno.XAggregation;com.sun.star.script.XInvocationAdapterFactory;com.sun.star.script.XInvocationAdapterFactory2;com.sun.star.script.XInvocation;com.sun.star.lang.XMultiServiceFactory;com.sun.star.registry.XSimpleRegistry;com.sun.star.lang.XInitialization;test.XLanguageBindingTest
TESTINTROSP:=ModuleA;ModuleA.XIntroTest;com.sun.star.beans.XPropertySet;com.sun.star.container.XIndexAccess;com.sun.star.container.XNameAccess;com.sun.star.beans.PropertyAttribute;com.sun.star.beans.PropertyConcept
TESTCONV:=com.sun.star.script.XTypeConverter
TESTPROXYFAC:=com.sun.star.reflection.XProxyFactory
TESTSECURITY:=com.sun.star.security.AllPermission;com.sun.star.security.XPolicy;com.sun.star.security.XAccessController;com.sun.star.io.FilePermission;com.sun.star.connection.SocketPermission;com.sun.star.uno.XCurrentContext

$(BIN)$/test1.rdb:
    +cd $(BIN) && regcomp -register -r test1.rdb -c $(MY_DLLPREFIX)at$(MY_DLLPOSTFIX)

$(BIN)$/test2.rdb:
    +cd $(BIN) && regcomp -register -r test2.rdb -c $(MY_DLLPREFIX)remotebridge$(MY_DLLPOSTFIX)

$(BIN)$/stoctest.rdb: $(ALLIDLFILES)
    +idlc -I$(PRJ) -I$(SOLARIDLDIR) -O$(BIN) $?
    +regmerge $@ /UCR $(BIN)$/{$(?:f:s/.idl/.urd/)}
    +regmerge $@ / $(SOLARBINDIR)$/udkapi.rdb
    +regcomp -register -r $@ -c corefl
    touch $@

$(MISC)$/test_types_generated.flag : $(BIN)$/stoctest.rdb  makefile.mk
    -rm -f $(MISC)$/test_types_generated.flag
    +cppumaker $(CPPUMAKERFLAGS) -BUCR -O$(UNOUCROUT) $(FACTORYTYPES) -T"$(TESTIADAPTER)" $(BIN)$/stoctest.rdb
    +cppumaker $(CPPUMAKERFLAGS) -BUCR -O$(UNOUCROUT) -T"$(TESTCOREFL)" $(BIN)$/stoctest.rdb
    +cppumaker $(CPPUMAKERFLAGS) -BUCR -O$(UNOUCROUT) -T"$(TESTINTROSP)" $(BIN)$/stoctest.rdb
    +cppumaker $(CPPUMAKERFLAGS) -BUCR -O$(UNOUCROUT) -T"$(TESTCONV)" $(BIN)$/stoctest.rdb
    +cppumaker $(CPPUMAKERFLAGS) -BUCR -O$(UNOUCROUT) -T"$(TESTPROXYFAC)" $(BIN)$/stoctest.rdb
    +cppumaker $(CPPUMAKERFLAGS) -BUCR -O$(UNOUCROUT) -T"$(TESTSECURITY)" $(BIN)$/stoctest.rdb
    touch $(MISC)$/test_types_generated.flag
