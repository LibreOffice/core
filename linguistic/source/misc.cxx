/*************************************************************************
 *
 *  $RCSfile: misc.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tl $ $Date: 2000-11-21 18:29:14 $
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

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif

#include "misc.hxx"
#include "defs.hxx"
#include "lngprops.hxx"

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XFASTPROPERTYSET_HPP_
#include <com/sun/star/beans/XFastPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XTERMINATELISTENER_HPP_
#include <com/sun/star/frame/XTerminateListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/linguistic2/XDictionary1.hpp>
#include <com/sun/star/linguistic2/DictionaryType.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;

namespace linguistic
{

///////////////////////////////////////////////////////////////////////////

osl::Mutex &    GetLinguMutex()
{
    static osl::Mutex   aMutex;
    return aMutex;
}

///////////////////////////////////////////////////////////////////////////

BOOL IsUseDicList( const PropertyValues &rProperties,
        const Reference< XPropertySet > &rxProp )
{
    BOOL bRes = TRUE;

    INT32 nLen = rProperties.getLength();
    const PropertyValue *pVal = rProperties.getConstArray();
    for (INT32 i = 0;  i < nLen;  ++i)
    {
        if (UPH_IS_USE_DICTIONARY_LIST == pVal[i].Handle)
        {
            pVal[i].Value >>= bRes;
            break;
        }
    }
    if (i >= nLen)  // no temporary value found in 'rProperties'
    {
        Reference< XFastPropertySet > xFast( rxProp, UNO_QUERY );
        if (xFast.is())
            xFast->getFastPropertyValue( UPH_IS_USE_DICTIONARY_LIST ) >>= bRes;
    }

    return bRes;
}


static BOOL lcl_HasHyphInfo( const Reference<XDictionaryEntry> &xEntry )
{
    BOOL bRes = FALSE;
    if (xEntry.is())
    {
        // there has to be (at least one) '=' denoting a hyphenation position
        // and it must not be before any character of the word
        sal_Int32 nIdx = xEntry->getDictionaryWord().indexOf( '=' );
        bRes = nIdx != -1  &&  nIdx != 0;
    }
    return bRes;
}


Reference< XDictionaryEntry > SearchDicList(
        const Reference< XDictionaryList > &xDicList,
        const OUString &rWord, INT16 nLanguage,
        BOOL bSearchPosDics, BOOL bSearchSpellEntry )
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XDictionaryEntry > xEntry;

    if (!xDicList.is())
        return xEntry;

    const uno::Sequence< Reference< XDictionary > >
            aDics( xDicList->getDictionaries() );
    const Reference< XDictionary >
            *pDic = aDics.getConstArray();
    INT32 nDics = xDicList->getCount();

    INT32 i;
    for (i = 0;  i < nDics;  i++)
    {
        Reference< XDictionary1 > axDic( pDic[i], UNO_QUERY );

        DictionaryType  eType = axDic->getDictionaryType();
        INT16           nLang = axDic->getLanguage();

        if ( axDic.is() && axDic->isActive()
            && (nLang == nLanguage  ||  nLang == LANGUAGE_NONE) )
        {
            DBG_ASSERT( eType != DictionaryType_MIXED,
                "lng : unexpected dictionary type" );

            if (   (!bSearchPosDics  &&  eType == DictionaryType_NEGATIVE)
                || ( bSearchPosDics  &&  eType == DictionaryType_POSITIVE))
            {
                if ( (xEntry = axDic->getEntry( rWord )).is() )
                {
                    if (bSearchSpellEntry || lcl_HasHyphInfo( xEntry ))
                        break;
                }
            }
        }
    }

    return xEntry;
}


///////////////////////////////////////////////////////////////////////////

Locale CreateLocale( LanguageType eLang )
{
    if ( eLang == LANGUAGE_NONE )
        eLang = LANGUAGE_SYSTEM;
    String aLangStr, aCtryStr;
    ConvertLanguageToIsoNames( eLang, aLangStr, aCtryStr );

    return Locale( aLangStr, aCtryStr, OUString() );
}

LanguageType LocaleToLanguage( const Locale& rLocale )
{
    //  empty language -> LANGUAGE_NONE
    if ( rLocale.Language.getLength() == 0 )
        return LANGUAGE_NONE;

    //  Variant of Locale is ignored
    LanguageType eRet =
            ConvertIsoNamesToLanguage( rLocale.Language, rLocale.Country );
    if ( eRet == LANGUAGE_SYSTEM )
        eRet = LANGUAGE_NONE;

    return eRet;
}


Locale& LanguageToLocale( Locale& rLocale, LanguageType eLang )
{
    String aLangStr, aCtryStr;
    if ( eLang == LANGUAGE_NONE )
        eLang = LANGUAGE_SYSTEM;
    ConvertLanguageToIsoNames( eLang, aLangStr, aCtryStr );
    rLocale.Language = aLangStr;
    rLocale.Country  = aCtryStr;

    return rLocale;
}

uno::Sequence< Locale > LangSeqToLocaleSeq( const uno::Sequence< INT16 > &rLangSeq )
{
    const INT16 *pLang = rLangSeq.getConstArray();
    INT32 nCount = rLangSeq.getLength();

    uno::Sequence< Locale > aLocales( nCount );
    Locale *pLocale = aLocales.getArray();
    for (INT32 i = 0;  i < nCount;  ++i)
    {
        LanguageToLocale( pLocale[i], pLang[ i ] );
    }

    return aLocales;
}

uno::Sequence< INT16 >
    LocaleSeqToLangSeq( uno::Sequence< Locale > &rLocaleSeq )
{
    const Locale *pLocale = rLocaleSeq.getConstArray();
    INT32 nCount = rLocaleSeq.getLength();

    uno::Sequence< INT16 >  aLangs( nCount );
    INT16 *pLang = aLangs.getArray();
    for (INT32 i = 0;  i < nCount;  ++i)
    {
        pLang[i] = LocaleToLanguage( pLocale[i] );

    }

    return aLangs;
}

///////////////////////////////////////////////////////////////////////////

// TL_TODO:
// replace this non performant implementations with better ones
//

BOOL IsUpper( const String &rText, INT16 nLanguage )
{
    sal_Int32 bUpperFlags =
        ::com::sun::star::i18n::KCharacterType::UPPER |
        ::com::sun::star::i18n::KCharacterType::LETTER |
        ::com::sun::star::i18n::KCharacterType::DIGIT;
    return bUpperFlags == CharClass( CreateLocale( nLanguage ) ).
                getStringType( rText, 0, rText.Len() );
}


BOOL IsLower( const String &rText, INT16 nLanguage )
{
    sal_Int32 bUpperFlags =
        ::com::sun::star::i18n::KCharacterType::LOWER |
        ::com::sun::star::i18n::KCharacterType::LETTER |
        ::com::sun::star::i18n::KCharacterType::DIGIT;
    return bUpperFlags == CharClass( CreateLocale( nLanguage ) ).
                getStringType( rText, 0, rText.Len() );
}


String ToLower( const String &rText, INT16 nLanguage )
{
    return CharClass( CreateLocale( nLanguage ) ).lower( rText );
}


sal_Unicode ToLower( const sal_Unicode cChar, INT16 nLanguage )
{
    return CharClass( CreateLocale( nLanguage ) ).lower( cChar ).GetChar(0);
}


sal_Unicode ToUpper( const sal_Unicode cChar, INT16 nLanguage )
{
    return CharClass( CreateLocale( nLanguage ) ).upper( cChar ).GetChar(0);
}


BOOL HasDigits( const String &rText )
{
    xub_StrLen nLen = rText.Len();

    xub_StrLen i = 0;
    while (i < nLen)
    {
        sal_Unicode cChar = rText.GetChar( i++ );
        if ((sal_Unicode)'0' <= cChar  &&  cChar <= (sal_Unicode)'9')
            return TRUE;
    }
    return FALSE;
}


BOOL IsNumeric( const String &rText )
{
    BOOL bRes = FALSE;
    xub_StrLen nLen = rText.Len();
    if (nLen)
    {
        bRes = TRUE;
        xub_StrLen i = 0;
        while (i < nLen)
        {
            sal_Unicode cChar = rText.GetChar( i++ );
            if ( !((sal_Unicode)'0' <= cChar  &&  cChar <= (sal_Unicode)'9') )
            {
                bRes = FALSE;
                break;
            }
        }
    }
    return bRes;
}


///////////////////////////////////////////////////////////////////////////

Reference< XInterface > GetOneInstanceService( const char *pServiceName )
{
    Reference< XInterface > xRef;

    if (pServiceName)
    {
        Reference< XMultiServiceFactory >  xMgr( getProcessServiceFactory() );
        if (xMgr.is())
        {
            xRef = xMgr->createInstance( A2OU( pServiceName ) );
        }
    }

    return xRef;
}

Reference< XPropertySet > GetLinguProperties()
{
    return Reference< XPropertySet > (
        GetOneInstanceService( SN_LINGU_PROPERTIES ), UNO_QUERY );
}

Reference< XSearchableDictionaryList > GetSearchableDictionaryList()
{
    return Reference< XSearchableDictionaryList > (
        GetOneInstanceService( SN_DICTIONARY_LIST ), UNO_QUERY );
}

Reference< XDictionaryList > GetDictionaryList()
{
    return Reference< XDictionaryList > (
        GetOneInstanceService( SN_DICTIONARY_LIST ), UNO_QUERY );
}

///////////////////////////////////////////////////////////////////////////

AppExitListener::AppExitListener()
{
    // add object to Desktop EventListeners in order to properly call
    // the AtExit function at appliction exit.
    Reference< XMultiServiceFactory >
        xMgr = getProcessServiceFactory();

    if (xMgr.is())
    {
        xDesktop = Reference< frame::XDesktop >(
                xMgr->createInstance( A2OU( SN_DESKTOP ) ), UNO_QUERY );
    }
}

AppExitListener::~AppExitListener()
{
}


void AppExitListener::Activate()
{
    if (xDesktop.is())
        xDesktop->addTerminateListener( this );
}


void AppExitListener::Deactivate()
{
    if (xDesktop.is())
        xDesktop->removeTerminateListener( this );
}


void SAL_CALL
    AppExitListener::disposing( const EventObject& rEvtSource )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (xDesktop.is()  &&  rEvtSource.Source == xDesktop)
    {
        xDesktop = NULL;    //! release reference to desktop
    }
}


void SAL_CALL
    AppExitListener::queryTermination( const EventObject& rEvtSource )
        throw(frame::TerminationVetoException, RuntimeException)
{
    //MutexGuard    aGuard( GetLinguMutex() );
}


void SAL_CALL
    AppExitListener::notifyTermination( const EventObject& rEvtSource )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (xDesktop.is()  &&  rEvtSource.Source == xDesktop)
    {
        AtExit();
    }
}

///////////////////////////////////////////////////////////////////////////

}   // namespace linguistic

