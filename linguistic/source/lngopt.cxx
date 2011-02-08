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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_linguistic.hxx"

#include <sal/macros.h>
#include "lngopt.hxx"
#include "lngprops.hxx"
#include "misc.hxx"
#include <tools/debug.hxx>
#include <unotools/lingucfg.hxx>

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations

#include <cppuhelper/factory.hxx>   // helper for factories
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <unotools/processfactory.hxx>
#include <i18npool/mslangid.hxx>

using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

using namespace com::sun::star::registry;

///////////////////////////////////////////////////////////////////////////


// static member intialization
SvtLinguOptions *   LinguOptions::pData = NULL;
oslInterlockedCount LinguOptions::nRefCount;


LinguOptions::LinguOptions()
{
    if (!pData)
    {
        pData = new SvtLinguOptions;
        SvtLinguConfig aLinguCfg;
        aLinguCfg.GetOptions( *pData );
    }

    osl_incrementInterlockedCount( &nRefCount );
}


LinguOptions::LinguOptions(const LinguOptions & /*rOpt*/)
{
    DBG_ASSERT( pData, "lng : data missing" );
    osl_incrementInterlockedCount( &nRefCount );
}


LinguOptions::~LinguOptions()
{
    MutexGuard  aGuard( GetLinguMutex() );

    if ( osl_decrementInterlockedCount( &nRefCount ) == 0 )
    {
        delete pData;   pData  = NULL;
    }
}


BOOL LinguOptions::SetLocale_Impl( INT16 &rLanguage, Any &rOld, const Any &rVal, sal_Int16 nType)
{
    BOOL bRes = FALSE;

    Locale  aNew;
    rVal >>= aNew;
        INT16 nNew = MsLangId::resolveSystemLanguageByScriptType(MsLangId::convertLocaleToLanguage(aNew), nType);
    if (nNew != rLanguage)
    {
        Locale  aLocale( CreateLocale( rLanguage ) );
        rOld.setValue( &aLocale, ::getCppuType((Locale*)0 ));
        rLanguage = nNew;
        bRes = TRUE;
    }

    return bRes;
}


BOOL LinguOptions::SetValue( Any &rOld, const Any &rVal, INT32 nWID )
{
    MutexGuard  aGuard( GetLinguMutex() );

    BOOL bRes = FALSE;

    INT16 *pnVal = 0;
    BOOL  *pbVal = 0;

    switch( nWID )
    {
        case WID_IS_GERMAN_PRE_REFORM :     /*! deprecated !*/ break;
        case WID_IS_USE_DICTIONARY_LIST :   pbVal = &pData->bIsUseDictionaryList;   break;
        case WID_IS_IGNORE_CONTROL_CHARACTERS : pbVal = &pData->bIsIgnoreControlCharacters; break;
        case WID_IS_HYPH_AUTO :             pbVal = &pData->bIsHyphAuto;    break;
        case WID_IS_HYPH_SPECIAL :          pbVal = &pData->bIsHyphSpecial; break;
        case WID_IS_SPELL_AUTO :            pbVal = &pData->bIsSpellAuto;   break;
        case WID_IS_SPELL_HIDE :            /*! deprecated !*/ break;
        case WID_IS_SPELL_IN_ALL_LANGUAGES :/*! deprecated !*/ break;
        case WID_IS_SPELL_SPECIAL :         pbVal = &pData->bIsSpellSpecial;    break;
        case WID_IS_WRAP_REVERSE :          pbVal = &pData->bIsSpellReverse;    break;
        case WID_DEFAULT_LANGUAGE :         pnVal = &pData->nDefaultLanguage;   break;
        case WID_IS_SPELL_CAPITALIZATION :  pbVal = &pData->bIsSpellCapitalization;     break;
        case WID_IS_SPELL_WITH_DIGITS :     pbVal = &pData->bIsSpellWithDigits; break;
        case WID_IS_SPELL_UPPER_CASE :      pbVal = &pData->bIsSpellUpperCase;      break;
        case WID_HYPH_MIN_LEADING :         pnVal = &pData->nHyphMinLeading;        break;
        case WID_HYPH_MIN_TRAILING :        pnVal = &pData->nHyphMinTrailing;   break;
        case WID_HYPH_MIN_WORD_LENGTH :     pnVal = &pData->nHyphMinWordLength; break;
        case WID_DEFAULT_LOCALE :
        {
            bRes = SetLocale_Impl( pData->nDefaultLanguage, rOld, rVal, ::com::sun::star::i18n::ScriptType::LATIN );
            break;
        }
        case WID_DEFAULT_LOCALE_CJK :
        {
            bRes = SetLocale_Impl( pData->nDefaultLanguage_CJK, rOld, rVal, ::com::sun::star::i18n::ScriptType::ASIAN );
            break;
        }
        case WID_DEFAULT_LOCALE_CTL :
        {
            bRes = SetLocale_Impl( pData->nDefaultLanguage_CTL, rOld, rVal, ::com::sun::star::i18n::ScriptType::COMPLEX );
            break;
        }
        default :
        {
            DBG_ASSERT( 0,"lng : unknown WID");
            bRes = FALSE;
        }
    }

    if (pbVal)
    {
        BOOL bNew = FALSE;
        rVal >>= bNew;
        if (bNew != *pbVal)
        {
            rOld <<= *pbVal;
            *pbVal = bNew;
            bRes = TRUE;
        }
    }
    if (pnVal)
    {
        INT16 nNew = 0;
        rVal >>= nNew;
        if (nNew != *pnVal)
        {
            rOld <<= *pnVal;
            *pnVal = nNew;
            bRes = TRUE;
        }
    }

//  if (bRes)
//      pData->SetModified();

    return bRes;
}

