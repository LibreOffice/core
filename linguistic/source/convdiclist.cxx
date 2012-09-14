/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <tools/fsys.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/lingucfg.hxx>
#include <rtl/instance.hxx>
#include <cppuhelper/factory.hxx>   // helper for factories
#include <unotools/localfilehelper.hxx>
#include <com/sun/star/linguistic2/XConversionDictionaryList.hpp>
#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/content.hxx>

#include "convdiclist.hxx"
#include "convdic.hxx"
#include "hhconvdic.hxx"
#include "linguistic/misc.hxx"
#include "defs.hxx"

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

using ::rtl::OUString;

#define SN_CONV_DICTIONARY_LIST  "com.sun.star.linguistic2.ConversionDictionaryList"



bool operator == ( const Locale &r1, const Locale &r2 )
{
    return  r1.Language == r2.Language &&
            r1.Country  == r2.Country  &&
            r1.Variant  == r2.Variant;
}


String GetConvDicMainURL( const String &rDicName, const String &rDirectoryURL )
{
    // build URL to use for new (persistent) dictionaries

    String aFullDicName( rDicName );
    aFullDicName.AppendAscii( CONV_DIC_DOT_EXT );

    INetURLObject aURLObj;
    aURLObj.SetSmartProtocol( INET_PROT_FILE );
    aURLObj.SetSmartURL( rDirectoryURL );
    aURLObj.Append( aFullDicName, INetURLObject::ENCODE_ALL );
    DBG_ASSERT(!aURLObj.HasError(), "invalid URL");
    if (aURLObj.HasError())
        return String();
    else
        return aURLObj.GetMainURL( INetURLObject::DECODE_TO_IURI );
}


class ConvDicNameContainer :
    public cppu::WeakImplHelper1
    <
        ::com::sun::star::container::XNameContainer
    >
{
    uno::Sequence< uno::Reference< XConversionDictionary > >   aConvDics;
    ConvDicList     &rConvDicList;

    // disallow copy-constructor and assignment-operator for now
    ConvDicNameContainer(const ConvDicNameContainer &);
    ConvDicNameContainer & operator = (const ConvDicNameContainer &);

    sal_Int32 GetIndexByName_Impl( const OUString& rName );

public:
    ConvDicNameContainer( ConvDicList &rMyConvDicList );
    virtual ~ConvDicNameContainer();

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XNameContainer
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const ::rtl::OUString& Name ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);


    // looks for conversion dictionaries with the specified extension
    // in the directory and adds them to the container
    void AddConvDics( const String &rSearchDirPathURL, const String &rExtension );

    // calls Flush for the dictionaries that support XFlushable
    void    FlushDics() const;

    sal_Int32   GetCount() const    { return aConvDics.getLength(); }
    uno::Reference< XConversionDictionary > GetByName( const OUString& rName );

    const uno::Reference< XConversionDictionary >    GetByIndex( sal_Int32 nIdx )
    {
        return aConvDics.getConstArray()[nIdx];
    }
};


ConvDicNameContainer::ConvDicNameContainer( ConvDicList &rMyConvDicList ) :
    rConvDicList( rMyConvDicList )
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
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return uno::Type( ::getCppuType( (uno::Reference< XConversionDictionary > *) 0) );
}


sal_Bool SAL_CALL ConvDicNameContainer::hasElements(  )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aConvDics.getLength() > 0;
}


uno::Any SAL_CALL ConvDicNameContainer::getByName( const OUString& rName )
    throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    uno::Reference< XConversionDictionary > xRes( GetByName( rName ) );
    if (!xRes.is())
        throw NoSuchElementException();
    return makeAny( xRes );
}


uno::Sequence< OUString > SAL_CALL ConvDicNameContainer::getElementNames(  )
    throw (RuntimeException)
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
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return GetByName( rName ).is();
}


