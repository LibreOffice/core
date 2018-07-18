/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_XMLOFF_TXTPARAE_HXX
#define INCLUDED_XMLOFF_TXTPARAE_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <xmloff/xmlexppr.hxx>
#include <xmloff/styleexp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/SinglePropertySetInfoCache.hxx>
#include <xmloff/XMLTextListAutoStylePool.hxx>
#include <memory>
#include <vector>

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
enum class XMLShapeExportFlags;

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

namespace basegfx
{
    class B2DPoint;
}

enum class TextPNS
{
    ODF,
    EXTENSION
};

class XMLOFF_DLLPUBLIC XMLTextParagraphExport : public XMLStyleExport
{
    struct Impl;
    std::unique_ptr<Impl> m_xImpl;

//  SvXMLExport& rExport;
    SvXMLAutoStylePoolP& rAutoStylePool;
    rtl::Reference < SvXMLExportPropertyMapper > xParaPropMapper;
    rtl::Reference < SvXMLExportPropertyMapper > xTextPropMapper;
    rtl::Reference < SvXMLExportPropertyMapper > xFramePropMapper;
    rtl::Reference < SvXMLExportPropertyMapper > xAutoFramePropMapper;
    rtl::Reference < SvXMLExportPropertyMapper > xSectionPropMapper;
    rtl::Reference < SvXMLExportPropertyMapper > xRubyPropMapper;

    const ::std::unique_ptr< ::xmloff::BoundFrameSets > pBoundFrameSets;
    std::unique_ptr<XMLTextFieldExport>          pFieldExport;
    std::unique_ptr<std::vector<OUString>>       pListElements;
    XMLTextListAutoStylePool                     maListAutoPool;
    std::unique_ptr<XMLSectionExport>            pSectionExport;
    std::unique_ptr<XMLIndexMarkExport>          pIndexMarkExport;

    /// may be NULL (if no redlines should be exported; e.g. in block mode)
    std::unique_ptr<XMLRedlineExport> pRedlineExport;

    bool                        bProgress;

    bool                        bBlock;

    // keep track of open rubies
    OUString                    sOpenRubyText;
    OUString                    sOpenRubyCharStyle;
    bool                        bOpenRuby;

    XMLTextListsHelper* mpTextListsHelper;
    ::std::vector< std::unique_ptr<XMLTextListsHelper> > maTextListsHelperStack;

    bool mbCollected;

    enum class FrameType { Text, Graphic, Embedded, Shape };
public:

    enum FieldmarkType { NONE, TEXT, CHECK }; // Used for simulating fieldmarks in OpenDocument 1.n Strict (for n <= 2). CHECK currently ignored.


    void exportTextRangeSpan(
            const css::uno::Reference< css::text::XTextRange > & rTextRange,
            css::uno::Reference< css::beans::XPropertySet > const & xPropSet,
            css::uno::Reference < css::beans::XPropertySetInfo > & xPropSetInfo,
            const bool bIsUICharStyle,
            const bool bHasAutoStyle,
            const OUString& sStyle,
            bool& rPrevCharIsSpace,
            FieldmarkType& openFieldMark);

private:

    // Implement Title/Description Elements UI (#i73249#)
    const OUString sTitle;
    const OUString sDescription;
    const OUString sAnchorCharStyleName;
    const OUString sAnchorPageNo;
    const OUString sAnchorType;
    const OUString sBeginNotice;
    const OUString sBookmark;
    const OUString sCategory;
    const OUString sChainNextName;
    const OUString sCharStyleName;
    const OUString sCharStyleNames;
    const OUString sContourPolyPolygon;
    const OUString sDocumentIndexMark;
    const OUString sEndNotice;
    const OUString sFootnote;
    const OUString sFootnoteCounting;
    const OUString sFrame;
    const OUString sGraphicFilter;
    const OUString sGraphicRotation;
    const OUString sHeight;
    const OUString sHoriOrient;
    const OUString sHoriOrientPosition;
    const OUString sHyperLinkName;
    const OUString sHyperLinkTarget;
    const OUString sHyperLinkURL;
    const OUString sIsAutomaticContour;
    const OUString sIsCollapsed;
    const OUString sIsPixelContour;
    const OUString sIsStart;
    const OUString sIsSyncHeightToWidth;
    const OUString sIsSyncWidthToHeight;
    const OUString sNumberingRules;
    const OUString sNumberingType;
    const OUString sPageDescName;
    const OUString sPageStyleName;
    const OUString sParaConditionalStyleName;
    const OUString sParagraphService;
    const OUString sParaStyleName;
    const OUString sPositionEndOfDoc;
    const OUString sPrefix;
    const OUString sRedline;
    const OUString sReferenceId;
    const OUString sReferenceMark;
    const OUString sRelativeHeight;
    const OUString sRelativeWidth;
    const OUString sRuby;
    const OUString sRubyCharStyleName;
    const OUString sRubyText;
    const OUString sServerMap;
    const OUString sShapeService;
    const OUString sSizeType;
    const OUString sSoftPageBreak;
    const OUString sStartAt;
    const OUString sSuffix;
    const OUString sTableService;
    const OUString sText;
    const OUString sTextContentService;
    const OUString sTextEmbeddedService;
    const OUString sTextEndnoteService;
    const OUString sTextField;
    const OUString sTextFieldService;
    const OUString sTextFrameService;
    const OUString sTextGraphicService;
    const OUString sTextPortionType;
    const OUString sTextSection;
    const OUString sUnvisitedCharStyleName;
    const OUString sVertOrient;
    const OUString sVertOrientPosition;
    const OUString sVisitedCharStyleName;
    const OUString sWidth;
    const OUString sWidthType;
    const OUString sTextFieldStart;
    const OUString sTextFieldEnd;
    const OUString sTextFieldStartEnd;

protected:
    const OUString sFrameStyleName;
    SinglePropertySetInfoCache aCharStyleNamesPropInfoCache;

    SvXMLAutoStylePoolP& GetAutoStylePool() { return rAutoStylePool; }
    const SvXMLAutoStylePoolP& GetAutoStylePool() const { return rAutoStylePool; }

public:
    const rtl::Reference < SvXMLExportPropertyMapper >& GetParaPropMapper() const
    {
        return xParaPropMapper;
    }

    const rtl::Reference < SvXMLExportPropertyMapper >& GetTextPropMapper() const
    {
        return xTextPropMapper;
    }

    const rtl::Reference < SvXMLExportPropertyMapper >& GetAutoFramePropMapper() const
    {
        return xAutoFramePropMapper;
    }
    const rtl::Reference < SvXMLExportPropertyMapper >& GetSectionPropMapper() const
    {
        return xSectionPropMapper;
    }
    const rtl::Reference < SvXMLExportPropertyMapper >& GetRubyPropMapper() const
    {
        return xRubyPropMapper;
    }

    OUString FindTextStyleAndHyperlink(
            const css::uno::Reference< css::beans::XPropertySet > & rPropSet,
            bool& rbHyperlink,
            bool& rbHasCharStyle,
            bool& rbHasAutoStyle,
            const XMLPropertyState** pAddState = nullptr) const;
    bool addHyperlinkAttributes(
        const css::uno::Reference< css::beans::XPropertySet > & rPropSet,
        const css::uno::Reference< css::beans::XPropertyState > & rPropState,
        const css::uno::Reference< css::beans::XPropertySetInfo > & rPropSetInfo );

    void exportTextRangeEnumeration(
        const css::uno::Reference< css::container::XEnumeration > & rRangeEnum,
        bool bAutoStyles, bool bProgress, bool & rPrevCharIsSpace);

protected:

    XMLShapeExportFlags addTextFrameAttributes(
        const css::uno::Reference< css::beans::XPropertySet >& rPropSet,
        bool bShape,
        basegfx::B2DPoint* pCenter = nullptr,
        OUString *pMinHeightValue = nullptr,
        OUString *pMinWidthValue = nullptr );

    virtual void exportStyleAttributes(
        const css::uno::Reference< css::style::XStyle > & rStyle ) override;

    void exportPageFrames( bool bProgress );
    void exportFrameFrames( bool bAutoStyles, bool bProgress,
            const css::uno::Reference< css::text::XTextFrame > *pParentTxtFrame );

    void exportNumStyles( bool bUsed );

    void exportText(
        const css::uno::Reference <
            css::text::XText > & rText,
        bool bAutoStyles, bool bProgress, bool bExportParagraph, TextPNS eExtensionNS = TextPNS::ODF );

    void exportText(
        const css::uno::Reference< css::text::XText > & rText,
        const css::uno::Reference< css::text::XTextSection > & rBaseSection,
        bool bAutoStyles, bool bProgress, bool bExportParagraph );