void LinguOptions::GetValue( Any &rVal, INT32 nWID ) const
{
    MutexGuard  aGuard( GetLinguMutex() );

    INT16 *pnVal = 0;
    BOOL  *pbVal = 0;
    BOOL  bDummy = FALSE;

    switch( nWID )
    {
        case WID_IS_GERMAN_PRE_REFORM :     pbVal = &bDummy; /*! deprecated !*/ break;
        case WID_IS_USE_DICTIONARY_LIST :   pbVal = &pData->bIsUseDictionaryList;   break;
        case WID_IS_IGNORE_CONTROL_CHARACTERS : pbVal = &pData->bIsIgnoreControlCharacters; break;
        case WID_IS_HYPH_AUTO :             pbVal = &pData->bIsHyphAuto;    break;
        case WID_IS_HYPH_SPECIAL :          pbVal = &pData->bIsHyphSpecial; break;
        case WID_IS_SPELL_AUTO :            pbVal = &pData->bIsSpellAuto;   break;
        case WID_IS_SPELL_HIDE :            pbVal = &bDummy; /*! deprecated !*/ break;
        case WID_IS_SPELL_IN_ALL_LANGUAGES :pbVal = &bDummy; /*! deprecated !*/ break;
        case WID_IS_SPELL_SPECIAL :         pbVal = &pData->bIsSpellSpecial;    break;
        case WID_IS_WRAP_REVERSE :          pbVal = &pData->bIsSpellReverse;    break;
        case WID_DEFAULT_LANGUAGE :         pnVal = &pData->nDefaultLanguage;   break;
        case WID_IS_SPELL_CAPITALIZATION :  pbVal = &pData->bIsSpellCapitalization;     break;
        case WID_IS_SPELL_WITH_DIGITS :     pbVal = &pData->bIsSpellWithDigits; break;
        case WID_IS_SPELL_UPPER_CASE :      pbVal = &pData->bIsSpellUpperCase;      break;
        case WID_HYPH_MIN_LEADING :         pnVal = &pData->nHyphMinLeading;        break;
        case WID_HYPH_MIN_TRAILING :        pnVal = &pData->nHyphMinTrailing;   break;
        case WID_HYPH_MIN_WORD_LENGTH :     pnVal = &pData->nHyphMinWordLength; break;
        case WID_DEFAULT_LOCALE :
        {
            Locale aLocale( MsLangId::convertLanguageToLocale( pData->nDefaultLanguage ) );
            rVal.setValue( &aLocale, ::getCppuType((Locale*)0 ));
            break;
        }
        case WID_DEFAULT_LOCALE_CJK :
        {
            Locale aLocale( MsLangId::convertLanguageToLocale( pData->nDefaultLanguage_CJK ) );
            rVal.setValue( &aLocale, ::getCppuType((Locale*)0 ));
            break;
        }
        case WID_DEFAULT_LOCALE_CTL :
        {
            Locale aLocale( MsLangId::convertLanguageToLocale( pData->nDefaultLanguage_CTL ) );
            rVal.setValue( &aLocale, ::getCppuType((Locale*)0 ));
            break;
        }
        default :
        {
            DBG_ASSERT( 0,"lng : unknown WID");
        }
    }

    if (pbVal)
        rVal <<= *pbVal;
    if (pnVal)
        rVal <<= *pnVal;
}


