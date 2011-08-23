/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _XMLOFF_TEXTPARAE_HXX_
#define _XMLOFF_TEXTPARAE_HXX_

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <bf_xmloff/uniref.hxx>
#include <bf_xmloff/xmlexppr.hxx>
#include <bf_xmloff/styleexp.hxx>
#include <bf_xmloff/xmltoken.hxx>
#include <bf_xmloff/SinglePropertySetInfoCache.hxx>
class SvLongs;
namespace com { namespace sun { namespace star
{
    namespace beans { class XPropertySet; class XPropertyState;
                      class XPropertySetInfo; }
    namespace container { class XEnumeration; class XIndexAccess; }
    namespace text { class XTextContent; class XTextRange; class XText;
                     class XFootnote; class XTextFrame; class XTextSection;
                     class XDocumentIndex; class XTextShapesSupplier; }
} } }
namespace binfilter {

class SvXMLExport;
class SvXMLAutoStylePoolP;
class XMLTextFieldExport;
class OUStrings_Impl;
class OUStringsSort_Impl;
class XMLTextNumRuleInfo;
class XMLTextListAutoStylePool;
class XMLSectionExport;
class XMLIndexMarkExport;
class XMLRedlineExport;
struct XMLPropertyState;
class MultiPropertySetHelper;

namespace xmloff { class OFormLayerXMLExport; }

class XMLTextParagraphExport : public XMLStyleExport
{
//	SvXMLExport& rExport;
    SvXMLAutoStylePoolP& rAutoStylePool;
    UniReference < SvXMLExportPropertyMapper > xParaPropMapper;
    UniReference < SvXMLExportPropertyMapper > xTextPropMapper;
    UniReference < SvXMLExportPropertyMapper > xFramePropMapper;
    UniReference < SvXMLExportPropertyMapper > xAutoFramePropMapper;
    UniReference < SvXMLExportPropertyMapper > xSectionPropMapper;
    UniReference < SvXMLExportPropertyMapper > xRubyPropMapper;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexAccess > xTextFrames;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexAccess > xGraphics;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexAccess > xEmbeddeds;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexAccess > xShapes;

    SvLongs 					*pPageTextFrameIdxs;
    SvLongs 					*pPageGraphicIdxs;
    SvLongs 					*pPageEmbeddedIdxs;
    SvLongs 					*pPageShapeIdxs;
    SvLongs 					*pFrameTextFrameIdxs;
    SvLongs 					*pFrameGraphicIdxs;
    SvLongs 					*pFrameEmbeddedIdxs;
    SvLongs 					*pFrameShapeIdxs;
    XMLTextFieldExport			*pFieldExport;
    OUStrings_Impl				*pListElements;
    OUStringsSort_Impl			*pExportedLists;
    XMLTextListAutoStylePool	*pListAutoPool;
    XMLSectionExport			*pSectionExport;
    XMLIndexMarkExport			*pIndexMarkExport;

    /// may be NULL (if no redlines should be exported; e.g. in block mode)
    XMLRedlineExport			*pRedlineExport;

    sal_Bool					bProgress;

    sal_Bool					bBlock;

    // keep track of open rubies
    ::rtl::OUString				sOpenRubyText;
    ::rtl::OUString				sOpenRubyCharStyle;
    sal_Bool					bOpenRuby;

    enum FrameType { FT_TEXT, FT_GRAPHIC, FT_EMBEDDED, FT_SHAPE };

protected:

