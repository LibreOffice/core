/*************************************************************************
 *
 *  $RCSfile: svgfilter.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 17:57:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SVGFILTER_HXX
#define SVGFILTER_HXX

#ifndef _COM_SUN_STAR_DRAWING_XMASTERPAGETARGET_HPP_
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XMASTERPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_XPRESENTATIONSUPPLIER_HPP_
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XIMPORTER_HPP_
#include <com/sun/star/document/XImporter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONEFFECT_HPP_
#include <com/sun/star/presentation/AnimationEffect.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONSPEED_HPP_
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_CLICKACTION_HPP_
#include <com/sun/star/presentation/ClickAction.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_FADEEFFECT_HPP_
#include <com/sun/star/presentation/FadeEffect.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif

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
#include <goodies/grfmgr.hxx>
#include <svx/unomodel.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdxcgv.hxx>
#include <svx/svdobj.hxx>
#include <xmloff/xmlexp.hxx>
#include <sj2/jnihelp.hxx>
#include "svgfilter.hxx"
#include "svgscript.hxx"

using namespace ::rtl;
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

// -------------
// - SVGExport -
// -------------

class SVGExport : public SvXMLExport
{
private:

                            SVGExport();

protected:

    virtual void            _ExportMeta() {}
    virtual void            _ExportStyles( BOOL bUsed ) {}
    virtual void            _ExportAutoStyles() {}
    virtual void            _ExportContent() {}
    virtual void            _ExportMasterStyles() {}
    virtual ULONG           exportDoc( enum ::xmloff::token::XMLTokenEnum eClass ) { return 0; }

public:

                            SVGExport( const Reference< XDocumentHandler >& rxHandler );
    virtual                 ~SVGExport();
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
    bool                            operator==( const ObjectRepresentation& rPresentation ) const;

    const Reference< XInterface >&  GetObject() const { return mxObject; }
    sal_Bool                        HasRepresentation() const { return mpMtf != NULL; }
    const GDIMetaFile&              GetRepresentation() const { return *mpMtf; }
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

class SVGFilter : public cppu::WeakImplHelper5 < XFilter,
                                                 XImporter,
                                                 XExporter,
                                                 XInitialization,
                                                 XServiceInfo >
{
    typedef ::std::hash_map< Reference< XInterface >, ObjectRepresentation, HashReferenceXInterface > ObjectMap;

private:

    ObjectMap*                          mpObjects;
    Reference< XMultiServiceFactory >   mxMSF;
    Reference< XComponent >             mxSrcDoc;
    Reference< XComponent >             mxDstDoc;
    SvXMLElementExport*                 mpSVGDoc;
    SVGExport*                          mpSVGExport;
    SVGFontExport*                      mpSVGFontExport;
    SVGActionWriter*                    mpSVGWriter;
    sal_Bool                            mbPresentation;

    sal_Bool                            implImport( const Sequence< PropertyValue >& rDescriptor ) throw (RuntimeException);

    sal_Bool                            implExport( const Sequence< PropertyValue >& rDescriptor ) throw (RuntimeException);
    Reference< XDocumentHandler >       implCreateExportDocumentHandler( const Reference< XOutputStream >& rxOStm );

    sal_Bool                            implGenerateMetaData( const Reference< XDrawPages >& rxMasterPages,
                                                              const Reference< XDrawPages >& rxDrawPages );
    sal_Bool                            implGenerateScript( const Reference< XDrawPages >& rxMasterPages,
                                                            const Reference< XDrawPages >& rxDrawPages );

    sal_Bool                            implExportDocumemt( const Reference< XDrawPages >& rxMasterPages,
                                                            const Reference< XDrawPages >& rxDrawPages );

    sal_Bool                            implExportPages( const Reference< XDrawPages >& rxMasterPages,
                                                         sal_Int32 nVisiblePage, sal_Bool bMaster );

    sal_Bool                            implExportShapes( const Reference< XShapes >& rxShapes );
    sal_Bool                            implExportShape( const Reference< XShape >& rxShape );

    sal_Bool                            implCreateObjects( const Reference< XDrawPages >& rxMasterPages,
                                                           const Reference< XDrawPages >& rxDrawPages );

    sal_Bool                            implCreateObjectsFromShapes( const Reference< XShapes >& rxShapes );
    sal_Bool                            implCreateObjectsFromShape( const Reference< XShape >& rxShape );
    sal_Bool                            implCreateObjectsFromBackground( const Reference< XDrawPage >& rxMasterPage );
    OUString                            implGetDescriptionFromShape( const Reference< XShape >& rxShape );
    OUString                            implGetValidIDFromInterface( const Reference< XInterface >& rxIf );

protected:

    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& rDescriptor ) throw(RuntimeException);
    virtual void SAL_CALL cancel( ) throw (RuntimeException);

    // XImporter
    virtual void SAL_CALL setTargetDocument( const Reference< XComponent >& xDoc ) throw(IllegalArgumentException, RuntimeException);

    // XExporter
    virtual void SAL_CALL setSourceDocument( const Reference< XComponent >& xDoc ) throw(IllegalArgumentException, RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()  throw(RuntimeException);

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