struct WID_Name
{
    INT32        nWID;
    const char  *pPropertyName;
};

//! order of entries is import (see LinguOptions::GetName)
//! since the WID is used as index in this table!
WID_Name aWID_Name[] =
{
    { 0,                                  0 },
    { WID_IS_USE_DICTIONARY_LIST,         UPN_IS_USE_DICTIONARY_LIST },
    { WID_IS_IGNORE_CONTROL_CHARACTERS,   UPN_IS_IGNORE_CONTROL_CHARACTERS },
    { WID_IS_SPELL_UPPER_CASE,            UPN_IS_SPELL_UPPER_CASE },
    { WID_IS_SPELL_WITH_DIGITS,           UPN_IS_SPELL_WITH_DIGITS },
    { WID_IS_SPELL_CAPITALIZATION,        UPN_IS_SPELL_CAPITALIZATION },
    { WID_HYPH_MIN_LEADING,               UPN_HYPH_MIN_LEADING },
    { WID_HYPH_MIN_TRAILING,              UPN_HYPH_MIN_TRAILING },
    { WID_HYPH_MIN_WORD_LENGTH,           UPN_HYPH_MIN_WORD_LENGTH },
    { WID_DEFAULT_LOCALE,                 UPN_DEFAULT_LOCALE },
    { WID_IS_SPELL_AUTO,                  UPN_IS_SPELL_AUTO },
    { 0,                                  0 },
    { 0,                                  0 },
    { WID_IS_SPELL_SPECIAL,               UPN_IS_SPELL_SPECIAL },
    { WID_IS_HYPH_AUTO,                   UPN_IS_HYPH_AUTO },
    { WID_IS_HYPH_SPECIAL,                UPN_IS_HYPH_SPECIAL },
    { WID_IS_WRAP_REVERSE,                UPN_IS_WRAP_REVERSE },
    { 0,                                  0 },
    { 0,                                  0 },
    { 0,                                  0 },
    { 0,                                  0 },
    { WID_DEFAULT_LANGUAGE,               UPN_DEFAULT_LANGUAGE },
    { WID_DEFAULT_LOCALE_CJK,             UPN_DEFAULT_LOCALE_CJK },
    { WID_DEFAULT_LOCALE_CTL,             UPN_DEFAULT_LOCALE_CTL }
};


OUString LinguOptions::GetName( INT32 nWID )
{
    MutexGuard  aGuard( GetLinguMutex() );

    OUString aRes;

    INT32 nLen = SAL_N_ELEMENTS( aWID_Name );
    if (0 <= nWID && nWID < nLen && aWID_Name[ nWID ].nWID == nWID)
        aRes = OUString::createFromAscii(aWID_Name[nWID].pPropertyName);
    else
        OSL_ENSURE(false, "lng : unknown WID");

    return aRes;
}


///////////////////////////////////////////////////////////////////////////

