#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

PRJ = ..$/..
PRJNAME = cli_ure

TARGET = climaker
TARGETTYPE = CUI
LIBTARGET = NO
ENABLE_EXCEPTIONS = TRUE

# disable caching to avoid stale objects
# on version changes
CCACHE_DISABLE=TRUE
.EXPORT : CCACHE_DISABLE

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

.IF "$(BUILD_FOR_CLI)" != ""

NO_OFFUH=TRUE
CPPUMAKERFLAGS =

UNOTYPES = \
    com.sun.star.uno.TypeClass					\
    com.sun.star.uno.XAggregation					\
    com.sun.star.uno.XWeak						\
    com.sun.star.uno.XComponentContext				\
    com.sun.star.lang.XTypeProvider					\
    com.sun.star.lang.XInitialization				\
    com.sun.star.lang.XComponent					\
    com.sun.star.lang.XMultiComponentFactory			\
    com.sun.star.lang.XMultiServiceFactory			\
    com.sun.star.lang.XSingleComponentFactory			\
    com.sun.star.container.XSet					\
    com.sun.star.container.XHierarchicalNameAccess			\
    com.sun.star.loader.XImplementationLoader			\
    com.sun.star.registry.XSimpleRegistry				\
    com.sun.star.registry.XRegistryKey				\
    com.sun.star.reflection.XTypeDescriptionEnumerationAccess	\
    com.sun.star.reflection.XConstantTypeDescription		\
    com.sun.star.reflection.XConstantsTypeDescription		\
    com.sun.star.reflection.XIndirectTypeDescription		\
    com.sun.star.reflection.XEnumTypeDescription			\
    com.sun.star.reflection.XInterfaceTypeDescription2		\
    com.sun.star.reflection.XInterfaceMethodTypeDescription		\
    com.sun.star.reflection.XInterfaceAttributeTypeDescription2	\
    com.sun.star.reflection.XCompoundTypeDescription		\
    com.sun.star.reflection.XServiceTypeDescription2		\
    com.sun.star.reflection.XSingletonTypeDescription2		\
    com.sun.star.reflection.XStructTypeDescription

CFLAGSCXX +=-AI$(BIN)


# When compiling for CLR, disable "warning C4339: use of undefined type detected
# in CLR meta-data - use of this type may lead to a runtime exception":
CFLAGSCXX += -clr -LN -wd4339 -wd4715

OBJFILES = \
    $(OBJ)$/climaker_app.obj	\
    $(OBJ)$/climaker_emit.obj

APP1TARGET = $(TARGET)
APP1OBJS = $(OBJFILES)


APP1STDLIBS = \
    $(CPPUHELPERLIB)		\
    $(CPPULIB)			\
    $(SALLIB)			\
    mscoree.lib

.IF "$(USE_DEBUG_RUNTIME)" == ""
APP1STDLIBS += \
    msvcmrt.lib
.ELSE
APP1STDLIBS += \
    msvcmrtd.lib
.ENDIF

.ENDIF

.INCLUDE : $(PRJ)$/util$/target.pmk
.INCLUDE :  target.mk

CLIMAKER_CONFIG = $(BIN)$/climaker.exe.config

ALLTAR: \
    $(CLIMAKER_CONFIG)

#Create the config file that is used with the policy assembly
$(CLIMAKER_CONFIG): climaker.exe.config
    $(COPY) $< $@
    chmod +x $@

.IF "$(BUILD_FOR_CLI)" != ""

$(OBJFILES): $(BIN)$/cli_basetypes.dll

.ENDIF
