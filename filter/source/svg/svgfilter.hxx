/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SVGFILTER_HXX
#define SVGFILTER_HXX

#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/document/XFilter.hpp>
#ifdef SOLAR_JAVA
#include <com/sun/star/document/XImporter.hpp>
#endif // SOLAR_JAVA
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/implbase1.hxx>
#ifdef SOLAR_JAVA
#include <cppuhelper/implbase5.hxx>
#else // !SOLAR_JAVA
#include <cppuhelper/implbase4.hxx>
#endif
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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <hash_map>
#include <osl/diagnose.h>
#include <rtl/process.h>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <tools/debug.hxx>
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
#include "svgfilter.hxx"
#include "svgscript.hxx"

#include <cstdio>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::java;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::xml::sax;
using namespace ::std;

// -----------
// - Defines -
// -----------

#define SVG_EXPORT_ALLPAGES ((sal_Int32)-1)

// -------------
// - SVGExport -
// -------------

// #110680#
class SVGExport : public SvXMLExport
{
    typedef ::std::list< ::basegfx::B2DPolyPolygon > B2DPolyPolygonList;

    rtl::OUString   maGlyphPlacement;

    sal_Bool    mbTinyProfile;
    sal_Bool    mbTSpans;
    sal_Bool    mbEmbedFonts;
    sal_Bool    mbNativeTextDecoration;
    sal_Bool    mbOpacity;
    sal_Bool    mbGradient;

    Rectangle   maViewBox;

public:

    SVGExport( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
                const Reference< XDocumentHandler >& rxHandler,
                const Sequence< PropertyValue >& rFilterData );

    virtual ~SVGExport();

    sal_Bool IsUseTinyProfile() const { return mbTinyProfile; };
    sal_Bool IsUseTSpans() const { return mbTSpans; };
    sal_Bool IsEmbedFonts() const { return mbEmbedFonts; };
    sal_Bool IsUseNativeTextDecoration() const { return mbNativeTextDecoration; };
    ::rtl::OUString GetGlyphPlacement() const { return maGlyphPlacement; };
    sal_Bool IsUseOpacity() const { return mbOpacity; };
    sal_Bool IsUseGradient() const { return mbGradient; };

    const Rectangle& GetViewBox() const { return maViewBox; };
    void SetViewBox( const Rectangle& rViewBox ) { maViewBox = rViewBox; };
    sal_Bool IsVisible( const Rectangle& rRect ) const { return GetViewBox().IsOver( rRect ); };

    void  pushClip( const ::basegfx::B2DPolyPolygon& rPolyPoly );
    void  popClip();
    sal_Bool  hasClip() const;
    const ::basegfx::B2DPolyPolygon* getCurClip() const;

    void writeMtf( const GDIMetaFile& rMtf );

protected:

virtual void            _ExportStyles( sal_Bool /* bUsed */ ) {}
virtual void            _ExportAutoStyles() {}
virtual void            _ExportContent() {}
virtual void            _ExportMasterStyles() {}
virtual sal_uInt32      exportDoc( enum ::xmloff::token::XMLTokenEnum /* eClass */ ) { return 0; }

private:

    B2DPolyPolygonList      maClipList;

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

    ObjectRepresentation&           operator=( const ObjectRepresentation& rPresentation );
    sal_Bool                            operator==( const ObjectRepresentation& rPresentation ) const;

