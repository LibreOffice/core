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

#include <sal/config.h>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#include <com/sun/star/linguistic2/XConversionDictionaryList.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/util/XFlushable.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/instance.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/useroptions.hxx>

#include "convdic.hxx"
#include "convdiclist.hxx"
#include "defs.hxx"
#include "hhconvdic.hxx"
#include "lngreg.hxx"
#include "linguistic/misc.hxx"

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

#define SN_CONV_DICTIONARY_LIST  "com.sun.star.linguistic2.ConversionDictionaryList"

OUString GetConvDicMainURL( const OUString &rDicName, const OUString &rDirectoryURL )
{
    // build URL to use for new (persistent) dictionaries

    OUString aFullDicName = rDicName + CONV_DIC_DOT_EXT;

    INetURLObject aURLObj;
    aURLObj.SetSmartProtocol( INetProtocol::File );
    aURLObj.SetSmartURL( rDirectoryURL );
    aURLObj.Append( aFullDicName, INetURLObject::EncodeMechanism::All );
    DBG_ASSERT(!aURLObj.HasError(), "invalid URL");
    if (aURLObj.HasError())
        return OUString();
    else
        return aURLObj.GetMainURL( INetURLObject::DecodeMechanism::ToIUri );
}

class ConvDicNameContainer :
    public cppu::WeakImplHelper< css::container::XNameContainer >
{
    std::vector< uno::Reference< XConversionDictionary > >   aConvDics;

    sal_Int32 GetIndexByName_Impl( const OUString& rName );

public:
    ConvDicNameContainer();
    ConvDicNameContainer(const ConvDicNameContainer&) = delete;
    ConvDicNameContainer& operator=(const ConvDicNameContainer&) = delete;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement ) override;

    // XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement ) override;
    virtual void SAL_CALL removeByName( const OUString& Name ) override;

    // looks for conversion dictionaries with the specified extension
    // in the directory and adds them to the container
    void AddConvDics( const OUString &rSearchDirPathURL, const OUString &rExtension );

    // calls Flush for the dictionaries that support XFlushable
    void    FlushDics() const;

    sal_Int32   GetCount() const    { return aConvDics.size(); }
    uno::Reference< XConversionDictionary > GetByName( const OUString& rName );

    const uno::Reference< XConversionDictionary >&  GetByIndex( sal_Int32 nIdx )
    {
        return aConvDics[nIdx];
    }
};

ConvDicNameContainer::ConvDicNameContainer()
{
}

void ConvDicNameContainer::FlushDics() const
{
    sal_Int32 nLen = aConvDics.size();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        uno::Reference< util::XFlushable > xFlush( aConvDics[i] , UNO_QUERY );
        if (xFlush.is())
        {
            try
            {
                xFlush->flush();
            }
            catch(Exception &)
            {
                OSL_FAIL( "flushing of conversion dictionary failed" );
            }
        }
    }
}

sal_Int32 ConvDicNameContainer::GetIndexByName_Impl(
        const OUString& rName )
{
    sal_Int32 nRes = -1;
    sal_Int32 nLen = aConvDics.size();
    for (sal_Int32 i = 0;  i < nLen && nRes == -1;  ++i)
    {
        if (rName == aConvDics[i]->getName())
            nRes = i;
    }
    return nRes;
}

uno::Reference< XConversionDictionary > ConvDicNameContainer::GetByName(
        const OUString& rName )
{
    uno::Reference< XConversionDictionary > xRes;
    sal_Int32 nIdx = GetIndexByName_Impl( rName );
    if ( nIdx != -1)
        xRes = aConvDics[nIdx];
    return xRes;
}

uno::Type SAL_CALL ConvDicNameContainer::getElementType(  )
{
    MutexGuard  aGuard( GetLinguMutex() );
    return uno::Type( cppu::UnoType<XConversionDictionary>::get());
}

sal_Bool SAL_CALL ConvDicNameContainer::hasElements(  )
{
    MutexGuard  aGuard( GetLinguMutex() );
    return !aConvDics.empty();
}

uno::Any SAL_CALL ConvDicNameContainer::getByName( const OUString& rName )
{
    MutexGuard  aGuard( GetLinguMutex() );
    uno::Reference< XConversionDictionary > xRes( GetByName( rName ) );
    if (!xRes.is())
        throw NoSuchElementException();
    return makeAny( xRes );
}