//! map must be sorted by first entry in alphabetical increasing order.
const SfxItemPropertyMapEntry* lcl_GetLinguProps()
{
    static const SfxItemPropertyMapEntry aLinguProps[] =
    {
        { MAP_CHAR_LEN(UPN_DEFAULT_LANGUAGE),           WID_DEFAULT_LANGUAGE,
                &::getCppuType( (sal_Int16*)0 ),    0, 0 },
        { MAP_CHAR_LEN(UPN_DEFAULT_LOCALE),             WID_DEFAULT_LOCALE,
                &::getCppuType( (Locale* )0),       0, 0 },
        { MAP_CHAR_LEN(UPN_DEFAULT_LOCALE_CJK),         WID_DEFAULT_LOCALE_CJK,
                &::getCppuType( (Locale* )0),       0, 0 },
        { MAP_CHAR_LEN(UPN_DEFAULT_LOCALE_CTL),         WID_DEFAULT_LOCALE_CTL,
                &::getCppuType( (Locale* )0),       0, 0 },
        { MAP_CHAR_LEN(UPN_HYPH_MIN_LEADING),           WID_HYPH_MIN_LEADING,
                &::getCppuType( (sal_Int16*)0 ),    0, 0 },
        { MAP_CHAR_LEN(UPN_HYPH_MIN_TRAILING),          WID_HYPH_MIN_TRAILING,
                &::getCppuType( (sal_Int16*)0 ),    0, 0 },
        { MAP_CHAR_LEN(UPN_HYPH_MIN_WORD_LENGTH),       WID_HYPH_MIN_WORD_LENGTH,
                &::getCppuType( (sal_Int16*)0 ),    0, 0 },
        { MAP_CHAR_LEN(UPN_IS_GERMAN_PRE_REFORM),       WID_IS_GERMAN_PRE_REFORM,       /*! deprecated !*/
                &::getBooleanCppuType(),            0, 0 },
        { MAP_CHAR_LEN(UPN_IS_HYPH_AUTO),               WID_IS_HYPH_AUTO,
                &::getBooleanCppuType(),            0, 0 },
        { MAP_CHAR_LEN(UPN_IS_HYPH_SPECIAL),            WID_IS_HYPH_SPECIAL,
                &::getBooleanCppuType(),            0, 0 },
        { MAP_CHAR_LEN(UPN_IS_IGNORE_CONTROL_CHARACTERS),   WID_IS_IGNORE_CONTROL_CHARACTERS,
                &::getBooleanCppuType(),            0, 0 },
        { MAP_CHAR_LEN(UPN_IS_SPELL_AUTO),              WID_IS_SPELL_AUTO,
                &::getBooleanCppuType(),            0, 0 },
        { MAP_CHAR_LEN(UPN_IS_SPELL_CAPITALIZATION),    WID_IS_SPELL_CAPITALIZATION,
                &::getBooleanCppuType(),            0, 0 },
        { MAP_CHAR_LEN(UPN_IS_SPELL_HIDE),              WID_IS_SPELL_HIDE,              /*! deprecated !*/
                &::getBooleanCppuType(),            0, 0 },
        { MAP_CHAR_LEN(UPN_IS_SPELL_IN_ALL_LANGUAGES),  WID_IS_SPELL_IN_ALL_LANGUAGES,  /*! deprecated !*/
                &::getBooleanCppuType(),            0, 0 },
        { MAP_CHAR_LEN(UPN_IS_SPELL_SPECIAL),           WID_IS_SPELL_SPECIAL,
                &::getBooleanCppuType(),            0, 0 },
        { MAP_CHAR_LEN(UPN_IS_SPELL_UPPER_CASE),        WID_IS_SPELL_UPPER_CASE,
                &::getBooleanCppuType(),            0, 0 },
        { MAP_CHAR_LEN(UPN_IS_SPELL_WITH_DIGITS),       WID_IS_SPELL_WITH_DIGITS,
                &::getBooleanCppuType(),            0, 0 },
        { MAP_CHAR_LEN(UPN_IS_USE_DICTIONARY_LIST),     WID_IS_USE_DICTIONARY_LIST,
                &::getBooleanCppuType(),            0, 0 },
        { MAP_CHAR_LEN(UPN_IS_WRAP_REVERSE),            WID_IS_WRAP_REVERSE,
                &::getBooleanCppuType(),            0, 0 },
        { 0,0,0,0,0,0 }
    };
    return aLinguProps;
}
LinguProps::LinguProps() :
    aEvtListeners   (GetLinguMutex()),
    aPropListeners  (GetLinguMutex()),
    aPropertyMap(lcl_GetLinguProps())
{
    bDisposing = FALSE;
}

