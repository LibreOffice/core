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

#ifndef INCLUDED_SVX_UNOMODEL_HXX
#define INCLUDED_SVX_UNOMODEL_HXX

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <sfx2/sfxbasemodel.hxx>
#include <svx/fmdmod.hxx>
#include <svx/svxdllapi.h>
#include <cppuhelper/weakref.hxx>

class SdrModel;

class SVXCORE_DLLPUBLIC SvxUnoDrawingModel
:   public SfxBaseModel, // implements SfxListener, OWEAKOBJECT & other
    public SvxFmMSFactory,
    public css::drawing::XDrawPagesSupplier,
    public css::lang::XServiceInfo,
    public css::ucb::XAnyCompareFactory
{
    friend class SvxUnoDrawPagesAccess;

private:
    SdrModel* mpDoc;

    css::uno::WeakReference< css::drawing::XDrawPages > mxDrawPagesAccess;

    css::uno::Reference< css::uno::XInterface > mxDashTable;
    css::uno::Reference< css::uno::XInterface > mxGradientTable;
    css::uno::Reference< css::uno::XInterface > mxHatchTable;
    css::uno::Reference< css::uno::XInterface > mxBitmapTable;
    css::uno::Reference< css::uno::XInterface > mxTransGradientTable;
    css::uno::Reference< css::uno::XInterface > mxMarkerTable;

    css::uno::Sequence< css::uno::Type > maTypeSequence;

protected:
    // SvxUnoDrawMSFactory
    virtual SdrModel& getSdrModelFromUnoModel() const override;

public:
    SvxUnoDrawingModel( SdrModel* pDoc ) noexcept;
    virtual ~SvxUnoDrawingModel() noexcept override;

    SdrModel* GetDoc() const { return mpDoc; }

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override
    {  SfxBaseModel::acquire(); }
    virtual void SAL_CALL release() noexcept override
    {  SfxBaseModel::release(); }

    // XModel
    virtual void SAL_CALL lockControllers(  ) override;
    virtual void SAL_CALL unlockControllers(  ) override;
    virtual sal_Bool SAL_CALL hasControllersLocked(  ) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    // XDrawPagesSupplier
    virtual css::uno::Reference< css::drawing::XDrawPages > SAL_CALL getDrawPages(  ) override;

    // XMultiServiceFactory ( SvxFmMSFactory )
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XAnyCompareFactory
    virtual css::uno::Reference< css::ucb::XAnyCompare > SAL_CALL createAnyCompareByName( const OUString& PropertyName ) override;
};

SVXCORE_DLLPUBLIC extern bool SvxDrawingLayerExport( SdrModel* pModel, const css::uno::Reference<css::io::XOutputStream>& xOut );
extern bool SvxDrawingLayerExport( SdrModel* pModel, const css::uno::Reference<css::io::XOutputStream>& xOut, const css::uno::Reference< css::lang::XComponent >& xComponent );
SVXCORE_DLLPUBLIC extern bool SvxDrawingLayerExport( SdrModel* pModel, const css::uno::Reference<css::io::XOutputStream>& xOut, const css::uno::Reference< css::lang::XComponent >& xComponent, const char* pExportService  );
SVXCORE_DLLPUBLIC extern bool SvxDrawingLayerImport( SdrModel* pModel, const css::uno::Reference<css::io::XInputStream>& xInputStream );
extern bool SvxDrawingLayerImport( SdrModel* pModel, const css::uno::Reference<css::io::XInputStream>& xInputStream, const css::uno::Reference< css::lang::XComponent >& xComponent  );
SVXCORE_DLLPUBLIC extern bool SvxDrawingLayerImport( SdrModel* pModel, const css::uno::Reference<css::io::XInputStream>& xInputStream, const css::uno::Reference< css::lang::XComponent >& xComponent, const char* pImportService );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
