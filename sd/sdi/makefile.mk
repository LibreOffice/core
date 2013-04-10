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

PRJNAME=sd
TARGET=sdslots
TARGET2=sdgslots
SDI1EXPORT=sdraw
SDI2EXPORT=sdraw
SVSDIINC=$(PRJ)$/source$/ui$/inc

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.IF "$(L10N_framework)"==""

# --- Files --------------------------------------------------------


SDI1NAME=$(TARGET)
SDI2NAME=$(TARGET2)
SIDHRCNAME=SDSLOTS.HRC

SVSDI1DEPEND= \
        $(SOLARINCXDIR)$/sfx2/sfx.sdi \
        $(SOLARINCXDIR)$/sfx2/sfxitems.sdi \
        $(SOLARINCXDIR)$/svx/svx.sdi \
        $(SOLARINCXDIR)$/svx/svxitems.sdi \
        $(SOLARINCXDIR)$/svx/xoitems.sdi \
    sdraw.sdi \
    app.sdi \
    _docsh.sdi \
    docshell.sdi \
    drtxtob.sdi \
        drbezob.sdi \
        drgrfob.sdi \
    _drvwsh.sdi \
    drviewsh.sdi \
        mediaob.sdi \
        tables.sdi\
    outlnvsh.sdi \
    SlideSorterController.sdi \
    ViewShellBase.sdi \
    sdslots.hrc \
    $(PRJ)$/inc$/app.hrc

SVSDI2DEPEND= \
        $(SOLARINCXDIR)$/sfx2/sfx.sdi \
        $(SOLARINCXDIR)$/sfx2/sfxitems.sdi \
        $(SOLARINCXDIR)$/svx/svx.sdi \
        $(SOLARINCXDIR)$/svx/svxitems.sdi \
        $(SOLARINCXDIR)$/svx/xoitems.sdi \
        sdraw.sdi \
    app.sdi \
    _docsh.sdi \
    grdocsh.sdi \
    drtxtob.sdi \
        drbezob.sdi \
        drgrfob.sdi \
    _drvwsh.sdi \
    grviewsh.sdi \
        mediaob.sdi \
    outlnvsh.sdi \
    ViewShellBase.sdi \
    sdslots.hrc \
    $(PRJ)$/inc$/app.hrc

# --- Targets -------------------------------------------------------
.ENDIF
.INCLUDE :  target.mk
