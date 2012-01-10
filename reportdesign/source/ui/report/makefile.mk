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
PRJINC=$(PRJ)$/source
PRJNAME=reportdesign
TARGET=report

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
# .INCLUDE : $(PRJ)$/util$/dll.pmk

IMGLST_SRS=$(SRS)$/$(TARGET).srs
# --- Files --------------------------------------------------------

SLOFILES =\
            $(SLO)$/SectionWindow.obj			\
            $(SLO)$/ReportController.obj		\
            $(SLO)$/ReportControllerObserver.obj \
            $(SLO)$/FormattedFieldBeautifier.obj \
            $(SLO)$/FixedTextColor.obj           \
            $(SLO)$/ReportSection.obj			\
            $(SLO)$/SectionView.obj				\
            $(SLO)$/ViewsWindow.obj				\
            $(SLO)$/ScrollHelper.obj			\
            $(SLO)$/StartMarker.obj				\
            $(SLO)$/EndMarker.obj				\
            $(SLO)$/dlgedfunc.obj				\
            $(SLO)$/dlgedfac.obj				\
            $(SLO)$/dlgedclip.obj				\
            $(SLO)$/ReportWindow.obj			\
            $(SLO)$/DesignView.obj				\
            $(SLO)$/propbrw.obj


SRS1NAME=$(TARGET)
SRC1FILES =  report.src

# --- Targets -------------------------------------------------------


.INCLUDE :  target.mk

$(SRS)$/$(TARGET).srs: $(SOLARINCDIR)$/svx$/globlmn.hrc
