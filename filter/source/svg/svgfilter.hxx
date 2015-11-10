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

#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/xml/sax/XWriter.hpp>

#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <rtl/process.h>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <svtools/grfmgr.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdxcgv.hxx>
#include <svx/svdobj.hxx>
#include <xmloff/xmlexp.hxx>
#include <cstdio>
#include <unordered_set>
#include <unordered_map>

using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

using namespace ::std;

#define SVG_EXPORT_ALLPAGES ((sal_Int32)-1)


// Placeholder tag used into the ImplWriteActions method to filter text placeholder fields
static const OUString sPlaceholderTag( "<[:isPlaceholder:]>" );

class SVGExport : public SvXMLExport
{
    typedef ::std::list< ::basegfx::B2DPolyPolygon > B2DPolyPolygonList;

    bool    mbIsUseTinyProfile;
    bool    mbIsEmbedFonts;
    bool    mbIsUseOpacity;
    bool    mbIsUseNativeTextDecoration;
    bool    mbIsUsePositionedCharacters;

public:

    SVGExport( const css::uno::Reference< css::uno::XComponentContext >& rContext,
                const Reference< XDocumentHandler >& rxHandler,
                const Sequence< PropertyValue >& rFilterData );

    virtual ~SVGExport();

    bool IsUseTinyProfile() const { return mbIsUseTinyProfile; };
    bool IsEmbedFonts() const { return mbIsEmbedFonts; };
    bool IsUseOpacity() const { return mbIsUseOpacity; };
    bool IsUseNativeTextDecoration() const { return mbIsUseNativeTextDecoration; };
    bool IsUsePositionedCharacters() const { return mbIsUsePositionedCharacters; };

    void writeMtf( const GDIMetaFile& rMtf );

protected:

    virtual void            _ExportStyles( bool /* bUsed */ ) override {}
    virtual void            _ExportAutoStyles() override {}
    virtual void            _ExportContent() override {}
    virtual void            _ExportMasterStyles() override {}
    virtual sal_uInt32        exportDoc( enum ::xmloff::token::XMLTokenEnum /* eClass */ ) override { return 0; }

private:

    SVGExport();
};

class ObjectRepresentation
{
private:

    Reference< XInterface >         mxObject;
    GDIMetaFile*                    mpMtf;

public:

                                      ObjectRepresentation();
                                      ObjectRepresentation( const Reference< XInterface >& rxIf,
                                                            const GDIMetaFile& rMtf );
                                      ObjectRepresentation( const ObjectRepresentation& rPresentation );
                                      ~ObjectRepresentation();

    ObjectRepresentation&             operator=( const ObjectRepresentation& rPresentation );
    bool                          operator==( const ObjectRepresentation& rPresentation ) const;

    const Reference< XInterface >&    GetObject() const { return mxObject; }
    bool                          HasRepresentation() const { return mpMtf != nullptr; }
    const GDIMetaFile&                GetRepresentation() const { return *mpMtf; }
};

struct PagePropertySet
{
    bool               bIsBackgroundVisible;
    bool               bAreBackgroundObjectsVisible;
    bool               bIsPageNumberFieldVisible;
    bool               bIsDateTimeFieldVisible;
    bool               bIsFooterFieldVisible;
    bool               bIsHeaderFieldVisible;
    sal_Int32              nPageNumberingType;
    bool               bIsDateTimeFieldFixed;
    sal_Int16              nPageNumber;
    sal_Int32              nDateTimeFormat;
    OUString        sDateTimeText;
    OUString        sFooterText;
    OUString        sHeaderText;
};