    const ::rtl::OUString sParagraphService;
    const ::rtl::OUString sTableService;
    const ::rtl::OUString sTextFieldService;
    const ::rtl::OUString sTextFrameService;
    const ::rtl::OUString sTextEmbeddedService;
    const ::rtl::OUString sTextGraphicService;
    const ::rtl::OUString sTextEndnoteService;
    const ::rtl::OUString sTextContentService;
    const ::rtl::OUString sShapeService;
    const ::rtl::OUString sParaStyleName;
    const ::rtl::OUString sParaConditionalStyleName;
    const ::rtl::OUString sParaChapterNumberingLevel;
    const ::rtl::OUString sCharStyleName;
    const ::rtl::OUString sCharStyleNames;
    const ::rtl::OUString sFrameStyleName;
    const ::rtl::OUString sText;
    const ::rtl::OUString sTextField;
    const ::rtl::OUString sFrame;
    const ::rtl::OUString sCategory;
    const ::rtl::OUString sNumberingRules;
    const ::rtl::OUString sTextPortionType;
    const ::rtl::OUString sFootnote;
    const ::rtl::OUString sBookmark;
    const ::rtl::OUString sReferenceMark;
    const ::rtl::OUString sIsCollapsed;
    const ::rtl::OUString sIsStart;
    const ::rtl::OUString sReferenceId;
    const ::rtl::OUString sNumberingType;
    const ::rtl::OUString sPageStyleName;
    const ::rtl::OUString sPageDescName;
    const ::rtl::OUString sPrefix;
    const ::rtl::OUString sStartAt;
    const ::rtl::OUString sSuffix;
    const ::rtl::OUString sPositionEndOfDoc;
    const ::rtl::OUString sFootnoteCounting;
    const ::rtl::OUString sEndNotice;
    const ::rtl::OUString sBeginNotice;
    const ::rtl::OUString sFrameWidthAbsolute;
    const ::rtl::OUString sFrameWidthPercent;
    const ::rtl::OUString sFrameHeightAbsolute;
    const ::rtl::OUString sFrameHeightPercent;
    const ::rtl::OUString sWidth;
    const ::rtl::OUString sRelativeWidth;
    const ::rtl::OUString sHeight;
    const ::rtl::OUString sRelativeHeight;
    const ::rtl::OUString sSizeType;
    const ::rtl::OUString sIsSyncWidthToHeight;
    const ::rtl::OUString sIsSyncHeightToWidth;
    const ::rtl::OUString sHoriOrient;
    const ::rtl::OUString sHoriOrientPosition;
    const ::rtl::OUString sVertOrient;
    const ::rtl::OUString sVertOrientPosition;
    const ::rtl::OUString sChainNextName;
    const ::rtl::OUString sAnchorType;
    const ::rtl::OUString sAnchorPageNo;
    const ::rtl::OUString sGraphicURL;
    const ::rtl::OUString sGraphicFilter;
    const ::rtl::OUString sGraphicRotation;
    const ::rtl::OUString sAlternativeText;
    const ::rtl::OUString sHyperLinkURL;
    const ::rtl::OUString sHyperLinkName;
    const ::rtl::OUString sHyperLinkTarget;
    const ::rtl::OUString sUnvisitedCharStyleName;
    const ::rtl::OUString sVisitedCharStyleName;
    const ::rtl::OUString sDocumentIndex;
    const ::rtl::OUString sTextSection;
    const ::rtl::OUString sDocumentIndexMark;
    const ::rtl::OUString sActualSize;
    const ::rtl::OUString sContourPolyPolygon;
    const ::rtl::OUString sIsPixelContour;
    const ::rtl::OUString sIsAutomaticContour;
    const ::rtl::OUString sAnchorCharStyleName;
    const ::rtl::OUString sServerMap;
    const ::rtl::OUString sRedline;
    const ::rtl::OUString sRuby;
    const ::rtl::OUString sRubyText;
    const ::rtl::OUString sRubyAdjust;
    const ::rtl::OUString sRubyCharStyleName;

    SinglePropertySetInfoCache aCharStyleNamesPropInfoCache;

    UniReference < SvXMLExportPropertyMapper > GetParaPropMapper() const
    {
        return xParaPropMapper;
    }

    UniReference < SvXMLExportPropertyMapper > GetTextPropMapper() const
    {
        return xTextPropMapper;
    }

    UniReference < SvXMLExportPropertyMapper > GetFramePropMapper() const
    {
        return xFramePropMapper;
    }
    UniReference < SvXMLExportPropertyMapper > GetAutoFramePropMapper() const
    {
        return xAutoFramePropMapper;
    }
    UniReference < SvXMLExportPropertyMapper > GetSectionPropMapper() const
    {
        return xSectionPropMapper;
    }
    UniReference < SvXMLExportPropertyMapper > GetRubyPropMapper() const
    {
        return xRubyPropMapper;
    }

    SvXMLAutoStylePoolP& GetAutoStylePool() { return rAutoStylePool; }
    const SvXMLAutoStylePoolP& GetAutoStylePool() const { return rAutoStylePool; }

public:
    ::rtl::OUString FindTextStyle(
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet > & rPropSet,
            sal_Bool& rbHasCharStyle ) const;
    ::rtl::OUString FindTextStyleAndHyperlink(
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet > & rPropSet,
            sal_Bool& rHyperlink,
            sal_Bool& rbHasCharStyle,
            const XMLPropertyState** pAddState = NULL) const;
    sal_Bool addHyperlinkAttributes(
        const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet > & rPropSet,
        const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertyState > & rPropState,
        const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySetInfo > & rPropSetInfo );

protected:

    sal_Int32 addTextFrameAttributes(
        const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet >& rPropSet,
        sal_Bool bShape );

    virtual void exportStyleAttributes(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle > & rStyle );

    void collectFrames( sal_Bool bBoundToFrameOnly );
    void exportPageFrames( sal_Bool bAutoStyles, sal_Bool bProgress );
    void exportFrameFrames( sal_Bool bAutoStyles, sal_Bool bProgress,
            const ::com::sun::star::uno::Reference <
                    ::com::sun::star::text::XTextFrame > *pParentTxtFrame = 0 );

    void exportNumStyles( sal_Bool bUsed );

    void exportText(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XText > & rText,
        sal_Bool bAutoStyles, sal_Bool bProgress, sal_Bool bExportParagraph );
    void exportText(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XText > & rText,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextSection > & rBaseSection,
        sal_Bool bAutoStyles, sal_Bool bProgress, sal_Bool bExportParagraph );
    sal_Bool exportTextContentEnumeration(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::container::XEnumeration > & rContentEnum,
        sal_Bool bAutoStyles,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextSection > & rBaseSection,
        sal_Bool bProgress,
        sal_Bool bExportParagraph = sal_True,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > *pRangePropSet = 0,
        sal_Bool bExportLevels = sal_True );
    void exportParagraph(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        sal_Bool bAutoStyles, sal_Bool bProgress,
        sal_Bool bExportParagraph,
        MultiPropertySetHelper& rPropSetHelper);
    virtual void exportTable(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        sal_Bool bAutoStyles, sal_Bool bProgress );
    void exportTextRangeEnumeration(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::container::XEnumeration > & rRangeEnum,
        sal_Bool bAutoStyles, sal_Bool bProgress,
        sal_Bool bPrvChrIsSpc = sal_True  );

    void exportTextField(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextRange > & rTextRange,
        sal_Bool bAutoStyles );

    void exportAnyTextFrame(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        FrameType eTxpe,
        sal_Bool bAutoStyles, sal_Bool bProgress,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > *pRangePropSet = 0 );
    void _exportTextFrame(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rPropSet,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySetInfo > & rPropSetInfo,
        sal_Bool bProgress );
    inline void exportTextFrame(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        sal_Bool bAutoStyles, sal_Bool bProgress,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > *pRangePropSet = 0 );
    inline void exportShape(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        sal_Bool bAutoStyles,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > *pRangePropSet = 0  );

    void exportContour(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rPropSet,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySetInfo > & rPropSetInfo );
    void _exportTextGraphic(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rPropSet,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySetInfo > & rPropSetInfo );
    inline void exportTextGraphic(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        sal_Bool bAutoStyles,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > *pRangePropSet = 0  );

    virtual void _collectTextEmbeddedAutoStyles(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rPropSet );
    virtual void _exportTextEmbedded(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rPropSet,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySetInfo > & rPropSetInfo );
    inline void exportTextEmbedded(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        sal_Bool bAutoStyles,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > *pRangePropSet = 0  );
    virtual void setTextEmbeddedGraphicURL(
        const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet >& rPropSet,
        ::rtl::OUString& rStreamName ) const;

    /// export a footnote and styles
    void exportTextFootnote(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropSet,
        const ::rtl::OUString& sString,
        sal_Bool bAutoStyles, sal_Bool bProgress );

    /// helper for exportTextFootnote
    void exportTextFootnoteHelper(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::text::XFootnote > & rPropSet,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::text::XText> & rText,
        const ::rtl::OUString& sString,
        sal_Bool bAutoStyles,
        sal_Bool bIsEndnote, sal_Bool bProgress );

    /// export footnote and endnote configuration elements
    void exportTextFootnoteConfiguration();

    void exportTextFootnoteConfigurationHelper(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rFootnoteSupplier,
        sal_Bool bIsEndnote);

    void exportTextMark(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & xPropSet,
        const ::rtl::OUString sProperty,
        const enum ::binfilter::xmloff::token::XMLTokenEnum pElements[],
        sal_Bool bAutoStyles);

    void exportIndexMark(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet,
        sal_Bool bAutoStyles);

    void exportTextRange(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextRange > & rTextRange,
        sal_Bool bAutoStyles,
        sal_Bool& rPrevCharWasSpace	);

    void exportListChange( const XMLTextNumRuleInfo& rPrvInfo,
                           const XMLTextNumRuleInfo& rNextInfo );

