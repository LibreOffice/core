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

#ifndef INCLUDED_FILTER_SOURCE_SVG_SVGFILTER_HXX
#define INCLUDED_FILTER_SOURCE_SVG_SVGFILTER_HXX

#include <memory>

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
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

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml::sax;

#define SVG_EXPORT_ALLPAGES ((sal_Int32)-1)


// Placeholder tag used into the ImplWriteActions method to filter text placeholder fields
const OUString sPlaceholderTag( "<[:isPlaceholder:]>" );

class SVGExport : public SvXMLExport
{
    bool    mbIsUseTinyProfile;
    bool    mbIsUseDTDString;
    bool    mbIsEmbedFonts;
    bool    mbIsUseOpacity;
    bool    mbIsUseNativeTextDecoration;
    bool    mbIsUsePositionedCharacters;

public:

    SVGExport( const css::uno::Reference< css::uno::XComponentContext >& rContext,
                const Reference< XDocumentHandler >& rxHandler,
                const Sequence< PropertyValue >& rFilterData );

    virtual ~SVGExport() override;

    bool IsUseTinyProfile() const { return mbIsUseTinyProfile; };
    bool IsUseDTDString() const { return mbIsUseDTDString; };
    bool IsEmbedFonts() const { return mbIsEmbedFonts; };
    bool IsUseOpacity() const { return mbIsUseOpacity; };
    bool IsUseNativeTextDecoration() const { return mbIsUseNativeTextDecoration; };
    bool IsUsePositionedCharacters() const { return mbIsUsePositionedCharacters; };

    void writeMtf( const GDIMetaFile& rMtf );

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

    Reference< XInterface >         mxObject;
    std::unique_ptr<GDIMetaFile>    mxMtf;

public:
    ObjectRepresentation();
    ObjectRepresentation(const Reference< XInterface >& rxIf,
                         const GDIMetaFile& rMtf);
    ObjectRepresentation(const ObjectRepresentation& rPresentation);

    ObjectRepresentation& operator=(const ObjectRepresentation& rPresentation);

    const Reference< XInterface >&    GetObject() const { return mxObject; }
    bool                              HasRepresentation() const { return static_cast<bool>(mxMtf); }
    const GDIMetaFile&                GetRepresentation() const { return *mxMtf; }
};