void SAL_CALL ConvDicNameContainer::replaceByName(
        const OUString& rName,
        const uno::Any& rElement )
    throw (IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
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
    throw (IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
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
    throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Int32 nRplcIdx = GetIndexByName_Impl( rName );
    if (nRplcIdx == -1)
        throw NoSuchElementException();

    // physically remove dictionary
    uno::Reference< XConversionDictionary > xDel = aConvDics.getArray()[nRplcIdx];
    String aName( xDel->getName() );
    String aDicMainURL( GetConvDicMainURL( aName, GetDictionaryWriteablePath() ) );
    INetURLObject aObj( aDicMainURL );
    DBG_ASSERT( aObj.GetProtocol() == INET_PROT_FILE, "+HangulHanjaOptionsDialog::OkHdl(): non-file URLs cannot be deleted" );
    if( aObj.GetProtocol() == INET_PROT_FILE )
    {
        try
        {
            ::ucbhelper::Content    aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ),
                                    uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(),
                                    comphelper::getProcessComponentContext() );
            aCnt.executeCommand( "delete", makeAny( sal_Bool( sal_True ) ) );
        }
        catch( ::com::sun::star::ucb::CommandAbortedException& )
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
        const String &rSearchDirPathURL,
        const String &rExtension )
{
    const Sequence< OUString > aDirCnt(
                utl::LocalFileHelper::GetFolderContents( rSearchDirPathURL, sal_False ) );
    const OUString *pDirCnt = aDirCnt.getConstArray();
    sal_Int32 nEntries = aDirCnt.getLength();

    for (sal_Int32 i = 0;  i < nEntries;  ++i)
    {
        String  aURL( pDirCnt[i] );

        xub_StrLen nPos  = aURL.SearchBackward('.');
        String  aExt(aURL.Copy(nPos + 1));
        aExt.ToLowerAscii();
        String  aSearchExt( rExtension );
        aSearchExt.ToLowerAscii();
        if(aExt != aSearchExt)
            continue;          // skip other files

        sal_Int16 nLang;
        sal_Int16 nConvType;
        if (IsConvDic( aURL, nLang, nConvType ))
        {
            // get decoded dictionary file name
            INetURLObject aURLObj( aURL );
            String aDicName = aURLObj.getBase( INetURLObject::LAST_SEGMENT,
                        true, INetURLObject::DECODE_WITH_CHARSET,
                        RTL_TEXTENCODING_UTF8 );

            uno::Reference < XConversionDictionary > xDic;
            if (nLang == LANGUAGE_KOREAN &&
                nConvType == ConversionDictionaryType::HANGUL_HANJA)
            {
                xDic = new HHConvDic( aDicName, aURL );
            }
            else if ((nLang == LANGUAGE_CHINESE_SIMPLIFIED || nLang == LANGUAGE_CHINESE_TRADITIONAL) &&
                      nConvType == ConversionDictionaryType::SCHINESE_TCHINESE)
            {
                xDic = new ConvDic( aDicName, nLang, nConvType, sal_False, aURL );
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
            return (cppu::OWeakObject *) new ConvDicList;
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
    pNameContainer = 0;
    bDisposing = sal_False;

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
        pNameContainer = new ConvDicNameContainer( *this );
        pNameContainer->AddConvDics( GetDictionaryWriteablePath(), ::rtl::OUString(CONV_DIC_EXT)  );
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


uno::Reference< container::XNameContainer > SAL_CALL ConvDicList::getDictionaryContainer(  ) throw (RuntimeException)
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
    throw (NoSupportException, ElementExistException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Int16 nLang = LocaleToLanguage( rLocale );

    if (GetNameContainer().hasByName( rName ))
        throw ElementExistException();

    uno::Reference< XConversionDictionary > xRes;
    String aDicMainURL( GetConvDicMainURL( rName, GetDictionaryWriteablePath() ) );
    if (nLang == LANGUAGE_KOREAN &&
        nConvDicType == ConversionDictionaryType::HANGUL_HANJA)
    {
        xRes = new HHConvDic( rName, aDicMainURL );
    }
    else if ((nLang == LANGUAGE_CHINESE_SIMPLIFIED || nLang == LANGUAGE_CHINESE_TRADITIONAL) &&
              nConvDicType == ConversionDictionaryType::SCHINESE_TCHINESE)
    {
        xRes = new ConvDic( rName, nLang, nConvDicType, sal_False, aDicMainURL );
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
    throw (IllegalArgumentException, NoSupportException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Int32 nCount = 0;
    uno::Sequence< OUString > aRes( 20 );
    OUString *pRes = aRes.getArray();

    sal_Bool bSupported = sal_False;
    sal_Int32 nLen = GetNameContainer().GetCount();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        const uno::Reference< XConversionDictionary > xDic( GetNameContainer().GetByIndex(i) );
        sal_Bool bMatch =   xDic.is()  &&
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
    throw (RuntimeException)
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
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    if (!bDisposing)
    {
        bDisposing = sal_True;
        EventObject aEvtObj( (XConversionDictionaryList *) this );
        aEvtListeners.disposeAndClear( aEvtObj );

        FlushDics();
    }
}


void SAL_CALL ConvDicList::addEventListener(
        const uno::Reference< XEventListener >& rxListener )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}


void SAL_CALL ConvDicList::removeEventListener(
        const uno::Reference< XEventListener >& rxListener )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}


OUString SAL_CALL ConvDicList::getImplementationName(  )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return getImplementationName_Static();
}


sal_Bool SAL_CALL ConvDicList::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return rServiceName == SN_CONV_DICTIONARY_LIST;
}


uno::Sequence< OUString > SAL_CALL ConvDicList::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return getSupportedServiceNames_Static();
}


uno::Sequence< OUString > ConvDicList::getSupportedServiceNames_Static()
    throw()
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = SN_CONV_DICTIONARY_LIST;
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
    void * pRet = 0;
    if ( !ConvDicList::getImplementationName_Static().compareToAscii( pImplName ) )
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
