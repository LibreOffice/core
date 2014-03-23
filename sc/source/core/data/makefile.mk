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
TARGET=data

PROJECTPCH4DLL=TRUE
PROJECTPCH=core_pch
PROJECTPCHSOURCE=..\pch\core_pch

AUTOSEG=true
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =  \
    $(SLO)$/attarray.obj \
    $(SLO)$/attrib.obj \
    $(SLO)$/autonamecache.obj \
    $(SLO)$/bcaslot.obj \
    $(SLO)$/cell.obj \
    $(SLO)$/cell2.obj \
        $(SLO)$/clipparam.obj \
    $(SLO)$/column.obj \
    $(SLO)$/column2.obj \
    $(SLO)$/column3.obj \
    $(SLO)$/compressedarray.obj \
    $(SLO)$/conditio.obj \
    $(SLO)$/dbdocutl.obj \
    $(SLO)$/dociter.obj \
    $(SLO)$/docpool.obj \
    $(SLO)$/documen2.obj \
    $(SLO)$/documen3.obj \
    $(SLO)$/documen4.obj \
    $(SLO)$/documen5.obj \
    $(SLO)$/documen6.obj \
    $(SLO)$/documen7.obj \
    $(SLO)$/documen8.obj \
    $(SLO)$/documen9.obj \
    $(SLO)$/document.obj \
        $(SLO)$/dpcachetable.obj \
        $(SLO)$/dpdimsave.obj \
    $(SLO)$/dpglobal.obj \
        $(SLO)$/dpgroup.obj \
    $(SLO)$/dpobject.obj \
    $(SLO)$/dpoutput.obj \
    $(SLO)$/dpoutputgeometry.obj \
    $(SLO)$/dpsave.obj \
    $(SLO)$/dpsdbtab.obj \
    $(SLO)$/dpshttab.obj \
    $(SLO)$/dptabdat.obj \
    $(SLO)$/dptabres.obj \
    $(SLO)$/dptabsrc.obj \
    $(SLO)$/dptablecache.obj\
    $(SLO)$/scdpoutputimpl.obj\
    $(SLO)$/drawpage.obj \
    $(SLO)$/drwlayer.obj \
    $(SLO)$/fillinfo.obj \
    $(SLO)$/global.obj \
    $(SLO)$/global2.obj \
    $(SLO)$/globalx.obj \
    $(SLO)$/markarr.obj \
    $(SLO)$/markdata.obj \
    $(SLO)$/olinetab.obj \
    $(SLO)$/pagepar.obj \
    $(SLO)$/patattr.obj \
    $(SLO)$/pivot2.obj \
    $(SLO)$/poolhelp.obj \
    $(SLO)$/sheetevents.obj \
    $(SLO)$/segmenttree.obj \
    $(SLO)$/sortparam.obj \
    $(SLO)$/stlpool.obj \
    $(SLO)$/stlsheet.obj \
        $(SLO)$/tabbgcolor.obj \
    $(SLO)$/table1.obj \
    $(SLO)$/table2.obj \
    $(SLO)$/table3.obj \
    $(SLO)$/table4.obj \
    $(SLO)$/table5.obj \
    $(SLO)$/table6.obj \
    $(SLO)$/tabprotection.obj \
    $(SLO)$/userdat.obj \
    $(SLO)$/validat.obj \
    $(SLO)$/postit.obj

EXCEPTIONSFILES= \
    $(SLO)$/autonamecache.obj \
    $(SLO)$/bcaslot.obj \
    $(SLO)$/cell2.obj \
    $(SLO)$/clipparam.obj \
    $(SLO)$/column.obj \
    $(SLO)$/column3.obj \
    $(SLO)$/documen2.obj \
    $(SLO)$/document.obj \
    $(SLO)$/dpdimsave.obj \
    $(SLO)$/dpglobal.obj \
    $(SLO)$/dpgroup.obj \
    $(SLO)$/dpshttab.obj \
    $(SLO)$/dptabres.obj \
    $(SLO)$/dptabdat.obj \
    $(SLO)$/global2.obj \
    $(SLO)$/pivot2.obj \
    $(SLO)$/tabbgcolor.obj \
    $(SLO)$/table1.obj \
    $(SLO)$/table2.obj \
    $(SLO)$/table3.obj \
    $(SLO)$/tabprotection.obj \
    $(SLO)$/postit.obj \
    $(SLO)$/documen3.obj \
    $(SLO)$/documen5.obj \
    $(SLO)$/documen6.obj \
    $(SLO)$/documen8.obj \
    $(SLO)$/documen9.obj \
    $(SLO)$/dpcachetable.obj \
    $(SLO)$/dptablecache.obj \
    $(SLO)$/scdpoutputimpl.obj \
    $(SLO)$/dpsdbtab.obj \
    $(SLO)$/dpobject.obj \
    $(SLO)$/dpoutput.obj \
    $(SLO)$/dpoutputgeometry.obj \
    $(SLO)$/dpsave.obj \
    $(SLO)$/dbdocutl.obj \
    $(SLO)$/dptabsrc.obj \
    $(SLO)$/drwlayer.obj \
    $(SLO)$/globalx.obj \
    $(SLO)$/segmenttree.obj

.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCSPARC"
NOOPTFILES= \
        $(SLO)$/column2.obj \
        $(SLO)$/column3.obj \
        $(SLO)$/table3.obj \
        $(SLO)$/table4.obj  \
        $(SLO)$/documen4.obj \
        $(SLO)$/conditio.obj \
        $(SLO)$/validat.obj
EXCEPTIONSNOOPTFILES= \
        $(SLO)$/cell.obj
.ELSE
EXCEPTIONSFILES+= \
    $(SLO)$/cell.obj \
    $(SLO)$/global.obj \
    $(SLO)$/table5.obj
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

