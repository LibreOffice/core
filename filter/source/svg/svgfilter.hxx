/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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

#ifndef SVGFILTER_HXX
#define SVGFILTER_HXX

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
#include <cppuhelper/implbase4.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/presentation/FadeEffect.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/java/XJavaVM.hpp>
#include <com/sun/star/java/XJavaThreadRegister_11.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <rtl/process.h>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/metaact.hxx>
#include <svtools/grfmgr.hxx>
#include <svx/unomodel.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdxcgv.hxx>
#include <svx/svdobj.hxx>
#include <xmloff/xmlexp.hxx>

#include <cstdio>


using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::java;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

using namespace ::std;

// -----------
// - Defines -
// -----------

#define SVG_EXPORT_ALLPAGES ((sal_Int32)-1)


// -----------
// - statics -
// -----------

// Placeholder tag used into the ImplWriteActions method to filter text placeholder fields
static const ::rtl::OUString sPlaceholderTag = ::rtl::OUString::createFromAscii( "<[:isPlaceholder:]>" );


// -------------
// - SVGExport -
// -------------

// #110680#
class SVGExport : public SvXMLExport
{
    typedef ::std::list< ::basegfx::B2DPolyPolygon > B2DPolyPolygonList;

public:

    SVGExport( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
                const Reference< XDocumentHandler >& rxHandler,
                const Sequence< PropertyValue >& rFilterData );

    virtual ~SVGExport();

    sal_Bool IsUseTinyProfile() const;
    sal_Bool IsEmbedFonts() const;
    sal_Bool IsUsePositionedCharacters() const;
    sal_Bool IsUseNativeTextDecoration() const;
    sal_Bool IsUseOpacity() const;

protected:

    virtual void            _ExportStyles( sal_Bool /* bUsed */ ) {}
    virtual void            _ExportAutoStyles() {}
    virtual void            _ExportContent() {}
    virtual void            _ExportMasterStyles() {}
    virtual sal_uInt32        exportDoc( enum ::xmloff::token::XMLTokenEnum /* eClass */ ) { return 0; }

private:

    const Sequence< PropertyValue >&    mrFilterData;

    SVGExport();
};

// ------------------------
// - ObjectRepresentation -
// ------------------------

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
    sal_Bool                          operator==( const ObjectRepresentation& rPresentation ) const;

    const Reference< XInterface >&    GetObject() const { return mxObject; }
    sal_Bool                          HasRepresentation() const { return mpMtf != NULL; }
    const GDIMetaFile&                GetRepresentation() const { return *mpMtf; }
};

// -------------------
// - PagePropertySet -
// -------------------

struct PagePropertySet
{
    sal_Bool               bIsBackgroundVisible;
    sal_Bool               bAreBackgroundObjectsVisible;
    sal_Bool               bIsPageNumberFieldVisible;
    sal_Bool               bIsDateTimeFieldVisible;
    sal_Bool               bIsFooterFieldVisible;
    sal_Bool               bIsHeaderFieldVisible;
    sal_Int32              nPageNumberingType;
    sal_Bool               bIsDateTimeFieldFixed;
    sal_Int16              nPageNumber;
    sal_Int32              nDateTimeFormat;
    ::rtl::OUString        sDateTimeText;
    ::rtl::OUString        sFooterText;
    ::rtl::OUString        sHeaderText;
};



// ---------------------------
// - HashReferenceXInterface -
// ---------------------------

struct HashReferenceXInterface
{
    size_t operator()( const Reference< XInterface >& rxIf ) const
    {
        return reinterpret_cast< size_t >( rxIf.get() );
    }
};

// ---------------------------
// - HashOUString -
// ---------------------------

struct HashOUString
{
    size_t operator()( const ::rtl::OUString& oustr ) const { return static_cast< size_t >( oustr.hashCode() ); }
};

// ---------------------------
// - HashUChar -
// ---------------------------

struct HashUChar
{
    size_t operator()( const sal_Unicode uchar ) const { return static_cast< size_t >( uchar ); }
};

// ---------------------------
// - HashBitmap -
// ---------------------------

struct HashBitmap
{
    size_t operator()( const ObjectRepresentation& rObjRep ) const;
};

// ---------------------------
// - EqualityBitmap -
// ---------------------------

struct EqualityBitmap
{
    bool operator()( const ObjectRepresentation& rObjRep1,
                     const ObjectRepresentation& rObjRep2 ) const;
};


// -------------
// - SVGFilter -
// -------------

class SVGFontExport;
class SVGActionWriter;
class EditFieldInfo;

