/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <externaldatauno.hxx>
#include <externaltransformationuno.hxx>

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <document.hxx>
#include <datamapper.hxx>

using namespace com::sun::star;

ScExternalDataProvidersObj::ScExternalDataProvidersObj(ScDocument* pDoc):
    mpDoc(pDoc)
{
}

ScExternalDataProvidersObj::~ScExternalDataProvidersObj()
{
}

void ScExternalDataProvidersObj::insertByIndex(sal_Int32 nIndex, const css::uno::Any& rAny)
{
     (void) nIndex;
     // (void) rAny;

     // mpDoc->GetExternalDataMapper().insertDataSource(sc::ExternalDataSource("~/Desktop/test.dat", "org.external.dat", mpDoc));

    css::uno::Reference<css::sheet::XExternalDataProvider> xData;
    if (! (rAny >>= xData) )
        throw lang::IllegalArgumentException();

    mpDoc->GetExternalDataMapper().insertDataSource(sc::ExternalDataSource(xData->getURL(), xData->getProvider(), mpDoc));

}

void ScExternalDataProvidersObj::removeByIndex(sal_Int32 nIndex)
{
    if (nIndex >= getCount())
        throw lang::IndexOutOfBoundsException();

    auto& rDataSources = mpDoc->GetExternalDataMapper().getDataSources();
    rDataSources.erase(rDataSources.begin() + nIndex);
}

css::uno::Type ScExternalDataProvidersObj::getElementType()
{
    return css::uno::Type();
}

sal_Bool ScExternalDataProvidersObj::hasElements()
{
    return mpDoc->GetExternalDataMapper().getDataSources().empty();
}

sal_Int32 ScExternalDataProvidersObj::getCount()
{
    return mpDoc->GetExternalDataMapper().getDataSources().size();
}

css::uno::Any ScExternalDataProvidersObj::getByIndex(sal_Int32 nIndex)
{
    if (nIndex >= getCount())
        throw lang::IndexOutOfBoundsException();

    css::uno::Any aRet;
    aRet <<= css::uno::Reference<css::sheet::XExternalDataProvider>(new ScExternalDataProviderObj(mpDoc, nIndex));
    return aRet;
}

void ScExternalDataProvidersObj::replaceByIndex(sal_Int32 nIndex, const css::uno::Any& /*rElement*/)
{
    if (nIndex >= getCount())
        throw lang::IndexOutOfBoundsException();
}

ScExternalDataProviderObj::ScExternalDataProviderObj(ScDocument* pDoc, sal_Int32 nIndex):
    mpDocument(pDoc),
    mnIndex(nIndex)
{
}

ScExternalDataProviderObj::~ScExternalDataProviderObj()
{
}

uno::Sequence<::css::sheet::ExternalDataSourceInfo> SAL_CALL ScExternalDataProviderObj::getDataSourcesForURL(const OUString& rURL)
{
    // TODO
    ::css::sheet::ExternalDataSourceInfo sourceInfo(rURL, rURL);
    return {sourceInfo};
}

void ScExternalDataProviderObj::Import()
{
   // already importing data
    if (mpDoc)
        return;

    mpDoc.reset(new ScDocument(SCDOCMODE_CLIP));
    mpDoc->ResetClip(mpDocument, SCTAB(0));
    // Launch a thread to fetch data from a source and then callImportFinished when done.
  //  mxFetchThread = new FetchThread(*mpDoc, getDataSource().getURL(), std::bind(&ScExternalDataProviderObj::ImportFinished, this), getDataSource().getDataTransformation());
  //  mxFetchThread->launch();
}

void ScExternalDataProviderObj::ImportFinished()
{
   // getDataSource().getDBManager()->WriteToDoc(*mpDoc);
   // mpDoc.reset();
}

OUString SAL_CALL ScExternalDataProviderObj::getURL()
{
    return getDataSource().getURL();
}

OUString SAL_CALL ScExternalDataProviderObj::getProvider()
{
    return getDataSource().getProvider();
}


sc::ExternalDataSource& ScExternalDataProviderObj::getDataSource()
{
    return mpDoc->GetExternalDataMapper().getDataSources().at(mnIndex);
}

void ScExternalDataProviderObj::addTransformation(sal_Int32 /*nIndex*/, const css::uno::Any& rTransformation)
{
    css::uno::Reference<css::sheet::XDataTransformations> xTransformation;
    if (! (rTransformation >>= xTransformation) )
        throw lang::IllegalArgumentException();

    //mpDoc->GetExternalDataMapper().getDataSources().at(mnIndex).AddDataTransformation(xTransformation);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
