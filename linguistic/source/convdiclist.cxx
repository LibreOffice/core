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

#include <boost/noncopyable.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#include <com/sun/star/linguistic2/XConversionDictionaryList.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/util/XFlushable.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/processfactory.hxx>
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

bool operator == ( const Locale &r1, const Locale &r2 )
{
    return  r1.Language == r2.Language &&
            r1.Country  == r2.Country  &&
            r1.Variant  == r2.Variant;
}

OUString GetConvDicMainURL( const OUString &rDicName, const OUString &rDirectoryURL )
{
    // build URL to use for new (persistent) dictionaries

    OUString aFullDicName = rDicName + CONV_DIC_DOT_EXT;

    INetURLObject aURLObj;
    aURLObj.SetSmartProtocol( INetProtocol::File );
    aURLObj.SetSmartURL( rDirectoryURL );
    aURLObj.Append( aFullDicName, INetURLObject::ENCODE_ALL );
    DBG_ASSERT(!aURLObj.HasError(), "invalid URL");
    if (aURLObj.HasError())
        return OUString();
    else
        return aURLObj.GetMainURL( INetURLObject::DECODE_TO_IURI );
}

class ConvDicNameContainer :
    public cppu::WeakImplHelper< css::container::XNameContainer >,
    private boost::noncopyable
{
    uno::Sequence< uno::Reference< XConversionDictionary > >   aConvDics;

    sal_Int32 GetIndexByName_Impl( const OUString& rName );

public:
    ConvDicNameContainer();
    virtual ~ConvDicNameContainer();

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (css::uno::RuntimeException, std::exception) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement ) throw (css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement ) throw (css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByName( const OUString& Name ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // looks for conversion dictionaries with the specified extension
    // in the directory and adds them to the container
    void AddConvDics( const OUString &rSearchDirPathURL, const OUString &rExtension );

    // calls Flush for the dictionaries that support XFlushable
    void    FlushDics() const;

    sal_Int32   GetCount() const    { return aConvDics.getLength(); }
    uno::Reference< XConversionDictionary > GetByName( const OUString& rName );

    const uno::Reference< XConversionDictionary >    GetByIndex( sal_Int32 nIdx )
    {
        return aConvDics.getConstArray()[nIdx];
    }
};

ConvDicNameContainer::ConvDicNameContainer()
{
}

ConvDicNameContainer::~ConvDicNameContainer()
{
}

void ConvDicNameContainer::FlushDics() const
{
    sal_Int32 nLen = aConvDics.getLength();
    const uno::Reference< XConversionDictionary > *pDic = aConvDics.getConstArray();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        uno::Reference< util::XFlushable > xFlush( pDic[i] , UNO_QUERY );
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
    sal_Int32 nLen = aConvDics.getLength();
    const uno::Reference< XConversionDictionary > *pDic = aConvDics.getConstArray();
    for (sal_Int32 i = 0;  i < nLen && nRes == -1;  ++i)
    {
        if (rName == pDic[i]->getName())
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
        xRes = aConvDics.getArray()[nIdx];
    return xRes;
}

uno::Type SAL_CALL ConvDicNameContainer::getElementType(  )
    throw (RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return uno::Type( cppu::UnoType<XConversionDictionary>::get());
}

sal_Bool SAL_CALL ConvDicNameContainer::hasElements(  )
    throw (RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aConvDics.getLength() > 0;
}

uno::Any SAL_CALL ConvDicNameContainer::getByName( const OUString& rName )
    throw (NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );
    uno::Reference< XConversionDictionary > xRes( GetByName( rName ) );
    if (!xRes.is())
        throw NoSuchElementException();
    return makeAny( xRes );
}

uno::Sequence< OUString > SAL_CALL ConvDicNameContainer::getElementNames(  )
    throw (RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Int32 nLen = aConvDics.getLength();
    uno::Sequence< OUString > aRes( nLen );
    OUString *pName = aRes.getArray();
    const uno::Reference< XConversionDictionary > *pDic = aConvDics.getConstArray();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
        pName[i] = pDic[i]->getName();
    return aRes;
}

sal_Bool SAL_CALL ConvDicNameContainer::hasByName( const OUString& rName )
    throw (RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return GetByName( rName ).is();
}

void SAL_CALL ConvDicNameContainer::replaceByName(
        const OUString& rName,
        const uno::Any& rElement )
    throw (IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Int32 nRplcIdx = GetIndexByName_Impl( rName );
    if (nRplcIdx == -1)
        throw NoSuchElementException();
    uno::Reference< XConversionDictionary > xNew;
    rElement >>= xNew;
    if (!xNew.is() || xNew->getName() != rName)
        throw IllegalArgumentException();
    aConvDics.getArray()[ nRplcIdx ] = xNew;
}

void SAL_CALL ConvDicNameContainer::insertByName(
        const OUString& rName,
        const Any& rElement )
    throw (IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (GetByName( rName ).is())
        throw ElementExistException();
    uno::Reference< XConversionDictionary > xNew;
    rElement >>= xNew;
    if (!xNew.is() || xNew->getName() != rName)
        throw IllegalArgumentException();

    sal_Int32 nLen = aConvDics.getLength();
    aConvDics.realloc( nLen + 1 );
    aConvDics.getArray()[ nLen ] = xNew;
}

void SAL_CALL ConvDicNameContainer::removeByName( const OUString& rName )
    throw (NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Int32 nRplcIdx = GetIndexByName_Impl( rName );
    if (nRplcIdx == -1)
        throw NoSuchElementException();

    // physically remove dictionary
    uno::Reference< XConversionDictionary > xDel = aConvDics.getArray()[nRplcIdx];
    OUString aName( xDel->getName() );
    OUString aDicMainURL( GetConvDicMainURL( aName, GetDictionaryWriteablePath() ) );
    INetURLObject aObj( aDicMainURL );
    DBG_ASSERT( aObj.GetProtocol() == INetProtocol::File, "+HangulHanjaOptionsDialog::OkHdl(): non-file URLs cannot be deleted" );
    if( aObj.GetProtocol() == INetProtocol::File )
    {
        try
        {
            ::ucbhelper::Content    aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ),
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

    sal_Int32 nLen = aConvDics.getLength();
    uno::Reference< XConversionDictionary > *pDic = aConvDics.getArray();
    for (sal_Int32 i = nRplcIdx;  i < nLen - 1;  ++i)
        pDic[i] = pDic[i + 1];
    aConvDics.realloc( nLen - 1 );
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
                        true, INetURLObject::DECODE_WITH_CHARSET );

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
                uno::Any aAny;
                aAny <<= xDic;
                insertByName( xDic->getName(), aAny );
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
    pNameContainer = nullptr;
    bDisposing = false;

    pExitListener = new MyAppExitListener( *this );
    xExitListener = pExitListener;
    pExitListener->Activate();
}

ConvDicList::~ConvDicList()
{

    if (!bDisposing && pNameContainer)
        pNameContainer->FlushDics();

    pExitListener->Deactivate();
}

void ConvDicList::FlushDics()
{
    // check only pointer to avoid creating the container when
    // the dictionaries were not accessed yet
    if (pNameContainer)
        pNameContainer->FlushDics();
}

ConvDicNameContainer & ConvDicList::GetNameContainer()
{
    if (!pNameContainer)
    {
        pNameContainer = new ConvDicNameContainer;
        pNameContainer->AddConvDics( GetDictionaryWriteablePath(), CONV_DIC_EXT  );
        xNameContainer = pNameContainer;

        // access list of text conversion dictionaries to activate
        SvtLinguOptions aOpt;
        SvtLinguConfig().GetOptions( aOpt );
        sal_Int32 nLen = aOpt.aActiveConvDics.getLength();
        const OUString *pActiveConvDics = aOpt.aActiveConvDics.getConstArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            uno::Reference< XConversionDictionary > xDic =
                    pNameContainer->GetByName( pActiveConvDics[i] );
            if (xDic.is())
                xDic->setActive( sal_True );
        }

        // since there is no UI to active/deactivate the dictionaries
        // for chinese text conversion they should be activated by default
        uno::Reference< XConversionDictionary > xS2TDic(
                    pNameContainer->GetByName( "ChineseS2T" ), UNO_QUERY );
        uno::Reference< XConversionDictionary > xT2SDic(
                    pNameContainer->GetByName( "ChineseT2S" ), UNO_QUERY );
            if (xS2TDic.is())
                xS2TDic->setActive( sal_True );
            if (xT2SDic.is())
                xT2SDic->setActive( sal_True );

    }
    return *pNameContainer;
}