uno::Sequence< OUString > SAL_CALL ConvDicNameContainer::getElementNames(  )
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Int32 nLen = aConvDics.size();
    uno::Sequence< OUString > aRes( nLen );
    OUString *pName = aRes.getArray();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
        pName[i] = aConvDics[i]->getName();
    return aRes;
}

sal_Bool SAL_CALL ConvDicNameContainer::hasByName( const OUString& rName )
{
    MutexGuard  aGuard( GetLinguMutex() );
    return GetByName( rName ).is();
}

void SAL_CALL ConvDicNameContainer::replaceByName(
        const OUString& rName,
        const uno::Any& rElement )
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Int32 nRplcIdx = GetIndexByName_Impl( rName );
    if (nRplcIdx == -1)
        throw NoSuchElementException();
    uno::Reference< XConversionDictionary > xNew;
    rElement >>= xNew;
    if (!xNew.is() || xNew->getName() != rName)
        throw IllegalArgumentException();
    aConvDics[ nRplcIdx ] = xNew;
}

void SAL_CALL ConvDicNameContainer::insertByName(
        const OUString& rName,
        const Any& rElement )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (GetByName( rName ).is())
        throw ElementExistException();
    uno::Reference< XConversionDictionary > xNew;
    rElement >>= xNew;
    if (!xNew.is() || xNew->getName() != rName)
        throw IllegalArgumentException();

    aConvDics.push_back(xNew);
}

void SAL_CALL ConvDicNameContainer::removeByName( const OUString& rName )
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Int32 nRplcIdx = GetIndexByName_Impl( rName );
    if (nRplcIdx == -1)
        throw NoSuchElementException();

    // physically remove dictionary
    uno::Reference< XConversionDictionary > xDel = aConvDics[nRplcIdx];
    OUString aName( xDel->getName() );
    OUString aDicMainURL( GetConvDicMainURL( aName, GetDictionaryWriteablePath() ) );
    INetURLObject aObj( aDicMainURL );
    DBG_ASSERT( aObj.GetProtocol() == INetProtocol::File, "+HangulHanjaOptionsDialog::OkHdl(): non-file URLs cannot be deleted" );
    if( aObj.GetProtocol() == INetProtocol::File )
    {
        try
        {
            ::ucbhelper::Content    aCnt( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ),
                                    uno::Reference< css::ucb::XCommandEnvironment >(),
                                    comphelper::getProcessComponentContext() );
            aCnt.executeCommand( "delete", makeAny( true ) );
        }
        catch( css::ucb::CommandAbortedException& )
        {
            SAL_WARN( "linguistic", "HangulHanjaOptionsDialog::OkHdl(): CommandAbortedException" );
        }
        catch( ... )
        {
            SAL_WARN( "linguistic", "HangulHanjaOptionsDialog::OkHdl(): Any other exception" );
        }
    }

    aConvDics.erase(aConvDics.begin() + nRplcIdx);
}

void ConvDicNameContainer::AddConvDics(
        const OUString &rSearchDirPathURL,
        const OUString &rExtension )
{
    const Sequence< OUString > aDirCnt(
                utl::LocalFileHelper::GetFolderContents( rSearchDirPathURL, false ) );
    const OUString *pDirCnt = aDirCnt.getConstArray();
    sal_Int32 nEntries = aDirCnt.getLength();

    for (sal_Int32 i = 0;  i < nEntries;  ++i)
    {
        OUString aURL( pDirCnt[i] );

        sal_Int32 nPos = aURL.lastIndexOf('.');
        OUString aExt( aURL.copy(nPos + 1).toAsciiLowerCase() );
        OUString aSearchExt( rExtension.toAsciiLowerCase() );
        if(aExt != aSearchExt)
            continue;          // skip other files

        sal_Int16 nLang;
        sal_Int16 nConvType;
        if (IsConvDic( aURL, nLang, nConvType ))
        {
            // get decoded dictionary file name
            INetURLObject aURLObj( aURL );
            OUString aDicName = aURLObj.getBase( INetURLObject::LAST_SEGMENT,
                        true, INetURLObject::DecodeMechanism::WithCharset );

            uno::Reference < XConversionDictionary > xDic;
            if (nLang == LANGUAGE_KOREAN &&
                nConvType == ConversionDictionaryType::HANGUL_HANJA)
            {
                xDic = new HHConvDic( aDicName, aURL );
            }
            else if ((nLang == LANGUAGE_CHINESE_SIMPLIFIED || nLang == LANGUAGE_CHINESE_TRADITIONAL) &&
                      nConvType == ConversionDictionaryType::SCHINESE_TCHINESE)
            {
                xDic = new ConvDic( aDicName, nLang, nConvType, false, aURL );
            }

            if (xDic.is())
            {
                insertByName( xDic->getName(), Any(xDic) );
            }
        }
    }
}

namespace
{
    struct StaticConvDicList : public rtl::StaticWithInit<
        uno::Reference<XInterface>, StaticConvDicList> {
        uno::Reference<XInterface> operator () () {
            return static_cast<cppu::OWeakObject *>(new ConvDicList);
        }
    };
}

void ConvDicList::MyAppExitListener::AtExit()
{
    rMyDicList.FlushDics();
    StaticConvDicList::get().clear();
}

ConvDicList::ConvDicList() :
    aEvtListeners( GetLinguMutex() )
{
    bDisposing = false;

    mxExitListener = new MyAppExitListener( *this );
    mxExitListener->Activate();
}

ConvDicList::~ConvDicList()
{
    if (!bDisposing && mxNameContainer.is())
        mxNameContainer->FlushDics();

    mxExitListener->Deactivate();
}

void ConvDicList::FlushDics()
{
    // check only pointer to avoid creating the container when
    // the dictionaries were not accessed yet
    if (mxNameContainer.is())
        mxNameContainer->FlushDics();
}

ConvDicNameContainer & ConvDicList::GetNameContainer()
{
    if (!mxNameContainer.is())
    {
        mxNameContainer = new ConvDicNameContainer;
        mxNameContainer->AddConvDics( GetDictionaryWriteablePath(), CONV_DIC_EXT  );

        // access list of text conversion dictionaries to activate
        SvtLinguOptions aOpt;
        SvtLinguConfig().GetOptions( aOpt );
        sal_Int32 nLen = aOpt.aActiveConvDics.getLength();
        const OUString *pActiveConvDics = aOpt.aActiveConvDics.getConstArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            uno::Reference< XConversionDictionary > xDic =
                    mxNameContainer->GetByName( pActiveConvDics[i] );
            if (xDic.is())
                xDic->setActive( true );
        }

        // since there is no UI to active/deactivate the dictionaries
        // for chinese text conversion they should be activated by default
        uno::Reference< XConversionDictionary > xS2TDic(
                    mxNameContainer->GetByName( "ChineseS2T" ), UNO_QUERY );
        uno::Reference< XConversionDictionary > xT2SDic(
                    mxNameContainer->GetByName( "ChineseT2S" ), UNO_QUERY );
            if (xS2TDic.is())
                xS2TDic->setActive( true );
            if (xT2SDic.is())
                xT2SDic->setActive( true );

    }
    return *mxNameContainer;
}

uno::Reference< container::XNameContainer > SAL_CALL ConvDicList::getDictionaryContainer(  )
{
    MutexGuard  aGuard( GetLinguMutex() );
    GetNameContainer();
    DBG_ASSERT( mxNameContainer.is(), "missing name container" );
    return mxNameContainer.get();
}

uno::Reference< XConversionDictionary > SAL_CALL ConvDicList::addNewDictionary(
        const OUString& rName,
        const Locale& rLocale,
        sal_Int16 nConvDicType )
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Int16 nLang = LinguLocaleToLanguage( rLocale );

    if (GetNameContainer().hasByName( rName ))
        throw ElementExistException();

    uno::Reference< XConversionDictionary > xRes;
    OUString aDicMainURL( GetConvDicMainURL( rName, GetDictionaryWriteablePath() ) );
    if (nLang == LANGUAGE_KOREAN &&
        nConvDicType == ConversionDictionaryType::HANGUL_HANJA)
    {
        xRes = new HHConvDic( rName, aDicMainURL );
    }
    else if ((nLang == LANGUAGE_CHINESE_SIMPLIFIED || nLang == LANGUAGE_CHINESE_TRADITIONAL) &&
              nConvDicType == ConversionDictionaryType::SCHINESE_TCHINESE)
    {
        xRes = new ConvDic( rName, nLang, nConvDicType, false, aDicMainURL );
    }

    if (!xRes.is())
        throw NoSupportException();
    else
    {
        xRes->setActive( true );
        GetNameContainer().insertByName( rName, Any(xRes) );
    }
    return xRes;
}

