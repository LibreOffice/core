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

PRJNAME=sc
TARGET=ui


# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES = \
    sc.src		 \
    pseudo.src	 \
    popup.src	 \
    toolbox.src	 \
    scstring.src \
    attrdlg.src  \
    sortdlg.src  \
    filter.src	 \
    namedlg.src  \
    dbnamdlg.src \
    subtdlg.src  \
    miscdlgs.src \
    autofmt.src  \
    solvrdlg.src \
    optsolver.src \
    solveroptions.src \
    tabopdlg.src \
    hdrcont.src  \
    globstr.src  \
    optdlg.src   \
    scerrors.src \
    textdlgs.src \
    scfuncs.src	 \
    crnrdlg.src	 \
    condfrmt.src \
    opredlin.src \
    simpref.src

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

$(SRS)$/ui.srs: $(SOLARINCDIR)$/svx$/globlmn.hrc

