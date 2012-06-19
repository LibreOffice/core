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

PRJNAME=bridges
TARGET=inter
LIBTARGET=NO
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

# output directory (one dir for each project)
UNOUCROUT=$(OUT)$/inc$/$(TARGET)

# adding to inludeoath
INCPRE+=$(UNOUCROUT)

UNOTYPESTYPES := \
        com.sun.star.lang.IllegalArgumentException \
        com.sun.star.uno.DeploymentException

SLOFILES=$(SLO)$/starter.obj $(SLO)$/thrower.obj

SHL1TARGET=starter
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1IMPLIB=i$(SHL1TARGET)
SHL1OBJS=$(SLO)$/starter.obj
DEF1NAME=$(SHL1TARGET)
SHL1STDLIBS+= $(CPPULIB) $(SALLIB)
SHL1VERSIONMAP=$(SHL1TARGET).map

SHL2TARGET=thrower
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
SHL2IMPLIB=i$(SHL2TARGET)
SHL2OBJS=$(SLO)$/thrower.obj
DEF2NAME=$(SHL2TARGET)
SHL2STDLIBS+= $(CPPULIB) $(SALLIB)
SHL2VERSIONMAP=$(SHL2TARGET).map

OBJFILES=$(OBJ)$/inter.obj
APP1TARGET=inter
APP1OBJS=$(OBJ)$/inter.obj
APP1STDLIBS+=\
        $(SALLIB)

.INCLUDE :  target.mk

