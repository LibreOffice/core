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

#include "sal/config.h"

#include <memory>

#include "xmloff/dllapi.h"
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <xmloff/uniref.hxx>
#include <xmloff/xmlexppr.hxx>
#include <xmloff/styleexp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/SinglePropertySetInfoCache.hxx>
#include <xmloff/XMLStringVector.hxx>
#include <vector>
#include <boost/scoped_ptr.hpp>

class XMLTextListsHelper;
class SvXMLExport;
class SvXMLAutoStylePoolP;
class XMLTextFieldExport;
class XMLTextNumRuleInfo;
class XMLTextListAutoStylePool;
class XMLSectionExport;
class XMLIndexMarkExport;
class XMLRedlineExport;
struct XMLPropertyState;
class MultiPropertySetHelper;

namespace com { namespace sun { namespace star
{
    namespace beans { class XPropertySet; class XPropertyState;
                      class XPropertySetInfo; }
    namespace container { class XEnumerationAccess; class XEnumeration; class XIndexAccess; }
    namespace text { class XTextContent; class XTextRange; class XText;
                     class XFootnote; class XTextFrame; class XTextSection;
                     class XTextField;
                     class XDocumentIndex; class XTextShapesSupplier; }
} } }

namespace xmloff
{
    class OFormLayerXMLExport;
    class BoundFrameSets;
}

class XMLOFF_DLLPUBLIC XMLTextParagraphExport : public XMLStyleExport
{
    struct Impl;
    ::boost::scoped_ptr<Impl> m_pImpl;

//  SvXMLExport& rExport;
    SvXMLAutoStylePoolP& rAutoStylePool;
    UniReference < SvXMLExportPropertyMapper > xParaPropMapper;
    UniReference < SvXMLExportPropertyMapper > xTextPropMapper;
    UniReference < SvXMLExportPropertyMapper > xFramePropMapper;
    UniReference < SvXMLExportPropertyMapper > xAutoFramePropMapper;
    UniReference < SvXMLExportPropertyMapper > xSectionPropMapper;
    UniReference < SvXMLExportPropertyMapper > xRubyPropMapper;

    const ::std::auto_ptr< ::xmloff::BoundFrameSets > pBoundFrameSets;
    XMLTextFieldExport          *pFieldExport;
    std::vector<rtl::OUString>  *pListElements;
    XMLTextListAutoStylePool    *pListAutoPool;
    XMLSectionExport            *pSectionExport;
    XMLIndexMarkExport          *pIndexMarkExport;

    /// may be NULL (if no redlines should be exported; e.g. in block mode)
    XMLRedlineExport            *pRedlineExport;
    XMLStringVector             *pHeadingStyles;

    sal_Bool                    bProgress;

    sal_Bool                    bBlock;

    // keep track of open rubies
    ::rtl::OUString             sOpenRubyText;
    ::rtl::OUString             sOpenRubyCharStyle;
    sal_Bool                    bOpenRuby;

    XMLTextListsHelper* mpTextListsHelper;
    ::std::vector< XMLTextListsHelper* > maTextListsHelperStack;

    enum FrameType { FT_TEXT, FT_GRAPHIC, FT_EMBEDDED, FT_SHAPE };
public:

    enum FieldmarkType { NONE, TEXT, CHECK }; // Used for simulating fieldmarks in OpenDocument 1.n Strict (for n <= 2). CHECK currently ignored.

protected:

