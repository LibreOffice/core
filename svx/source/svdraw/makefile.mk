#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: obo $ $Date: 2004-08-12 09:05:08 $
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

PRJNAME=svx
TARGET=svdraw

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES=\
         $(SLO)$/svdobj.obj          \
         $(SLO)$/svdoattr.obj        \
         $(SLO)$/svdotext.obj        \
         $(SLO)$/svdotxat.obj        \
         $(SLO)$/svdotxdr.obj        \
         $(SLO)$/svdotxed.obj        \
         $(SLO)$/svdotxfl.obj        \
         $(SLO)$/svdotxln.obj        \
         $(SLO)$/svdotxtr.obj        \
         $(SLO)$/svdorect.obj        \
         $(SLO)$/svdoedge.obj        \
         $(SLO)$/svdomeas.obj        \
         $(SLO)$/svdopath.obj        \
         $(SLO)$/svdocapt.obj        \
         $(SLO)$/svdocirc.obj        \
         $(SLO)$/svdograf.obj        \
         $(SLO)$/svdogrp.obj         \
         $(SLO)$/svdoole2.obj        \
         $(SLO)$/svdopage.obj        \
         $(SLO)$/svdouno.obj         \
         $(SLO)$/svdoutl.obj         \
         $(SLO)$/svdovirt.obj        \
         $(SLO)$/svdoutlinercache.obj \
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
         $(SLO)$/impgrfll.obj        \
         $(SLO)$/svdpage.obj         \
         $(SLO)$/sdrmasterpagedescriptor.obj         \
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
         $(SLO)$/svdxcgv.obj		 \
         $(SLO)$/clonelist.obj		 \
         $(SLO)$/svdoashp.obj		 \
         $(SLO)$/svdomedia.obj		 

EXCEPTIONSFILES= \
         $(SLO)$/svdhdl.obj          \
         $(SLO)$/svdmrkv.obj         \
         $(SLO)$/svdogrp.obj         \
         $(SLO)$/svdotxln.obj        \
         $(SLO)$/svdouno.obj         \
         $(SLO)$/svdfppt.obj         \
         $(SLO)$/svdpagv.obj		 \
         $(SLO)$/svdoashp.obj
         

SRS1NAME=svdstr
SRC1FILES= svdstr.src

.INCLUDE :  target.mk
