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
TARGET=excel

AUTOSEG=true

PROJECTPCH4DLL=TRUE
PROJECTPCH=filt_pch
PROJECTPCHSOURCE=..\pch\filt_pch
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/colrowst.obj				\
        $(SLO)$/excdoc.obj					\
        $(SLO)$/excel.obj					\
        $(SLO)$/excform.obj					\
        $(SLO)$/excform8.obj				\
        $(SLO)$/excimp8.obj					\
        $(SLO)$/excrecds.obj				\
        $(SLO)$/exctools.obj				\
        $(SLO)$/expop2.obj					\
        $(SLO)$/fontbuff.obj				\
        $(SLO)$/frmbase.obj					\
        $(SLO)$/impop.obj					\
        $(SLO)$/namebuff.obj				\
        $(SLO)$/read.obj					\
        $(SLO)$/tokstack.obj				\
        $(SLO)$/xechart.obj					\
        $(SLO)$/xecontent.obj				\
        $(SLO)$/xeescher.obj				\
        $(SLO)$/xeformula.obj				\
        $(SLO)$/xehelper.obj				\
        $(SLO)$/xelink.obj					\
        $(SLO)$/xename.obj					\
        $(SLO)$/xepage.obj					\
        $(SLO)$/xepivot.obj					\
        $(SLO)$/xerecord.obj				\
        $(SLO)$/xeroot.obj					\
        $(SLO)$/xestream.obj				\
        $(SLO)$/xestring.obj				\
        $(SLO)$/xestyle.obj					\
        $(SLO)$/xetable.obj					\
        $(SLO)$/xeview.obj					\
        $(SLO)$/xichart.obj					\
        $(SLO)$/xicontent.obj				\
        $(SLO)$/xiescher.obj				\
        $(SLO)$/xiformula.obj				\
        $(SLO)$/xihelper.obj				\
        $(SLO)$/xilink.obj					\
        $(SLO)$/xiname.obj					\
        $(SLO)$/xipage.obj					\
        $(SLO)$/xipivot.obj					\
        $(SLO)$/xiroot.obj					\
        $(SLO)$/xistream.obj				\
        $(SLO)$/xistring.obj				\
        $(SLO)$/xistyle.obj					\
        $(SLO)$/xiview.obj					\
        $(SLO)$/xladdress.obj				\
        $(SLO)$/xlchart.obj					\
        $(SLO)$/xlescher.obj				\
        $(SLO)$/xlformula.obj				\
        $(SLO)$/xlpage.obj					\
        $(SLO)$/xlpivot.obj					\
        $(SLO)$/xlroot.obj					\
        $(SLO)$/xlstyle.obj					\
        $(SLO)$/xltools.obj					\
        $(SLO)$/xltracer.obj				\
        $(SLO)$/xlview.obj

.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCSPARC"
NOOPTFILES = \
        $(SLO)$/xiescher.obj
.ENDIF

EXCEPTIONSFILES = \
        $(SLO)$/colrowst.obj				\
        $(SLO)$/excdoc.obj					\
        $(SLO)$/excel.obj					\
        $(SLO)$/excform.obj					\
        $(SLO)$/excform8.obj				\
        $(SLO)$/excimp8.obj					\
        $(SLO)$/excrecds.obj				\
        $(SLO)$/expop2.obj					\
        $(SLO)$/impop.obj					\
        $(SLO)$/namebuff.obj				\
        $(SLO)$/tokstack.obj				\
        $(SLO)$/xecontent.obj				\
        $(SLO)$/xeescher.obj				\
        $(SLO)$/xeformula.obj				\
        $(SLO)$/xehelper.obj				\
        $(SLO)$/xelink.obj					\
        $(SLO)$/xename.obj					\
        $(SLO)$/xepage.obj					\
        $(SLO)$/xepivot.obj					\
        $(SLO)$/xechart.obj					\
        $(SLO)$/xestream.obj				\
        $(SLO)$/xestring.obj				\
        $(SLO)$/xestyle.obj					\
        $(SLO)$/xetable.obj					\
        $(SLO)$/xeview.obj					\
        $(SLO)$/xichart.obj					\
        $(SLO)$/xicontent.obj				\
        $(SLO)$/xiescher.obj				\
        $(SLO)$/xihelper.obj				\
        $(SLO)$/xilink.obj					\
        $(SLO)$/xipage.obj					\
        $(SLO)$/xipivot.obj					\
        $(SLO)$/xistream.obj				\
        $(SLO)$/xistring.obj				\
        $(SLO)$/xistyle.obj					\
        $(SLO)$/xladdress.obj				\
        $(SLO)$/xiescher.obj				\
        $(SLO)$/xlchart.obj					\
        $(SLO)$/xlformula.obj				\
        $(SLO)$/xlpivot.obj					\
        $(SLO)$/xlroot.obj					\
        $(SLO)$/xlstyle.obj					\
        $(SLO)$/xltools.obj					\
        $(SLO)$/xlview.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

