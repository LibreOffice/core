#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: mib $ $Date: 2000-11-13 08:42:13 $
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
        adjushdl.cxx	\
        backhdl.cxx		\
        bordrhdl.cxx	\
        breakhdl.cxx	\
        cdouthdl.cxx	\
        chrhghdl.cxx	\
        chrlohdl.cxx	\
        csmaphdl.cxx	\
        escphdl.cxx		\
        fonthdl.cxx		\
        impastp1.cxx    \
        impastp2.cxx    \
        impastp3.cxx    \
        impastp4.cxx    \
        kernihdl.cxx	\
        lspachdl.cxx	\
        numehelp.cxx	\
        opaquhdl.cxx	\
        postuhdl.cxx	\
        prhdlfac.cxx	\
        prstylei.cxx	\
        shadwhdl.cxx	\
        shdwdhdl.cxx	\
        styleexp.cxx	\
        splithdl.cxx	\
        tabsthdl.cxx	\
        undlihdl.cxx	\
        uniref.cxx		\
        weighhdl.cxx	\
        xmlastpl.cxx	\
        xmlaustp.cxx	\
        xmlbahdl.cxx	\
        xmlexpit.cxx	\
        xmlexppr.cxx	\
        xmlimpit.cxx	\
        xmlimppr.cxx	\
        xmlitem.cxx		\
        xmlitmpr.cxx	\
        xmlnume.cxx		\
        xmlnumfe.cxx	\
        xmlnumfi.cxx	\
        xmlnumi.cxx		\
        xmlprcon.cxx	\
        xmlprhdl.cxx	\
        xmlprmap.cxx	\
        xmlstyle.cxx	\
        xmltabe.cxx		\
        xmltabi.cxx		\
        DashStyle.cxx					\
        EnumPropertyHdl.cxx				\
        FillStyleContext.cxx			\
        GradientStyle.cxx				\
        HatchStyle.cxx					\
        ImageStyle.cxx					\
        MarkerStyle.cxx					\
        DashStyle.cxx					\
        AttributeContainerHandler.cxx	\
        NamedBoolPropertyHdl.cxx		\
        TransGradientStyle.cxx			\
        XMLBackgroundImageContext.cxx	\
        XMLBackgroundImageExport.cxx	\
        XMLClipPropertyHandler.cxx		\
        XMLConstantsPropertyHandler.cxx	\
        XMLElementPropertyContext.cxx	\
        XMLFontAutoStylePool.cxx		\
        XMLFontStylesContext.cxx		\
        XMLPageExport.cxx				\
        PageMasterPropHdl.cxx			\
        PageMasterPropHdlFactory.cxx	\
        PageMasterStyleMap.cxx			\
        PageMasterPropMapper.cxx		\
        PageMasterExportPropMapper.cxx	\
        PageMasterImportPropMapper.cxx	\
        PageMasterImportContext.cxx		\
        PagePropertySetContext.cxx		\
        PageHeaderFooterContext.cxx


SLOFILES =	\
        $(SLO)$/adjushdl.obj	\
        $(SLO)$/backhdl.obj		\
        $(SLO)$/bordrhdl.obj	\
        $(SLO)$/breakhdl.obj	\
        $(SLO)$/cdouthdl.obj	\
        $(SLO)$/chrhghdl.obj	\
        $(SLO)$/chrlohdl.obj	\
        $(SLO)$/csmaphdl.obj	\
        $(SLO)$/escphdl.obj		\
        $(SLO)$/fonthdl.obj		\
        $(SLO)$/impastp1.obj    \
        $(SLO)$/impastp2.obj    \
        $(SLO)$/impastp3.obj    \
        $(SLO)$/impastp4.obj    \
        $(SLO)$/kernihdl.obj	\
        $(SLO)$/lspachdl.obj	\
        $(SLO)$/numehelp.obj	\
        $(SLO)$/opaquhdl.obj	\
        $(SLO)$/postuhdl.obj	\
        $(SLO)$/prhdlfac.obj	\
        $(SLO)$/prstylei.obj	\
        $(SLO)$/shadwhdl.obj	\
        $(SLO)$/shdwdhdl.obj	\
        $(SLO)$/styleexp.obj	\
        $(SLO)$/splithdl.obj	\
        $(SLO)$/tabsthdl.obj	\
        $(SLO)$/undlihdl.obj	\
        $(SLO)$/uniref.obj		\
        $(SLO)$/weighhdl.obj	\
        $(SLO)$/xmlastpl.obj	\
        $(SLO)$/xmlaustp.obj	\
        $(SLO)$/xmlbahdl.obj	\
        $(SLO)$/xmlexpit.obj	\
        $(SLO)$/xmlexppr.obj	\
        $(SLO)$/xmlimpit.obj	\
        $(SLO)$/xmlimppr.obj	\
        $(SLO)$/xmlitem.obj		\
        $(SLO)$/xmlitmpr.obj	\
        $(SLO)$/xmlnume.obj		\
        $(SLO)$/xmlnumfe.obj	\
        $(SLO)$/xmlnumfi.obj	\
        $(SLO)$/xmlnumi.obj		\
        $(SLO)$/xmlprcon.obj	\
        $(SLO)$/xmlprhdl.obj	\
        $(SLO)$/xmlprmap.obj	\
        $(SLO)$/xmlstyle.obj	\
        $(SLO)$/xmltabe.obj		\
        $(SLO)$/xmltabi.obj		\
        $(SLO)$/DashStyle.obj					\
        $(SLO)$/EnumPropertyHdl.obj				\
        $(SLO)$/FillStyleContext.obj			\
        $(SLO)$/GradientStyle.obj				\
        $(SLO)$/HatchStyle.obj					\
        $(SLO)$/ImageStyle.obj					\
        $(SLO)$/MarkerStyle.obj					\
        $(SLO)$/NamedBoolPropertyHdl.obj		\
        $(SLO)$/TransGradientStyle.obj			\
        $(SLO)$/XMLBackgroundImageContext.obj	\
        $(SLO)$/XMLBackgroundImageExport.obj	\
        $(SLO)$/XMLClipPropertyHandler.obj		\
        $(SLO)$/XMLConstantsPropertyHandler.obj	\
        $(SLO)$/AttributeContainerHandler.obj	\
        $(SLO)$/XMLElementPropertyContext.obj	\
        $(SLO)$/XMLFontAutoStylePool.obj		\
        $(SLO)$/XMLFontStylesContext.obj		\
        $(SLO)$/XMLPageExport.obj				\
        $(SLO)$/PageMasterPropHdl.obj			\
        $(SLO)$/PageMasterPropHdlFactory.obj	\
        $(SLO)$/PageMasterStyleMap.obj			\
        $(SLO)$/PageMasterPropMapper.obj		\
        $(SLO)$/PageMasterExportPropMapper.obj	\
        $(SLO)$/PageMasterImportPropMapper.obj	\
        $(SLO)$/PageMasterImportContext.obj		\
        $(SLO)$/PagePropertySetContext.obj		\
        $(SLO)$/PageHeaderFooterContext.obj


# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk
