#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************


PRJ=..

PRJNAME=cppuhelper
TARGET=cppuhelper

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
VISIBILITY_HIDDEN=TRUE

# not strictly a bootstrap service but containing
# bootstrap code that may require generated files
# without "-L" (light) switch
BOOTSTRAP_SERVICE=TRUE

.IF "$(OS)" != "WNT" && "$(GUI)"!="OS2" && "$(OS)" != "MACOSX"
UNIXVERSIONNAMES=UDK
.ENDIF # WNT OS2 MACOSX

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRDEP=$(UNOUCRRDB)
UNOUCROUT=$(OUT)$/inc$/$(TARGET)
INCPRE+=$(OUT)$/inc$/$(TARGET) $(OUT)$/inc$/private

CPPUMAKERFLAGS= -C

UNOTYPES= \
        com.sun.star.beans.PropertyAttribute \
        com.sun.star.beans.PropertyValue \
        com.sun.star.beans.XFastPropertySet \
        com.sun.star.beans.XMultiPropertySet \
        com.sun.star.beans.XPropertyAccess \
        com.sun.star.beans.XPropertySet \
        com.sun.star.bridge.UnoUrlResolver \
        com.sun.star.bridge.XUnoUrlResolver \
        com.sun.star.connection.SocketPermission \
        com.sun.star.container.XContentEnumerationAccess \
        com.sun.star.container.XElementAccess \
        com.sun.star.container.XEnumerationAccess \
        com.sun.star.container.XHierarchicalNameAccess \
        com.sun.star.container.XNameAccess \
        com.sun.star.container.XNameReplace \
        com.sun.star.container.XNameContainer \
        com.sun.star.container.XSet \
        com.sun.star.io.FilePermission \
        com.sun.star.io.IOException \
        com.sun.star.lang.DisposedException \
        com.sun.star.lang.WrappedTargetRuntimeException \
        com.sun.star.lang.XComponent \
        com.sun.star.lang.XEventListener \
        com.sun.star.lang.XInitialization \
        com.sun.star.lang.XMultiComponentFactory \
        com.sun.star.lang.XMultiServiceFactory \
        com.sun.star.lang.XServiceInfo \
        com.sun.star.lang.XSingleComponentFactory \
        com.sun.star.lang.XSingleServiceFactory \
        com.sun.star.lang.XUnoTunnel \
        com.sun.star.lang.XTypeProvider \
        com.sun.star.loader.XImplementationLoader \
        com.sun.star.reflection.XArrayTypeDescription \
        com.sun.star.reflection.XCompoundTypeDescription \
        com.sun.star.reflection.XEnumTypeDescription \
        com.sun.star.reflection.XIdlClass \
        com.sun.star.reflection.XIdlClassProvider \
        com.sun.star.reflection.XIdlField2 \
        com.sun.star.reflection.XIdlReflection \
        com.sun.star.reflection.XIndirectTypeDescription \
        com.sun.star.reflection.XInterfaceAttributeTypeDescription \
        com.sun.star.reflection.XInterfaceAttributeTypeDescription2 \
        com.sun.star.reflection.XInterfaceMemberTypeDescription \
        com.sun.star.reflection.XInterfaceMethodTypeDescription \
        com.sun.star.reflection.XInterfaceTypeDescription2 \
        com.sun.star.reflection.XMethodParameter \
        com.sun.star.reflection.XStructTypeDescription \
        com.sun.star.reflection.XTypeDescription \
        com.sun.star.reflection.XUnionTypeDescription \
        com.sun.star.registry.XImplementationRegistration \
        com.sun.star.registry.XRegistryKey \
        com.sun.star.registry.XSimpleRegistry \
        com.sun.star.security.RuntimePermission \
        com.sun.star.security.XAccessController \
        com.sun.star.uno.DeploymentException \
        com.sun.star.uno.RuntimeException \
        com.sun.star.uno.XAggregation \
        com.sun.star.uno.XComponentContext \
        com.sun.star.uno.XCurrentContext \
        com.sun.star.uno.XUnloadingPreference \
        com.sun.star.uno.XWeak \
        com.sun.star.util.XMacroExpander

.IF "$(debug)" != ""
# msvc++: no inlining for debugging
.IF "$(COM)" == "MSC"
CFLAGS += -Ob0
.ENDIF
.ENDIF

CFLAGS += -DCPPUHELPER_DLLIMPLEMENTATION

SLOFILES= \
        $(SLO)$/typeprovider.obj 	\
        $(SLO)$/exc_thrower.obj 	\
        $(SLO)$/servicefactory.obj 	\
        $(SLO)$/bootstrap.obj 		\
        $(SLO)$/primeweak.obj 		\
        $(SLO)$/implbase.obj 		\
        $(SLO)$/implbase_ex.obj 	\
        $(SLO)$/propshlp.obj 		\
        $(SLO)$/weak.obj		\
        $(SLO)$/interfacecontainer.obj	\
        $(SLO)$/stdidlclass.obj 	\
        $(SLO)$/factory.obj		\
        $(SLO)$/component_context.obj	\
        $(SLO)$/component.obj		\
        $(SLO)$/shlib.obj		\
        $(SLO)$/tdmgr.obj		\
        $(SLO)$/implementationentry.obj	\
        $(SLO)$/access_control.obj	\
        $(SLO)$/macro_expander.obj \
            $(SLO)$/unourl.obj \
        $(SLO)$/propertysetmixin.obj \
        $(SLO)$/findsofficepath.obj

OBJFILES = $(OBJ)$/findsofficepath.obj

.IF "$(GUI)" == "WNT"
SHL1TARGET=$(TARGET)$(UDK_MAJOR)$(COMID)
.ELIF "$(GUI)" == "OS2"
SHL1TARGET=cppuh$(UDK_MAJOR)
.ELSE
SHL1TARGET=uno_$(TARGET)$(COMID)
.ENDIF

SHL1STDLIBS= \
        $(SALLIB)		\
        $(SALHELPERLIB)	\
        $(CPPULIB)
.IF "$(OS)" == "WNT"
SHL1STDLIBS += $(ADVAPI32LIB)
.ENDIF

SHL1DEPN=
SHL1IMPLIB=i$(TARGET)
SHL1OBJS = $(SLOFILES)
SHL1RPATH=URELIB

SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)

.IF "$(COMNAME)"=="msci"
.ELIF "$(COMNAME)"=="mscx"
.ELIF "$(COMNAME)"=="sunpro5"
SHL1VERSIONMAP=cc5_solaris_sparc.map
.ELIF "$(GUI)$(COMNAME)"=="OS2gcc3"
SHL1VERSIONMAP=gcc3os2.map
.ELSE
SHL1VERSIONMAP=gcc3.map
.ENDIF

# --- Targets ------------------------------------------------------

.IF "$(diag)"!=""
CFLAGS += -DDIAG=$(diag)
.ENDIF

.INCLUDE :	target.mk