struct PagePropertySet
{
    bool               bIsBackgroundVisible;
    bool               bAreBackgroundObjectsVisible;
    bool               bIsPageNumberFieldVisible;
    bool               bIsDateTimeFieldVisible;
    bool               bIsFooterFieldVisible;
    bool               bIsHeaderFieldVisible;
    sal_Int32          nPageNumberingType;
    bool               bIsDateTimeFieldFixed;
    SvxDateFormat      nDateTimeFormat;
    PagePropertySet()
        : bIsBackgroundVisible(false)
        , bAreBackgroundObjectsVisible(false)
        , bIsPageNumberFieldVisible(false)
        , bIsDateTimeFieldVisible(false)
        , bIsFooterFieldVisible(false)
        , bIsHeaderFieldVisible(false)
        , nPageNumberingType(0)
        , bIsDateTimeFieldFixed(false)
        , nDateTimeFormat(SvxDateFormat::AppDefault)
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

class SVGFontExport;
class SVGActionWriter;
class EditFieldInfo;

class SVGFilter : public cppu::WeakImplHelper < XFilter,
                                                 XImporter,
                                                 XExporter,
                                                 XExtendedFilterDetection, XServiceInfo >
{
public:
    typedef std::unordered_map< Reference< XInterface >, ObjectRepresentation >    ObjectMap;
    typedef std::unordered_set< Reference< XInterface > >                          ObjectSet;

    typedef std::unordered_set< sal_Unicode, HashUChar >                           UCharSet;
    typedef std::unordered_map< OUString, UCharSet >                               UCharSetMap;
    typedef std::unordered_map< Reference< XInterface >, UCharSetMap >             UCharSetMapMap;

    typedef std::unordered_set< ObjectRepresentation, HashBitmap, EqualityBitmap > MetaBitmapActionSet;

private:

    /// Generally use members

    Reference< XComponentContext >      mxContext;
    SvXMLElementExport*                 mpSVGDoc;
    rtl::Reference<SVGExport>           mpSVGExport;
    SVGFontExport*                      mpSVGFontExport;
    SVGActionWriter*                    mpSVGWriter;
    bool                                mbSinglePage;
    sal_Int32                           mnVisiblePage;
    ObjectMap*                          mpObjects;
    Reference< XComponent >             mxSrcDoc;
    Reference< XComponent >             mxDstDoc;
    // #i124608# explicit ShapeSelection for export when export of the selection is wanted
    Reference< css::drawing::XShapes >  maShapeSelection;
    bool                                mbExportShapeSelection;
    Sequence< PropertyValue >           maFilterData;
    Reference< css::drawing::XDrawPage > mxDefaultPage;
    std::vector< Reference< css::drawing::XDrawPage > > mSelectedPages;

    bool                                mbWriterFilter;
    bool                                mbCalcFilter;
    bool                                mbImpressFilter;


    /// Impress / draw only members

    SdrPage*                            mpDefaultSdrPage;
    bool                                mbPresentation;
    PagePropertySet                     mVisiblePagePropSet;
    OUString                            msClipPathId;
    UCharSetMapMap                      mTextFieldCharSets;
    Reference< XInterface >             mCreateOjectsCurrentMasterPage;
    std::unordered_map< Reference< XInterface >, OUString >
                                        mTextShapeIdListMap;
    MetaBitmapActionSet                 mEmbeddedBitmapActionSet;
    ObjectMap                           mEmbeddedBitmapActionMap;
    std::vector< Reference< css::drawing::XDrawPage > > mMasterPageTargets;

    Link<EditFieldInfo*,void>           maOldFieldHdl;
    Link<EditFieldInfo*,void>           maNewFieldHdl;

    /// @throws css::uno::RuntimeException
    bool                            implExport( const Sequence< PropertyValue >& rDescriptor );
    bool                            implExportImpressOrDraw( const Reference< XOutputStream >& rxOStm );
    bool                            implExportWriterOrCalc( const Reference< XOutputStream >& rxOStm );
    bool                            implExportWriterTextGraphic( const Reference< view::XSelectionSupplier >& xSelectionSupplier );

    static Reference< XWriter >     implCreateExportDocumentHandler( const Reference< XOutputStream >& rxOStm );

    void                            implGetPagePropSet( const Reference< css::drawing::XDrawPage > & rxPage );
    void                            implGenerateMetaData();
    void                            implExportTextShapeIndex();
    void                            implEmbedBulletGlyphs();
    void                            implEmbedBulletGlyph( sal_Unicode cBullet, const OUString & sPathData );
    void                            implExportTextEmbeddedBitmaps();
    void                            implGenerateScript();

    bool                            implExportDocument();
    void                            implExportDocumentHeaderImpressOrDraw(sal_Int32 nDocX, sal_Int32 nDocY,
                                                                          sal_Int32 nDocWidth, sal_Int32 nDocHeight);
    void                            implExportDocumentHeaderWriterOrCalc(sal_Int32 nDocX, sal_Int32 nDocY,
                                                                         sal_Int32 nDocWidth, sal_Int32 nDocHeight);
    void                            implExportAnimations();

    bool                            implExportMasterPages( const std::vector< Reference< css::drawing::XDrawPage > >& rxPages,
                                                               sal_Int32 nFirstPage, sal_Int32 nLastPage );
    void                            implExportDrawPages( const std::vector< Reference< css::drawing::XDrawPage > >& rxPages,
                                                             sal_Int32 nFirstPage, sal_Int32 nLastPage );
    bool                            implExportPage( std::u16string_view sPageId,
                                                        const Reference< css::drawing::XDrawPage > & rxPage,
                                                        const Reference< css::drawing::XShapes > & xShapes,
                                                        bool bMaster );

    bool                            implExportShapes( const Reference< css::drawing::XShapes >& rxShapes,
                                                          bool bMaster );
    bool                            implExportShape( const Reference< css::drawing::XShape >& rxShape,
                                                         bool bMaster );

    bool                            implCreateObjects();
    bool                            implCreateObjectsFromShapes( const Reference< css::drawing::XDrawPage > & rxPage, const Reference< css::drawing::XShapes >& rxShapes );
    bool                            implCreateObjectsFromShape( const Reference< css::drawing::XDrawPage > & rxPage, const Reference< css::drawing::XShape >& rxShape );
    void                            implCreateObjectsFromBackground( const Reference< css::drawing::XDrawPage >& rxMasterPage );

    static OUString                 implGetClassFromShape( const Reference< css::drawing::XShape >& rxShape );
    void                            implRegisterInterface( const Reference< XInterface >& rxIf );
    const OUString &                implGetValidIDFromInterface( const Reference< XInterface >& rxIf );
    static OUString                 implGetInterfaceName( const Reference< XInterface >& rxIf );
    bool                            implLookForFirstVisiblePage();
    static Any                      implSafeGetPagePropSet( const OUString & sPropertyName,
                                                                const Reference< XPropertySet > & rxPropSet,
                                                                const Reference< XPropertySetInfo > & rxPropSetInfo );
    DECL_LINK( CalcFieldHdl, EditFieldInfo*, void );

    bool filterImpressOrDraw( const Sequence< PropertyValue >& rDescriptor );
    bool filterWriterOrCalc( const Sequence< PropertyValue >& rDescriptor );

protected:

    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& rDescriptor ) override;
    virtual void SAL_CALL cancel( ) override;

    // XImporter
    virtual void SAL_CALL setTargetDocument( const Reference< XComponent >& xDoc ) override;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const Reference< XComponent >& xDoc ) override;

    // XExtendedFilterDetection
    virtual OUString SAL_CALL detect( Sequence< PropertyValue >& io_rDescriptor ) override;

public:

    explicit SVGFilter( const Reference< XComponentContext >& rxCtx );
    virtual    ~SVGFilter() override;

    //  XServiceInfo
    virtual sal_Bool SAL_CALL supportsService(const OUString& sServiceName) override;
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

#endif // INCLUDED_FILTER_SOURCE_SVG_SVGFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
