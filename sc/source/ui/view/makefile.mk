#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 16:45:09 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=sc
TARGET=view

PROJECTPCH4DLL=TRUE
PROJECTPCH=ui_pch
PDBTARGET=ui_pch
PROJECTPCHSOURCE=..\pch\ui_pch

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------
# drawattr.cxx fuer IDL (enums), sollte in den Svx gehen??

CXXFILES = \
        tabview.cxx \
        tabview2.cxx \
        tabview3.cxx \
        tabview4.cxx \
        tabview5.cxx \
        viewfunc.cxx \
        viewfun2.cxx \
        viewfun3.cxx \
        viewfun4.cxx \
        viewfun5.cxx \
        viewfun6.cxx \
        viewfun7.cxx \
        dbfunc.cxx \
        dbfunc2.cxx \
        dbfunc3.cxx \
        dbfunc4.cxx \
        tabvwsh.cxx \
        tabvwsh2.cxx \
        tabvwsh3.cxx \
        tabvwsh4.cxx \
        tabvwsh5.cxx \
        tabvwsh7.cxx \
        tabvwsh8.cxx \
        tabvwsh9.cxx \
        tabvwsha.cxx \
        tabvwshb.cxx \
        tabvwshc.cxx \
        tabvwshd.cxx \
        tabvwshe.cxx \
        tabvwshf.cxx \
        tabvwshg.cxx \
        tabvwshh.cxx \
        printfun.cxx \
        preview.cxx \
        prevwsh.cxx \
        prevwsh2.cxx \
        editsh.cxx \
        editsh2.cxx \
        pivotsh.cxx \
        auditsh.cxx \
        gridwin.cxx \
        gridwin2.cxx \
        gridwin3.cxx \
        gridwin4.cxx \
        gridwin5.cxx \
        drawview.cxx \
        drawvie2.cxx \
        drawvie3.cxx \
        drawvie4.cxx \
        drawutil.cxx \
        output.cxx \
        output2.cxx \
        output3.cxx \
        gridmerg.cxx \
        invmerge.cxx \
        select.cxx \
        olinewin.cxx \
        hintwin.cxx \
        notemark.cxx \
        tabcont.cxx \
        tabsplit.cxx \
        viewdata.cxx \
        viewutil.cxx \
        hdrcont.cxx \
        colrowba.cxx \
        olkact.cxx \
        galwrap.cxx \
        imapwrap.cxx \
        reffact.cxx \
        spelleng.cxx \
        waitoff.cxx \
        cellsh.cxx \
        cellsh1.cxx \
        cellsh2.cxx \
        cellsh3.cxx \
        cellsh4.cxx \
        formatsh.cxx \
        pgbrksh.cxx

#       drawattr.cxx                ??????


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
        $(SLO)$/tabvwsh7.obj \
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
        $(SLO)$/preview.obj \
        $(SLO)$/prevwsh.obj \
        $(SLO)$/prevwsh2.obj \
        $(SLO)$/editsh.obj \
        $(SLO)$/editsh2.obj \
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
        $(SLO)$/viewdata.obj \
        $(SLO)$/viewutil.obj \
        $(SLO)$/hdrcont.obj \
        $(SLO)$/colrowba.obj \
        $(SLO)$/olkact.obj \
        $(SLO)$/galwrap.obj \
        $(SLO)$/imapwrap.obj \
        $(SLO)$/reffact.obj \
        $(SLO)$/spelleng.obj \
        $(SLO)$/waitoff.obj \
        $(SLO)$/cellsh.obj \
        $(SLO)$/cellsh1.obj\
        $(SLO)$/cellsh2.obj\
        $(SLO)$/cellsh3.obj\
        $(SLO)$/cellsh4.obj\
        $(SLO)$/formatsh.obj\
        $(SLO)$/pgbrksh.obj\


NOOPTFILES=\
    $(SLO)$/drawview.obj	\
    $(SLO)$/dbfunc2.obj     \
    $(SLO)$/tabvwsh2.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