    void exportTextContentEnumeration(
        const css::uno::Reference< css::container::XEnumeration > & rContentEnum,
        bool bAutoStyles,
        const css::uno::Reference< css::text::XTextSection > & rBaseSection,
        bool bProgress,
        bool bExportParagraph = true,
        const css::uno::Reference< css::beans::XPropertySet > *pRangePropSet = nullptr,
        TextPNS eExtensionNS = TextPNS::ODF);
    void exportParagraph(
        const css::uno::Reference< css::text::XTextContent > & rTextContent,
        bool bAutoStyles, bool bProgress,
        bool bExportParagraph,
        MultiPropertySetHelper& rPropSetHelper,
        TextPNS eExtensionNS);

    virtual void exportTable(
        const css::uno::Reference< css::text::XTextContent > & rTextContent,
        bool bAutoStyles, bool bProgress );

    void exportTextField(
        const css::uno::Reference< css::text::XTextRange > & rTextRange,
        bool bAutoStyles, bool bProgress, bool * pPrevCharIsSpace);

    void exportTextField(
        const css::uno::Reference< css::text::XTextField> & xTextField,
        const bool bAutoStyles, const bool bProgress,
        const bool bRecursive, bool * pPrevCharIsSpace);

    void exportAnyTextFrame(
        const css::uno::Reference< css::text::XTextContent > & rTextContent,
        FrameType eTxpe,
        bool bAutoStyles, bool bProgress, bool bExportContent,
        const css::uno::Reference< css::beans::XPropertySet > *pRangePropSet );
    void _exportTextFrame(
        const css::uno::Reference< css::beans::XPropertySet > & rPropSet,
        const css::uno::Reference< css::beans::XPropertySetInfo > & rPropSetInfo,
        bool bProgress );
    inline void exportTextFrame(
        const css::uno::Reference< css::text::XTextContent > & rTextContent,
        bool bAutoStyles, bool bProgress, bool bExportContent,
        const css::uno::Reference< css::beans::XPropertySet > *pRangePropSet = nullptr );
    inline void exportShape(
        const css::uno::Reference< css::text::XTextContent > & rTextContent,
        bool bAutoStyles,
        const css::uno::Reference< css::beans::XPropertySet > *pRangePropSet = nullptr  );

    void exportContour(
        const css::uno::Reference< css::beans::XPropertySet > & rPropSet,
        const css::uno::Reference< css::beans::XPropertySetInfo > & rPropSetInfo );
    void _exportTextGraphic(
        const css::uno::Reference< css::beans::XPropertySet > & rPropSet,
        const css::uno::Reference< css::beans::XPropertySetInfo > & rPropSetInfo );
    inline void exportTextGraphic(
        const css::uno::Reference< css::text::XTextContent > & rTextContent,
        bool bAutoStyles,
        const css::uno::Reference< css::beans::XPropertySet > *pRangePropSet = nullptr  );

    virtual void _collectTextEmbeddedAutoStyles(
        const css::uno::Reference< css::beans::XPropertySet > & rPropSet );
    virtual void _exportTextEmbedded(
        const css::uno::Reference< css::beans::XPropertySet > & rPropSet,
        const css::uno::Reference< css::beans::XPropertySetInfo > & rPropSetInfo );
    inline void exportTextEmbedded(
        const css::uno::Reference< css::text::XTextContent > & rTextContent,
        bool bAutoStyles,
        const css::uno::Reference< css::beans::XPropertySet > *pRangePropSet = nullptr  );

    /// export a footnote and styles
    void exportTextFootnote(
        const css::uno::Reference< css::beans::XPropertySet > & rPropSet,
        const OUString& sString,
        bool bAutoStyles, bool bProgress );

    /// helper for exportTextFootnote
    void exportTextFootnoteHelper(
        const css::uno::Reference< css::text::XFootnote > & rPropSet,
        const css::uno::Reference< css::text::XText> & rText,
        const OUString& sString,
        bool bAutoStyles,
        bool bIsEndnote, bool bProgress );

    /// export footnote and endnote configuration elements
    void exportTextFootnoteConfiguration();

    void exportTextFootnoteConfigurationHelper(
        const css::uno::Reference< css::beans::XPropertySet> & rFootnoteSupplier,
        bool bIsEndnote);

    void exportTextMark(
        const css::uno::Reference< css::beans::XPropertySet> & xPropSet,
        const OUString& rProperty,
        const enum ::xmloff::token::XMLTokenEnum pElements[],
        bool bAutoStyles);

    void exportSoftPageBreak();

    void exportTextRange(
        const css::uno::Reference< css::text::XTextRange > & rTextRange,
        bool bAutoStyles,
        bool& rPrevCharWasSpace,
        FieldmarkType& openFieldmarkType );

