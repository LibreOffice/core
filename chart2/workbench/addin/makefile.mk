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
PRJNAME=chartaddin

TARGET=chartsampleaddin

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
LIBTARGET=NO

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Types -------------------------------------

UNOUCROUT=$(OUT)$/inc$/$(PRJNAME)$/$(TARGET)
INCPRE+=$(UNOUCROUT)

# --- Types -------------------------------------

# comprehensive type info, so rdb needn't be installed
# CPPUMAKERFLAGS*=-C

# UNOTYPES=\
# 	com.sun.star.lang.XInitialization \
# 	com.sun.star.lang.XServiceName \
# 	com.sun.star.lang.XServiceInfo \
# 	com.sun.star.util.XRefreshable \
# 	com.sun.star.lang.XLocalizable  \
# 	com.sun.star.chart.XDiagram \
# 	com.sun.star.chart.XChartDocument \
# 	com.sun.star.chart.XAxisXSupplier \
# 	com.sun.star.chart.XAxisYSupplier \
# 	com.sun.star.chart.XStatisticDisplay \
# 	com.sun.star.lang.XMultiServiceFactory

# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/sampleaddin.obj

# --- Library -----------------------------------

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS= \
        $(CPPUHELPERLIB)	\
        $(CPPULIB)			\
        $(VOSLIB)			\
        $(SALLIB) 

SHL1DEPN=makefile.mk
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# --- Targets ----------------------------------

.INCLUDE : target.mk
