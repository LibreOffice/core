#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: hr $ $Date: 2007-09-27 09:15:34 $
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

PRJNAME=sw
TARGET=text

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

.IF "$(mydebug)" != ""
CDEFS+=-Dmydebug
.ENDIF

# --- Files --------------------------------------------------------

.IF "$(product)$(cap)" == ""
CXXFILES += \
        txtio.cxx
.ENDIF



SLOFILES =  \
                $(SLO)$/atrstck.obj \
                $(SLO)$/EnhancedPDFExportHelper.obj \
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

EXCEPTIONSFILES = \
                $(SLO)$/EnhancedPDFExportHelper.obj \
        $(SLO)$/itradj.obj \
        $(SLO)$/porlay.obj \
        $(SLO)$/pormulti.obj \
        $(SLO)$/txtfly.obj \
        $(SLO)$/wrong.obj


.IF "$(CPUNAME)" == "SPARC"
.IF "$(OS)" == "NETBSD"
NOOPTFILES = \
    $(SLO)$/txtftn.obj
.ENDIF
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

