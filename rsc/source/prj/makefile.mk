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



PRJ=..$/..

PRJNAME=rsc
TARGETTYPE=CUI
TARGET=rsc
LIBTARGET=NO

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------------

OBJFILES=   $(OBJ)$/gui.obj          \
            $(OBJ)$/start.obj

APP1TARGET= rsc
APP1STDLIBS=$(TOOLSLIB) $(I18NISOLANGLIB) $(VOSLIB) $(SALLIB) # $(RTLLIB)
APP1LIBS=   $(LB)$/rsctoo.lib
APP1OBJS=   $(OBJ)$/start.obj
.IF "$(GUI)" != "OS2"
# why not this way?
APP1STACK=64000
#APP1STACK=32768
.ENDIF
APP1RPATH=NONE

APP2TARGET= rsc2
.IF "$(OS)"=="SCO"
# SCO hat Probleme mit fork/exec und einigen shared libraries. 
# rsc2 muss daher statisch gelinkt werden
APP2STDLIBS=$(STATIC) -latools $(BPICONVLIB) $(VOSLIB) $(OSLLIB) $(RTLLIB) $(DYNAMIC)
.ELSE
APP2STDLIBS=$(TOOLSLIB) $(I18NISOLANGLIB) $(VOSLIB) $(SALLIB) # $(RTLLIB)
.ENDIF
APP2LIBS=   $(LB)$/rsctoo.lib \
            $(LB)$/rscres.lib \
            $(LB)$/rscpar.lib \
            $(LB)$/rscrsc.lib \
            $(LB)$/rscmis.lib
APP2OBJS=   $(OBJ)$/gui.obj
.IF "$(GUI)" != "OS2"
# why not this way?
APP2STACK=64000
#APP2STACK=32768
.ENDIF
APP2RPATH=NONE

# --- Targets ------------------------------------------------------------

.INCLUDE :  target.mk

