#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: kz $ $Date: 2005-01-21 16:56:21 $
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
TARGET=engine3d

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES= \
        $(SLO)$/obj3d.obj 		\
        $(SLO)$/pntobj3d.obj     \
        $(SLO)$/label3d.obj		\
        $(SLO)$/light3d.obj 		\
        $(SLO)$/polyob3d.obj		\
        $(SLO)$/scene3d.obj		\
        $(SLO)$/polysc3d.obj		\
        $(SLO)$/cube3d.obj		\
        $(SLO)$/sphere3d.obj		\
        $(SLO)$/extrud3d.obj     \
        $(SLO)$/lathe3d.obj		\
        $(SLO)$/polygn3d.obj 		\
        $(SLO)$/svx3ditems.obj	\
        $(SLO)$/deflt3d.obj		\
        $(SLO)$/class3d.obj		\
        $(SLO)$/e3dundo.obj      \
        $(SLO)$/volume3d.obj		\
        $(SLO)$/viewpt3d.obj		\
        $(SLO)$/camera3d.obj		\
        $(SLO)$/poly3d.obj 		\
        $(SLO)$/volmrk3d.obj 	\
        $(SLO)$/objfac3d.obj 	\
        $(SLO)$/dlight3d.obj 	\
        $(SLO)$/plight3d.obj 	\
        $(SLO)$/dragmt3d.obj		\
        $(SLO)$/view3d.obj		\
        $(SLO)$/view3d1.obj       \
        $(SLO)$/e3dcmpt.obj       \
        $(SLO)$/float3d.obj

.IF "$(COM)"=="ICC"
NOOPTFILES=\
        $(SLO)$/class3d.obj
.ENDIF

SRS1NAME=engine3d
SRC1FILES=\
        string3d.src	\
        float3d.src

.INCLUDE :  target.mk
