#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 17:01:28 $
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
PROJECTPCH=xout
PROJECTPCHSOURCE=xoutpch
TARGET=xout
AUTOSEG=true
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

HXX1TARGET	=	xoutx
HXX1EXT		=	hxx
HXX1FILES	=	$(INC)$/xoutx.hxx
HXX1EXCL	= 	-K:*compat.hxx

HXX2TARGET	=	xattr
HXX2EXT		=	hxx
HXX2FILES	=	$(INC)$/xattr.hxx
HXX2EXCL	= 	-K:*compat.hxx

HXX3TARGET	=	xpoly
HXX3EXT		=	hxx
HXX3FILES	=	$(INC)$/xpoly.hxx
HXX3EXCL	= 	-K:*compat.hxx

HXX4TARGET	=	xtable
HXX4EXT		=	hxx
HXX4FILES	=	$(INC)$/xtable.hxx
HXX4EXCL	= 	-K:*compat.hxx

HXX5TARGET	=	xoutbmp
HXX5EXT		=	hxx
HXX5FILES	=	$(INC)$/xoutbmp.hxx
HXX5EXCL	= 	-K:*compat.hxx

HXX6TARGET	=	xpool
HXX6EXT		=	hxx
HXX6FILES	=	$(INC)$/xpool.hxx
HXX6EXCL	= 	-K:*compat.hxx

.IF "$(header)" == ""

CXXFILES= \
        xcpoly.cxx		\
        xoutpch.cxx 	\
        xout.cxx		\
        xout1.cxx		\
        xattr.cxx		\
        xattr2.cxx		\
        xattrbmp.cxx	\
        xline.cxx		\
        xpool.cxx		\
        xtable.cxx		\
        xtabcolr.cxx	\
        xtablend.cxx	\
        xtabdash.cxx	\
        xtabhtch.cxx	\
        xtabgrdt.cxx	\
        xtabbtmp.cxx	\
        xexch.cxx       \
        xiocomp.cxx		\
        _xpoly.cxx		\
        _ximp.cxx		\
        _ximp2.cxx		\
        _xoutbmp.cxx	\
        _xfont.cxx

SLOFILES= \
        $(SLO)$/xcpoly.obj 		\
        $(SLO)$/xout.obj   		\
        $(SLO)$/xout1.obj  		\
        $(SLO)$/xattr.obj  		\
        $(SLO)$/xattr2.obj  	\
        $(SLO)$/xattrbmp.obj	\
        $(SLO)$/xline.obj   	\
        $(SLO)$/xpool.obj  		\
        $(SLO)$/xtable.obj 		\
        $(SLO)$/xtabcolr.obj	\
        $(SLO)$/xtablend.obj	\
        $(SLO)$/xtabdash.obj	\
        $(SLO)$/xtabhtch.obj	\
        $(SLO)$/xtabgrdt.obj	\
        $(SLO)$/xtabbtmp.obj	\
        $(SLO)$/xexch.obj		\
        $(SLO)$/xiocomp.obj		\
        $(SLO)$/_xpoly.obj  	\
        $(SLO)$/_ximp.obj   	\
        $(SLO)$/_xoutbmp.obj	\
        $(SLO)$/_xfont.obj

SVXLIGHTOBJFILES= \
        $(OBJ)$/xcpoly.obj 		\
        $(OBJ)$/xout.obj   		\
        $(OBJ)$/xout1.obj  		\
        $(OBJ)$/xattr.obj  		\
        $(OBJ)$/xattr2.obj  	\
        $(OBJ)$/xattrbmp.obj	\
        $(OBJ)$/xline.obj   	\
        $(OBJ)$/xpool.obj  		\
        $(OBJ)$/xtable.obj 		\
        $(OBJ)$/xtabcolr.obj	\
        $(OBJ)$/xtablend.obj	\
        $(OBJ)$/xtabdash.obj	\
        $(OBJ)$/xtabhtch.obj	\
        $(OBJ)$/xtabgrdt.obj	\
        $(OBJ)$/xtabbtmp.obj	\
        $(OBJ)$/xexch.obj		\
        $(OBJ)$/xiocomp.obj		\
        $(OBJ)$/_xpoly.obj  	\
        $(OBJ)$/_ximp.obj   	\
        $(OBJ)$/_ximp2.obj  	\
        $(OBJ)$/_xoutbmp.obj	\
        $(OBJ)$/_xfont.obj

.ENDIF

HXXCOPYFILES=\
     $(PRJ)$/inc$/xenum.hxx


.INCLUDE :  target.mk
