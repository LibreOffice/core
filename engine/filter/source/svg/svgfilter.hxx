/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <memory>

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/xml/sax/XWriter.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <editeng/flditem.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/svapp.hxx>
#include <svx/unoapi.hxx>
#include <xmloff/xmlexp.hxx>
#include <cstdio>
#include <string_view>
#include <unordered_set>
#include <unordered_map>

namespace com::sun::star::frame { class XController; }

// Placeholder tag used into the ImplWriteActions method to filter text placeholder fields
inline constexpr OUString sPlaceholderTag = u"<[:isPlaceholder:]>"_ustr;
// This tag is used for exporting a slide background made of tiled bitmaps
inline constexpr OString sTiledBackgroundTag = "SLIDE_BACKGROUND"_ostr;

class SVGExport : public SvXMLExport
{
    bool    mbIsUseTinyProfile;
    bool    mbIsUseDTDString;
    bool    mbIsEmbedFonts;
    bool    mbIsUseOpacity;
    bool    mbIsUseNativeTextDecoration;
    bool    mbIsUsePositionedCharacters;
    std::set<sal_Unicode> maEmbeddedBulletGlyphs;

public:

    SVGExport( const css::uno::Reference< css::uno::XComponentContext >& rContext,
                const css::uno::Reference< css::xml::sax::XDocumentHandler >& rxHandler,
                const css::uno::Sequence< css::beans::PropertyValue >& rFilterData );

    virtual ~SVGExport() override;

    bool IsUseTinyProfile() const { return mbIsUseTinyProfile; };
    bool IsUseDTDString() const { return mbIsUseDTDString; };
    bool IsEmbedFonts() const { return mbIsEmbedFonts; };
    bool IsUseOpacity() const { return mbIsUseOpacity; };
    bool IsUseNativeTextDecoration() const { return mbIsUseNativeTextDecoration; };
    bool IsUsePositionedCharacters() const { return mbIsUsePositionedCharacters; };

    void writeMtf( const GDIMetaFile& rMtf );

    void SetEmbeddedBulletGlyph(sal_Unicode cBullet);
    bool IsEmbeddedBulletGlyph(sal_Unicode cBullet) const;

protected:

    virtual void            ExportStyles_( bool /* bUsed */ ) override {}
    virtual void            ExportAutoStyles_() override {}
    virtual void            ExportContent_() override {}
    virtual void            ExportMasterStyles_() override {}
    virtual ErrCode         exportDoc( enum ::xmloff::token::XMLTokenEnum /* eClass */ ) override { return ERRCODE_NONE; }
};

class ObjectRepresentation
{
private:

    css::uno::Reference< css::uno::XInterface >         mxObject;
    std::unique_ptr<GDIMetaFile>    mxMtf;

public:
    ObjectRepresentation();
    ObjectRepresentation(const css::uno::Reference< css::uno::XInterface >& rxIf,
                         const GDIMetaFile& rMtf);
    ObjectRepresentation(const ObjectRepresentation& rPresentation);

    ObjectRepresentation& operator=(const ObjectRepresentation& rPresentation);

    const css::uno::Reference< css::uno::XInterface >&    GetObject() const { return mxObject; }
    bool                              HasRepresentation() const { return static_cast<bool>(mxMtf); }
    const GDIMetaFile&                GetRepresentation() const { return *mxMtf; }
};

struct PagePropertySet
{
    bool               bIsBackgroundVisible;
    bool               bAreBackgroundObjectsVisible;
    bool               bIsPageNumberFieldVisible;
    sal_Int32          nPageNumberingType;
    PagePropertySet()
        : bIsBackgroundVisible(false)
        , bAreBackgroundObjectsVisible(false)
        , bIsPageNumberFieldVisible(false)
        , nPageNumberingType(0)
    {
    }
};

struct HashUChar
{
    size_t operator()( const sal_Unicode uchar ) const { return static_cast< size_t >( uchar ); }
};

struct HashBitmap
{
    size_t operator()( const ObjectRepresentation& rObjRep ) const;
};

struct EqualityBitmap
{
    bool operator()( const ObjectRepresentation& rObjRep1,
                     const ObjectRepresentation& rObjRep2 ) const;
};

// This must match the same type definition in svgwriter.hxx
typedef std::unordered_map< BitmapChecksum, std::unique_ptr< GDIMetaFile > > MetaBitmapActionMap;