    const Reference< XInterface >&  GetObject() const { return mxObject; }
    sal_Bool                        HasRepresentation() const { return mpMtf != NULL; }
    const GDIMetaFile&              GetRepresentation() const { return *mpMtf; }
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

// -------------
// - SVGFilter -
// -------------

class SVGFontExport;
class SVGActionWriter;
class EditFieldInfo;

#ifdef SOLAR_JAVA
class SVGFilter : public cppu::WeakImplHelper5 < XFilter,
                                                 XImporter,
                                                 XExporter,
                                                 XInitialization,
                                                 XServiceInfo >
#else // !SOLAR_JAVA
class SVGFilter : public cppu::WeakImplHelper4 < XFilter,
                                                 XExporter,
                                                 XInitialization,
                                                 XServiceInfo >
#endif
{
    typedef ::std::hash_map< Reference< XInterface >, ObjectRepresentation, HashReferenceXInterface > ObjectMap;
    typedef ::std::vector< ::rtl::OUString > UniqueIdVector;

private:

    Reference< XMultiServiceFactory >   mxMSF;
    SvXMLElementExport*                 mpSVGDoc;
    SVGExport*                          mpSVGExport;
    SVGFontExport*                      mpSVGFontExport;
    SVGActionWriter*                    mpSVGWriter;
    SdrPage*                            mpDefaultSdrPage;
    SdrModel*                           mpSdrModel;
    sal_Bool                            mbPresentation;

    ObjectMap*                          mpObjects;
    Reference< XComponent >             mxSrcDoc;
#ifdef SOLAR_JAVA
    Reference< XComponent >             mxDstDoc;
#endif
    Reference< XDrawPage >              mxDefaultPage;
    Sequence< PropertyValue >           maFilterData;

    // #124608# explicit ShapeSelection for export when export of the selection is wanted
    Reference< XShapes >                maShapeSelection;
    bool                                mbExportSelection;

    UniqueIdVector                      maUniqueIdVector;
    sal_Int32                           mnMasterSlideId;
    sal_Int32                           mnSlideId;
    sal_Int32                           mnDrawingGroupId;
    sal_Int32                           mnDrawingId;
    Link                                maOldFieldHdl;

#ifdef SOLAR_JAVA
    sal_Bool                            implImport( const Sequence< PropertyValue >& rDescriptor ) throw (RuntimeException);
#endif

    sal_Bool                            implExport( const Sequence< PropertyValue >& rDescriptor ) throw (RuntimeException);
    Reference< XDocumentHandler >       implCreateExportDocumentHandler( const Reference< XOutputStream >& rxOStm );

    sal_Bool                            implGenerateMetaData( const Reference< XDrawPages >& rxMasterPages,
                                                              const Reference< XDrawPages >& rxDrawPages );
    sal_Bool                            implGenerateScript( const Reference< XDrawPages >& rxMasterPages,
                                                            const Reference< XDrawPages >& rxDrawPages );

    sal_Bool                            implExportDocument( const Reference< XDrawPages >& rxMasterPages,
                                                            const Reference< XDrawPages >& rxDrawPages,
                                                            sal_Int32 nPageToExport );

    sal_Bool                            implExportPages( const Reference< XDrawPages >& rxPages,
                                                         sal_Int32 nFirstPage, sal_Int32 nLastPage,
                                                         sal_Int32 nVisiblePage, sal_Bool bMaster );

    sal_Bool                            implExportShapes( const Reference< XShapes >& rxShapes );
    sal_Bool                            implExportShape( const Reference< XShape >& rxShape );

    sal_Bool                            implCreateObjects( const Reference< XDrawPages >& rxMasterPages,
                                                           const Reference< XDrawPages >& rxDrawPages,
                                                           sal_Int32 nPageToExport );
    sal_Bool                            implCreateObjectsFromShapes( const Reference< XShapes >& rxShapes );
    sal_Bool                            implCreateObjectsFromShape( const Reference< XShape >& rxShape );
    sal_Bool                            implCreateObjectsFromBackground( const Reference< XDrawPage >& rxMasterPage );

    ::rtl::OUString                     implGetDescriptionFromShape( const Reference< XShape >& rxShape );
    ::rtl::OUString                     implGetValidIDFromInterface( const Reference< XInterface >& rxIf, sal_Bool bUnique = sal_False );

    sal_Bool                                implHasText( const GDIMetaFile& rMtf ) const;

                                        DECL_LINK( CalcFieldHdl, EditFieldInfo* );

protected:

    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& rDescriptor ) throw(RuntimeException);
    virtual void SAL_CALL cancel( ) throw (RuntimeException);

#ifdef SOLAR_JAVA
    // XImporter
    virtual void SAL_CALL setTargetDocument( const Reference< XComponent >& xDoc ) throw(IllegalArgumentException, RuntimeException);
#endif

    // XExporter
    virtual void SAL_CALL setSourceDocument( const Reference< XComponent >& xDoc ) throw(IllegalArgumentException, RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()  throw(RuntimeException);

public:

                SVGFilter( const Reference< XMultiServiceFactory > &rxMSF );
    virtual     ~SVGFilter();
};

// -----------------------------------------------------------------------------

::rtl::OUString SVGFilter_getImplementationName ()
    throw ( ::com::sun::star::uno::RuntimeException );

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL SVGFilter_supportsService( const ::rtl::OUString& ServiceName )
    throw ( ::com::sun::star::uno::RuntimeException );

// -----------------------------------------------------------------------------

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL SVGFilter_getSupportedServiceNames(  )
    throw ( ::com::sun::star::uno::RuntimeException );

// -----------------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL SVGFilter_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr)
    throw ( ::com::sun::star::uno::Exception );

#endif // SVGFILTER_HXX
