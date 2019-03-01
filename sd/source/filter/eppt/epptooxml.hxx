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

#ifndef INCLUDED_SD_SOURCE_FILTER_EPPT_EPPTOOXML_HXX
#define INCLUDED_SD_SOURCE_FILTER_EPPT_EPPTOOXML_HXX

#include <oox/core/xmlfilterbase.hxx>
#include <oox/vml/vmldrawing.hxx>
#include <oox/export/shapes.hxx>
#include "epptbase.hxx"

using ::sax_fastparser::FSHelperPtr;

namespace oox {

namespace core {

struct LayoutInfo
{
    std::vector< sal_Int32 > mnFileIdArray;
};

enum PlaceholderType
{
    None,
    SlideImage,
    Notes,
    Header,
    Footer,
    SlideNumber,
    DateAndTime,
    Outliner,
    Title,
    Subtitle
};

class PowerPointExport final : public XmlFilterBase, public PPTWriterBase
{
    friend class PowerPointShapeExport;
public:

    PowerPointExport(const css::uno::Reference<css::uno::XComponentContext> & rContext, const css::uno::Sequence<css::uno::Any>& rArguments);

    virtual ~PowerPointExport() override;

    // from FilterBase
    virtual bool importDocument() throw() override;
    virtual bool exportDocument() override;

    // only needed for import, leave them empty, refactor later XmlFilterBase to export and import base?
    virtual oox::vml::Drawing* getVmlDrawing() override { return nullptr; }
    virtual const oox::drawingml::Theme* getCurrentTheme() const override { return nullptr; }
    virtual const oox::drawingml::table::TableStyleListPtr getTableStyles() override { return oox::drawingml::table::TableStyleListPtr(); }
    virtual oox::drawingml::chart::ChartConverter* getChartConverter() override { return nullptr; }

    static const char* GetSideDirection( sal_uInt8 nDirection );
    static const char* GetCornerDirection( sal_uInt8 nDirection );
    static const char* Get8Direction( sal_uInt8 nDirection );
    static       int   GetPPTXLayoutId( int nOffset );

    sal_Int32 GetShapeID(const css::uno::Reference<css::drawing::XShape>& rXShape);
    sal_Int32 GetNextAnimationNodeID();

    void embedEffectAudio(const FSHelperPtr& pFS, const OUString& sUrl, OUString& sRelId, OUString& sName);

private:

    virtual void ImplWriteSlide( sal_uInt32 nPageNum, sal_uInt32 nMasterNum, sal_uInt16 nMode,
                                 bool bHasBackground, css::uno::Reference< css::beans::XPropertySet > const & aXBackgroundPropSet ) override;
    virtual void ImplWriteNotes( sal_uInt32 nPageNum ) override;
    virtual void ImplWriteSlideMaster( sal_uInt32 nPageNum, css::uno::Reference< css::beans::XPropertySet > const & aXBackgroundPropSet ) override;
    void ImplWritePPTXLayout( sal_Int32 nOffset, sal_uInt32 nMasterNum );
    bool WriteColorSchemes(const FSHelperPtr& pFS, const OUString& rThemePath);
    static void WriteDefaultColorSchemes(const FSHelperPtr& pFS);
    void WriteTheme( sal_Int32 nThemeNum );

    virtual bool ImplCreateDocument() override;
    virtual bool ImplCreateMainNotes() override;
    virtual ::oox::ole::VbaProject* implCreateVbaProject() const override;
    void WriteNotesMaster();

    bool WriteComments( sal_uInt32 nPageNum );
    void ImplWriteBackground( const ::sax_fastparser::FSHelperPtr& pFS, const css::uno::Reference< css::beans::XPropertySet >& aXBackgroundPropSet );
    void WriteTransition( const ::sax_fastparser::FSHelperPtr& pFS );

    sal_Int32 GetLayoutFileId( sal_Int32 nOffset, sal_uInt32 nMasterNum );

    // shapes
    void WriteShapeTree( const ::sax_fastparser::FSHelperPtr& pFS, PageType ePageType, bool bMaster );

    sal_uInt32 GetNewSlideId() { return mnSlideIdMax ++; }
    sal_uInt32 GetNewSlideMasterId() { return mnSlideMasterIdMax ++; }
    sal_Int32 GetAuthorIdAndLastIndex( const OUString& sAuthor, sal_Int32& nLastIndex );

    // Write docProps/core.xml and docprops/custom.xml and docprops/app.xml
    void writeDocumentProperties();

    void AddLayoutIdAndRelation( const ::sax_fastparser::FSHelperPtr& pFS, sal_Int32 nLayoutFileId );

    virtual OUString SAL_CALL getImplementationName() override;

    /// Should we export as .pptm, ie. do we contain macros?
    bool mbPptm;

    // Export as a template
    bool mbExportTemplate;

    ::sax_fastparser::FSHelperPtr mPresentationFS;

    LayoutInfo mLayoutInfo[EPP_LAYOUT_SIZE];
    std::vector< ::sax_fastparser::FSHelperPtr > mpSlidesFSArray;
    sal_Int32 mnLayoutFileIdMax;

    sal_uInt32 mnSlideIdMax;
    sal_uInt32 mnSlideMasterIdMax;
    sal_uInt32 mnAnimationNodeIdMax;

    bool mbCreateNotes;

    ::oox::drawingml::ShapeExport::ShapeHashMap maShapeMap;

    struct AuthorComments {
        sal_Int32 nId;
        sal_Int32 nLastIndex;
    };
    typedef std::unordered_map< OUString, struct AuthorComments > AuthorsMap;
    AuthorsMap maAuthors;

    void WriteAuthors();

    /// If this is PPTM, output the VBA stream.
    void WriteVBA();
};

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
