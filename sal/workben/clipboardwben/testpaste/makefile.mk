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

PRJ=..$/..$/..

PRJNAME=sal
TARGET=cbptest
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# --- Files --------------------------------------------------------

#
# test clipboard paste
#
    # --- Resources ----------------------------------------------------
    RCFILES=  cbptest.rc

    OBJFILES= $(OBJ)$/cbptest.obj

    APP1TARGET=	$(TARGET)
    APP1OBJS=	$(OBJFILES)
    APP1NOSAL=  TRUE
    APP1NOSVRES= $(RES)$/$(TARGET).res

    APP1STDLIBS+=$(OLE32LIB) $(USER32LIB) $(KERNEL32LIB)
    
    APP1LIBS=$(LB)$/ole9x.lib \
             $(LB)$/tools32.lib 

    APP1DEPN=   makefile.mk $(APP1NOSVRES)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


