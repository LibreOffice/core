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
#include <oox/helper/zipstorage.hxx>
#include <oox/vml/vmldrawing.hxx>
#include <oox/export/shapes.hxx>
#include "epptbase.hxx"

namespace com { namespace sun { namespace star {
    namespace animations {
        class XAnimate;
        class XAnimationNode;
    }
}}}

namespace oox {
    namespace drawingml {
        class ShapeExport;
    }
namespace core {

class PowerPointShapeExport;

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

class PowerPointExport : public XmlFilterBase, public PPTWriterBase
{
    friend class PowerPointShapeExport;
public:

    PowerPointExport( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > & rxCtxt  );

    virtual ~PowerPointExport();

    // from FilterBase
    virtual bool importDocument() throw() SAL_OVERRIDE;
    virtual bool exportDocument() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // only needed for import, leave them empty, refactor later XmlFilterBase to export and import base?
    static sal_Int32 getSchemeClr( sal_Int32 /* nColorSchemeToken */ ) { return 0; }
    virtual oox::vml::Drawing* getVmlDrawing() SAL_OVERRIDE { return NULL; }
    virtual const oox::drawingml::Theme* getCurrentTheme() const SAL_OVERRIDE { return NULL; }
    virtual const oox::drawingml::table::TableStyleListPtr getTableStyles() SAL_OVERRIDE { return oox::drawingml::table::TableStyleListPtr(); }
    virtual oox::drawingml::chart::ChartConverter* getChartConverter() SAL_OVERRIDE { return NULL; }

    static const char* GetSideDirection( sal_uInt8 nDirection );
    static const char* GetCornerDirection( sal_uInt8 nDirection );
    static const char* Get8Direction( sal_uInt8 nDirection );
    static       int   GetPPTXLayoutId( int nOffset );

protected:

    virtual void ImplWriteSlide( sal_uInt32 nPageNum, sal_uInt32 nMasterNum, sal_uInt16 nMode,
                                 bool bHasBackground, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXBackgroundPropSet ) SAL_OVERRIDE;
    virtual void ImplWriteNotes( sal_uInt32 nPageNum ) SAL_OVERRIDE;
    virtual void ImplWriteSlideMaster( sal_uInt32 nPageNum, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXBackgroundPropSet ) SAL_OVERRIDE;
    virtual void ImplWriteLayout( sal_Int32 nOffset, sal_uInt32 nMasterNum ) SAL_OVERRIDE;
    void ImplWritePPTXLayout( sal_Int32 nOffset, sal_uInt32 nMasterNum );
    void WriteTheme( sal_Int32 nThemeNum );

    virtual bool ImplCreateDocument() SAL_OVERRIDE;
    virtual bool ImplCreateMainNotes() SAL_OVERRIDE;
    virtual ::oox::ole::VbaProject* implCreateVbaProject() const SAL_OVERRIDE;
    bool WriteNotesMaster();

    static void WriteAnimateTo( ::sax_fastparser::FSHelperPtr pFS, const css::uno::Any& rValue, const OUString& rAttributeName );
    static void WriteAnimateValues( ::sax_fastparser::FSHelperPtr pFS, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimate >& rXAnimate );
    static void WriteAnimationCondition( ::sax_fastparser::FSHelperPtr pFS, ::com::sun::star::uno::Any& rAny, bool bWriteEvent, bool bMainSeqChild );
    static void WriteAnimationCondition( ::sax_fastparser::FSHelperPtr pFS, const char* pDelay, const char* pEvent, double fDelay, bool bHasFDelay );
    void WriteAnimations( ::sax_fastparser::FSHelperPtr pFS );
    static void WriteAnimationAttributeName( ::sax_fastparser::FSHelperPtr pFS, const OUString& rAttributeName );
    void WriteAnimationNode( ::sax_fastparser::FSHelperPtr pFS, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& rXNode, bool bMainSeqChild );
    void WriteAnimationNodeAnimate( ::sax_fastparser::FSHelperPtr pFS, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& rXNode, sal_Int32 nXmlNodeType, bool bMainSeqChild );
    void WriteAnimationNodeAnimateInside( ::sax_fastparser::FSHelperPtr pFS, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& rXNode, bool bMainSeqChild, bool bSimple );
    void WriteAnimationNodeSeq( ::sax_fastparser::FSHelperPtr pFS, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& rXNode, sal_Int32 nXmlNodeType, bool bMainSeqChild );
    void WriteAnimationNodeEffect( ::sax_fastparser::FSHelperPtr pFS, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& rXNode, sal_Int32 nXmlNodeType, bool bMainSeqChild );
    void WriteAnimationNodeCommonPropsStart( ::sax_fastparser::FSHelperPtr pFS, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& rXNode, bool bSingle, bool bMainSeqChild );
    static void WriteAnimationProperty( ::sax_fastparser::FSHelperPtr pFS, const ::com::sun::star::uno::Any& rAny );
    void WriteAnimationTarget( ::sax_fastparser::FSHelperPtr pFS, const css::uno::Any& rTarget );
    bool WriteComments( sal_uInt32 nPageNum );
    void ImplWriteBackground( ::sax_fastparser::FSHelperPtr pFS, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXBackgroundPropSet );
    void WriteTransition( ::sax_fastparser::FSHelperPtr pFS );

    sal_Int32 GetLayoutFileId( sal_Int32 nOffset, sal_uInt32 nMasterNum );

    // shapes
    void WriteShapeTree( ::sax_fastparser::FSHelperPtr pFS, PageType ePageType, bool bMaster );

    sal_uInt32 GetNewSlideId() { return mnSlideIdMax ++; }
    sal_uInt32 GetNewSlideMasterId() { return mnSlideMasterIdMax ++; }
    sal_Int32 GetAuthorIdAndLastIndex( const OUString& sAuthor, sal_Int32& nLastIndex );

private:
    // Write docProps/core.xml and docprops/custom.xml and docprops/app.xml
    void writeDocumentProperties();

    void AddLayoutIdAndRelation( ::sax_fastparser::FSHelperPtr pFS, sal_Int32 nLayoutFileId );

    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    std::shared_ptr< ::oox::drawingml::chart::ChartConverter > mxChartConv;

    ::sax_fastparser::FSHelperPtr mPresentationFS;

    LayoutInfo mLayoutInfo[EPP_LAYOUT_SIZE];
    std::vector< ::sax_fastparser::FSHelperPtr > mpSlidesFSArray;
    sal_Int32 mnLayoutFileIdMax;

    sal_uInt32 mnSlideIdMax;
    sal_uInt32 mnSlideMasterIdMax;
    sal_uInt32 mnAnimationNodeIdMax;

    bool mbCreateNotes;

    static sal_Int32 nStyleLevelToken[5];

    ::oox::drawingml::ShapeExport::ShapeHashMap maShapeMap;

    struct AuthorComments {
        sal_Int32 nId;
        sal_Int32 nLastIndex;
    };
    typedef std::unordered_map< OUString, struct AuthorComments, OUStringHash > AuthorsMap;
    AuthorsMap maAuthors;

    void WriteAuthors();
};

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