struct HashReferenceXInterface
{
    size_t operator()( const Reference< XInterface >& rxIf ) const
    {
        return reinterpret_cast< size_t >( rxIf.get() );
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
                                                 XExtendedFilterDetection >
{
public:
    typedef std::unordered_map< Reference< XInterface >, ObjectRepresentation, HashReferenceXInterface >    ObjectMap;
    typedef std::unordered_set< Reference< XInterface >, HashReferenceXInterface >                          ObjectSet;
    typedef Sequence< Reference< XInterface > >                                                                 ObjectSequence;
    typedef Sequence< Reference< XDrawPage > >                                                                  XDrawPageSequence;

    typedef std::unordered_set< sal_Unicode, HashUChar >                                                    UCharSet;
    typedef std::unordered_map< OUString, UCharSet, OUStringHash >                                          UCharSetMap;
    typedef std::unordered_map< Reference< XInterface >, UCharSetMap, HashReferenceXInterface >             UCharSetMapMap;

    typedef std::unordered_map< Reference< XInterface >, OUString, HashReferenceXInterface >         UOStringMap;

    typedef std::unordered_set< ObjectRepresentation, HashBitmap, EqualityBitmap >                  MetaBitmapActionSet;

private:

    Reference< XComponentContext >      mxContext;
    SvXMLElementExport*                 mpSVGDoc;
    SVGExport*                          mpSVGExport;
    SVGFontExport*                      mpSVGFontExport;
    SVGActionWriter*                    mpSVGWriter;
    SdrPage*                            mpDefaultSdrPage;
    SdrModel*                           mpSdrModel;
    bool                            mbPresentation;
    bool                            mbSinglePage;
    sal_Int32                           mnVisiblePage;
    PagePropertySet                     mVisiblePagePropSet;
    OUString                     msClipPathId;
    UCharSetMapMap                      mTextFieldCharSets;
    Reference< XInterface >             mCreateOjectsCurrentMasterPage;
    UOStringMap                         mTextShapeIdListMap;
#ifndef DISABLE_EXPORT
    MetaBitmapActionSet                 mEmbeddedBitmapActionSet;
    ObjectMap                           mEmbeddedBitmapActionMap;
#endif
    ObjectMap*                          mpObjects;
    Reference< XComponent >             mxSrcDoc;
    Reference< XComponent >             mxDstDoc;
    Reference< XDrawPage >              mxDefaultPage;
    Sequence< PropertyValue >           maFilterData;
    // #i124608# explicit ShapeSelection for export when export of the selection is wanted
    Reference< XShapes >                maShapeSelection;
    bool                                mbExportSelection;
    XDrawPageSequence                   mSelectedPages;
    XDrawPageSequence                   mMasterPageTargets;

    Link<EditFieldInfo*,void>           maOldFieldHdl;
    Link<EditFieldInfo*,void>           maNewFieldHdl;

    bool                            implImport( const Sequence< PropertyValue >& rDescriptor ) throw (RuntimeException, std::exception);

    bool                            implExport( const Sequence< PropertyValue >& rDescriptor ) throw (RuntimeException, std::exception);
    static Reference< XWriter >     implCreateExportDocumentHandler( const Reference< XOutputStream >& rxOStm );

    bool                            implGetPagePropSet( const Reference< XDrawPage > & rxPage );
    bool                            implGenerateMetaData();
    void                            implExportTextShapeIndex();
    void                            implEmbedBulletGlyphs();
    void                            implEmbedBulletGlyph( sal_Unicode cBullet, const OUString & sPathData );
    bool                            implExportTextEmbeddedBitmaps();
    bool                            implGenerateScript();

    bool                            implExportDocument();
    bool                            implExportAnimations();

    bool                            implExportMasterPages( const XDrawPageSequence& rxPages,
                                                               sal_Int32 nFirstPage, sal_Int32 nLastPage );
    bool                            implExportDrawPages( const XDrawPageSequence& rxPages,
                                                             sal_Int32 nFirstPage, sal_Int32 nLastPage );
    bool                            implExportPage( const OUString & sPageId,
                                                        const Reference< XDrawPage > & rxPage,
                                                        const Reference< XShapes > & xShapes,
                                                        bool bMaster );

    bool                            implExportShapes( const Reference< XShapes >& rxShapes,
                                                          bool bMaster );
    bool                            implExportShape( const Reference< XShape >& rxShape,
                                                         bool bMaster );

    bool                            implCreateObjects();
    bool                            implCreateObjectsFromShapes( const Reference< XDrawPage > & rxPage, const Reference< XShapes >& rxShapes );
    bool                            implCreateObjectsFromShape( const Reference< XDrawPage > & rxPage, const Reference< XShape >& rxShape );
    bool                            implCreateObjectsFromBackground( const Reference< XDrawPage >& rxMasterPage );

    static OUString                 implGetClassFromShape( const Reference< XShape >& rxShape );
    void                            implRegisterInterface( const Reference< XInterface >& rxIf );
    const OUString &                implGetValidIDFromInterface( const Reference< XInterface >& rxIf );
    static OUString                 implGetInterfaceName( const Reference< XInterface >& rxIf );
    bool                            implLookForFirstVisiblePage();
    static Any                      implSafeGetPagePropSet( const OUString & sPropertyName,
                                                                const Reference< XPropertySet > & rxPropSet,
                                                                const Reference< XPropertySetInfo > & rxPropSetInfo );
    DECL_LINK_TYPED( CalcFieldHdl, EditFieldInfo*, void );

    static bool isStreamGZip(css::uno::Reference<css::io::XInputStream> xInput);
    static bool isStreamSvg(css::uno::Reference<css::io::XInputStream> xInput);

protected:

    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& rDescriptor ) throw(RuntimeException, std::exception) override;
    virtual void SAL_CALL cancel( ) throw (RuntimeException, std::exception) override;

    // XImporter
    virtual void SAL_CALL setTargetDocument( const Reference< XComponent >& xDoc ) throw(IllegalArgumentException, RuntimeException, std::exception) override;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const Reference< XComponent >& xDoc ) throw(IllegalArgumentException, RuntimeException, std::exception) override;

    // XExtendedFilterDetection
    virtual OUString SAL_CALL detect( Sequence< PropertyValue >& io_rDescriptor ) throw (RuntimeException, std::exception) override;

public:

    explicit SVGFilter( const Reference< XComponentContext >& rxCtx );
    virtual    ~SVGFilter();
};

#endif // INCLUDED_FILTER_SOURCE_SVG_SVGFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
