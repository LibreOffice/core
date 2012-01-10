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



PRJ=				..$/..$/..
PRJINC=				$(PRJ)$/source
PRJNAME=			chart2
TARGET=				chcmenus

ENABLE_EXCEPTIONS=	TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# i26518 the gcc-3.0.4 requires to enhance the template-depth
# this seems to be a compiler issue, so we recommend not to use 3.0.x anymore
.IF "$(COM)"=="GCC" 
    CFLAGS+=-ftemplate-depth-128
.ENDIF

# --- Resources ---------------------------------------------------------------

SRC1FILES=		\
                ShapeContextMenu.src \
                ShapeEditContextMenu.src

SRS1NAME=$(TARGET)

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk

#dependencies:

$(SRS)$/chcmenus.srs: \
        ..$/inc$/MenuResIds.hrc

#$(SRS)$/$(TARGET).srs: $(SOLARINCDIR)$/svx$/globlmn.hrc
