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



PRJ=..$/

PRJNAME=basebmp
TARGET=wrapper
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk


# TODO(F3) unify BASEBMP_NO_NESTED_TEMPLATE_PARAMETER and
# VIGRA_WITHOUT_NESTED_TEMPLATES_PARAMS
#
# - VIGRA_WITHOUT_NESTED_TEMPLATES_PARAMS avoids template parameters
#   to templates - the template keyword is broken for SunCC
# - VIGRA_HAS_LONG_LONG enables long long overloads (but appears to not work
#   reliably in some environments, and is thus not enabled)
# - VIGRA_HAS_LONG_DOUBLE enables long double overloads 

.IF "$(OS)"!="WNT"
CDEFS+= -DVIGRA_HAS_LONG_DOUBLE
.ENDIF

.IF "$(OS)"=="SOLARIS" && "$(COM)"!="GCC"
CDEFS+= -DBASEBMP_NO_NESTED_TEMPLATE_PARAMETER -DVIGRA_WITHOUT_NESTED_TEMPLATE_PARAMS
.ENDIF

# SunStudio 12 (-m64 and -m32 modes): three test cases of the unit tests fail 
# if compiled with default -xalias_level (and optimization level -xO3)
.IF "$(OS)"=="SOLARIS"
# For Sun Studio 8 this switch does not work: compilation fails on bitmapdevice.cxx
.IF "$(CCNUMVER)"!="00050005"
CDEFS+=-xalias_level=compatible
.ENDIF
.ENDIF

# --- Common ----------------------------------------------------------

SLOFILES =	\
    $(SLO)$/bitmapdevice.obj \
    $(SLO)$/debug.obj \
    $(SLO)$/polypolygonrenderer.obj

#	$(SLO)$/genericintegerimageaccessor.obj \

# ==========================================================================

.INCLUDE :	target.mk
