#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: bm $ $Date: 2000-09-27 13:57:18 $
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

PRJNAME=xmloff
TARGET=style
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

CXXFILES = \
        xmlastpl.cxx	\
        xmlexpit.cxx	\
        xmlimpit.cxx	\
        xmlitem.cxx		\
        xmlitmpr.cxx	\
        xmlnumfe.cxx	\
        xmlnumfi.cxx	\
        xmlstyle.cxx	\
        xmltabi.cxx		\
        xmlnumi.cxx		\
        xmlnume.cxx		\
        xmltabe.cxx		\
        xmlaustp.cxx	\
        impastp1.cxx    \
        impastp2.cxx    \
        impastp3.cxx    \
        impastp4.cxx    \
        xmlprmap.cxx	\
        xmlexppr.cxx	\
        xmlimppr.cxx	\
        xmlprcon.cxx	\
        prhdlfac.cxx	\
        xmlprhdl.cxx	\
        xmlbahdl.cxx	\
        uniref.cxx		\
        shadwhdl.cxx	\
        splithdl.cxx	\
        shdwdhdl.cxx	\
        kernihdl.cxx	\
        opaquhdl.cxx	\
        adjushdl.cxx	\
        backhdl.cxx		\
        breakhdl.cxx	\
        cdouthdl.cxx	\
        csmaphdl.cxx	\
        fonthdl.cxx		\
        postuhdl.cxx	\
        tabsthdl.cxx	\
        undlihdl.cxx	\
        weighhdl.cxx	\
        prstylei.cxx	\
        styleexp.cxx	\
        escphdl.cxx		\
        chrhghdl.cxx	\
        chrlohdl.cxx	\
        lspachdl.cxx	\
        bordrhdl.cxx	\
        EnumPropertyHdl.cxx				\
        NamedBoolPropertyHdl.cxx		\
        bordrhdl.cxx	\
        numehelp.cxx	\
        FillStyleContext.cxx			\
        GradientStyle.cxx				\
        HatchStyle.cxx					\
        ImageStyle.cxx					\
        TransGradientStyle.cxx			\
        MarkerStyle.cxx					\
        DashStyle.cxx					\
        XMLElementPropertyContext.cxx	\
        XMLConstantsPropertyHandler.cxx	\
        XMLClipPropertyHandler.cxx


SLOFILES =	\
        $(SLO)$/xmlastpl.obj	\
        $(SLO)$/xmlexpit.obj	\
        $(SLO)$/xmlimpit.obj	\
        $(SLO)$/xmlitem.obj		\
        $(SLO)$/xmlitmpr.obj	\
        $(SLO)$/xmlnumfe.obj	\
        $(SLO)$/xmlnumfi.obj	\
        $(SLO)$/xmlstyle.obj	\
        $(SLO)$/xmltabi.obj		\
        $(SLO)$/xmlnumi.obj		\
        $(SLO)$/xmlnume.obj		\
        $(SLO)$/xmltabe.obj		\
        $(SLO)$/xmlaustp.obj	\
        $(SLO)$/impastp1.obj	\
        $(SLO)$/impastp2.obj	\
        $(SLO)$/impastp3.obj	\
        $(SLO)$/impastp4.obj	\
        $(SLO)$/xmlprmap.obj	\
        $(SLO)$/xmlexppr.obj	\
        $(SLO)$/xmlimppr.obj	\
        $(SLO)$/xmlprcon.obj	\
        $(SLO)$/prhdlfac.obj	\
        $(SLO)$/xmlprhdl.obj	\
        $(SLO)$/xmlbahdl.obj	\
        $(SLO)$/uniref.obj		\
        $(SLO)$/shadwhdl.obj	\
        $(SLO)$/splithdl.obj	\
        $(SLO)$/shdwdhdl.obj	\
        $(SLO)$/kernihdl.obj	\
        $(SLO)$/opaquhdl.obj	\
        $(SLO)$/adjushdl.obj	\
        $(SLO)$/backhdl.obj		\
        $(SLO)$/breakhdl.obj	\
        $(SLO)$/cdouthdl.obj	\
        $(SLO)$/csmaphdl.obj	\
        $(SLO)$/fonthdl.obj		\
        $(SLO)$/postuhdl.obj	\
        $(SLO)$/tabsthdl.obj	\
        $(SLO)$/undlihdl.obj	\
        $(SLO)$/weighhdl.obj	\
        $(SLO)$/prstylei.obj	\
        $(SLO)$/styleexp.obj	\
        $(SLO)$/escphdl.obj		\
        $(SLO)$/chrhghdl.obj	\
        $(SLO)$/chrlohdl.obj	\
        $(SLO)$/lspachdl.obj	\
        $(SLO)$/bordrhdl.obj	\
        $(SLO)$/EnumPropertyHdl.obj				\
        $(SLO)$/NamedBoolPropertyHdl.obj		\
        $(SLO)$/numehelp.obj	\
        $(SLO)$/FillStyleContext.obj			\
        $(SLO)$/GradientStyle.obj				\
        $(SLO)$/HatchStyle.obj					\
        $(SLO)$/ImageStyle.obj					\
        $(SLO)$/TransGradientStyle.obj			\
        $(SLO)$/MarkerStyle.obj					\
        $(SLO)$/DashStyle.obj					\
        $(SLO)$/XMLElementPropertyContext.obj	\
        $(SLO)$/XMLConstantsPropertyHandler.obj	\
        $(SLO)$/XMLClipPropertyHandler.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk
