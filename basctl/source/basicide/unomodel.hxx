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
#ifndef INCLUDED_BASCTL_SOURCE_BASICIDE_UNOMODEL_HXX
#define INCLUDED_BASCTL_SOURCE_BASICIDE_UNOMODEL_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <sfx2/sfxbasemodel.hxx>

namespace basctl
{

class SIDEModel : public SfxBaseModel,
                public com::sun::star::lang::XServiceInfo
{
    /// @throws css::io::IOException
    static void notImplemented();
public:
    explicit SIDEModel(SfxObjectShell *pObjSh);
    virtual ~SIDEModel() override;

    //XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire(  ) throw() override;
    virtual void SAL_CALL release(  ) throw() override;

    //XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
    // XStorable2
    virtual void SAL_CALL storeSelf( const  css::uno::Sequence< css::beans::PropertyValue >& ) override { notImplemented(); }
    //  XStorable
    virtual void SAL_CALL store() override;
    virtual void SAL_CALL storeAsURL( const   OUString& sURL,
                                      const   css::uno::Sequence< css::beans::PropertyValue >&   seqArguments    ) override;
    virtual void SAL_CALL storeToURL( const   OUString& sURL,
                                      const   css::uno::Sequence< css::beans::PropertyValue >&   seqArguments    ) override;

    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();
    static OUString getImplementationName_Static();
};

/// @throws com::sun::star::uno::Exception
css::uno::Reference< css::uno::XInterface > SIDEModel_createInstance(
                const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_BASICIDE_UNOMODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