struct PatternData
{
    BitmapChecksum aBitmapChecksum;
    Point aPos;
    Size aSize;
    Size aSlideSize;
};
typedef std::map<OUString, PatternData> PatternPropertySet;

class SVGFontExport;
class SVGActionWriter;
class EditFieldInfo;

class SVGFilter : public cppu::WeakImplHelper < css::document::XFilter,
                                                 css::document::XImporter,
                                                 css::document::XExporter,
                                                 css::document::XExtendedFilterDetection, css::lang::XServiceInfo >
{
public:
    typedef std::unordered_map< css::uno::Reference< XInterface >, ObjectRepresentation >    ObjectMap;
    typedef std::unordered_set< css::uno::Reference< XInterface > >                          ObjectSet;

    typedef std::unordered_set< sal_Unicode, HashUChar >                           UCharSet;
    typedef std::unordered_map< OUString, UCharSet >                               UCharSetMap;
    typedef std::unordered_map< css::uno::Reference< css::drawing::XDrawPage >, UCharSetMap > UCharSetMapMap;

    typedef std::unordered_set< ObjectRepresentation, HashBitmap, EqualityBitmap > MetaBitmapActionSet;

private:

    /// Generally use members

    css::uno::Reference< css::uno::XComponentContext > mxContext;
    SvXMLElementExport*                 mpSVGDoc;
    rtl::Reference<SVGExport>           mpSVGExport;
    SVGFontExport*                      mpSVGFontExport;
    SVGActionWriter*                    mpSVGWriter;
    bool                                mbSinglePage;
    sal_Int32                           mnVisiblePage;
    ObjectMap*                          mpObjects;
    css::uno::Reference< css::lang::XComponent > mxSrcDoc;
    css::uno::Reference< css::lang::XComponent > mxDstDoc;
    // #i124608# explicit ShapeSelection for export when export of the selection is wanted
    css::uno::Reference< css::drawing::XShapes > maShapeSelection;
    bool                                mbExportShapeSelection;
    css::uno::Sequence< css::beans::PropertyValue > maFilterData;
    css::uno::Reference< css::drawing::XDrawPage > mxDefaultPage;
    css::uno::Reference<css::task::XStatusIndicator> mxStatusIndicator;
    std::vector< css::uno::Reference< css::drawing::XDrawPage > > mSelectedPages;
    bool                                mbIsPreview;
    bool                                mbShouldCompress;

    bool                                mbWriterFilter;
    bool                                mbCalcFilter;
    bool                                mbImpressFilter;


    /// Impress / draw only members

    SdrPage*                            mpDefaultSdrPage;
    bool                                mbPresentation;
    PagePropertySet                     mVisiblePagePropSet;
    OUString                            msClipPathId;
    UCharSetMapMap                      mTextFieldCharSets;
    css::uno::Reference< css::drawing::XDrawPage > mCreateOjectsCurrentMasterPage;
    std::unordered_map< css::uno::Reference< css::drawing::XDrawPage >, OUString >
                                        mTextShapeIdListMap;
    MetaBitmapActionSet                 mEmbeddedBitmapActionSet;
    ObjectMap                           mEmbeddedBitmapActionMap;
    MetaBitmapActionMap                 maBitmapActionMap;
    PatternPropertySet                  maPatterProps;
    std::vector< css::uno::Reference< css::drawing::XDrawPage > > mMasterPageTargets;

    Link<EditFieldInfo*,void>           maOldFieldHdl;
    Link<EditFieldInfo*,void>           maNewFieldHdl;

    /// @throws css::uno::RuntimeException
    bool                            implExport( const css::uno::Sequence< css::beans::PropertyValue >& rDescriptor );
    bool                            implExportImpressOrDraw( const css::uno::Reference< css::io::XOutputStream >& rxOStm );
    bool                            implExportWriterOrCalc( const css::uno::Reference< css::io::XOutputStream >& rxOStm );
    bool                            implExportWriterTextGraphic( const css::uno::Reference< css::view::XSelectionSupplier >& xSelectionSupplier );

    static css::uno::Reference< css::xml::sax::XWriter > implCreateExportDocumentHandler( const css::uno::Reference< css::io::XOutputStream >& rxOStm );

    void                            implGetPagePropSet( const css::uno::Reference< css::drawing::XDrawPage > & rxPage );
    void                            implGenerateMetaData();
    void                            implExportTextShapeIndex();
    void                            implEmbedBulletGlyphs();
    void                            implEmbedBulletGlyph( sal_Unicode cBullet, const OUString & sPathData );
    void                            implExportTextEmbeddedBitmaps();
    void                            implExportBackgroundBitmaps();
    void                            implExportTiledBackground();
    void                            implGenerateScript();

    bool                            implExportDocument();
    void                            implExportDocumentHeaderImpressOrDraw(sal_Int32 nDocX, sal_Int32 nDocY,
                                                                          sal_Int32 nDocWidth, sal_Int32 nDocHeight);
    void                            implExportDocumentHeaderWriterOrCalc(sal_Int32 nDocX, sal_Int32 nDocY,
                                                                         sal_Int32 nDocWidth, sal_Int32 nDocHeight);
    void                            implExportAnimations();

    bool                            implExportMasterPages( const std::vector< css::uno::Reference< css::drawing::XDrawPage > >& rxPages,
                                                               sal_Int32 nFirstPage, sal_Int32 nLastPage );
    void                            implExportDrawPages( const std::vector< css::uno::Reference< css::drawing::XDrawPage > >& rxPages,
                                                             sal_Int32 nFirstPage, sal_Int32 nLastPage );
    bool                            implExportPage( std::u16string_view sPageId,
                                                        const css::uno::Reference< css::drawing::XDrawPage > & rxPage,
                                                        const css::uno::Reference< css::drawing::XShapes > & xShapes,
                                                        bool bMaster );

    bool                            implExportShapes( const css::uno::Reference< css::drawing::XShapes >& rxShapes,
                                                          bool bMaster );
    bool                            implExportShape( const css::uno::Reference< css::drawing::XShape >& rxShape,
                                                         bool bMaster );

    bool                            implCreateObjects();
    bool                            implCreateObjectsFromShapes( const css::uno::Reference< css::drawing::XDrawPage > & rxPage, const css::uno::Reference< css::drawing::XShapes >& rxShapes );
    bool                            implCreateObjectsFromShape( const css::uno::Reference< css::drawing::XDrawPage > & rxPage, const css::uno::Reference< css::drawing::XShape >& rxShape );
    void                            implCreateObjectsFromBackground( const css::uno::Reference< css::drawing::XDrawPage >& rxMasterPage );

    static OUString                 implGetClassFromShape( const css::uno::Reference< css::drawing::XShape >& rxShape );
    void                            implRegisterInterface( const css::uno::Reference< XInterface >& rxIf );
    const OUString &                implGetValidIDFromInterface( const css::uno::Reference< XInterface >& rxIf );
    static OUString                 implGetInterfaceName( const css::uno::Reference< XInterface >& rxIf );
    bool                            implLookForFirstVisiblePage();
    static css::uno::Any            implSafeGetPagePropSet( const OUString & sPropertyName,
                                                                const css::uno::Reference< css::beans::XPropertySet > & rxPropSet,
                                                                const css::uno::Reference< css::beans::XPropertySetInfo > & rxPropSetInfo );
    DECL_LINK( CalcFieldHdl, EditFieldInfo*, void );

    bool filterImpressOrDraw( const css::uno::Sequence< css::beans::PropertyValue >& rDescriptor );
    bool filterWriterOrCalc( const css::uno::Sequence< css::beans::PropertyValue >& rDescriptor );

    css::uno::Reference<css::frame::XController> getSourceController() const;
    css::uno::Reference<css::frame::XController> fillDrawImpressSelectedPages();

protected:

    // XFilter
    virtual bool SAL_CALL filter( const css::uno::Sequence< css::beans::PropertyValue >& rDescriptor ) override;
    virtual void SAL_CALL cancel( ) override;

    // XImporter
    virtual void SAL_CALL setTargetDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) override;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) override;

    // XExtendedFilterDetection
    virtual OUString SAL_CALL detect( css::uno::Sequence< css::beans::PropertyValue >& io_rDescriptor ) override;

public:

    explicit SVGFilter( const css::uno::Reference< css::uno::XComponentContext >& rxCtx );
    virtual    ~SVGFilter() override;

    //  XServiceInfo
    virtual bool SAL_CALL supportsService(const OUString& sServiceName) override;
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
