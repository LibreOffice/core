/*************************************************************************
 *
 *  $RCSfile: convdiclist.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-04-27 16:06:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif
#ifndef _TOOLS_INTN_HXX
#include <tools/intn.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#include <svtools/useroptions.hxx>
#endif
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#include <cppuhelper/factory.hxx>   // helper for factories

#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif

#ifndef _COM_SUN_STAR_LINGUISTIC2_XCONVERSIONDICTIONARYLIST_HPP_
#include <com/sun/star/linguistic2/XConversionDictionaryList.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XCONVERSIONDICTIONARY_HPP_
#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_CONVERSIONDICTIONARYTYPE_HPP_
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HPP_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#include <ucbhelper/content.hxx>

#include "convdiclist.hxx"
#include "convdic.hxx"
#include "hhconvdic.hxx"
#include "misc.hxx"
#include "defs.hxx"

//using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

#define SN_CONV_DICTIONARY_LIST  "com.sun.star.linguistic2.ConversionDictionaryList"


///////////////////////////////////////////////////////////////////////////

bool operator == ( const Locale &r1, const Locale &r2 )
{
    return  r1.Language == r2.Language &&
            r1.Country  == r2.Country  &&
            r1.Variant  == r2.Variant;
}

///////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////

class ConvDicNameContainer :
    public cppu::WeakImplHelper1
    <
        ::com::sun::star::container::XNameContainer
    >
{
    uno::Sequence< Reference< XConversionDictionary > >   aConvDics;
    ConvDicList     &rConvDicList;

    // disallow copy-constructor and assignment-operator for now
    ConvDicNameContainer(const ConvDicNameContainer &);
    ConvDicNameContainer & operator = (const ConvDicNameContainer &);

    INT32 GetIndexByName_Impl( const OUString& rName );

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

    INT32   GetCount() const    { return aConvDics.getLength(); }
    Reference< XConversionDictionary > GetByName( const OUString& rName );

    const Reference< XConversionDictionary >    GetByIndex( INT32 nIdx )
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
    INT32 nLen = aConvDics.getLength();
    const Reference< XConversionDictionary > *pDic = aConvDics.getConstArray();
    for (INT32 i = 0;  i < nLen;  ++i)
    {
        Reference< util::XFlushable > xFlush( pDic[i] , UNO_QUERY );
        if (xFlush.is())
        {
            try
            {
                xFlush->flush();
            }
            catch(Exception &)
            {
                DBG_ERROR( "flushing of conversion dictionary failed" );
            }
        }
    }
}


INT32 ConvDicNameContainer::GetIndexByName_Impl(
        const OUString& rName )
{
    INT32 nRes = -1;
    INT32 nLen = aConvDics.getLength();
    const Reference< XConversionDictionary > *pDic = aConvDics.getConstArray();
    for (INT32 i = 0;  i < nLen && nRes == -1;  ++i)
    {
        if (rName == pDic[i]->getName())
            nRes = i;
    }
    return nRes;
}


Reference< XConversionDictionary > ConvDicNameContainer::GetByName(
        const OUString& rName )
{
    Reference< XConversionDictionary > xRes;
    INT32 nIdx = GetIndexByName_Impl( rName );
    if ( nIdx != -1)
        xRes = aConvDics.getArray()[nIdx];
    return xRes;
}


uno::Type SAL_CALL ConvDicNameContainer::getElementType(  )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return uno::Type( ::getCppuType( (Reference< XConversionDictionary > *) 0) );
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
    Reference< XConversionDictionary > xRes( GetByName( rName ) );
    if (!xRes.is())
        throw NoSuchElementException();
    return makeAny( xRes );
}


uno::Sequence< OUString > SAL_CALL ConvDicNameContainer::getElementNames(  )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    INT32 nLen = aConvDics.getLength();
    uno::Sequence< OUString > aRes( nLen );
    OUString *pName = aRes.getArray();
    const Reference< XConversionDictionary > *pDic = aConvDics.getConstArray();
    for (INT32 i = 0;  i < nLen;  ++i)
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

    INT32 nRplcIdx = GetIndexByName_Impl( rName );
    if (nRplcIdx == -1)
        throw NoSuchElementException();
    Reference< XConversionDictionary > xNew;
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
    Reference< XConversionDictionary > xNew;
    rElement >>= xNew;
    if (!xNew.is() || xNew->getName() != rName)
        throw IllegalArgumentException();

    INT32 nLen = aConvDics.getLength();
    aConvDics.realloc( nLen + 1 );
    aConvDics.getArray()[ nLen ] = xNew;
}


void SAL_CALL ConvDicNameContainer::removeByName( const OUString& rName )
    throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    INT32 nRplcIdx = GetIndexByName_Impl( rName );
    if (nRplcIdx == -1)
        throw NoSuchElementException();

    // physically remove dictionary
    Reference< XConversionDictionary > xDel = aConvDics.getArray()[nRplcIdx];
    String aName( xDel->getName() );
    String aDicMainURL( GetConvDicMainURL( aName, SvtPathOptions().GetUserDictionaryPath() ) );
    INetURLObject aObj( aDicMainURL );
    DBG_ASSERT( aObj.GetProtocol() == INET_PROT_FILE, "+HangulHanjaOptionsDialog::OkHdl(): non-file URLs cannot be deleted" );
    if( aObj.GetProtocol() == INET_PROT_FILE )
    {
        try
        {
            ::ucb::Content  aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ),
                                    Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
            aCnt.executeCommand( OUString::createFromAscii( "delete" ), makeAny( sal_Bool( sal_True ) ) );
        }
        catch( ::com::sun::star::ucb::CommandAbortedException& )
        {
            DBG_ERRORFILE( "HangulHanjaOptionsDialog::OkHdl(): CommandAbortedException" );
        }
        catch( ... )
        {
            DBG_ERRORFILE( "HangulHanjaOptionsDialog::OkHdl(): Any other exception" );
        }
    }

    INT32 nLen = aConvDics.getLength();
    Reference< XConversionDictionary > *pDic = aConvDics.getArray();
    for (INT32 i = nRplcIdx;  i < nLen - 1;  ++i)
        pDic[i] = pDic[i + 1];
    aConvDics.realloc( nLen - 1 );
}


void ConvDicNameContainer::AddConvDics(
        const String &rSearchDirPathURL,
        const String &rExtension )
{
    const Sequence< OUString > aDirCnt(
                utl::LocalFileHelper::GetFolderContents( rSearchDirPathURL, FALSE ) );
    const OUString *pDirCnt = aDirCnt.getConstArray();
    INT32 nEntries = aDirCnt.getLength();

    for (INT32 i = 0;  i < nEntries;  ++i)
    {
        String  aURL( pDirCnt[i] );

        xub_StrLen nPos  = aURL.SearchBackward('.');
        String  aExt(aURL.Copy(nPos + 1));
        aExt.ToLowerAscii();
        String  aSearchExt( rExtension );
        aSearchExt.ToLowerAscii();
        if(aExt != aSearchExt)
            continue;          // skip other files

        INT16 nLang;
        sal_Int16 nConvType;
        if (IsConvDic( aURL, nLang, nConvType ))
        {
            Reference < XConversionDictionary > xDic;
            if (nLang == LANGUAGE_KOREAN &&
                nConvType == ConversionDictionaryType::HANGUL_HANJA)
            {
                // get decoded dictionary file name
                INetURLObject aURLObj( aURL );
                String aDicName = aURLObj.getBase( INetURLObject::LAST_SEGMENT,
                            true, INetURLObject::DECODE_WITH_CHARSET,
                            RTL_TEXTENCODING_UTF8 );
                xDic = new HHConvDic( aDicName, aURL );
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

///////////////////////////////////////////////////////////////////////////

void ConvDicList::MyAppExitListener::AtExit()
{
    rMyDicList.FlushDics();
}

ConvDicList::ConvDicList() :
    aEvtListeners( GetLinguMutex() )
{
    pNameContainer = 0;
    bDisposing = FALSE;

    pExitListener = new MyAppExitListener( *this );
    xExitListener = pExitListener;
    pExitListener->Activate();
}


ConvDicList::~ConvDicList()
{
    // NameContainer will deleted when the reference xNameContainer
    // is destroyed.
    // delete pNameContainer;

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
        pNameContainer->AddConvDics( SvtPathOptions().GetUserDictionaryPath(),
                                     A2OU( CONV_DIC_EXT ) );
        xNameContainer = pNameContainer;

        // access list of text conversion dictionaries to activate
        SvtLinguOptions aOpt;
        SvtLinguConfig().GetOptions( aOpt );
        INT32 nLen = aOpt.aActiveConvDics.getLength();
        const OUString *pActiveConvDics = aOpt.aActiveConvDics.getConstArray();
        for (INT32 i = 0;  i < nLen;  ++i)
        {
            Reference< XConversionDictionary > xDic =
                    pNameContainer->GetByName( pActiveConvDics[i] );
            if (xDic.is())
                xDic->setActive( sal_True );
        }

    }
    return *pNameContainer;
}


Reference< container::XNameContainer > SAL_CALL ConvDicList::getDictionaryContainer(  ) throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    GetNameContainer();
    DBG_ASSERT( xNameContainer.is(), "missing name container" );
    return xNameContainer;
}


Reference< XConversionDictionary > SAL_CALL ConvDicList::addNewDictionary(
        const OUString& rName,
        const Locale& rLocale,
        sal_Int16 nConversionDictionaryType )
    throw (NoSupportException, ElementExistException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    INT16 nLang = LocaleToLanguage( rLocale );
    if (LANGUAGE_KOREAN != nLang  ||
        nConversionDictionaryType != ConversionDictionaryType::HANGUL_HANJA)
        throw NoSupportException();

    if (GetNameContainer().hasByName( rName ))
        throw ElementExistException();
    Reference< XConversionDictionary > xRes;
    DBG_ASSERT( nLang == LANGUAGE_KOREAN, "unexpected language" );
    DBG_ASSERT( nConversionDictionaryType == ConversionDictionaryType::HANGUL_HANJA,
            "unexpected conversion type" );
    String aDicMainURL( GetConvDicMainURL( rName, SvtPathOptions().GetUserDictionaryPath() ) );
    xRes = new HHConvDic( rName, aDicMainURL );
    if (xRes.is())
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

    INT16 nLang = LocaleToLanguage( rLocale );
    if (LANGUAGE_KOREAN != nLang  ||
        nConversionDictionaryType != ConversionDictionaryType::HANGUL_HANJA)
        throw NoSupportException();

    INT32 nCount = 0;
    uno::Sequence< OUString > aRes( 20 );
    OUString *pRes = aRes.getArray();

    INT32 nLen = GetNameContainer().GetCount();
    for (INT32 i = 0;  i < nLen;  ++i)
    {
        const Reference< XConversionDictionary > xDic( GetNameContainer().GetByIndex(i) );
        if (xDic.is()  &&
            xDic->getLocale() == rLocale  &&
            xDic->getConversionType() == nConversionDictionaryType)
        {
            Sequence< OUString > aNewConv( xDic->getConversions(
                                rText, nStartPos, nLength,
                                eDirection, nTextConversionOptions ) );
            INT32 nNewLen = aNewConv.getLength();
            if (nNewLen > 0)
            {
                if (nCount + nNewLen > aRes.getLength())
                {
                    aRes.realloc( nCount + nNewLen + 20 );
                    pRes = aRes.getArray();
                }
                const OUString *pNewConv = aNewConv.getConstArray();
                for (INT32 k = 0;  k < nNewLen;  ++k)
                    pRes[nCount++] = pNewConv[k];
            }
        }
    }

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
    INT32 nLen = GetNameContainer().GetCount();
    for (INT32 i = 0;  i < nLen;  ++i)
    {
        const Reference< XConversionDictionary > xDic( GetNameContainer().GetByIndex(i) );
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
        bDisposing = TRUE;
        EventObject aEvtObj( (XConversionDictionaryList *) this );
        aEvtListeners.disposeAndClear( aEvtObj );

        FlushDics();
    }
}


void SAL_CALL ConvDicList::addEventListener(
        const Reference< XEventListener >& rxListener )
    throw (RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}


void SAL_CALL ConvDicList::removeEventListener(
        const Reference< XEventListener >& rxListener )
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
    return rServiceName.equalsAscii( SN_CONV_DICTIONARY_LIST );
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
    aSNS.getArray()[0] = A2OU( SN_CONV_DICTIONARY_LIST );
    return aSNS;
}


///////////////////////////////////////////////////////////////////////////

Reference< XInterface > SAL_CALL ConvDicList_CreateInstance(
        const Reference< XMultiServiceFactory > & rSMgr )
    throw(Exception)
{
    Reference< XInterface > xService = (cppu::OWeakObject *) new ConvDicList;
    return xService;
}


sal_Bool SAL_CALL ConvDicList_writeInfo(
    void * /*pServiceManager*/, registry::XRegistryKey * pRegistryKey )
{
    try
    {
        String aImpl( '/' );
        aImpl += ConvDicList::getImplementationName_Static().getStr();
        aImpl.AppendAscii( "/UNO/SERVICES" );
        Reference< registry::XRegistryKey > xNewKey =
                pRegistryKey->createKey(aImpl );
        uno::Sequence< OUString > aServices =
                ConvDicList::getSupportedServiceNames_Static();
        for( INT32 i = 0; i < aServices.getLength(); i++ )
            xNewKey->createKey( aServices.getConstArray()[i]);

        return sal_True;
    }
    catch(Exception &)
    {
        return sal_False;
    }
}


void * SAL_CALL ConvDicList_getFactory(
        const sal_Char * pImplName,
        XMultiServiceFactory * pServiceManager, void *  )
{
    void * pRet = 0;
    if ( !ConvDicList::getImplementationName_Static().compareToAscii( pImplName ) )
    {
/*        Reference< XSingleServiceFactory > xFactory =
            cppu::createOneInstanceFactory(
                pServiceManager,
                ConvDicList::getImplementationName_Static(),
                ConvDicList_CreateInstance,
                ConvDicList::getSupportedServiceNames_Static());
*/
        Reference< XSingleServiceFactory > xFactory =
            cppu::createSingleFactory( pServiceManager,
                ConvDicList::getImplementationName_Static(),
                ConvDicList_CreateInstance,
                ConvDicList::getSupportedServiceNames_Static() );
        // acquire, because we return an interface pointer instead of a reference
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

///////////////////////////////////////////////////////////////////////////