    void exportListChange( const XMLTextNumRuleInfo& rPrvInfo,
                           const XMLTextNumRuleInfo& rNextInfo );

    /// check if current section or current list has changed;
    /// calls exortListChange as appropriate
    void exportListAndSectionChange(
        css::uno::Reference< css::text::XTextSection > & rOldSection,
        const css::uno::Reference< css::text::XTextSection > & rNewSection,
        const XMLTextNumRuleInfo& rOldList,
        const XMLTextNumRuleInfo& rNewList,
        bool bAutoStyles );

    /// overload for exportListAndSectionChange;
    /// takes new content rather than new section.
    void exportListAndSectionChange(
        css::uno::Reference< css::text::XTextSection > & rOldSection,
        const css::uno::Reference< css::text::XTextContent > & rNewContent,
        const XMLTextNumRuleInfo& rOldList,
        const XMLTextNumRuleInfo& rNewList,
        bool bAutoStyles );
    void exportListAndSectionChange(
        css::uno::Reference< css::text::XTextSection > & rOldSection,
        MultiPropertySetHelper& rPropSetHelper,
        sal_Int16 nTextSectionId,
        const css::uno::Reference< css::text::XTextContent > & rNewContent,
        const XMLTextNumRuleInfo& rOldList,
        const XMLTextNumRuleInfo& rNewList,
        bool bAutoStyles );

    /// export a ruby
    void exportRuby(
        const css::uno::Reference< css::beans::XPropertySet> & rPortionPropSet,
        bool bAutoStyles );

    /// export a text:meta
    void exportMeta(
        const css::uno::Reference< css::beans::XPropertySet> & i_xPortion,
        bool i_bAutoStyles, bool i_isProgress, bool & rPrevCharIsSpace);

public:

    XMLTextParagraphExport(
            SvXMLExport& rExp,
               SvXMLAutoStylePoolP & rASP
                          );
    virtual ~XMLTextParagraphExport() override;

    /// add autostyle for specified family
    void Add(
        sal_uInt16 nFamily,
        MultiPropertySetHelper& rPropSetHelper,
        const css::uno::Reference< css::beans::XPropertySet > & rPropSet );
    void Add(
        sal_uInt16 nFamily,
        const css::uno::Reference< css::beans::XPropertySet > & rPropSet,
        const XMLPropertyState** pAddState = nullptr, bool bDontSeek = false );

    /// find style name for specified family and parent
    OUString Find(
        sal_uInt16 nFamily,
        const css::uno::Reference< css::beans::XPropertySet > & rPropSet,
        const OUString& rParent,
        const XMLPropertyState** pAddState = nullptr ) const;

    static SvXMLExportPropertyMapper *CreateShapeExtPropMapper(
                                                SvXMLExport& rExport );
    static SvXMLExportPropertyMapper *CreateCharExtPropMapper(
                                                SvXMLExport& rExport);
    static SvXMLExportPropertyMapper *CreateParaExtPropMapper(
                                                SvXMLExport& rExport);
    static SvXMLExportPropertyMapper *CreateParaDefaultExtPropMapper(
                                                SvXMLExport& rExport);

    // This methods exports all (or all used) styles
    void exportTextStyles( bool bUsed, bool bProg );

    /// This method exports (text field) declarations etc.
    void exportTextDeclarations();

    /// export the (text field) declarations for a particular XText
    void exportTextDeclarations(
        const css::uno::Reference< css::text::XText > & rText );

    /// export all declarations
    void exportUsedDeclarations();

    /// Export the list of change information (enclosed by <tracked-changes>)
    /// (or the necessary automatic styles)
    void exportTrackedChanges(bool bAutoStyle);

    /// Export the list of change information (enclosed by <tracked-changes>)
    /// (or the necessary automatic styles)
    void exportTrackedChanges(const css::uno::Reference< css::text::XText > & rText,
                              bool bAutoStyle );

    /// Record tracked changes for this particular XText
    /// (empty reference stop recording)
    /// This should be used if tracked changes for e.g. footers are to
    /// be exported separately via the exportTrackedChanges(bool,
    /// Reference<XText>) method.
    void recordTrackedChangesForXText(
        const css::uno::Reference< css::text::XText > & rText );


    /// Stop recording tracked changes.
    /// This is the same as calling recordTrackedChanges(...) with an
    /// empty reference.
    void recordTrackedChangesNoXText();


    // This method exports the given OUString
    void exportCharacterData(
        const OUString& rText,
        bool& rPrevCharWasSpace);

