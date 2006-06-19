#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: hr $ $Date: 2006-06-19 21:40:35 $
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

PRJNAME=goodies
TARGET=base3d
AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------
EXCEPTIONSFILES=\
                $(SLO)$/b3dgeom.obj

SLOFILES= \
                $(SLO)$/base3d.obj       \
                $(SLO)$/b3dtrans.obj     \
                $(SLO)$/b3ddeflt.obj     \
                $(SLO)$/b3dopngl.obj     \
                $(SLO)$/b3dprint.obj     \
                $(SLO)$/b3dcommn.obj     \
                $(SLO)$/vector3d.obj     \
                $(SLO)$/point4d.obj		\
                $(SLO)$/matril3d.obj		\
                $(SLO)$/b3dcolor.obj		\
                $(SLO)$/b3dlight.obj		\
                $(SLO)$/b3dentty.obj		\
                $(SLO)$/b3dcompo.obj		\
                $(SLO)$/hmatrix.obj		\
                $(SLO)$/b3dtex.obj		\
                $(SLO)$/b3dgeom.obj		\
                $(SLO)$/b3dvolum.obj


.INCLUDE :  target.mk

