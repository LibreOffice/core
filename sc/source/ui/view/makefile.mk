#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************
PRJ=..$/..$/..

PRJNAME=sc
TARGET=view

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------
# drawattr.cxx fuer IDL (enums), sollte in den Svx gehen??


SLOFILES =  \
        $(EXCEPTIONSFILES) \
        $(SLO)$/cellmergeoption.obj

EXCEPTIONSFILES= \
        $(SLO)$/auditsh.obj \
        $(SLO)$/cellsh.obj \
        $(SLO)$/cellsh1.obj \
        $(SLO)$/cellsh2.obj \
        $(SLO)$/cellsh3.obj \
        $(SLO)$/cellsh4.obj \
        $(SLO)$/colrowba.obj \
        $(SLO)$/dbfunc.obj \
        $(SLO)$/dbfunc2.obj \
        $(SLO)$/dbfunc3.obj \
        $(SLO)$/dbfunc4.obj \
        $(SLO)$/drawutil.obj \
        $(SLO)$/drawvie2.obj \
        $(SLO)$/drawvie3.obj \
        $(SLO)$/drawvie4.obj \
        $(SLO)$/drawview.obj \
        $(SLO)$/editsh.obj \
        $(SLO)$/formatsh.obj \
        $(SLO)$/galwrap.obj \
        $(SLO)$/gridmerg.obj \
        $(SLO)$/gridwin.obj \
        $(SLO)$/gridwin2.obj \
        $(SLO)$/gridwin3.obj \
        $(SLO)$/gridwin4.obj \
        $(SLO)$/gridwin5.obj \
        $(SLO)$/hdrcont.obj \
        $(SLO)$/hintwin.obj \
        $(SLO)$/imapwrap.obj \
        $(SLO)$/invmerge.obj \
        $(SLO)$/notemark.obj \
        $(SLO)$/olinewin.obj \
        $(SLO)$/olkact.obj \
        $(SLO)$/output.obj \
        $(SLO)$/output2.obj \
        $(SLO)$/output3.obj \
        $(SLO)$/overlayobject.obj \
        $(SLO)$/pfuncache.obj \
        $(SLO)$/pgbrksh.obj \
        $(SLO)$/pivotsh.obj \
        $(SLO)$/preview.obj \
        $(SLO)$/prevloc.obj \
        $(SLO)$/prevwsh.obj \
        $(SLO)$/prevwsh2.obj \
        $(SLO)$/printfun.obj \
        $(SLO)$/reffact.obj \
        $(SLO)$/scextopt.obj \
        $(SLO)$/select.obj \
        $(SLO)$/selectionstate.obj \
        $(SLO)$/spelldialog.obj \
        $(SLO)$/spelleng.obj \
        $(SLO)$/tabcont.obj \
        $(SLO)$/tabsplit.obj \
        $(SLO)$/tabview.obj \
        $(SLO)$/tabview2.obj \
        $(SLO)$/tabview3.obj \
        $(SLO)$/tabview4.obj \
        $(SLO)$/tabview5.obj \
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
        $(SLO)$/viewdata.obj \
        $(SLO)$/viewfun2.obj \
        $(SLO)$/viewfun3.obj \
        $(SLO)$/viewfun4.obj \
        $(SLO)$/viewfun5.obj \
        $(SLO)$/viewfun6.obj \
        $(SLO)$/viewfun7.obj \
        $(SLO)$/viewfunc.obj \
        $(SLO)$/viewutil.obj \
        $(SLO)$/waitoff.obj

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

# goal seek -O2


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

