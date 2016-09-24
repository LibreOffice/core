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
PRJ=..$/..
TARGET= testjavavm
PRJNAME=	stoc
TARGET6=	testjavavm
TARGETTYPE=	CUI
LIBTARGET=	NO
ENABLE_EXCEPTIONS=TRUE

UNOUCRDEP=	$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=	$(SOLARBINDIR)$/udkapi.rdb
UNOUCROUT=	$(OUT)$/inc$
INCPRE+=	$(OUT)$/inc$

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Application 6 - testjavavm ------------------------------------

UNOTYPES= com.sun.star.lang.XMultiComponentFactory \
    com.sun.star.uno.XWeak	\
    com.sun.star.java.XJavaVM	\
    com.sun.star.java.XJavaThreadRegister_11	\
    com.sun.star.registry.XSimpleRegistry \
    com.sun.star.lang.XComponent \
    com.sun.star.registry.XImplementationRegistration \
    com.sun.star.lang.XSingleServiceFactory \
    com.sun.star.uno.TypeClass \
    com.sun.star.lang.XMultiServiceFactory

APP6TARGET= 	$(TARGET6)
APP6OBJS  = 	$(OBJ)$/testjavavm.obj 
APP6STDLIBS= \
        $(CPPULIB) 		\
        $(CPPUHELPERLIB) 	\
        $(SALHELPERLIB) 	\
        $(SALLIB)	

# --- Target ------------------------------------------------

.INCLUDE :	target.mk

