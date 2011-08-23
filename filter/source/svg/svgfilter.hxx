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

#ifndef SVGFILTER_HXX
#define SVGFILTER_HXX

#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <hash_map>
#include <osl/diagnose.h>
#include <rtl/process.h>
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
private:

                            SVGExport();

protected:

    virtual void			_ExportMeta() {}
    virtual void			_ExportStyles( BOOL /* bUsed */ ) {}
    virtual void			_ExportAutoStyles() {}
    virtual void			_ExportContent() {}
    virtual void			_ExportMasterStyles() {}
    virtual sal_uInt32		exportDoc( enum ::xmloff::token::XMLTokenEnum /* eClass */ ) { return 0; }
                            
public:						
                            
    SVGExport( 
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        const Reference< XDocumentHandler >& rxHandler );
    virtual					~SVGExport();
};

// ------------------------
// - ObjectRepresentation -
// ------------------------

class ObjectRepresentation
{
private:

    Reference< XInterface >			mxObject;
    GDIMetaFile*					mpMtf;
                                    
public:								
                                    
                                    ObjectRepresentation();
                                    ObjectRepresentation( const Reference< XInterface >& rxIf, 
                                                          const GDIMetaFile& rMtf );
                                    ObjectRepresentation( const ObjectRepresentation& rPresentation );
                                    ~ObjectRepresentation();
                                    
    ObjectRepresentation&			operator=( const ObjectRepresentation& rPresentation );
    bool							operator==( const ObjectRepresentation& rPresentation ) const;

    const Reference< XInterface >&	GetObject() const { return mxObject; }	
    sal_Bool						HasRepresentation() const { return mpMtf != NULL; }
    const GDIMetaFile&				GetRepresentation() const { return *mpMtf; }	
};

// ---------------------------
// - HashReferenceXInterface -
// ---------------------------

struct HashReferenceXInterface
{
    size_t operator()( const Reference< XInterface >& rxIf ) const { return reinterpret_cast< size_t >( rxIf.get() ); }
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
    typedef ::std::hash_map< Reference< XInterface >, ObjectRepresentation, HashReferenceXInterface > ObjectMap;

private:

    Reference< XMultiServiceFactory >	mxMSF;
    SvXMLElementExport*					mpSVGDoc;
    SVGExport*							mpSVGExport;
    SVGFontExport*						mpSVGFontExport;
    SVGActionWriter*					mpSVGWriter;
    SdrPage*							mpDefaultSdrPage;
    SdrModel*							mpSdrModel;
    sal_Bool							mbPresentation;

    ObjectMap*							mpObjects;
    Reference< XComponent >				mxSrcDoc;
    Reference< XComponent >				mxDstDoc;
    Reference< XDrawPage > 				mxDefaultPage;
    Link								maOldFieldHdl;

    sal_Bool                            implImport( const Sequence< PropertyValue >& rDescriptor ) throw (RuntimeException);

    sal_Bool                            implExport( const Sequence< PropertyValue >& rDescriptor ) throw (RuntimeException);
    Reference< XDocumentHandler >       implCreateExportDocumentHandler( const Reference< XOutputStream >& rxOStm );

    sal_Bool							implGenerateMetaData( const Reference< XDrawPages >& rxMasterPages, 
                                                              const Reference< XDrawPages >& rxDrawPages );
    sal_Bool                            implGenerateScript( const Reference< XDrawPages >& rxMasterPages, 
                                                            const Reference< XDrawPages >& rxDrawPages );

    sal_Bool							implExportDocument( const Reference< XDrawPages >& rxMasterPages,
                                                            const Reference< XDrawPages >& rxDrawPages,
                                                            sal_Int32 nPageToExport );
    
    sal_Bool							implExportPages( const Reference< XDrawPages >& rxPages,
                                                         sal_Int32 nFirstPage, sal_Int32 nLastPage,
                                                         sal_Int32 nVisiblePage, sal_Bool bMaster );
    
    sal_Bool							implExportShapes( const Reference< XShapes >& rxShapes );
    sal_Bool                            implExportShape( const Reference< XShape >& rxShape );

    sal_Bool							implCreateObjects( const Reference< XDrawPages >& rxMasterPages,
                                                           const Reference< XDrawPages >& rxDrawPages,
                                                           sal_Int32 nPageToExport );
    sal_Bool							implCreateObjectsFromShapes( const Reference< XShapes >& rxShapes );
    sal_Bool                            implCreateObjectsFromShape( const Reference< XShape >& rxShape );
    sal_Bool							implCreateObjectsFromBackground( const Reference< XDrawPage >& rxMasterPage );
    
    ::rtl::OUString						implGetDescriptionFromShape( const Reference< XShape >& rxShape );
    ::rtl::OUString						implGetValidIDFromInterface( const Reference< XInterface >& rxIf );
    
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
    virtual	~SVGFilter();
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

// -----------------------------------------------------------------------------

class SvStream;
class Graphic;

bool importSvg(SvStream & rStream, Graphic & rGraphic );

#endif // SVGFILTER_HXX