    const ::rtl::OUString sActualSize;
    // Implement Title/Description Elements UI (#i73249#)
    const ::rtl::OUString sTitle;
    const ::rtl::OUString sDescription;
    const ::rtl::OUString sAnchorCharStyleName;
    const ::rtl::OUString sAnchorPageNo;
    const ::rtl::OUString sAnchorType;
    const ::rtl::OUString sBeginNotice;
    const ::rtl::OUString sBookmark;
    const ::rtl::OUString sCategory;
    const ::rtl::OUString sChainNextName;
    const ::rtl::OUString sCharStyleName;
    const ::rtl::OUString sCharStyleNames;
    const ::rtl::OUString sContourPolyPolygon;
    const ::rtl::OUString sDocumentIndex;
    const ::rtl::OUString sDocumentIndexMark;
    const ::rtl::OUString sEndNotice;
    const ::rtl::OUString sFootnote;
    const ::rtl::OUString sFootnoteCounting;
    const ::rtl::OUString sFrame;
    const ::rtl::OUString sFrameHeightAbsolute;
    const ::rtl::OUString sFrameHeightPercent;
    const ::rtl::OUString sFrameStyleName;
    const ::rtl::OUString sFrameWidthAbsolute;
    const ::rtl::OUString sFrameWidthPercent;
    const ::rtl::OUString sGraphicFilter;
    const ::rtl::OUString sGraphicRotation;
    const ::rtl::OUString sGraphicURL;
    const ::rtl::OUString sHeight;
    const ::rtl::OUString sHoriOrient;
    const ::rtl::OUString sHoriOrientPosition;
    const ::rtl::OUString sHyperLinkName;
    const ::rtl::OUString sHyperLinkTarget;
    const ::rtl::OUString sHyperLinkURL;
    const ::rtl::OUString sIsAutomaticContour;
    const ::rtl::OUString sIsCollapsed;
    const ::rtl::OUString sIsPixelContour;
    const ::rtl::OUString sIsStart;
    const ::rtl::OUString sIsSyncHeightToWidth;
    const ::rtl::OUString sIsSyncWidthToHeight;
    const ::rtl::OUString sNumberingRules;
    const ::rtl::OUString sNumberingType;
    const ::rtl::OUString sPageDescName;
    const ::rtl::OUString sPageStyleName;
    const ::rtl::OUString sParaChapterNumberingLevel;
    const ::rtl::OUString sParaConditionalStyleName;
    const ::rtl::OUString sParagraphService;
    const ::rtl::OUString sParaStyleName;
    const ::rtl::OUString sPositionEndOfDoc;
    const ::rtl::OUString sPrefix;
    const ::rtl::OUString sRedline;
    const ::rtl::OUString sReferenceId;
    const ::rtl::OUString sReferenceMark;
    const ::rtl::OUString sRelativeHeight;
    const ::rtl::OUString sRelativeWidth;
    const ::rtl::OUString sRuby;
    const ::rtl::OUString sRubyAdjust;
    const ::rtl::OUString sRubyCharStyleName;
    const ::rtl::OUString sRubyText;
    const ::rtl::OUString sServerMap;
    const ::rtl::OUString sShapeService;
    const ::rtl::OUString sSizeType;
    const ::rtl::OUString sSoftPageBreak;
    const ::rtl::OUString sStartAt;
    const ::rtl::OUString sSuffix;
    const ::rtl::OUString sTableService;
    const ::rtl::OUString sText;
    const ::rtl::OUString sTextContentService;
    const ::rtl::OUString sTextEmbeddedService;
    const ::rtl::OUString sTextEndnoteService;
    const ::rtl::OUString sTextField;
    const ::rtl::OUString sTextFieldService;
    const ::rtl::OUString sTextFrameService;
    const ::rtl::OUString sTextGraphicService;
    const ::rtl::OUString sTextPortionType;
    const ::rtl::OUString sTextSection;
    const ::rtl::OUString sUnvisitedCharStyleName;
    const ::rtl::OUString sVertOrient;
    const ::rtl::OUString sVertOrientPosition;
    const ::rtl::OUString sVisitedCharStyleName;
    const ::rtl::OUString sWidth;
    const ::rtl::OUString sWidthType;
    const ::rtl::OUString sTextFieldStart;
    const ::rtl::OUString sTextFieldEnd;
    const ::rtl::OUString sTextFieldStartEnd;

    SinglePropertySetInfoCache aCharStyleNamesPropInfoCache;

//  SvXMLExport& GetExport() { return rExport; }
//  const SvXMLExport& GetExport() const  { return rExport; }

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
    ::rtl::OUString FindTextStyleAndHyperlink(
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet > & rPropSet,
            sal_Bool& rbHyperlink,
            sal_Bool& rbHasCharStyle,
            sal_Bool& rbHasAutoStyle,
            const XMLPropertyState** pAddState = NULL) const;
    sal_Bool addHyperlinkAttributes(
        const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet > & rPropSet,
        const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertyState > & rPropState,
        const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySetInfo > & rPropSetInfo );

    void exportTextRangeEnumeration(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::container::XEnumeration > & rRangeEnum,
        sal_Bool bAutoStyles, sal_Bool bProgress,
        sal_Bool bPrvChrIsSpc = sal_True );

protected:

    sal_Int32 addTextFrameAttributes(
        const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet >& rPropSet,
        sal_Bool bShape,
        ::rtl::OUString *pMinHeightValue = 0,
        ::rtl::OUString *pMinWidthValue = 0 );

    virtual void exportStyleAttributes(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle > & rStyle );

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

    void exportTextField(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextRange > & rTextRange,
        sal_Bool bAutoStyles, sal_Bool bProgress );

    void exportTextField(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextField> & xTextField,
        const sal_Bool bAutoStyles, const sal_Bool bProgress,
        const sal_Bool bRecursive );

