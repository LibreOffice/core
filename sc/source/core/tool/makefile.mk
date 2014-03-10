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
TARGET=tool

PROJECTPCH4DLL=TRUE
PROJECTPCH=core_pch
PROJECTPCHSOURCE=..\pch\core_pch
ENABLE_EXCEPTIONS=TRUE
AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =  \
        $(SLO)$/addincfg.obj \
        $(SLO)$/addincol.obj \
        $(SLO)$/addinhelpid.obj \
        $(SLO)$/addinlis.obj \
        $(SLO)$/address.obj \
        $(SLO)$/adiasync.obj \
        $(SLO)$/appoptio.obj \
        $(SLO)$/autoform.obj \
        $(SLO)$/callform.obj \
        $(SLO)$/cellform.obj \
        $(SLO)$/cellkeytranslator.obj \
        $(SLO)$/charthelper.obj \
        $(SLO)$/chartarr.obj \
        $(SLO)$/chartpos.obj \
        $(SLO)$/chartlis.obj \
        $(SLO)$/chartlock.obj \
        $(SLO)$/chgtrack.obj \
        $(SLO)$/chgviset.obj \
        $(SLO)$/collect.obj  \
        $(SLO)$/compiler.obj \
        $(SLO)$/consoli.obj  \
        $(SLO)$/dbcolect.obj \
        $(SLO)$/ddelink.obj \
        $(SLO)$/detdata.obj  \
        $(SLO)$/detfunc.obj  \
        $(SLO)$/docoptio.obj \
        $(SLO)$/doubleref.obj \
        $(SLO)$/editutil.obj \
        $(SLO)$/filtopt.obj \
        $(SLO)$/formulaparserpool.obj \
        $(SLO)$/hints.obj \
        $(SLO)$/inputopt.obj \
        $(SLO)$/interpr1.obj \
        $(SLO)$/interpr2.obj \
        $(SLO)$/interpr3.obj \
        $(SLO)$/interpr4.obj \
        $(SLO)$/interpr5.obj \
        $(SLO)$/interpr6.obj \
        $(SLO)$/lookupcache.obj \
        $(SLO)$/navicfg.obj \
        $(SLO)$/odffmap.obj \
        $(SLO)$/optutil.obj \
        $(SLO)$/parclass.obj \
        $(SLO)$/printopt.obj \
        $(SLO)$/prnsave.obj \
        $(SLO)$/progress.obj \
        $(SLO)$/queryparam.obj \
        $(SLO)$/rangelst.obj \
        $(SLO)$/rangenam.obj \
        $(SLO)$/rangeseq.obj \
        $(SLO)$/rangeutl.obj \
        $(SLO)$/rechead.obj  \
        $(SLO)$/refdata.obj \
        $(SLO)$/reffind.obj \
        $(SLO)$/refreshtimer.obj \
        $(SLO)$/reftokenhelper.obj \
        $(SLO)$/refupdat.obj \
        $(SLO)$/scmatrix.obj \
        $(SLO)$/stringutil.obj \
        $(SLO)$/subtotal.obj \
        $(SLO)$/token.obj \
        $(SLO)$/unitconv.obj \
        $(SLO)$/userlist.obj \
        $(SLO)$/viewopti.obj \
        $(SLO)$/zforauto.obj

EXCEPTIONSFILES= \
        $(SLO)$/addincol.obj \
        $(SLO)$/cellkeytranslator.obj \
        $(SLO)$/charthelper.obj \
        $(SLO)$/chartarr.obj \
        $(SLO)$/chartlis.obj \
        $(SLO)$/chartlock.obj \
        $(SLO)$/chgtrack.obj \
        $(SLO)$/compiler.obj \
        $(SLO)$/doubleref.obj \
        $(SLO)$/formulaparserpool.obj \
        $(SLO)$/interpr1.obj \
        $(SLO)$/interpr2.obj \
        $(SLO)$/interpr3.obj \
        $(SLO)$/interpr4.obj \
        $(SLO)$/interpr5.obj \
        $(SLO)$/lookupcache.obj \
        $(SLO)$/prnsave.obj \
        $(SLO)$/queryparam.obj \
        $(SLO)$/reftokenhelper.obj \
        $(SLO)$/stringutil.obj \
        $(SLO)$/token.obj

# [kh] POWERPC compiler problem
.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCPOWERPC"
NOOPTFILES= \
                $(SLO)$/subtotal.obj
.ENDIF

.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCSPARC"
NOOPTFILES= \
        $(SLO)$/interpr2.obj \
        $(SLO)$/interpr4.obj \
        $(SLO)$/token.obj    \
        $(SLO)$/chartarr.obj
.ENDIF

.IF "$(GUI)"=="OS2"
NOOPTFILES= \
        $(SLO)$/interpr6.obj
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

# avoid quotung problems
$(INCCOM)$/osversiondef.hxx :
    @@-$(RM) $@
    @$(TYPE) $(mktmp #define SC_INFO_OSVERSION "$(OS)") > $@

$(SLO)$/interpr5.obj : $(INCCOM)$/osversiondef.hxx

