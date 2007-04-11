/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unomodel.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:45:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SVX_UNOMODEL_HXX
#define SVX_UNOMODEL_HXX

#ifndef SVX_LIGHT

#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XANYCOMPAREFACTORY_HPP_
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _SFX_SFXBASEMODEL_HXX_
#include <sfx2/sfxbasemodel.hxx>
#endif

#ifndef _SVX_FMDMOD_HXX
#include <svx/fmdmod.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SdrModel;

class SVX_DLLPUBLIC SvxUnoDrawingModel : public SfxBaseModel, // implements SfxListener, OWEAKOBJECT & other
                           public SvxFmMSFactory,
                           public ::com::sun::star::drawing::XDrawPagesSupplier,
                           public ::com::sun::star::lang::XServiceInfo,
                           public ::com::sun::star::ucb::XAnyCompareFactory
{
    friend class SvxUnoDrawPagesAccess;

private:
    SdrModel* mpDoc;

    ::com::sun::star::uno::WeakReference< ::com::sun::star::drawing::XDrawPages > mxDrawPagesAccess;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxDashTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxGradientTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxHatchTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxBitmapTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxTransGradientTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxMarkerTable;

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > maTypeSequence;

public:
    SvxUnoDrawingModel( SdrModel* pDoc ) throw();
    virtual ~SvxUnoDrawingModel() throw();

    SdrModel* GetDoc() const { return mpDoc; }

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XModel
    virtual void SAL_CALL lockControllers(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL unlockControllers(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasControllersLocked(  ) throw(::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    // XDrawPagesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPages > SAL_CALL getDrawPages(  ) throw(::com::sun::star::uno::RuntimeException);

    // XMultiServiceFactory ( SvxFmMSFactory )
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const ::rtl::OUString& aServiceSpecifier ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XAnyCompareFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XAnyCompare > SAL_CALL createAnyCompareByName( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);
};

SVX_DLLPUBLIC extern sal_Bool SvxDrawingLayerExport( SdrModel* pModel, com::sun::star::uno::Reference<com::sun::star::io::XOutputStream> xOut );
SVX_DLLPUBLIC extern sal_Bool SvxDrawingLayerExport( SdrModel* pModel, com::sun::star::uno::Reference<com::sun::star::io::XOutputStream> xOut, com::sun::star::uno::Reference< com::sun::star::lang::XComponent > xComponent );
SVX_DLLPUBLIC extern sal_Bool SvxDrawingLayerExport( SdrModel* pModel, com::sun::star::uno::Reference<com::sun::star::io::XOutputStream> xOut, com::sun::star::uno::Reference< com::sun::star::lang::XComponent > xComponent, const char* pExportService  );
SVX_DLLPUBLIC extern sal_Bool SvxDrawingLayerImport( SdrModel* pModel, com::sun::star::uno::Reference<com::sun::star::io::XInputStream> xInputStream );
SVX_DLLPUBLIC extern sal_Bool SvxDrawingLayerImport( SdrModel* pModel, com::sun::star::uno::Reference<com::sun::star::io::XInputStream> xInputStream, com::sun::star::uno::Reference< com::sun::star::lang::XComponent > xComponent  );
SVX_DLLPUBLIC extern sal_Bool SvxDrawingLayerImport( SdrModel* pModel, com::sun::star::uno::Reference<com::sun::star::io::XInputStream> xInputStream, com::sun::star::uno::Reference< com::sun::star::lang::XComponent > xComponent, const char* pImportService );

#endif

#endif

