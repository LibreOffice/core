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

#include <string_view>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#include <com/sun/star/linguistic2/XConversionDictionaryList.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/util/XFlushable.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/lingucfg.hxx>
#include <comphelper/diagnose_ex.hxx>

#include "convdic.hxx"
#include "convdiclist.hxx"
#include "hhconvdic.hxx"
#include <linguistic/misc.hxx>

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

static OUString GetConvDicMainURL( std::u16string_view rDicName, std::u16string_view rDirectoryURL )
{
    // build URL to use for new (persistent) dictionaries

    OUString aFullDicName = OUString::Concat(rDicName) + CONV_DIC_DOT_EXT;

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

    sal_Int32 GetIndexByName_Impl( std::u16string_view rName );

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
    uno::Reference< XConversionDictionary > GetByName( std::u16string_view rName );

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
        std::u16string_view rName )
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
        std::u16string_view rName )
{
    uno::Reference< XConversionDictionary > xRes;
    sal_Int32 nIdx = GetIndexByName_Impl( rName );
    if ( nIdx != -1)
        xRes = aConvDics[nIdx];
    return xRes;
}

uno::Type SAL_CALL ConvDicNameContainer::getElementType(  )
{
    return cppu::UnoType<XConversionDictionary>::get();
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
    return Any( xRes );
}

uno::Sequence< OUString > SAL_CALL ConvDicNameContainer::getElementNames(  )
{
    MutexGuard  aGuard( GetLinguMutex() );

    std::vector<OUString> aRes;
    aRes.reserve(aConvDics.size());

    std::transform(aConvDics.begin(), aConvDics.end(), std::back_inserter(aRes),
        [](const uno::Reference<XConversionDictionary>& rDic) { return rDic->getName(); });

    return comphelper::containerToSequence(aRes);
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
            aCnt.executeCommand( u"delete"_ustr, Any( true ) );
        }
        catch( ... )
        {
            TOOLS_WARN_EXCEPTION( "linguistic", "HangulHanjaOptionsDialog::OkHdl()" );
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

    for (const OUString& aURL : aDirCnt)
    {
        sal_Int32 nPos = aURL.lastIndexOf('.');
        OUString aExt( aURL.copy(nPos + 1).toAsciiLowerCase() );
        OUString aSearchExt( rExtension.toAsciiLowerCase() );
        if(aExt != aSearchExt)
            continue;          // skip other files

        LanguageType nLang;
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
    rtl::Reference<ConvDicList>& StaticConvDicList()
    {
        static rtl::Reference<ConvDicList> SINGLETON = new ConvDicList;
        return SINGLETON;
    };
}

void ConvDicList::MyAppExitListener::AtExit()
{
    rMyDicList.FlushDics();
    StaticConvDicList().clear();
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
        for (const OUString& rActiveConvDic : aOpt.aActiveConvDics)
        {
            uno::Reference< XConversionDictionary > xDic =
                    mxNameContainer->GetByName( rActiveConvDic );
            if (xDic.is())
                xDic->setActive( true );
        }

        // since there is no UI to active/deactivate the dictionaries
        // for chinese text conversion they should be activated by default
        uno::Reference< XConversionDictionary > xS2TDic =
                    mxNameContainer->GetByName( u"ChineseS2T" );
        uno::Reference< XConversionDictionary > xT2SDic =
                    mxNameContainer->GetByName( u"ChineseT2S" );
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
    return mxNameContainer;
}

uno::Reference< XConversionDictionary > SAL_CALL ConvDicList::addNewDictionary(
        const OUString& rName,
        const Locale& rLocale,
        sal_Int16 nConvDicType )
{
    MutexGuard  aGuard( GetLinguMutex() );

    LanguageType nLang = LinguLocaleToLanguage( rLocale );

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

    xRes->setActive( true );
    GetNameContainer().insertByName( rName, Any(xRes) );
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
            const Sequence< OUString > aNewConv( xDic->getConversions(
                                rText, nStartPos, nLength,
                                eDirection, nTextConversionOptions ) );
            aRes.insert( aRes.end(), aNewConv.begin(), aNewConv.end() );
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
    return u"com.sun.star.lingu2.ConvDicList"_ustr;
}

sal_Bool SAL_CALL ConvDicList::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL ConvDicList::getSupportedServiceNames()
{
    return { u"com.sun.star.linguistic2.ConversionDictionaryList"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
linguistic_ConvDicList_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(StaticConvDicList().get());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