void LinguProps::launchEvent( const PropertyChangeEvent &rEvt ) const
{
    cppu::OInterfaceContainerHelper *pContainer =
            aPropListeners.getContainer( rEvt.PropertyHandle );
    if (pContainer)
    {
        cppu::OInterfaceIteratorHelper aIt( *pContainer );
        while (aIt.hasMoreElements())
        {
            Reference< XPropertyChangeListener > xRef( aIt.next(), UNO_QUERY );
            if (xRef.is())
                xRef->propertyChange( rEvt );
        }
    }
}

Reference< XInterface > SAL_CALL LinguProps_CreateInstance(
            const Reference< XMultiServiceFactory > & /*rSMgr*/ )
        throw(Exception)
{
    Reference< XInterface > xService = (cppu::OWeakObject*)new LinguProps;
    return xService;
}

Reference< XPropertySetInfo > SAL_CALL LinguProps::getPropertySetInfo()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    static Reference< XPropertySetInfo > aRef =
            new SfxItemPropertySetInfo( &aPropertyMap );
    return aRef;
}

void SAL_CALL LinguProps::setPropertyValue(
            const OUString& rPropertyName, const Any& rValue )
        throw(UnknownPropertyException, PropertyVetoException,
              IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    const SfxItemPropertySimpleEntry* pCur = aPropertyMap.getByName( rPropertyName );
    if (pCur)
    {
        Any aOld( aConfig.GetProperty( pCur->nWID ) );
        if (aOld != rValue && aConfig.SetProperty( pCur->nWID, rValue ))
        {
            PropertyChangeEvent aChgEvt( (XPropertySet *) this, rPropertyName,
                    FALSE, pCur->nWID, aOld, rValue );
            launchEvent( aChgEvt );
        }
    }
#ifdef LINGU_EXCEPTIONS
    else
    {
        throw UnknownPropertyException();
    }
#endif
}

Any SAL_CALL LinguProps::getPropertyValue( const OUString& rPropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Any aRet;

    const SfxItemPropertySimpleEntry* pCur = aPropertyMap.getByName( rPropertyName );
    if(pCur)
    {
        aRet = aConfig.GetProperty( pCur->nWID );
    }
#ifdef LINGU_EXCEPTIONS
    else
    {
        throw UnknownPropertyException();
    }
#endif

    return aRet;
}

void SAL_CALL LinguProps::addPropertyChangeListener(
            const OUString& rPropertyName,
            const Reference< XPropertyChangeListener >& rxListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
    {
        const SfxItemPropertySimpleEntry* pCur = aPropertyMap.getByName( rPropertyName );
        if(pCur)
            aPropListeners.addInterface( pCur->nWID, rxListener );
#ifdef LINGU_EXCEPTIONS
        else
        {
            throw UnknownPropertyException();
        }
#endif
    }
}

void SAL_CALL LinguProps::removePropertyChangeListener(
            const OUString& rPropertyName,
            const Reference< XPropertyChangeListener >& rxListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
    {
        const SfxItemPropertySimpleEntry* pCur = aPropertyMap.getByName( rPropertyName );
        if(pCur)
            aPropListeners.removeInterface( pCur->nWID, rxListener );
#ifdef LINGU_EXCEPTIONS
        else
        {
            throw UnknownPropertyException();
        }
#endif
    }
}

void SAL_CALL LinguProps::addVetoableChangeListener(
            const OUString& /*rPropertyName*/,
            const Reference< XVetoableChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
//  MutexGuard  aGuard( GetLinguMutex() );
}

void SAL_CALL LinguProps::removeVetoableChangeListener(
            const OUString& /*rPropertyName*/,
            const Reference< XVetoableChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
//  MutexGuard  aGuard( GetLinguMutex() );
}


void SAL_CALL LinguProps::setFastPropertyValue( sal_Int32 nHandle, const Any& rValue )
        throw(UnknownPropertyException, PropertyVetoException,
              IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Any aOld( aConfig.GetProperty( nHandle ) );
    if (aOld != rValue && aConfig.SetProperty( nHandle, rValue ))
    {
        PropertyChangeEvent aChgEvt( (XPropertySet *) this,
                LinguOptions::GetName( nHandle ), FALSE, nHandle, aOld, rValue );
        launchEvent( aChgEvt );
    }
}


Any SAL_CALL LinguProps::getFastPropertyValue( sal_Int32 nHandle )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Any aRes( aConfig.GetProperty( nHandle ) );
    return aRes;
}