    /// check if current section or current list has changed;
    /// calls exortListChange as appropriate
    void exportListAndSectionChange(
        ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextSection > & rOldSection,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextSection > & rNewSection,
        const XMLTextNumRuleInfo& rOldList,
        const XMLTextNumRuleInfo& rNewList,
        sal_Bool bAutoStyles );

    /// overload for exportListAndSectionChange;
    /// takes new content rather than new section.
    void exportListAndSectionChange(
        ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextSection > & rOldSection,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rNewContent,
        const XMLTextNumRuleInfo& rOldList,
        const XMLTextNumRuleInfo& rNewList,
        sal_Bool bAutoStyles );
    void exportListAndSectionChange(
        ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextSection > & rOldSection,
        MultiPropertySetHelper& rPropSetHelper,
        sal_Int16 nTextSectionId,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rNewContent,
        const XMLTextNumRuleInfo& rOldList,
        const XMLTextNumRuleInfo& rNewList,
        sal_Bool bAutoStyles );

    /// export a redline text portion
    void exportChange(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rPropSet,
        sal_Bool bAutoStyle);

    /// export a ruby
    void exportRuby(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPortionPropSet,
        sal_Bool bAutoStyles );

public:

    XMLTextParagraphExport(
            SvXMLExport& rExp,
               SvXMLAutoStylePoolP & rASP
                          );
    virtual ~XMLTextParagraphExport();

    /// add autostyle for specified family
    void Add(
        sal_uInt16 nFamily,
        MultiPropertySetHelper& rPropSetHelper,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rPropSet,
        const XMLPropertyState** pAddState = NULL );
    void Add(
        sal_uInt16 nFamily,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rPropSet,
        const XMLPropertyState** pAddState = NULL );

    /// find style name for specified family and parent
    ::rtl::OUString Find(
        sal_uInt16 nFamily,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rPropSet,
        const ::rtl::OUString& rParent,
        const XMLPropertyState** pAddState = NULL ) const;

    static SvXMLExportPropertyMapper *CreateShapeExtPropMapper(
                                                SvXMLExport& rExport );
    static SvXMLExportPropertyMapper *CreateCharExtPropMapper(
                                                SvXMLExport& rExport);
    static SvXMLExportPropertyMapper *CreateParaExtPropMapper(
                                                SvXMLExport& rExport);

    // This methods exports all (or all used) styles
    void exportTextStyles( sal_Bool bUsed
                           , sal_Bool bProg = sal_False
                         );

    /// This method exports (text field) declarations etc.
    void exportTextDeclarations();

    /// export the (text field) declarations for a particular XText
    void exportTextDeclarations(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XText > & rText );

    /// true: export only those declarations that are used;
    /// false: export all declarations
    void exportUsedDeclarations( sal_Bool bOnlyUsed );

    /// Export the list of change information (enclosed by <tracked-changes>)
    /// (or the necessary automatic styles)
    void exportTrackedChanges(sal_Bool bAutoStyle);

    /// Export the list of change information (enclosed by <tracked-changes>)
    /// (or the necessary automatic styles)
    void exportTrackedChanges(const ::com::sun::star::uno::Reference <
                                  ::com::sun::star::text::XText > & rText,
                              sal_Bool bAutoStyle );

    /// Record tracked changes for this particular XText
    /// (empty reference stop recording)
    /// This should be used if tracked changes for e.g. footers are to
    /// be exported seperately via the exportTrackedChanges(sal_Bool,
    /// Reference<XText>) method.
    void recordTrackedChangesForXText(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XText > & rText );

    /// Stop recording tracked changes.
    /// This is the same as calling recordTrackedChanges(...) with an
    /// empty reference.
    void recordTrackedChangesNoXText();

    // This method exports the given OUString
    void exportText(
        const ::rtl::OUString& rText,
        sal_Bool& rPrevCharWasSpace );

    // This method collects all automatic styles for the given XText
    void collectTextAutoStyles(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XText > & rText,
        sal_Bool bProgressIn = sal_False,
        sal_Bool bExportParagraph = sal_True )
    {
        exportText( rText, sal_True, bProgressIn, bExportParagraph );
    }

    void collectTextAutoStyles(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XText > & rText,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextSection > & rBaseSection,
        sal_Bool bProgressIn = sal_False,
        sal_Bool bExportParagraph = sal_True )
    {
        exportText( rText, rBaseSection, sal_True, bProgressIn, bExportParagraph );
    }

