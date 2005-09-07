#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 17:56:32 $
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

PRJ=..$/..

PRJNAME=sfx2
TARGET=bastyp
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =\
        $(SLO)$/plugwin.obj				\
        $(SLO)$/propset.obj				\
        $(SLO)$/itmfac.obj				\
        $(SLO)$/sfxhtml.obj				\
        $(SLO)$/frmhtml.obj				\
        $(SLO)$/frmhtmlw.obj				\
        $(SLO)$/misc.obj				\
        $(SLO)$/idpool.obj				\
        $(SLO)$/progress.obj			\
        $(SLO)$/sfxresid.obj			\
        $(SLO)$/newhdl.obj				\
        $(SLO)$/bitset.obj				\
        $(SLO)$/minarray.obj			\
        $(SLO)$/fltfnc.obj				\
        $(SLO)$/mieclip.obj				\
        $(SLO)$/fltdlg.obj				\
        $(SLO)$/fltlst.obj				\
        $(SLO)$/helper.obj

SRS1NAME=$(TARGET)
SRC1FILES =\
        fltdlg.src	\
        fltfnc.src	\
        bastyp.src	\
        newhdl.src

EXCEPTIONSFILES =\
    $(SLO)$/propset.obj		\
    $(SLO)$/helper.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

