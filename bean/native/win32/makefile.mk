# *************************************************************
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
# *************************************************************
PRJ=..$/..

PRJNAME=odk
TARGET=officebean
ENABLE_EXCEPTIONS=TRUE
NO_DEFAULT_STL=TRUE
NO_BSYMBOLIC=TRUE
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES = \
    $(SLO)$/com_sun_star_comp_beans_LocalOfficeWindow.obj \
    $(SLO)$/com_sun_star_beans_LocalOfficeWindow.obj

SHL1TARGET=$(TARGET)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
#SHL1IMPLIB=	i$(TARGET)
SHL1STDLIBS=$(AWTLIB) $(SALLIB)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	$(TARGET).dxp
DEF1DES=officebean

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