    // This method prepares the collection of auto styles for frames
    // that are bound to a frame.
    void collectFramesBoundToFrameAutoStyles( sal_Bool bProgressIn = sal_False )
    {
        // unused
        bProgressIn;
        collectFrames( sal_True );
    }

    // This method prepares the collection of auto styles for frames
    // that are bound to a frame and it collects auto styles
    // for frames bound to a page.
    void collectFramesBoundToPageOrFrameAutoStyles( sal_Bool bProgressIn = sal_False )
    {
        collectFrames( sal_False );
        exportPageFrames( sal_True, bProgressIn );
    }

    void collectFramesBoundToFrameAutoStyles(
            const ::com::sun::star::uno::Reference <
                    ::com::sun::star::text::XTextFrame >& rParentTxtFrame,
            sal_Bool bProgressIn = sal_False )
    {
        exportFrameFrames( sal_True, bProgressIn, &rParentTxtFrame );
    }

    // This method exports all automatic styles that have been collected.
    virtual void exportTextAutoStyles();

    void exportEvents( const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > & rPropSet );
    void exportAlternativeText( const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > & rPropSet,
                                const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySetInfo > & rPropSetInfo );

    // This method exports the given XText
    void exportText(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XText > & rText,
        sal_Bool bProgressIn = sal_False,
        sal_Bool bExportParagraph = sal_True)
    {
        exportText( rText, sal_False, bProgressIn, bExportParagraph );
    }

    void exportText(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XText > & rText,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextSection > & rBaseSection,
        sal_Bool bProgressIn = sal_False,
        sal_Bool bExportParagraph = sal_True)
    {
        exportText( rText, rBaseSection, sal_False, bProgressIn, bExportParagraph );
    }

    void exportFramesBoundToPage( sal_Bool bProgressIn = sal_False )
    {
        exportPageFrames( sal_False, bProgressIn );
    }
    void exportFramesBoundToFrame(
            const ::com::sun::star::uno::Reference <
                    ::com::sun::star::text::XTextFrame >& rParentTxtFrame,
            sal_Bool bProgressIn = sal_False )
    {
        exportFrameFrames( sal_False, bProgressIn, &rParentTxtFrame );
    }
    inline const XMLTextListAutoStylePool& GetListAutoStylePool() const;

    void SetBlockMode( sal_Bool bSet ) { bBlock = bSet; }
    sal_Bool IsBlockMode() const { return bBlock; }

    UniReference < SvXMLExportPropertyMapper > GetParagraphPropertyMapper() const
    {
        return xParaPropMapper;
    }

    /** exclude form controls which are in mute sections.
     *
     * This method is necessary to prevent the form layer export from exporting
     * control models whose controls are not represented in the document.  To
     * achieve this, this method iterates over all shapes, checks to see if
     * they are control shapes, and if so, whether they should be exported or
     * not. If not, the form layer export will be notified accordingly.
     *
     * The reason this method is located here is tha it needs to access the
     * XMLSectionExport, which is only available here.
     */
    void PreventExportOfControlsInMuteSections(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XIndexAccess> & rShapes,
        UniReference<xmloff::OFormLayerXMLExport> xFormExport );

    SinglePropertySetInfoCache& GetCharStyleNamesPropInfoCache() { return aCharStyleNamesPropInfoCache; }
};

inline const XMLTextListAutoStylePool&
    XMLTextParagraphExport::GetListAutoStylePool() const
{
    return *pListAutoPool;
}

inline void XMLTextParagraphExport::exportTextFrame(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        sal_Bool bAutoStyles, sal_Bool bProgressIn,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > *pRangePropSet)
{
    exportAnyTextFrame( rTextContent, FT_TEXT, bAutoStyles, bProgressIn,
                        pRangePropSet );
}

inline void XMLTextParagraphExport::exportTextGraphic(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        sal_Bool bAutoStyles,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > *pRangePropSet )
{
    exportAnyTextFrame( rTextContent, FT_GRAPHIC, bAutoStyles, sal_False,
                        pRangePropSet );
}

inline void XMLTextParagraphExport::exportTextEmbedded(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        sal_Bool bAutoStyles,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > *pRangePropSet )
{
    exportAnyTextFrame( rTextContent, FT_EMBEDDED, bAutoStyles, sal_False,
                        pRangePropSet );
}

inline void XMLTextParagraphExport::exportShape(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        sal_Bool bAutoStyles,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > *pRangePropSet )
{
    exportAnyTextFrame( rTextContent, FT_SHAPE, bAutoStyles, sal_False,
                        pRangePropSet );
}

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
