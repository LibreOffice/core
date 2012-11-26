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



#ifndef SVX_UNOMODEL_HXX
#define SVX_UNOMODEL_HXX

#ifndef SVX_LIGHT

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <sfx2/sfxbasemodel.hxx>
#include <svx/fmdmod.hxx>
#include "svx/svxdllapi.h"

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

protected:
    // SvxUnoDrawMSFactory
    virtual SdrModel* getSdrModel() const;

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

SVX_DLLPUBLIC extern sal_Bool SvxDrawingLayerExport( SdrModel* pModel, const com::sun::star::uno::Reference<com::sun::star::io::XOutputStream>& xOut );
SVX_DLLPUBLIC extern sal_Bool SvxDrawingLayerExport( SdrModel* pModel, const com::sun::star::uno::Reference<com::sun::star::io::XOutputStream>& xOut, const com::sun::star::uno::Reference< com::sun::star::lang::XComponent >& xComponent );
SVX_DLLPUBLIC extern sal_Bool SvxDrawingLayerExport( SdrModel* pModel, const com::sun::star::uno::Reference<com::sun::star::io::XOutputStream>& xOut, const com::sun::star::uno::Reference< com::sun::star::lang::XComponent >& xComponent, const char* pExportService  );
SVX_DLLPUBLIC extern sal_Bool SvxDrawingLayerImport( SdrModel* pModel, const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream );
SVX_DLLPUBLIC extern sal_Bool SvxDrawingLayerImport( SdrModel* pModel, const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream, const com::sun::star::uno::Reference< com::sun::star::lang::XComponent >& xComponent  );
SVX_DLLPUBLIC extern sal_Bool SvxDrawingLayerImport( SdrModel* pModel, const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream, const com::sun::star::uno::Reference< com::sun::star::lang::XComponent >& xComponent, const char* pImportService );

#endif

#endif

