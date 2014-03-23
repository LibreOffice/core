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
TARGET=view
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =  \
        $(SLO)$/tabview.obj \
        $(SLO)$/tabview2.obj \
        $(SLO)$/tabview3.obj \
        $(SLO)$/tabview4.obj \
        $(SLO)$/tabview5.obj \
        $(SLO)$/viewfunc.obj \
        $(SLO)$/viewfun2.obj \
        $(SLO)$/viewfun3.obj \
        $(SLO)$/viewfun4.obj \
        $(SLO)$/viewfun5.obj \
        $(SLO)$/viewfun6.obj \
        $(SLO)$/viewfun7.obj \
        $(SLO)$/dbfunc.obj \
        $(SLO)$/dbfunc2.obj \
        $(SLO)$/dbfunc3.obj \
        $(SLO)$/dbfunc4.obj \
        $(SLO)$/tabvwsh.obj \
        $(SLO)$/tabvwsh2.obj \
        $(SLO)$/tabvwsh3.obj \
        $(SLO)$/tabvwsh4.obj \
        $(SLO)$/tabvwsh5.obj \
        $(SLO)$/tabvwsh8.obj \
        $(SLO)$/tabvwsh9.obj \
        $(SLO)$/tabvwsha.obj \
        $(SLO)$/tabvwshb.obj \
        $(SLO)$/tabvwshc.obj \
        $(SLO)$/tabvwshd.obj \
        $(SLO)$/tabvwshe.obj \
        $(SLO)$/tabvwshf.obj \
        $(SLO)$/tabvwshg.obj \
        $(SLO)$/tabvwshh.obj \
        $(SLO)$/printfun.obj \
        $(SLO)$/pfuncache.obj \
        $(SLO)$/preview.obj \
        $(SLO)$/prevwsh.obj \
        $(SLO)$/prevwsh2.obj \
        $(SLO)$/prevloc.obj \
        $(SLO)$/editsh.obj \
        $(SLO)$/pivotsh.obj \
        $(SLO)$/auditsh.obj \
        $(SLO)$/gridwin.obj \
        $(SLO)$/gridwin2.obj \
        $(SLO)$/gridwin3.obj \
        $(SLO)$/gridwin4.obj \
        $(SLO)$/gridwin5.obj \
        $(SLO)$/drawview.obj \
        $(SLO)$/drawvie2.obj \
        $(SLO)$/drawvie3.obj \
        $(SLO)$/drawvie4.obj \
        $(SLO)$/drawutil.obj \
        $(SLO)$/output.obj \
        $(SLO)$/output2.obj \
        $(SLO)$/output3.obj \
        $(SLO)$/gridmerg.obj \
        $(SLO)$/invmerge.obj \
        $(SLO)$/select.obj \
        $(SLO)$/olinewin.obj \
        $(SLO)$/hintwin.obj \
        $(SLO)$/notemark.obj \
        $(SLO)$/tabcont.obj \
        $(SLO)$/tabsplit.obj \
        $(SLO)$/viewutil.obj \
        $(SLO)$/hdrcont.obj \
        $(SLO)$/colrowba.obj \
        $(SLO)$/olkact.obj \
        $(SLO)$/imapwrap.obj \
        $(SLO)$/reffact.obj \
        $(SLO)$/selectionstate.obj \
        $(SLO)$/spelleng.obj \
        $(SLO)$/spelldialog.obj \
        $(SLO)$/waitoff.obj \
        $(SLO)$/cellsh.obj \
        $(SLO)$/cellsh1.obj\
        $(SLO)$/cellsh2.obj\
        $(SLO)$/cellsh3.obj\
        $(SLO)$/cellsh4.obj\
        $(SLO)$/formatsh.obj\
        $(SLO)$/pgbrksh.obj\
        $(SLO)$/viewdata.obj\
        $(SLO)$/scextopt.obj

.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCSPARC"
    NOOPTFILES= \
        $(SLO)$/drawview.obj	\
        $(SLO)$/dbfunc2.obj     \
        $(SLO)$/tabvwsh2.obj \
        $(SLO)$/viewfun4.obj \
        $(SLO)$/viewfun2.obj
.ELIF "$(OS)$(COM)$(CPUNAME)"=="SOLARISC52INTEL"
    NOOPTFILES=\
        $(SLO)$/drawview.obj    \
        $(SLO)$/dbfunc2.obj     \
        $(SLO)$/tabvwsh2.obj
.ELSE
    NOOPTFILES=\
        $(SLO)$/drawview.obj	\
        $(SLO)$/dbfunc2.obj     \
        $(SLO)$/tabvwsh2.obj
.ENDIF

EXCEPTIONSFILES= \
    $(SLO)$/dbfunc3.obj \
    $(SLO)$/gridwin.obj \
    $(SLO)$/invmerge.obj \
    $(SLO)$/output2.obj \
    $(SLO)$/pfuncache.obj \
    $(SLO)$/spelldialog.obj \
    $(SLO)$/cellsh1.obj \
    $(SLO)$/drawvie4.obj \
    $(SLO)$/formatsh.obj \
    $(SLO)$/gridwin2.obj \
    $(SLO)$/scextopt.obj \
    $(SLO)$/tabvwshb.obj \
    $(SLO)$/tabvwshf.obj \
    $(SLO)$/viewdata.obj \
    $(SLO)$/viewfunc.obj \
    $(SLO)$/viewfun2.obj \
    $(SLO)$/viewfun3.obj \
    $(SLO)$/viewfun5.obj \
    $(SLO)$/viewfun7.obj \
    $(SLO)$/reffact.obj

# goal seek -O2


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

