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

INCPRE=$(MISC)

PRJNAME=l10ntools
TARGET=tralay
#TARGETTYPE=GUI
TARGETTYPE=CUI
LIBTARGET=no

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  settings.mk

.IF "$(SYSTEM_EXPAT)" == "YES"
CFLAGS+=-DSYSTEM_EXPAT
.ENDIF

# --- Files --------------------------------------------------------

APP1TARGET=$(TARGET)

OBJFILES =\
    $(OBJ)/export2.obj\
    $(OBJ)/helpmerge.obj\
    $(OBJ)/layoutparse.obj\
    $(OBJ)/merge.obj\
    $(OBJ)/tralay.obj\
    $(OBJ)/xmlparse.obj

APP1OBJS = $(OBJFILES)

APP1STDLIBS =\
    $(TOOLSLIB)\
    $(EXPATASCII3RDLIB)\
    $(VOSLIB)\
    $(CPPULIB) \
    $(SALLIB)	

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

test .PHONY:
    ../$(INPATH)/bin/tralay -l en-US -o out.sdf zoom.xml
    cat out.sdf > trans.sdf
    sed 's/en-US\t/de\tde:/' out.sdf >> trans.sdf 
    ../$(INPATH)/bin/tralay -m trans.sdf -l de -o zoom-DE.xml zoom.xml
