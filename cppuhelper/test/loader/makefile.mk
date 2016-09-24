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

PRJ     := ..$/..
PRJNAME := cppuhelper
TARGET  := loader.test


ENABLE_EXCEPTIONS  := TRUE
USE_DEFFILE        := TRUE

.INCLUDE :  settings.mk

ENVINCPRE  := -I$(OUT)$/inc$/$(TARGET)

APP1TARGET  := $(TARGET)
APP1OBJS    := $(OBJ)$/loader.test.obj
.IF "$(OS)"=="WNT"
APP1STDLIBS := iTestComponent.uno.lib
.ELSE
APP1STDLIBS := -lTestComponent.uno

.ENDIF

APP1STDLIBS +=  $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) 

.INCLUDE :  target.mk