uno::Reference< container::XNameContainer > SAL_CALL ConvDicList::getDictionaryContainer(  ) throw (RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );
    GetNameContainer();
    DBG_ASSERT( xNameContainer.is(), "missing name container" );
    return xNameContainer;
}

uno::Reference< XConversionDictionary > SAL_CALL ConvDicList::addNewDictionary(
        const OUString& rName,
        const Locale& rLocale,
        sal_Int16 nConvDicType )
    throw (NoSupportException, ElementExistException, RuntimeException, std::exception)
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
        xRes->setActive( sal_True );
        uno::Any aAny;
        aAny <<= xRes;
        GetNameContainer().insertByName( rName, aAny );
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
    throw (IllegalArgumentException, NoSupportException, RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Int32 nCount = 0;
    uno::Sequence< OUString > aRes( 20 );
    OUString *pRes = aRes.getArray();

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
                if (nCount + nNewLen > aRes.getLength())
                {
                    aRes.realloc( nCount + nNewLen + 20 );
                    pRes = aRes.getArray();
                }
                const OUString *pNewConv = aNewConv.getConstArray();
                for (sal_Int32 k = 0;  k < nNewLen;  ++k)
                    pRes[nCount++] = pNewConv[k];
            }
        }
    }

    if (!bSupported)
        throw NoSupportException();

    aRes.realloc( nCount );
    return aRes;
}

sal_Int16 SAL_CALL ConvDicList::queryMaxCharCount(
        const Locale& rLocale,
        sal_Int16 nConversionDictionaryType,
        ConversionDirection eDirection )
    throw (RuntimeException, std::exception)
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
    throw (RuntimeException, std::exception)
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
    throw (RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );
    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}

void SAL_CALL ConvDicList::removeEventListener(
        const uno::Reference< XEventListener >& rxListener )
    throw (RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );
    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}

OUString SAL_CALL ConvDicList::getImplementationName()
    throw (RuntimeException, std::exception)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL ConvDicList::supportsService( const OUString& rServiceName )
    throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL ConvDicList::getSupportedServiceNames()
    throw (RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}

uno::Sequence< OUString > ConvDicList::getSupportedServiceNames_Static()
    throw()
{
    uno::Sequence<OUString> aSNS { SN_CONV_DICTIONARY_LIST };
    return aSNS;
}

uno::Reference< uno::XInterface > SAL_CALL ConvDicList_CreateInstance(
        const uno::Reference< XMultiServiceFactory > & /*rSMgr*/ )
    throw(Exception)
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