uno::Sequence< OUString > SAL_CALL ConvDicList::queryConversions(
        const OUString& rText,
        sal_Int32 nStartPos,
        sal_Int32 nLength,
        const Locale& rLocale,
        sal_Int16 nConversionDictionaryType,
        ConversionDirection eDirection,
        sal_Int32 nTextConversionOptions )
{
    MutexGuard  aGuard( GetLinguMutex() );

    std::vector< OUString > aRes;

    bool bSupported = false;
    sal_Int32 nLen = GetNameContainer().GetCount();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        const uno::Reference< XConversionDictionary > xDic( GetNameContainer().GetByIndex(i) );
        bool bMatch =   xDic.is()  &&
                            xDic->getLocale() == rLocale  &&
                            xDic->getConversionType() == nConversionDictionaryType;
        bSupported |= bMatch;
        if (bMatch  &&  xDic->isActive())
        {
            Sequence< OUString > aNewConv( xDic->getConversions(
                                rText, nStartPos, nLength,
                                eDirection, nTextConversionOptions ) );
            sal_Int32 nNewLen = aNewConv.getLength();
            if (nNewLen > 0)
            {
                for (sal_Int32 k = 0;  k < nNewLen;  ++k)
                    aRes.push_back(aNewConv[k]);
            }
        }
    }

    if (!bSupported)
        throw NoSupportException();

    return comphelper::containerToSequence(aRes);
}

sal_Int16 SAL_CALL ConvDicList::queryMaxCharCount(
        const Locale& rLocale,
        sal_Int16 nConversionDictionaryType,
        ConversionDirection eDirection )
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Int16 nRes = 0;
    GetNameContainer();
    sal_Int32 nLen = GetNameContainer().GetCount();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        const uno::Reference< XConversionDictionary > xDic( GetNameContainer().GetByIndex(i) );
        if (xDic.is()  &&
            xDic->getLocale() == rLocale  &&
            xDic->getConversionType() == nConversionDictionaryType)
        {
            sal_Int16 nC = xDic->getMaxCharCount( eDirection );
            if (nC > nRes)
                nRes = nC;
        }
    }
    return nRes;
}

void SAL_CALL ConvDicList::dispose(  )
{
    MutexGuard  aGuard( GetLinguMutex() );
    if (!bDisposing)
    {
        bDisposing = true;
        EventObject aEvtObj( static_cast<XConversionDictionaryList *>(this) );
        aEvtListeners.disposeAndClear( aEvtObj );

        FlushDics();
    }
}

void SAL_CALL ConvDicList::addEventListener(
        const uno::Reference< XEventListener >& rxListener )
{
    MutexGuard  aGuard( GetLinguMutex() );
    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}

void SAL_CALL ConvDicList::removeEventListener(
        const uno::Reference< XEventListener >& rxListener )
{
    MutexGuard  aGuard( GetLinguMutex() );
    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}

OUString SAL_CALL ConvDicList::getImplementationName()
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL ConvDicList::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL ConvDicList::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

uno::Sequence< OUString > ConvDicList::getSupportedServiceNames_Static()
    throw()
{
    uno::Sequence<OUString> aSNS { SN_CONV_DICTIONARY_LIST };
    return aSNS;
}

/// @throws css::uno::Exception
uno::Reference< uno::XInterface > SAL_CALL ConvDicList_CreateInstance(
        const uno::Reference< XMultiServiceFactory > & /*rSMgr*/ )
{
    return StaticConvDicList::get();
}

void * SAL_CALL ConvDicList_getFactory(
        const sal_Char * pImplName,
        XMultiServiceFactory * pServiceManager, void *  )
{
    void * pRet = nullptr;
    if ( ConvDicList::getImplementationName_Static().equalsAscii( pImplName ) )
    {
        uno::Reference< XSingleServiceFactory > xFactory =
            cppu::createOneInstanceFactory(
                pServiceManager,
                ConvDicList::getImplementationName_Static(),
                ConvDicList_CreateInstance,
                ConvDicList::getSupportedServiceNames_Static());
        // acquire, because we return an interface pointer instead of a reference
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
