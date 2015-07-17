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
    static void notImplemented() throw ( css::io::IOException );
public:
    explicit SIDEModel(SfxObjectShell *pObjSh = 0);
    virtual ~SIDEModel();

    //XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL acquire(  ) throw() SAL_OVERRIDE;
    virtual void SAL_CALL release(  ) throw() SAL_OVERRIDE;

    //XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    // XStorable2
    virtual void SAL_CALL storeSelf( const  css::uno::Sequence< css::beans::PropertyValue >& )
        throw (css::lang::IllegalArgumentException, css::io::IOException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE { notImplemented(); }
    //  XStorable
    virtual void SAL_CALL store() throw (css::io::IOException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL storeAsURL( const   OUString& sURL,
                                      const   css::uno::Sequence< css::beans::PropertyValue >&   seqArguments    )
        throw (css::io::IOException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL storeToURL( const   OUString& sURL,
                                      const   css::uno::Sequence< css::beans::PropertyValue >&   seqArguments    )
        throw (css::io::IOException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();
    static OUString getImplementationName_Static();
};

css::uno::Reference< css::uno::XInterface > SAL_CALL SIDEModel_createInstance(
                const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr )
                    throw( com::sun::star::uno::Exception );

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_BASICIDE_UNOMODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
