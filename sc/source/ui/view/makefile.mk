#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: kz $ $Date: 2008-03-06 15:25:42 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************
PRJ=..$/..$/..

PRJNAME=sc
TARGET=view

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------
# drawattr.cxx fuer IDL (enums), sollte in den Svx gehen??


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
        $(SLO)$/galwrap.obj \
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
        $(SLO)$/scextopt.obj \
    $(SLO)$/tabvwshb.obj \
    $(SLO)$/viewdata.obj \
    $(SLO)$/viewfun5.obj \
    $(SLO)$/viewfun7.obj

# goal seek -O2


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