    // This method collects all automatic styles for the given XText
    void collectTextAutoStyles(
        const css::uno::Reference< css::text::XText > & rText,
        bool bIsProgress = false,
        bool bExportParagraph = true )
    {
        exportText( rText, true, bIsProgress, bExportParagraph );
    }

    void collectTextAutoStyles(
        const css::uno::Reference< css::text::XText > & rText,
        const css::uno::Reference< css::text::XTextSection > & rBaseSection,
        bool bIsProgress )
    {
        exportText( rText, rBaseSection, true, bIsProgress, true/*bExportParagraph*/ );
    }

    // It the model implements the xAutoStylesSupplier interface, the automatic
    // styles can exported without iterating over the text portions
    void collectTextAutoStylesOptimized( bool bIsProgress );

    // This method exports all automatic styles that have been collected.
    void exportTextAutoStyles();

    void exportEvents( const css::uno::Reference< css::beans::XPropertySet > & rPropSet );

    // Implement Title/Description Elements UI (#i73249#)
    void exportTitleAndDescription( const css::uno::Reference< css::beans::XPropertySet > & rPropSet,
                                    const css::uno::Reference< css::beans::XPropertySetInfo > & rPropSetInfo );

    // This method exports the given XText
    void exportText(
        const css::uno::Reference< css::text::XText > & rText,
        bool bIsProgress = false,
        bool bExportParagraph = true, TextPNS eExtensionNS = TextPNS::ODF)
    {
        exportText( rText, false, bIsProgress, bExportParagraph, eExtensionNS );
    }

    void exportText(
        const css::uno::Reference< css::text::XText > & rText,
        const css::uno::Reference< css::text::XTextSection > & rBaseSection,
        bool bIsProgress)
    {
        exportText( rText, rBaseSection, false, bIsProgress, true/*bExportParagraph*/ );
    }

    void exportFramesBoundToPage( bool bIsProgress )
    {
        exportPageFrames( bIsProgress );
    }
    inline const XMLTextListAutoStylePool& GetListAutoStylePool() const;

    void SetBlockMode( bool bSet ) { bBlock = bSet; }
    bool IsBlockMode() const { return bBlock; }


    const rtl::Reference < SvXMLExportPropertyMapper >& GetParagraphPropertyMapper() const
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
        const css::uno::Reference< css::container::XIndexAccess> & rShapes,
        const rtl::Reference<xmloff::OFormLayerXMLExport>& xFormExport );

    SinglePropertySetInfoCache& GetCharStyleNamesPropInfoCache() { return aCharStyleNamesPropInfoCache; }

    void PushNewTextListsHelper();

    void PopTextListsHelper();

private:
        XMLTextParagraphExport(XMLTextParagraphExport &) = delete;
};

inline const XMLTextListAutoStylePool&
    XMLTextParagraphExport::GetListAutoStylePool() const
{
    return maListAutoPool;
}

inline void XMLTextParagraphExport::exportTextFrame(
        const css::uno::Reference< css::text::XTextContent > & rTextContent,
        bool bAutoStyles, bool bIsProgress, bool bExportContent,
        const css::uno::Reference< css::beans::XPropertySet > *pRangePropSet)
{
    exportAnyTextFrame( rTextContent, FrameType::Text, bAutoStyles, bIsProgress,
                        bExportContent, pRangePropSet );
}

inline void XMLTextParagraphExport::exportTextGraphic(
        const css::uno::Reference< css::text::XTextContent > & rTextContent,
        bool bAutoStyles,
        const css::uno::Reference< css::beans::XPropertySet > *pRangePropSet )
{
    exportAnyTextFrame( rTextContent, FrameType::Graphic, bAutoStyles, false,
                        true, pRangePropSet );
}

inline void XMLTextParagraphExport::exportTextEmbedded(
        const css::uno::Reference< css::text::XTextContent > & rTextContent,
        bool bAutoStyles,
        const css::uno::Reference< css::beans::XPropertySet > *pRangePropSet )
{
    exportAnyTextFrame( rTextContent, FrameType::Embedded, bAutoStyles, false,
                        true, pRangePropSet );
}

inline void XMLTextParagraphExport::exportShape(
        const css::uno::Reference< css::text::XTextContent > & rTextContent,
        bool bAutoStyles,
        const css::uno::Reference< css::beans::XPropertySet > *pRangePropSet )
{
    exportAnyTextFrame( rTextContent, FrameType::Shape, bAutoStyles, false,
                        true, pRangePropSet );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
