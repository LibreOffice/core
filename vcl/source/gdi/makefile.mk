#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 17:05:38 $
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

PRJNAME=vcl
TARGET=gdi

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Settings -----------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

.IF "$(COM)"=="ICC"
CDEFS+=-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES=	$(SLO)$/salmisc.obj 	\
            $(SLO)$/animate.obj 	\
            $(SLO)$/impanmvw.obj	\
            $(SLO)$/bitmap.obj		\
            $(SLO)$/bitmap2.obj 	\
            $(SLO)$/bitmap3.obj 	\
            $(SLO)$/bitmap4.obj 	\
            $(SLO)$/alpha.obj		\
            $(SLO)$/bitmapex.obj	\
            $(SLO)$/imgcons.obj 	\
            $(SLO)$/bmpacc.obj		\
            $(SLO)$/bmpacc2.obj 	\
            $(SLO)$/bmpacc3.obj 	\
            $(SLO)$/color.obj		\
            $(SLO)$/cvtsvm.obj		\
            $(SLO)$/cvtgrf.obj		\
            $(SLO)$/font.obj		\
            $(SLO)$/gdimtf.obj		\
            $(SLO)$/gfxlink.obj 	\
            $(SLO)$/gradient.obj	\
            $(SLO)$/hatch.obj		\
            $(SLO)$/graph.obj		\
            $(SLO)$/image.obj		\
            $(SLO)$/impbmp.obj		\
            $(SLO)$/impgraph.obj	\
            $(SLO)$/impimage.obj	\
            $(SLO)$/impprn.obj		\
            $(SLO)$/impvect.obj 	\
            $(SLO)$/implncvt.obj	\
            $(SLO)$/jobset.obj		\
            $(SLO)$/line.obj		\
            $(SLO)$/lineinfo.obj	\
            $(SLO)$/mapmod.obj		\
            $(SLO)$/metaact.obj 	\
            $(SLO)$/metric.obj		\
            $(SLO)$/octree.obj		\
            $(SLO)$/outmap.obj		\
            $(SLO)$/outdev.obj		\
            $(SLO)$/outdev2.obj 	\
            $(SLO)$/outdev3.obj 	\
            $(SLO)$/outdev4.obj 	\
            $(SLO)$/outdev5.obj 	\
            $(SLO)$/outdev6.obj 	\
            $(SLO)$/poly.obj		\
            $(SLO)$/poly2.obj		\
            $(SLO)$/print.obj		\
            $(SLO)$/print2.obj		\
            $(SLO)$/regband.obj 	\
            $(SLO)$/region.obj		\
            $(SLO)$/virdev.obj		\
            $(SLO)$/wall.obj		\
            $(SLO)$/opengl.obj

.IF "$(remote)"!=""
EXCEPTIONSFILES=	$(SLO)$/bitmap.obj		\
                    $(SLO)$/color.obj		\
                    $(SLO)$/gfxlink.obj		\
                    $(SLO)$/impgraph.obj	\
                    $(SLO)$/impvect.obj 	\
                    $(SLO)$/outdev.obj		\
                    $(SLO)$/outdev3.obj 	\
                    $(SLO)$/outdev6.obj		\
                    $(SLO)$/print.obj		\
                    $(SLO)$/print2.obj		\
                    $(SLO)$/virdev.obj		

.ELSE
EXCEPTIONSFILES=	$(SLO)$/outdev.obj		\
                    $(SLO)$/gfxlink.obj		\
                    $(SLO)$/impgraph.obj	
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