class SVGFilter : public cppu::WeakImplHelper4 < XFilter,
                                                 XImporter,
                                                 XExporter,
                                                 XExtendedFilterDetection >
{
public:
    typedef ::boost::unordered_map< Reference< XInterface >, ObjectRepresentation, HashReferenceXInterface >    ObjectMap;
    typedef ::boost::unordered_set< Reference< XInterface >, HashReferenceXInterface >                          ObjectSet;
    typedef Sequence< Reference< XInterface > >                                                                 ObjectSequence;
    typedef Sequence< Reference< XDrawPage > >                                                                  XDrawPageSequence;

    typedef ::boost::unordered_set< sal_Unicode, HashUChar >                                                    UCharSet;
    typedef ::boost::unordered_map< ::rtl::OUString, UCharSet, HashOUString >                                   UCharSetMap;
    typedef ::boost::unordered_map< Reference< XInterface >, UCharSetMap, HashReferenceXInterface >             UCharSetMapMap;

    typedef ::boost::unordered_map< Reference< XInterface >, ::rtl::OUString, HashReferenceXInterface >         UOStringMap;

    typedef ::boost::unordered_set< ObjectRepresentation, HashBitmap, EqualityBitmap >                  MetaBitmapActionSet;

private:

    Reference< XMultiServiceFactory >   mxMSF;
    SvXMLElementExport*                 mpSVGDoc;
    SVGExport*                          mpSVGExport;
    SVGFontExport*                      mpSVGFontExport;
    SVGActionWriter*                    mpSVGWriter;
    SdrPage*                            mpDefaultSdrPage;
    SdrModel*                           mpSdrModel;
    sal_Bool                            mbPresentation;
    sal_Bool                            mbExportAll;
    sal_Bool                            mbSinglePage;
    sal_Int32                           mnVisiblePage;
    PagePropertySet                     mVisiblePagePropSet;
    ::rtl::OUString                     msClipPathId;
    UCharSetMapMap                      mTextFieldCharSets;
    Reference< XInterface >             mCreateOjectsCurrentMasterPage;
    UOStringMap                         mTextShapeIdListMap;
    MetaBitmapActionSet                 mEmbeddedBitmapActionSet;
    ObjectMap                           mEmbeddedBitmapActionMap;

    ObjectMap*                          mpObjects;
    Reference< XComponent >             mxSrcDoc;
    Reference< XComponent >             mxDstDoc;
    Reference< XDrawPage >              mxDefaultPage;
    Sequence< PropertyValue >           maFilterData;
    XDrawPageSequence                   mSelectedPages;
    XDrawPageSequence                   mMasterPageTargets;

    Link                                maOldFieldHdl;

    sal_Bool                            implImport( const Sequence< PropertyValue >& rDescriptor ) throw (RuntimeException);

    sal_Bool                            implExport( const Sequence< PropertyValue >& rDescriptor ) throw (RuntimeException);
    Reference< XDocumentHandler >       implCreateExportDocumentHandler( const Reference< XOutputStream >& rxOStm );

    sal_Bool                            implGetPagePropSet( const Reference< XDrawPage > & rxPage );
    sal_Bool                            implGenerateMetaData();
    void                                implExportTextShapeIndex();
    void                                implEmbedBulletGlyphs();
    void                                implEmbedBulletGlyph( sal_Unicode cBullet, const ::rtl::OUString & sPathData );
    sal_Bool                            implExportTextEmbeddedBitmaps();
    sal_Bool                            implGenerateScript();

    sal_Bool                            implExportDocument();
    sal_Bool                            implExportAnimations();

    sal_Bool                            implExportMasterPages( const XDrawPageSequence& rxPages,
                                                               sal_Int32 nFirstPage, sal_Int32 nLastPage );
    sal_Bool                            implExportDrawPages( const XDrawPageSequence& rxPages,
                                                             sal_Int32 nFirstPage, sal_Int32 nLastPage );
    sal_Bool                            implExportPage( const ::rtl::OUString & sPageId,
                                                        const Reference< XDrawPage > & rxPage,
                                                        const Reference< XShapes > & xShapes,
                                                        sal_Bool bMaster );

    sal_Bool                            implExportShapes( const Reference< XShapes >& rxShapes );
    sal_Bool                            implExportShape( const Reference< XShape >& rxShape );

    sal_Bool                            implCreateObjects();
    sal_Bool                            implCreateObjectsFromShapes( const Reference< XDrawPage > & rxPage, const Reference< XShapes >& rxShapes );
    sal_Bool                            implCreateObjectsFromShape( const Reference< XDrawPage > & rxPage, const Reference< XShape >& rxShape );
    sal_Bool                            implCreateObjectsFromBackground( const Reference< XDrawPage >& rxMasterPage );

    ::rtl::OUString                     implGetClassFromShape( const Reference< XShape >& rxShape );
    void                                implRegisterInterface( const Reference< XInterface >& rxIf );
    const ::rtl::OUString &             implGetValidIDFromInterface( const Reference< XInterface >& rxIf );
    ::rtl::OUString                     implGetInterfaceName( const Reference< XInterface >& rxIf );
    sal_Bool                            implLookForFirstVisiblePage();
    Any                                 implSafeGetPagePropSet( const ::rtl::OUString & sPropertyName,
                                                                const Reference< XPropertySet > & rxPropSet,
                                                                const Reference< XPropertySetInfo > & rxPropSetInfo );
                                        DECL_LINK( CalcFieldHdl, EditFieldInfo* );

protected:

    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& rDescriptor ) throw(RuntimeException);
    virtual void SAL_CALL cancel( ) throw (RuntimeException);

    // XImporter
    virtual void SAL_CALL setTargetDocument( const Reference< XComponent >& xDoc ) throw(IllegalArgumentException, RuntimeException);

    // XExporter
    virtual void SAL_CALL setSourceDocument( const Reference< XComponent >& xDoc ) throw(IllegalArgumentException, RuntimeException);

    // XExtendedFilterDetection
    virtual rtl::OUString SAL_CALL detect( Sequence< PropertyValue >& io_rDescriptor ) throw (RuntimeException);

public:

    explicit SVGFilter( const Reference< XComponentContext >& rxCtx );
    virtual    ~SVGFilter();
};

#endif // SVGFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