    void exportAnyTextFrame(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        FrameType eTxpe,
        sal_Bool bAutoStyles, sal_Bool bProgress, sal_Bool bExportContent,
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
        sal_Bool bAutoStyles, sal_Bool bProgress, sal_Bool bExportContent,
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
        const ::rtl::OUString& rProperty,
        const enum ::xmloff::token::XMLTokenEnum pElements[],
        sal_Bool bAutoStyles);

    void exportIndexMark(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet,
        sal_Bool bAutoStyles);

    void exportSoftPageBreak(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet,
        sal_Bool bAutoStyles);

    void exportTextRange(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextRange > & rTextRange,
        sal_Bool bAutoStyles,
        bool& rPrevCharWasSpace,
        FieldmarkType& openFieldmarkType );

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

    /// export a text:meta
    void exportMeta(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & i_xPortion,
        sal_Bool i_bAutoStyles, sal_Bool i_isProgress );

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
        const XMLPropertyState** pAddState = NULL, bool bDontSeek = false );

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
    static SvXMLExportPropertyMapper *CreateParaDefaultExtPropMapper(
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
        bool& rPrevCharWasSpace );

    // This method collects all automatic styles for the given XText
    void collectTextAutoStyles(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XText > & rText,
        sal_Bool bIsProgress = sal_False,
        sal_Bool bExportParagraph = sal_True )
    {
        exportText( rText, sal_True, bIsProgress, bExportParagraph );
    }

    void collectTextAutoStyles(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XText > & rText,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextSection > & rBaseSection,
        sal_Bool bIsProgress = sal_False,
        sal_Bool bExportParagraph = sal_True )
    {
        exportText( rText, rBaseSection, sal_True, bIsProgress, bExportParagraph );
    }

    // It the model implements the xAutoStylesSupplier interface, the automatic
    // styles can exported without iterating over the text portions
    bool collectTextAutoStylesOptimized(
        sal_Bool bIsProgress = sal_False );

    // This method exports all automatic styles that have been collected.
    virtual void exportTextAutoStyles();

    void exportEvents( const ::com::sun::star::uno::Reference < com::sun::star::beans::XPropertySet > & rPropSet );

    // Implement Title/Description Elements UI (#i73249#)
    void exportTitleAndDescription( const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > & rPropSet,
                                    const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySetInfo > & rPropSetInfo );

    // This method exports the given XText
    void exportText(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XText > & rText,
        sal_Bool bIsProgress = sal_False,
        sal_Bool bExportParagraph = sal_True)
    {
        exportText( rText, sal_False, bIsProgress, bExportParagraph );
    }

    void exportText(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XText > & rText,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextSection > & rBaseSection,
        sal_Bool bIsProgress = sal_False,
        sal_Bool bExportParagraph = sal_True)
    {
        exportText( rText, rBaseSection, sal_False, bIsProgress, bExportParagraph );
    }

    void exportFramesBoundToPage( sal_Bool bIsProgress = sal_False )
    {
        exportPageFrames( sal_False, bIsProgress );
    }
    void exportFramesBoundToFrame(
            const ::com::sun::star::uno::Reference <
                    ::com::sun::star::text::XTextFrame >& rParentTxtFrame,
            sal_Bool bIsProgress = sal_False )
    {
        exportFrameFrames( sal_False, bIsProgress, &rParentTxtFrame );
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

    void PushNewTextListsHelper();

    void PopTextListsHelper();

private:
        XMLTextParagraphExport(XMLTextParagraphExport &); // private copy-ctor because of explicit copy-ctor of auto_ptr
};

inline const XMLTextListAutoStylePool&
    XMLTextParagraphExport::GetListAutoStylePool() const
{
    return *pListAutoPool;
}

inline void XMLTextParagraphExport::exportTextFrame(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        sal_Bool bAutoStyles, sal_Bool bIsProgress, sal_Bool bExportContent,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > *pRangePropSet)
{
    exportAnyTextFrame( rTextContent, FT_TEXT, bAutoStyles, bIsProgress,
                        bExportContent, pRangePropSet );
}

inline void XMLTextParagraphExport::exportTextGraphic(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        sal_Bool bAutoStyles,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > *pRangePropSet )
{
    exportAnyTextFrame( rTextContent, FT_GRAPHIC, bAutoStyles, sal_False,
                        sal_True, pRangePropSet );
}

inline void XMLTextParagraphExport::exportTextEmbedded(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        sal_Bool bAutoStyles,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > *pRangePropSet )
{
    exportAnyTextFrame( rTextContent, FT_EMBEDDED, bAutoStyles, sal_False,
                        sal_True, pRangePropSet );
}

inline void XMLTextParagraphExport::exportShape(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        sal_Bool bAutoStyles,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > *pRangePropSet )
{
    exportAnyTextFrame( rTextContent, FT_SHAPE, bAutoStyles, sal_False,
                        sal_True, pRangePropSet );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
