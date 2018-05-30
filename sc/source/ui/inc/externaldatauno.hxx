/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_EXTERNALDATAPROVIDER_HXX
#define INCLUDED_SC_INC_EXTERNALDATAPROVIDER_HXX

#include <cppuhelper/implbase.hxx>
#include <document.hxx>
#include <com/sun/star/sheet/XExternalDataProvider.hpp>
#include <com/sun/star/sheet/ExternalDataSourceInfo.hpp>
#include <com/sun/star/sheet/XExternalDataProviders.hpp>

class ScDocument;

namespace sc {

class ExternalDataSource;

}

class ScExternalDataProvidersObj : public cppu::WeakImplHelper<css::sheet::XExternalDataProviders>
{
public:

    ScExternalDataProvidersObj(ScDocument* pDoc);
    virtual ~ScExternalDataProvidersObj();

    void SAL_CALL insertByIndex(sal_Int32 nIndex, const css::uno::Any& rAny) override;
    void SAL_CALL removeByIndex(sal_Int32 nIndex) override;

    css::uno::Type SAL_CALL getElementType() override;
    sal_Bool SAL_CALL hasElements() override;
    sal_Int32 SAL_CALL getCount() override;
    css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;
    void SAL_CALL replaceByIndex(sal_Int32 nIndex, const css::uno::Any& rElement) override;

private:

    ScDocument* mpDoc;
};

class ScExternalDataProviderObj : public cppu::WeakImplHelper<css::sheet::XExternalDataProvider>
{
public:
    ScExternalDataProviderObj(ScDocument* pDoc, sal_Int32 nIndex);
    virtual ~ScExternalDataProviderObj();

    /// XExternalDataProvider
    virtual ::css::uno::Sequence< ::css::sheet::ExternalDataSourceInfo > SAL_CALL getDataSourcesForURL( const OUString& rURL ) override;
    virtual OUString SAL_CALL getURL() override;
    virtual OUString SAL_CALL getProvider() override;
    virtual void Import() override;
    virtual void addTransformation(sal_Int32 nIndex, const css::uno::Any& rTransformation);
    virtual void ImportFinished() override;

private:

    OUString mURL;
    OUString mId;
    OUString mProvider;

    sc::ExternalDataSource& getDataSource();

    ScDocument* mpDocument;
    ScDocumentUniquePtr mpDoc;
    sal_Int32 mnIndex;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
