#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 17:01:24 $
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

PRJ=..$/..

PRJNAME=SVX
TARGET=svdraw
AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES=\
         $(SLO)$/dialdll.obj         \
         $(SLO)$/gradtrns.obj         \
         $(SLO)$/svdattr.obj         \
         $(SLO)$/svdcrtmt.obj        \
         $(SLO)$/svdcrtv.obj         \
         $(SLO)$/svddrag.obj         \
         $(SLO)$/svddrgmt.obj        \
         $(SLO)$/svddrgv.obj         \
         $(SLO)$/svdedtv.obj         \
         $(SLO)$/svdedtv1.obj        \
         $(SLO)$/svdedtv2.obj        \
         $(SLO)$/svdedxv.obj         \
         $(SLO)$/svdetc.obj          \
         $(SLO)$/svdfmtf.obj		\
         $(SLO)$/svdfppt.obj         \
         $(SLO)$/svdglev.obj         \
         $(SLO)$/svdglue.obj         \
         $(SLO)$/svdhdl.obj          \
         $(SLO)$/svdhlpln.obj        \
         $(SLO)$/svdibrow.obj        \
         $(SLO)$/svdio.obj           \
         $(SLO)$/svditer.obj         \
         $(SLO)$/svdlayer.obj        \
         $(SLO)$/svdmark.obj         \
         $(SLO)$/svdmodel.obj        \
         $(SLO)$/svdmrkv.obj         \
         $(SLO)$/svdmrkv1.obj        \
         $(SLO)$/svdoattr.obj        \
         $(SLO)$/svdobj.obj          \
         $(SLO)$/svdocapt.obj        \
         $(SLO)$/svdocirc.obj        \
         $(SLO)$/svdoedge.obj        \
         $(SLO)$/svdograf.obj        \
         $(SLO)$/svdogrp.obj         \
         $(SLO)$/svdomeas.obj        \
         $(SLO)$/svdoole2.obj        \
         $(SLO)$/svdopage.obj        \
         $(SLO)$/svdopath.obj        \
         $(SLO)$/svdorect.obj        \
         $(SLO)$/svdotext.obj        \
         $(SLO)$/svdotxat.obj        \
         $(SLO)$/svdotxdr.obj        \
         $(SLO)$/svdotxed.obj        \
         $(SLO)$/svdotxfl.obj        \
         $(SLO)$/svdotxln.obj        \
         $(SLO)$/svdotxtr.obj        \
         $(SLO)$/svdouno.obj         \
         $(SLO)$/svdoutl.obj         \
         $(SLO)$/svdovirt.obj        \
         $(SLO)$/svdpage.obj         \
         $(SLO)$/svdpagv.obj         \
         $(SLO)$/svdpntv.obj         \
         $(SLO)$/svdpoev.obj         \
         $(SLO)$/svdscrol.obj        \
         $(SLO)$/svdsnpv.obj         \
         $(SLO)$/svdsuro.obj         \
         $(SLO)$/svdtouch.obj        \
         $(SLO)$/svdtrans.obj        \
         $(SLO)$/svdtxhdl.obj        \
         $(SLO)$/svdundo.obj         \
         $(SLO)$/svdview.obj         \
         $(SLO)$/svdviter.obj        \
         $(SLO)$/svdvmark.obj        \
         $(SLO)$/svdxcgv.obj

SVXLIGHTOBJFILES=\
         $(OBJ)$/dialdll.obj        \
         $(OBJ)$/gradtrns.obj        \
         $(OBJ)$/svdattr.obj         \
         $(OBJ)$/svdcrtmt.obj        \
         $(OBJ)$/svdcrtv.obj         \
         $(OBJ)$/svddrag.obj         \
         $(OBJ)$/svddrgmt.obj        \
         $(OBJ)$/svddrgv.obj         \
         $(OBJ)$/svdedtv.obj         \
         $(OBJ)$/svdedtv1.obj        \
         $(OBJ)$/svdedtv2.obj        \
         $(OBJ)$/svdedxv.obj         \
         $(OBJ)$/svdetc.obj          \
         $(OBJ)$/svdfmtf.obj		\
         $(OBJ)$/svdfppt.obj         \
         $(OBJ)$/svdglev.obj         \
         $(OBJ)$/svdglue.obj         \
         $(OBJ)$/svdhdl.obj          \
         $(OBJ)$/svdhlpln.obj        \
         $(OBJ)$/svdibrow.obj        \
         $(OBJ)$/svdio.obj           \
         $(OBJ)$/svditer.obj         \
         $(OBJ)$/svdlayer.obj        \
         $(OBJ)$/svdmark.obj         \
         $(OBJ)$/svdmodel.obj        \
         $(OBJ)$/svdmrkv.obj         \
         $(OBJ)$/svdmrkv1.obj        \
         $(OBJ)$/svdoattr.obj        \
         $(OBJ)$/svdobj.obj          \
         $(OBJ)$/svdocapt.obj        \
         $(OBJ)$/svdocirc.obj        \
         $(OBJ)$/svdoedge.obj        \
         $(OBJ)$/svdograf.obj        \
         $(OBJ)$/svdogrp.obj         \
         $(OBJ)$/svdomeas.obj        \
         $(OBJ)$/svdoole2.obj        \
         $(OBJ)$/svdopage.obj        \
         $(OBJ)$/svdopath.obj        \
         $(OBJ)$/svdorect.obj        \
         $(OBJ)$/svdotext.obj        \
         $(OBJ)$/svdotxat.obj        \
         $(OBJ)$/svdotxdr.obj        \
         $(OBJ)$/svdotxed.obj        \
         $(OBJ)$/svdotxfl.obj        \
         $(OBJ)$/svdotxln.obj        \
         $(OBJ)$/svdotxtr.obj        \
         $(OBJ)$/svdouno.obj         \
         $(OBJ)$/svdoutl.obj         \
         $(OBJ)$/svdovirt.obj        \
         $(OBJ)$/svdpage.obj         \
         $(OBJ)$/svdpagv.obj         \
         $(OBJ)$/svdpntv.obj         \
         $(OBJ)$/svdpoev.obj         \
         $(OBJ)$/svdscrol.obj        \
         $(OBJ)$/svdsnpv.obj         \
         $(OBJ)$/svdsuro.obj         \
         $(OBJ)$/svdtouch.obj        \
         $(OBJ)$/svdtrans.obj        \
         $(OBJ)$/svdtxhdl.obj        \
         $(OBJ)$/svdundo.obj         \
         $(OBJ)$/svdview.obj         \
         $(OBJ)$/svdviter.obj        \
         $(OBJ)$/svdvmark.obj        \
         $(OBJ)$/svdxcgv.obj

EXCEPTIONSFILES= \
         $(OBJ)$/sxl_svdogrp.obj         \
         $(OBJ)$/sxl_svdotxln.obj        \
         $(SLO)$/svdhdl.obj          \
         $(SLO)$/svdmrkv.obj         \
         $(SLO)$/svdogrp.obj         \
         $(SLO)$/svdotxln.obj        \
         $(SLO)$/svdouno.obj         \
         $(SLO)$/svdpagv.obj

SRS1NAME=svdstr
SRC1FILES=  \
            svdstr.src

.INCLUDE :  target.mk

