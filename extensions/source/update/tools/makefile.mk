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



PRJ = ..$/..$/..
PRJNAME = extensions
TARGET = ztool
LIBTARGET = NO
TARGETTYPE = CUI
ENABLE_EXCEPTIONS=TRUE

.INCLUDE: settings.mk

OBJFILES = \
    $(OBJ)$/ztool.obj

APP1TARGET = ztool
APP1OBJS = $(OBJ)$/ztool.obj
APP1STDLIBS = $(TOOLSLIB)

#APP1TARGET = ztool
#APP1OBJS = $(OBJ)$/ztool.obj
#.IF "$(GUI)" == "UNX"
#APP1STDLIBS = $(TOOLSLIB)
#.ELSE
#APP1LIBS = $(LB)$/itools.lib
#.ENDIF

#APP2TARGET = urltest
#APP2OBJS = $(OBJ)$/urltest.obj
#.IF "$(GUI)" == "UNX"
#APP2STDLIBS = $(TOOLSLIB) $(VOSLIB) $(SALLIB) $(CPPULIB) $(CPPUHELPERLIB)
#.ELSE
#APP2STDLIBS = $(LB)$/itools.lib ivos.lib isal.lib icppu.lib icppuhelper.lib
#.ENDIF

# APP3TARGET = tldem
# APP3OBJS = $(OBJ)$/tldem.obj
# .IF "$(GUI)" == "UNX"
# APP3STDLIBS = $(TOOLSLIB)
# .ELSE
# APP3STDLIBS = $(LB)$/itools.lib
# .ENDIF

.INCLUDE: target.mk
