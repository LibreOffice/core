#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: ama $ $Date: 2000-10-16 13:19:16 $
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

PRJNAME=sw
TARGET=text

AUTOSEG=true

PROJECTPCH=core_pch
PDBTARGET=core_pch
PROJECTPCHSOURCE=..$/core_1st$/core_pch

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

INCEXT=s:\solar\inc\hm

.IF "$(mydebug)" != ""
CDEFS+=-Dmydebug
.ENDIF

.IF "$(GUI)$(COM)" == "WINMSC"
LIBFLAGS=/NOI /NOE /PAGE:512
.ENDIF

# --- Files --------------------------------------------------------

CXXFILES = \
        frmcrsr.cxx \
        frmform.cxx \
        frminf.cxx \
        frmpaint.cxx \
        guess.cxx \
        inftxt.cxx \
        itradj.cxx \
        itratr.cxx \
        itrcrsr.cxx \
        itrform2.cxx \
        itrpaint.cxx \
        itrtxt.cxx \
        porexp.cxx \
        porfld.cxx \
        porfly.cxx \
        porglue.cxx \
        porlay.cxx \
        porlin.cxx \
        pormulti.cxx \
        porref.cxx \
        porrst.cxx \
        portox.cxx \
        portxt.cxx \
        redlnitr.cxx \
        txtcache.cxx \
        txtdrop.cxx \
        txtfld.cxx \
        txtfly.cxx \
        txtfrm.cxx \
        txtftn.cxx \
        txthyph.cxx \
        txtinit.cxx \
        txtpaint.cxx \
        txttab.cxx \
        widorp.cxx \
        blink.cxx \
        noteurl.cxx \
        wrong.cxx

.IF "$(product)$(cap)" == ""
CXXFILES += \
        txtio.cxx
.ENDIF



SLOFILES =  \
        $(SLO)$/frmcrsr.obj \
        $(SLO)$/frmform.obj \
        $(SLO)$/frminf.obj \
        $(SLO)$/frmpaint.obj \
        $(SLO)$/guess.obj \
        $(SLO)$/inftxt.obj \
        $(SLO)$/itradj.obj \
        $(SLO)$/itratr.obj \
        $(SLO)$/itrcrsr.obj \
        $(SLO)$/itrform2.obj \
        $(SLO)$/itrpaint.obj \
        $(SLO)$/itrtxt.obj \
        $(SLO)$/porexp.obj \
        $(SLO)$/porfld.obj \
        $(SLO)$/porfly.obj \
        $(SLO)$/porglue.obj \
        $(SLO)$/porlay.obj \
        $(SLO)$/porlin.obj \
        $(SLO)$/pormulti.obj \
        $(SLO)$/porref.obj \
        $(SLO)$/porrst.obj \
        $(SLO)$/portox.obj \
        $(SLO)$/portxt.obj \
        $(SLO)$/redlnitr.obj \
        $(SLO)$/txtcache.obj \
        $(SLO)$/txtdrop.obj \
        $(SLO)$/txtfld.obj \
        $(SLO)$/txtfly.obj \
        $(SLO)$/txtfrm.obj \
        $(SLO)$/txtftn.obj \
        $(SLO)$/txthyph.obj \
        $(SLO)$/txtinit.obj \
        $(SLO)$/txtpaint.obj \
        $(SLO)$/txttab.obj \
        $(SLO)$/widorp.obj \
        $(SLO)$/blink.obj \
        $(SLO)$/noteurl.obj \
        $(SLO)$/wrong.obj

.IF "$(product)$(cap)" == ""
SLOFILES +=  \
        $(SLO)$/txtio.obj
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

