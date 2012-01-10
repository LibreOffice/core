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



PRJ=..$/..$/..
PRJNAME=filter
TARGET=icgm
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------
.IF "$(L10N_framework)"==""
SLOFILES =	$(SLO)$/cgm.obj		\
            $(SLO)$/chart.obj	\
            $(SLO)$/class0.obj	\
            $(SLO)$/class1.obj	\
            $(SLO)$/class2.obj	\
            $(SLO)$/class3.obj	\
            $(SLO)$/class4.obj	\
            $(SLO)$/class5.obj	\
            $(SLO)$/class7.obj	\
            $(SLO)$/classx.obj	\
            $(SLO)$/outact.obj	\
            $(SLO)$/actimpr.obj	\
            $(SLO)$/bundles.obj	\
            $(SLO)$/bitmap.obj	\
            $(SLO)$/elements.obj
#			$(SLO)$/svdem.obj

SHL1TARGET	=	icg$(DLLPOSTFIX)
SHL1IMPLIB	=	icgm
SHL1VERSIONMAP=exports.map
SHL1DEF		=	$(MISC)$/$(SHL1TARGET).def
SHL1LIBS	=	$(SLB)$/icgm.lib

DEF1NAME=$(SHL1TARGET)

.IF "$(GUI)"=="OS2"
SHL1OBJS	=	$(SLO)$/class0.obj
.ENDIF

SHL1STDLIBS = \
            $(TKLIB)		\
            $(VCLLIB)		\
            $(UNOTOOLSLIB)	\
            $(TOOLSLIB)		\
            $(CPPULIB)		\
            $(SALLIB)
.ENDIF

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

