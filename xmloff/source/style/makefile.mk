#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.31 $
#
#   last change: $Author: rt $ $Date: 2006-10-30 09:06:31 $
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

PRJNAME=xmloff
TARGET=style

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

CXXFILES = \
        XMLPercentOrMeasurePropertyHandler.cxx \
        XMLIsPercentagePropertyHandler.cxx \
        XMLRectangleMembersHandler.cxx \
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
        tabsthdl.cxx	\
        undlihdl.cxx	\
        uniref.cxx		\
        weighhdl.cxx	\
        xmlaustp.cxx	\
        xmlbahdl.cxx	\
        xmlexppr.cxx	\
        xmlimppr.cxx	\
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
        MultiPropertySetHelper.cxx		\
        SinglePropertySetInfoCache.cxx	\
        DashStyle.cxx					\
        AttributeContainerHandler.cxx	\
        NamedBoolPropertyHdl.cxx		\
        TransGradientStyle.cxx			\
        XMLBackgroundImageContext.cxx	\
        XMLBackgroundImageExport.cxx	\
        XMLClipPropertyHandler.cxx		\
        XMLConstantsPropertyHandler.cxx	\
        XMLElementPropertyContext.cxx	\
        XMLFootnoteSeparatorExport.cxx	\
        XMLFootnoteSeparatorImport.cxx	\
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
        PageHeaderFooterContext.cxx		\
        StyleMap.cxx		\
        XMLBitmapRepeatOffsetPropertyHandler.cxx \
        XMLFillBitmapSizePropertyHandler.cxx \
        XMLBitmapLogicalSizePropertyHandler.cxx \
        durationhdl.cxx \
        VisAreaExport.cxx \
        VisAreaContext.cxx \
        DrawAspectHdl.cxx


SLOFILES =	\
        $(SLO)$/XMLPercentOrMeasurePropertyHandler.obj \
        $(SLO)$/XMLIsPercentagePropertyHandler.obj \
        $(SLO)$/XMLRectangleMembersHandler.obj \
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
        $(SLO)$/tabsthdl.obj	\
        $(SLO)$/undlihdl.obj	\
        $(SLO)$/uniref.obj		\
        $(SLO)$/weighhdl.obj	\
        $(SLO)$/xmlaustp.obj	\
        $(SLO)$/xmlbahdl.obj	\
        $(SLO)$/xmlexppr.obj	\
        $(SLO)$/xmlimppr.obj	\
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
        $(SLO)$/MultiPropertySetHelper.obj		\
        $(SLO)$/SinglePropertySetInfoCache.obj	\
        $(SLO)$/NamedBoolPropertyHdl.obj		\
        $(SLO)$/TransGradientStyle.obj			\
        $(SLO)$/XMLBackgroundImageContext.obj	\
        $(SLO)$/XMLBackgroundImageExport.obj	\
        $(SLO)$/XMLClipPropertyHandler.obj		\
        $(SLO)$/XMLConstantsPropertyHandler.obj	\
        $(SLO)$/AttributeContainerHandler.obj	\
        $(SLO)$/XMLElementPropertyContext.obj	\
        $(SLO)$/XMLFootnoteSeparatorExport.obj	\
        $(SLO)$/XMLFootnoteSeparatorImport.obj	\
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
        $(SLO)$/PageHeaderFooterContext.obj		\
        $(SLO)$/StyleMap.obj					\
        $(SLO)$/XMLBitmapRepeatOffsetPropertyHandler.obj \
        $(SLO)$/XMLFillBitmapSizePropertyHandler.obj	\
        $(SLO)$/XMLBitmapLogicalSizePropertyHandler.obj	\
        $(SLO)$/durationhdl.obj	\
        $(SLO)$/VisAreaExport.obj \
        $(SLO)$/VisAreaContext.obj \
        $(SLO)$/DrawAspectHdl.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk
