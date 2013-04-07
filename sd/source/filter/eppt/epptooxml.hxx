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

#ifndef EPPT_POWERPOINT_EXPORT_HXX
#define EPPT_POWERPOINT_EXPORT_HXX

#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/zipstorage.hxx>
#include <oox/vml/vmldrawing.hxx>
#include <oox/export/shapes.hxx>
#include <comphelper/stl_types.hxx>
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

    ~PowerPointExport();

    // from FilterBase
    virtual bool importDocument() throw();
    virtual bool exportDocument() throw();

    // only needed for import, leave them empty, refactor later XmlFilterBase to export and import base?
    virtual sal_Int32 getSchemeClr( sal_Int32 /* nColorSchemeToken */ ) const { return 0; }
    virtual oox::vml::Drawing* getVmlDrawing() { return NULL; }
    virtual const oox::drawingml::Theme* getCurrentTheme() const { return NULL; }
    virtual const oox::drawingml::table::TableStyleListPtr getTableStyles() { return oox::drawingml::table::TableStyleListPtr(); }
    virtual oox::drawingml::chart::ChartConverter* getChartConverter() { return NULL; }

    static const char* GetSideDirection( sal_uInt8 nDirection );
    static const char* GetCornerDirection( sal_uInt8 nDirection );
    static const char* Get8Direction( sal_uInt8 nDirection );
    static       int   GetPPTXLayoutId( int nOffset );

protected:

    virtual void ImplWriteSlide( sal_uInt32 nPageNum, sal_uInt32 nMasterNum, sal_uInt16 nMode,
                                 sal_Bool bHasBackground, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXBackgroundPropSet );
    virtual void ImplWriteNotes( sal_uInt32 nPageNum );
    virtual void ImplWriteSlideMaster( sal_uInt32 nPageNum, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXBackgroundPropSet );
    virtual void ImplWriteLayout( sal_Int32 nOffset, sal_uInt32 nMasterNum );
    void ImplWritePPTXLayout( sal_Int32 nOffset, sal_uInt32 nMasterNum );
    void WriteTheme( sal_Int32 nThemeNum );

    virtual sal_Bool ImplCreateDocument();
    virtual sal_Bool ImplCreateMainNotes();
    virtual ::oox::ole::VbaProject* implCreateVbaProject() const;
    sal_Bool WriteNotesMaster();

    void WriteAnimateTo( ::sax_fastparser::FSHelperPtr pFS, const ::com::sun::star::uno::Any aValue, const OUString& rAttributeName );
    void WriteAnimateValues( ::sax_fastparser::FSHelperPtr pFS, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimate >& rXAnimate );
    void WriteAnimationCondition( ::sax_fastparser::FSHelperPtr pFS, ::com::sun::star::uno::Any& rAny, sal_Bool bWriteEvent, sal_Bool bMainSeqChild );
    void WriteAnimationCondition( ::sax_fastparser::FSHelperPtr pFS, const char* pDelay, const char* pEvent, double fDelay, sal_Bool bHasFDelay );
    void WriteAnimations( ::sax_fastparser::FSHelperPtr pFS );
    void WriteAnimationAttributeName( ::sax_fastparser::FSHelperPtr pFS, const OUString& rAttributeName );
    void WriteAnimationNode( ::sax_fastparser::FSHelperPtr pFS, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& rXNode, sal_Bool bMainSeqChild );
    void WriteAnimationNodeAnimate( ::sax_fastparser::FSHelperPtr pFS, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& rXNode, sal_Int32 nXmlNodeType, sal_Bool bMainSeqChild );
    void WriteAnimationNodeAnimateInside( ::sax_fastparser::FSHelperPtr pFS, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& rXNode, sal_Bool bMainSeqChild, sal_Bool bSimple );
    void WriteAnimationNodeSeq( ::sax_fastparser::FSHelperPtr pFS, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& rXNode, sal_Int32 nXmlNodeType, sal_Bool bMainSeqChild );
    void WriteAnimationNodeEffect( ::sax_fastparser::FSHelperPtr pFS, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& rXNode, sal_Int32 nXmlNodeType, sal_Bool bMainSeqChild );
    void WriteAnimationNodeCommonPropsStart( ::sax_fastparser::FSHelperPtr pFS, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& rXNode, sal_Bool bSingle, sal_Bool bMainSeqChild );
    void WriteAnimationProperty( ::sax_fastparser::FSHelperPtr pFS, const ::com::sun::star::uno::Any& rAny );
    void WriteAnimationTarget( ::sax_fastparser::FSHelperPtr pFS, ::com::sun::star::uno::Any aTarget );
    bool WriteComments( sal_uInt32 nPageNum );
    void ImplWriteBackground( ::sax_fastparser::FSHelperPtr pFS, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXBackgroundPropSet );
    void WriteTransition( ::sax_fastparser::FSHelperPtr pFS );

    sal_Int32 GetLayoutFileId( sal_Int32 nOffset, sal_uInt32 nMasterNum );

    // shapes
    void WriteShapeTree( ::sax_fastparser::FSHelperPtr pFS, PageType ePageType, sal_Bool bMaster );

    sal_uInt32 GetNewSlideId() { return mnSlideIdMax ++; }
    sal_uInt32 GetNewSlideMasterId() { return mnSlideMasterIdMax ++; }
    sal_Int32 GetAuthorIdAndLastIndex( OUString sAuthor, sal_Int32& nLastIndex );

private:
    void AddLayoutIdAndRelation( ::sax_fastparser::FSHelperPtr pFS, sal_Int32 nLayoutFileId );

    virtual OUString implGetImplementationName() const;

    ::boost::shared_ptr< ::oox::drawingml::chart::ChartConverter > mxChartConv;

    ::sax_fastparser::FSHelperPtr mPresentationFS;

    LayoutInfo mLayoutInfo[EPP_LAYOUT_SIZE];
    std::vector< ::sax_fastparser::FSHelperPtr > mpSlidesFSArray;
    std::vector< ::sax_fastparser::FSHelperPtr > mpMasterFSArray;
    sal_Int32 mnLayoutFileIdMax;

    sal_uInt32 mnSlideIdMax;
    sal_uInt32 mnSlideMasterIdMax;
    sal_uInt32 mnAnimationNodeIdMax;

    sal_Bool mbCreateNotes;

    static sal_Int32 nStyleLevelToken[5];

    ::oox::drawingml::ShapeExport::ShapeHashMap maShapeMap;

    struct AuthorComments {
        sal_Int32 nId;
        sal_Int32 nLastIndex;
    };
    typedef ::boost::unordered_map< OUString, struct AuthorComments, OUStringHash, comphelper::UStringEqual > AuthorsMap;
    AuthorsMap maAuthors;

    void WriteAuthors();
};

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