Sequence< PropertyValue > SAL_CALL
    LinguProps::getPropertyValues()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    INT32 nLen = aPropertyMap.getSize();
    Sequence< PropertyValue > aProps( nLen );
    PropertyValue *pProp = aProps.getArray();
    PropertyEntryVector_t aPropEntries = aPropertyMap.getPropertyEntries();
    PropertyEntryVector_t::const_iterator aIt = aPropEntries.begin();
    for (INT32 i = 0;  i < nLen;  ++i, ++aIt)
    {
        PropertyValue &rVal = pProp[i];
        Any aAny( aConfig.GetProperty( aIt->nWID ) );

        rVal.Name   = aIt->sName;
        rVal.Handle = aIt->nWID;
        rVal.Value  = aAny;
        rVal.State  = PropertyState_DIRECT_VALUE ;
    }
    return aProps;
}

void SAL_CALL
    LinguProps::setPropertyValues( const Sequence< PropertyValue >& rProps )
        throw(UnknownPropertyException, PropertyVetoException,
              IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    INT32 nLen = rProps.getLength();
    const PropertyValue *pVal = rProps.getConstArray();
    for (INT32 i = 0;  i < nLen;  ++i)
    {
        const PropertyValue &rVal = pVal[i];
        setPropertyValue( rVal.Name, rVal.Value );
    }
}

void SAL_CALL
    LinguProps::dispose()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing)
    {
        bDisposing = TRUE;

        //! its too late to save the options here!
        // (see AppExitListener for saving)
        //aOpt.Save();  // save (possible) changes before exiting

        EventObject aEvtObj( (XPropertySet *) this );
        aEvtListeners.disposeAndClear( aEvtObj );
        aPropListeners.disposeAndClear( aEvtObj );
    }
}

void SAL_CALL
    LinguProps::addEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}

void SAL_CALL
    LinguProps::removeEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}


///////////////////////////////////////////////////////////////////////////
// Service specific part
//

// XServiceInfo
OUString SAL_CALL LinguProps::getImplementationName()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return getImplementationName_Static();
}

// XServiceInfo
sal_Bool SAL_CALL LinguProps::supportsService( const OUString& ServiceName )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    uno::Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( INT32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return TRUE;
    return FALSE;
}

// XServiceInfo
uno::Sequence< OUString > SAL_CALL LinguProps::getSupportedServiceNames()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return getSupportedServiceNames_Static();
}

// ORegistryServiceManager_Static
uno::Sequence< OUString > LinguProps::getSupportedServiceNames_Static()
        throw()
{
    MutexGuard  aGuard( GetLinguMutex() );

    uno::Sequence< OUString > aSNS( 1 );    // more than 1 service possible
    aSNS.getArray()[0] = A2OU( SN_LINGU_PROPERTIES );
    return aSNS;
}


sal_Bool SAL_CALL LinguProps_writeInfo( void * /*pServiceManager*/,
            XRegistryKey * pRegistryKey )
{
    try
    {
        String aImpl( '/' );
        aImpl += LinguProps::getImplementationName_Static().getStr();
        aImpl.AppendAscii( "/UNO/SERVICES" );
        Reference< XRegistryKey > xNewKey =
            pRegistryKey->createKey(aImpl );
        uno::Sequence< OUString > aServices = LinguProps::getSupportedServiceNames_Static();
        for( INT32 i = 0; i < aServices.getLength(); i++ )
            xNewKey->createKey( aServices.getConstArray()[i]);

        return sal_True;
    }
    catch(Exception &)
    {
        return sal_False;
    }
}

void * SAL_CALL LinguProps_getFactory( const sal_Char * pImplName,
            XMultiServiceFactory *pServiceManager, void * )
{
    void * pRet = 0;
    if ( !LinguProps::getImplementationName_Static().compareToAscii( pImplName ) )
    {
        Reference< XSingleServiceFactory > xFactory =
            cppu::createOneInstanceFactory(
                pServiceManager,
                LinguProps::getImplementationName_Static(),
                LinguProps_CreateInstance,
                LinguProps::getSupportedServiceNames_Static());
        // acquire, because we return an interface pointer instead of a reference
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

///////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
