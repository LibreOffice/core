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

#include "localizationmgr.hxx"

#include "basidesh.hxx"
#include "baside3.hxx"
#include "basobj.hxx"
#include "iderdll.hxx"
#include "dlged.hxx"

#include <com/sun/star/resource/XStringResourceSupplier.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <sfx2/dispatch.hxx>

namespace basctl
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::resource;

static ::rtl::OUString aDot( RTL_CONSTASCII_USTRINGPARAM( "." ));
static ::rtl::OUString aEsc( RTL_CONSTASCII_USTRINGPARAM( "&" ));
static ::rtl::OUString aSemi( RTL_CONSTASCII_USTRINGPARAM( ";" ));


LocalizationMgr::LocalizationMgr( Shell* pShell,
    const ScriptDocument& rDocument, ::rtl::OUString aLibName,
    const Reference< XStringResourceManager >& xStringResourceManager )
        : m_xStringResourceManager( xStringResourceManager )
        , m_pShell( pShell )
        , m_aDocument( rDocument )
        , m_aLibName( aLibName )
{
}

bool LocalizationMgr::isLibraryLocalized( void )
{
    bool bRet = false;
    if( m_xStringResourceManager.is() )
    {
        Sequence< Locale > aLocaleSeq = m_xStringResourceManager->getLocales();
        bRet = ( aLocaleSeq.getLength() > 0 );
    }
    return bRet;
}

void LocalizationMgr::handleTranslationbar( void )
{
    static ::rtl::OUString aLayoutManagerName( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" ));
    static ::rtl::OUString aToolBarResName( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/translationbar" ));

    Reference< beans::XPropertySet > xFrameProps
        ( m_pShell->GetViewFrame()->GetFrame().GetFrameInterface(), uno::UNO_QUERY );
    if ( xFrameProps.is() )
    {
        Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
        uno::Any a = xFrameProps->getPropertyValue( aLayoutManagerName );
        a >>= xLayoutManager;
        if ( xLayoutManager.is() )
        {
            if ( !isLibraryLocalized() )
            {
                xLayoutManager->destroyElement( aToolBarResName );
            }
            else
            {
                xLayoutManager->createElement( aToolBarResName );
                xLayoutManager->requestElement( aToolBarResName );
            }
        }
    }
}


// TODO: -> export from toolkit

struct LanguageDependentProp
{
    const char* pPropName;
    sal_Int32   nPropNameLength;
};

static LanguageDependentProp aLanguageDependentProp[] =
{
    { "Text",            4 },
    { "Label",           5 },
    { "Title",           5 },
    { "HelpText",        8 },
    { "CurrencySymbol", 14 },
    { "StringItemList", 14 },
    { 0, 0                 }
};

bool isLanguageDependentProperty( ::rtl::OUString aName )
{
    bool bRet = false;

    LanguageDependentProp* pLangDepProp = aLanguageDependentProp;
    while( pLangDepProp->pPropName != 0 )
    {
        if( aName.equalsAsciiL( pLangDepProp->pPropName, pLangDepProp->nPropNameLength ))
        {
            bRet = true;
            break;
        }
        pLangDepProp++;
    }
    return bRet;
}


void LocalizationMgr::implEnableDisableResourceForAllLibraryDialogs( HandleResourceMode eMode )
{
    Sequence< ::rtl::OUString > aDlgNames = m_aDocument.getObjectNames( E_DIALOGS, m_aLibName );
    sal_Int32 nDlgCount = aDlgNames.getLength();
    const ::rtl::OUString* pDlgNames = aDlgNames.getConstArray();

    Reference< XStringResourceResolver > xDummyStringResolver;
    for( sal_Int32 i = 0 ; i < nDlgCount ; i++ )
    {
        String aDlgName = pDlgNames[ i ];
        if (DialogWindow* pWin = m_pShell->FindDlgWin(m_aDocument, m_aLibName, aDlgName))
        {
            Reference< container::XNameContainer > xDialog = pWin->GetDialog();
            if( xDialog.is() )
            {
                // Handle dialog itself as control
                Any aDialogCtrl;
                aDialogCtrl <<= xDialog;
                implHandleControlResourceProperties( aDialogCtrl, aDlgName,
                    ::rtl::OUString(), m_xStringResourceManager, xDummyStringResolver, eMode );

                // Handle all controls
                Sequence< ::rtl::OUString > aNames = xDialog->getElementNames();
                const ::rtl::OUString* pNames = aNames.getConstArray();
                sal_Int32 nCtrls = aNames.getLength();
                for( sal_Int32 j = 0 ; j < nCtrls ; ++j )
                {
                    ::rtl::OUString aCtrlName( pNames[j] );
                    Any aCtrl = xDialog->getByName( aCtrlName );
                    implHandleControlResourceProperties( aCtrl, aDlgName,
                        aCtrlName, m_xStringResourceManager, xDummyStringResolver, eMode );
                }
            }
        }
    }
}


::rtl::OUString implCreatePureResourceId
    ( const ::rtl::OUString& aDialogName, const ::rtl::OUString& aCtrlName,
      const ::rtl::OUString& aPropName,
      Reference< XStringResourceManager > xStringResourceManager )
{
    sal_Int32 nUniqueId = xStringResourceManager->getUniqueNumericId();
    ::rtl::OUString aPureIdStr = ::rtl::OUString::valueOf( nUniqueId );
    aPureIdStr += aDot;
    aPureIdStr += aDialogName;
    aPureIdStr += aDot;
    if( !aCtrlName.isEmpty() )
    {
        aPureIdStr += aCtrlName;
        aPureIdStr += aDot;
    }
    aPureIdStr += aPropName;
    return aPureIdStr;
}

extern bool localesAreEqual( const ::com::sun::star::lang::Locale& rLocaleLeft,
                             const ::com::sun::star::lang::Locale& rLocaleRight );

// Works on xStringResourceManager's current language for SET_IDS/RESET_IDS,
// anyway only one language should exist when calling this method then,
// either the first one for mode SET_IDS or the last one for mode RESET_IDS
sal_Int32 LocalizationMgr::implHandleControlResourceProperties
    ( Any aControlAny, const ::rtl::OUString& aDialogName, const ::rtl::OUString& aCtrlName,
        Reference< XStringResourceManager > xStringResourceManager,
        Reference< XStringResourceResolver > xSourceStringResolver, HandleResourceMode eMode )
{
    sal_Int32 nChangedCount = 0;

    Reference< XPropertySet > xPropertySet;
    aControlAny >>= xPropertySet;
    if( xPropertySet.is() )
    {
        Sequence< Locale > aLocaleSeq = xStringResourceManager->getLocales();
        sal_Int32 nLocaleCount = aLocaleSeq.getLength();
        if( nLocaleCount == 0 )
            return 0;

        Reference< XPropertySetInfo > xPropertySetInfo = xPropertySet->getPropertySetInfo();
        if( xPropertySetInfo.is() )
        {
            // get sequence of control properties
            Sequence< Property > aPropSeq = xPropertySetInfo->getProperties();
            const Property* pProps = aPropSeq.getConstArray();
            sal_Int32 nCtrlProps = aPropSeq.getLength();

            // create a map of tab indices and control names, sorted by tab index
            for( sal_Int32 j = 0 ; j < nCtrlProps ; ++j )
            {
                const Property& rProp = pProps[j];
                ::rtl::OUString aPropName = rProp.Name;
                TypeClass eType = rProp.Type.getTypeClass();
                bool bLanguageDependentProperty =
                    (eType == TypeClass_STRING || eType == TypeClass_SEQUENCE)
                    && isLanguageDependentProperty( aPropName );
                if( !bLanguageDependentProperty )
                    continue;

                if( eType == TypeClass_STRING )
                {
                    Any aPropAny = xPropertySet->getPropertyValue( aPropName );
                    ::rtl::OUString aPropStr;
                    aPropAny >>= aPropStr;

                    // Replace string by id, add id+string to StringResource
                    if( eMode == SET_IDS )
                    {
                        bool bEscAlreadyExisting = (!aPropStr.isEmpty() && aPropStr.getStr()[0] == '&' );
                        if( bEscAlreadyExisting )
                            continue;

                        ::rtl::OUString aPureIdStr = implCreatePureResourceId
                            ( aDialogName, aCtrlName, aPropName, xStringResourceManager );

                        // Set Id for all locales
                        const Locale* pLocales = aLocaleSeq.getConstArray();
                        for( sal_Int32 i = 0 ; i < nLocaleCount ; i++ )
                        {
                            const Locale& rLocale = pLocales[ i ];
                            xStringResourceManager->setStringForLocale( aPureIdStr, aPropStr, rLocale );
                        }

                        ::rtl::OUString aPropIdStr = aEsc;
                        aPropIdStr += aPureIdStr;
                        // TODO?: Change here and in toolkit
                        (void)aSemi;
                        aPropAny <<= aPropIdStr;
                        xPropertySet->setPropertyValue( aPropName, aPropAny );
                    }
                    // Replace id by string from StringResource
                    else if( eMode == RESET_IDS )
                    {
                        if( aPropStr.getLength() > 1 )
                        {
                            ::rtl::OUString aPureIdStr = aPropStr.copy( 1 );
                            ::rtl::OUString aNewPropStr = aPropStr;
                            try
                            {
                                aNewPropStr = xStringResourceManager->resolveString( aPureIdStr );
                            }
                            catch(const MissingResourceException&)
                            {
                            }
                            aPropAny <<= aNewPropStr;
                            xPropertySet->setPropertyValue( aPropName, aPropAny );
                        }
                    }
                    // Remove Id for all locales
                    else if( eMode == REMOVE_IDS_FROM_RESOURCE )
                    {
                        if( aPropStr.getLength() > 1 )
                        {
                            ::rtl::OUString aPureIdStr = aPropStr.copy( 1 );

                            const Locale* pLocales = aLocaleSeq.getConstArray();
                            for( sal_Int32 i = 0 ; i < nLocaleCount ; i++ )
                            {
                                const Locale& rLocale = pLocales[ i ];
                                try
                                {
                                    xStringResourceManager->removeIdForLocale( aPureIdStr, rLocale );
                                }
                                catch(const MissingResourceException&)
                                {
                                }
                            }
                        }
                    }
                    // Rename resource id
                    else if( eMode == RENAME_DIALOG_IDS || eMode == RENAME_CONTROL_IDS )
                    {
                        ::rtl::OUString aSourceIdStr = aPropStr;
                        ::rtl::OUString aPureSourceIdStr = aSourceIdStr.copy( 1 );

                        ::rtl::OUString aPureIdStr = implCreatePureResourceId
                            ( aDialogName, aCtrlName, aPropName, xStringResourceManager );

                        // Set new Id and remove old one for all locales
                        const Locale* pLocales = aLocaleSeq.getConstArray();
                        for( sal_Int32 i = 0 ; i < nLocaleCount ; i++ )
                        {
                            const Locale& rLocale = pLocales[ i ];
                            ::rtl::OUString aResStr;
                            try
                            {
                                aResStr = xStringResourceManager->resolveStringForLocale
                                    ( aPureSourceIdStr, rLocale );
                                xStringResourceManager->removeIdForLocale( aPureSourceIdStr, rLocale );
                                xStringResourceManager->setStringForLocale( aPureIdStr, aResStr, rLocale );
                            }
                            catch(const MissingResourceException&)
                            {}
                        }

                        ::rtl::OUString aPropIdStr = aEsc;
                        aPropIdStr += aPureIdStr;
                        // TODO?: Change here and in toolkit
                        (void)aSemi;
                        aPropAny <<= aPropIdStr;
                        xPropertySet->setPropertyValue( aPropName, aPropAny );
                    }
                    // Replace string by string from source StringResourceResolver
                    else if( eMode == MOVE_RESOURCES && xSourceStringResolver.is() )
                    {
                        ::rtl::OUString aSourceIdStr = aPropStr;
                        ::rtl::OUString aPureSourceIdStr = aSourceIdStr.copy( 1 );

                        ::rtl::OUString aPureIdStr = implCreatePureResourceId
                            ( aDialogName, aCtrlName, aPropName, xStringResourceManager );

                        const Locale& rDefaultLocale = xSourceStringResolver->getDefaultLocale();

                        // Set Id for all locales
                        const Locale* pLocales = aLocaleSeq.getConstArray();
                        for( sal_Int32 i = 0 ; i < nLocaleCount ; i++ )
                        {
                            const Locale& rLocale = pLocales[ i ];
                            ::rtl::OUString aResStr;
                            try
                            {
                                aResStr = xSourceStringResolver->resolveStringForLocale
                                    ( aPureSourceIdStr, rLocale );
                            }
                            catch(const MissingResourceException&)
                            {
                                aResStr = xSourceStringResolver->resolveStringForLocale
                                    ( aPureSourceIdStr, rDefaultLocale );
                            }
                            xStringResourceManager->setStringForLocale( aPureIdStr, aResStr, rLocale );
                        }

                        ::rtl::OUString aPropIdStr = aEsc;
                        aPropIdStr += aPureIdStr;
                        // TODO?: Change here and in toolkit
                        (void)aSemi;
                        aPropAny <<= aPropIdStr;
                        xPropertySet->setPropertyValue( aPropName, aPropAny );
                    }
                    // Copy string from source to target resource
                    else if( eMode == COPY_RESOURCES && xSourceStringResolver.is() )
                    {
                        ::rtl::OUString aSourceIdStr = aPropStr;
                        ::rtl::OUString aPureSourceIdStr = aSourceIdStr.copy( 1 );

                        const Locale& rDefaultLocale = xSourceStringResolver->getDefaultLocale();

                        // Copy Id for all locales
                        const Locale* pLocales = aLocaleSeq.getConstArray();
                        for( sal_Int32 i = 0 ; i < nLocaleCount ; i++ )
                        {
                            const Locale& rLocale = pLocales[ i ];
                            ::rtl::OUString aResStr;
                            try
                            {
                                aResStr = xSourceStringResolver->resolveStringForLocale
                                    ( aPureSourceIdStr, rLocale );
                            }
                            catch(const MissingResourceException&)
                            {
                                aResStr = xSourceStringResolver->resolveStringForLocale
                                    ( aPureSourceIdStr, rDefaultLocale );
                            }
                            xStringResourceManager->setStringForLocale( aPureSourceIdStr, aResStr, rLocale );
                        }
                    }
                    nChangedCount++;
                }

                // Listbox / Combobox
                else if( eType == TypeClass_SEQUENCE )
                {
                    Any aPropAny = xPropertySet->getPropertyValue( aPropName );
                    Sequence< ::rtl::OUString > aPropStrings;
                    aPropAny >>= aPropStrings;

                    const ::rtl::OUString* pPropStrings = aPropStrings.getConstArray();
                    sal_Int32 nPropStringCount = aPropStrings.getLength();
                    if( nPropStringCount == 0 )
                        continue;

                    // Replace string by id, add id+string to StringResource
                    if( eMode == SET_IDS )
                    {
                        Sequence< ::rtl::OUString > aIdStrings;
                        aIdStrings.realloc( nPropStringCount );
                        ::rtl::OUString* pIdStrings = aIdStrings.getArray();

                        ::rtl::OUString aIdStrBase = aDot;
                        aIdStrBase += aCtrlName;
                        aIdStrBase += aDot;
                        aIdStrBase += aPropName;

                        const Locale* pLocales = aLocaleSeq.getConstArray();
                        sal_Int32 i;
                        for ( i = 0; i < nPropStringCount; ++i )
                        {
                            ::rtl::OUString aPropStr = pPropStrings[i];
                            bool bEscAlreadyExisting = (!aPropStr.isEmpty() && aPropStr.getStr()[0] == '&' );
                            if( bEscAlreadyExisting )
                            {
                                pIdStrings[i] = aPropStr;
                                continue;
                            }

                            sal_Int32 nUniqueId = xStringResourceManager->getUniqueNumericId();
                            ::rtl::OUString aPureIdStr = ::rtl::OUString::valueOf( nUniqueId );
                            aPureIdStr += aIdStrBase;

                            // Set Id for all locales
                            for( sal_Int32 iLocale = 0 ; iLocale < nLocaleCount ; iLocale++ )
                            {
                                const Locale& rLocale = pLocales[ iLocale ];
                                xStringResourceManager->setStringForLocale( aPureIdStr, aPropStr, rLocale );
                            }

                            ::rtl::OUString aPropIdStr = aEsc;
                            aPropIdStr += aPureIdStr;
                            pIdStrings[i] = aPropIdStr;
                        }
                        aPropAny <<= aIdStrings;
                        xPropertySet->setPropertyValue( aPropName, aPropAny );
                    }
                    // Replace id by string from StringResource
                    else if( eMode == RESET_IDS )
                    {
                        Sequence< ::rtl::OUString > aNewPropStrings;
                        aNewPropStrings.realloc( nPropStringCount );
                        ::rtl::OUString* pNewPropStrings = aNewPropStrings.getArray();

                        sal_Int32 i;
                        for ( i = 0; i < nPropStringCount; ++i )
                        {
                            ::rtl::OUString aIdStr = pPropStrings[i];
                            ::rtl::OUString aNewPropStr = aIdStr;
                            if( aIdStr.getLength() > 1 )
                            {
                                ::rtl::OUString aPureIdStr = aIdStr.copy( 1 );
                                try
                                {
                                    aNewPropStr = xStringResourceManager->resolveString( aPureIdStr );
                                }
                                catch(const MissingResourceException&)
                                {
                                }
                            }
                            pNewPropStrings[i] = aNewPropStr;
                        }
                        aPropAny <<= aNewPropStrings;
                        xPropertySet->setPropertyValue( aPropName, aPropAny );
                    }
                    // Remove Id for all locales
                    else if( eMode == REMOVE_IDS_FROM_RESOURCE )
                    {
                        Sequence< ::rtl::OUString > aNewPropStrings;
                        aNewPropStrings.realloc( nPropStringCount );

                        const Locale* pLocales = aLocaleSeq.getConstArray();
                        sal_Int32 i;
                        for ( i = 0; i < nPropStringCount; ++i )
                        {
                            ::rtl::OUString aIdStr = pPropStrings[i];
                            if( aIdStr.getLength() > 1 )
                            {
                                ::rtl::OUString aPureIdStr = aIdStr.copy( 1 );

                                for( sal_Int32 iLocale = 0 ; iLocale < nLocaleCount ; iLocale++ )
                                {
                                    const Locale& rLocale = pLocales[iLocale];
                                    try
                                    {
                                        xStringResourceManager->removeIdForLocale( aPureIdStr, rLocale );
                                    }
                                    catch(const MissingResourceException&)
                                    {
                                    }
                                }
                            }
                        }
                    }
                    // Rename resource id
                    else if( eMode == RENAME_CONTROL_IDS )
                    {
                        Sequence< ::rtl::OUString > aIdStrings;
                        aIdStrings.realloc( nPropStringCount );
                        ::rtl::OUString* pIdStrings = aIdStrings.getArray();

                        ::rtl::OUString aIdStrBase = aDot;
                        aIdStrBase += aCtrlName;
                        aIdStrBase += aDot;
                        aIdStrBase += aPropName;

                        const Locale* pLocales = aLocaleSeq.getConstArray();
                        sal_Int32 i;
                        for ( i = 0; i < nPropStringCount; ++i )
                        {
                            ::rtl::OUString aSourceIdStr = pPropStrings[i];
                            ::rtl::OUString aPureSourceIdStr = aSourceIdStr.copy( 1 );

                            sal_Int32 nUniqueId = xStringResourceManager->getUniqueNumericId();
                            ::rtl::OUString aPureIdStr = ::rtl::OUString::valueOf( nUniqueId );
                            aPureIdStr += aIdStrBase;

                            // Set Id for all locales
                            for( sal_Int32 iLocale = 0 ; iLocale < nLocaleCount ; iLocale++ )
                            {
                                const Locale& rLocale = pLocales[ iLocale ];

                                ::rtl::OUString aResStr;
                                try
                                {
                                    aResStr = xStringResourceManager->resolveStringForLocale
                                        ( aPureSourceIdStr, rLocale );
                                    xStringResourceManager->removeIdForLocale( aPureSourceIdStr, rLocale );
                                    xStringResourceManager->setStringForLocale( aPureIdStr, aResStr, rLocale );
                                }
                                catch(const MissingResourceException&)
                                {}
                            }

                            ::rtl::OUString aPropIdStr = aEsc;
                            aPropIdStr += aPureIdStr;
                            pIdStrings[i] = aPropIdStr;
                        }
                        aPropAny <<= aIdStrings;
                        xPropertySet->setPropertyValue( aPropName, aPropAny );
                    }
                    // Replace string by string from source StringResourceResolver
                    else if( eMode == MOVE_RESOURCES && xSourceStringResolver.is() )
                    {
                        Sequence< ::rtl::OUString > aIdStrings;
                        aIdStrings.realloc( nPropStringCount );
                        ::rtl::OUString* pIdStrings = aIdStrings.getArray();

                        ::rtl::OUString aIdStrBase = aDot;
                        aIdStrBase += aCtrlName;
                        aIdStrBase += aDot;
                        aIdStrBase += aPropName;

                        const Locale& rDefaultLocale = xSourceStringResolver->getDefaultLocale();

                        const Locale* pLocales = aLocaleSeq.getConstArray();
                        sal_Int32 i;
                        for ( i = 0; i < nPropStringCount; ++i )
                        {
                            ::rtl::OUString aSourceIdStr = pPropStrings[i];
                            ::rtl::OUString aPureSourceIdStr = aSourceIdStr.copy( 1 );

                            sal_Int32 nUniqueId = xStringResourceManager->getUniqueNumericId();
                            ::rtl::OUString aPureIdStr = ::rtl::OUString::valueOf( nUniqueId );
                            aPureIdStr += aIdStrBase;

                            // Set Id for all locales
                            for( sal_Int32 iLocale = 0 ; iLocale < nLocaleCount ; iLocale++ )
                            {
                                const Locale& rLocale = pLocales[ iLocale ];

                                ::rtl::OUString aResStr;
                                try
                                {
                                    aResStr = xSourceStringResolver->resolveStringForLocale
                                        ( aPureSourceIdStr, rLocale );
                                }
                                catch(const MissingResourceException&)
                                {
                                    aResStr = xSourceStringResolver->resolveStringForLocale
                                        ( aPureSourceIdStr, rDefaultLocale );
                                }
                                xStringResourceManager->setStringForLocale( aPureIdStr, aResStr, rLocale );
                            }

                            ::rtl::OUString aPropIdStr = aEsc;
                            aPropIdStr += aPureIdStr;
                            pIdStrings[i] = aPropIdStr;
                        }
                        aPropAny <<= aIdStrings;
                        xPropertySet->setPropertyValue( aPropName, aPropAny );
                    }
                    // Copy string from source to target resource
                    else if( eMode == COPY_RESOURCES && xSourceStringResolver.is() )
                    {
                        const Locale& rDefaultLocale = xSourceStringResolver->getDefaultLocale();

                        const Locale* pLocales = aLocaleSeq.getConstArray();
                        sal_Int32 i;
                        for ( i = 0; i < nPropStringCount; ++i )
                        {
                            ::rtl::OUString aSourceIdStr = pPropStrings[i];
                            ::rtl::OUString aPureSourceIdStr = aSourceIdStr.copy( 1 );

                            // Set Id for all locales
                            for( sal_Int32 iLocale = 0 ; iLocale < nLocaleCount ; iLocale++ )
                            {
                                const Locale& rLocale = pLocales[ iLocale ];

                                ::rtl::OUString aResStr;
                                try
                                {
                                    aResStr = xSourceStringResolver->resolveStringForLocale
                                        ( aPureSourceIdStr, rLocale );
                                }
                                catch(const MissingResourceException&)
                                {
                                    aResStr = xSourceStringResolver->resolveStringForLocale
                                        ( aPureSourceIdStr, rDefaultLocale );
                                }
                                xStringResourceManager->setStringForLocale( aPureSourceIdStr, aResStr, rLocale );
                            }
                        }
                    }
                    nChangedCount++;
                }
            }
        }
    }
    return nChangedCount;
}


void LocalizationMgr::handleAddLocales( Sequence< Locale > aLocaleSeq )
{
    const Locale* pLocales = aLocaleSeq.getConstArray();
    sal_Int32 nLocaleCount = aLocaleSeq.getLength();

    if( isLibraryLocalized() )
    {
        for( sal_Int32 i = 0 ; i < nLocaleCount ; i++ )
        {
            const Locale& rLocale = pLocales[ i ];
            m_xStringResourceManager->newLocale( rLocale );
        }
    }
    else
    {
        DBG_ASSERT( nLocaleCount==1, "LocalizationMgr::handleAddLocales(): Only one first locale allowed" );

        const Locale& rLocale = pLocales[ 0 ];
        m_xStringResourceManager->newLocale( rLocale );
        enableResourceForAllLibraryDialogs();
    }

    MarkDocumentModified( m_aDocument );

    // update locale toolbar
    if (SfxBindings* pBindings = GetBindingsPtr())
        pBindings->Invalidate( SID_BASICIDE_CURRENT_LANG );

    handleTranslationbar();
}


void LocalizationMgr::handleRemoveLocales( Sequence< Locale > aLocaleSeq )
{
    const Locale* pLocales = aLocaleSeq.getConstArray();
    sal_Int32 nLocaleCount = aLocaleSeq.getLength();
    bool bConsistant = true;
    bool bModified = false;

    for( sal_Int32 i = 0 ; i < nLocaleCount ; i++ )
    {
        const Locale& rLocale = pLocales[ i ];
        bool bRemove = true;

        // Check if last locale
        Sequence< Locale > aResLocaleSeq = m_xStringResourceManager->getLocales();
        if( aResLocaleSeq.getLength() == 1 )
        {
            const Locale& rLastResLocale = aResLocaleSeq.getConstArray()[ 0 ];
            if( localesAreEqual( rLocale, rLastResLocale ) )
            {
                disableResourceForAllLibraryDialogs();
            }
            else
            {
                // Inconsistancy, keep last locale
                bConsistant = false;
                bRemove = false;
            }
        }

        if( bRemove )
        {
            try
            {
                m_xStringResourceManager->removeLocale( rLocale );
                bModified = true;
            }
            catch(const IllegalArgumentException&)
            {
                bConsistant = false;
            }
        }
    }
    if( bModified )
    {
        MarkDocumentModified( m_aDocument );

        // update slots
        if (SfxBindings* pBindings = GetBindingsPtr())
        {
            pBindings->Invalidate( SID_BASICIDE_CURRENT_LANG );
            pBindings->Invalidate( SID_BASICIDE_MANAGE_LANG );
        }

        handleTranslationbar();
    }

    DBG_ASSERT( bConsistant,
        "LocalizationMgr::handleRemoveLocales(): sequence contains unsupported locales" );
    (void)bConsistant;
}

void LocalizationMgr::handleSetDefaultLocale( Locale aLocale )
{
    if( m_xStringResourceManager.is() )
    {
        try
        {
            m_xStringResourceManager->setDefaultLocale( aLocale );
        }
        catch(const IllegalArgumentException&)
        {
            OSL_FAIL( "LocalizationMgr::handleSetDefaultLocale: Invalid locale" );
        }

        // update locale toolbar
        if (SfxBindings* pBindings = GetBindingsPtr())
            pBindings->Invalidate( SID_BASICIDE_CURRENT_LANG );
    }
}

void LocalizationMgr::handleSetCurrentLocale( ::com::sun::star::lang::Locale aLocale )
{
    if( m_xStringResourceManager.is() )
    {
        try
        {
            m_xStringResourceManager->setCurrentLocale( aLocale, false );
        }
        catch(const IllegalArgumentException&)
        {
            OSL_FAIL( "LocalizationMgr::handleSetCurrentLocale: Invalid locale" );
        }

        // update locale toolbar
        if (SfxBindings* pBindings = GetBindingsPtr())
            pBindings->Invalidate( SID_BASICIDE_CURRENT_LANG );

        if (DialogWindow* pDlgWin = dynamic_cast<DialogWindow*>(m_pShell->GetCurWindow()))
            if (!pDlgWin->IsSuspended())
                if (DlgEditor* pWinEditor = pDlgWin->GetEditor())
                    pWinEditor->UpdatePropertyBrowserDelayed();
    }
}

void LocalizationMgr::handleBasicStarted( void )
{
    if( m_xStringResourceManager.is() )
        m_aLocaleBeforeBasicStart = m_xStringResourceManager->getCurrentLocale();
}

void LocalizationMgr::handleBasicStopped( void )
{
    try
    {
        if( m_xStringResourceManager.is() )
            m_xStringResourceManager->setCurrentLocale( m_aLocaleBeforeBasicStart, true );
    }
    catch(const IllegalArgumentException&)
    {
    }
}


DialogWindow* FindDialogWindowForEditor( DlgEditor* pEditor )
{
    Shell::WindowTable const& aWindowTable = GetShell()->GetWindowTable();
    for (Shell::WindowTableIt it = aWindowTable.begin(); it != aWindowTable.end(); ++it )
    {
        BaseWindow* pWin = it->second;
        if (!pWin->IsSuspended())
            if (DialogWindow* pDlgWin = dynamic_cast<DialogWindow*>(pWin))
            {
                DlgEditor* pWinEditor = pDlgWin->GetEditor();
                if( pWinEditor == pEditor )
                    return pDlgWin;
            }
    }
    return 0;
}


void LocalizationMgr::setControlResourceIDsForNewEditorObject( DlgEditor* pEditor,
    Any aControlAny, const ::rtl::OUString& aCtrlName )
{
    // Get library for DlgEditor
    DialogWindow* pDlgWin = FindDialogWindowForEditor( pEditor );
    if( !pDlgWin )
        return;
    ScriptDocument aDocument( pDlgWin->GetDocument() );
    DBG_ASSERT( aDocument.isValid(), "LocalizationMgr::setControlResourceIDsForNewEditorObject: invalid document!" );
    if ( !aDocument.isValid() )
        return;
    const String& rLibName = pDlgWin->GetLibName();
    Reference< container::XNameContainer > xDialogLib( aDocument.getLibrary( E_DIALOGS, rLibName, true ) );
    Reference< XStringResourceManager > xStringResourceManager =
        LocalizationMgr::getStringResourceFromDialogLibrary( xDialogLib );

    // Set resource property
    if( !xStringResourceManager.is() || xStringResourceManager->getLocales().getLength() == 0 )
        return;

    ::rtl::OUString aDialogName = pDlgWin->GetName();
    Reference< XStringResourceResolver > xDummyStringResolver;
    sal_Int32 nChangedCount = implHandleControlResourceProperties
        ( aControlAny, aDialogName, aCtrlName, xStringResourceManager,
          xDummyStringResolver, SET_IDS );

    if( nChangedCount )
        MarkDocumentModified( aDocument );
}

void LocalizationMgr::renameControlResourceIDsForEditorObject( DlgEditor* pEditor,
    ::com::sun::star::uno::Any aControlAny, const ::rtl::OUString& aNewCtrlName )
{
    // Get library for DlgEditor
    DialogWindow* pDlgWin = FindDialogWindowForEditor( pEditor );
    if( !pDlgWin )
        return;
    ScriptDocument aDocument( pDlgWin->GetDocument() );
    DBG_ASSERT( aDocument.isValid(), "LocalizationMgr::renameControlResourceIDsForEditorObject: invalid document!" );
    if ( !aDocument.isValid() )
        return;
    const String& rLibName = pDlgWin->GetLibName();
    Reference< container::XNameContainer > xDialogLib( aDocument.getLibrary( E_DIALOGS, rLibName, true ) );
    Reference< XStringResourceManager > xStringResourceManager =
        LocalizationMgr::getStringResourceFromDialogLibrary( xDialogLib );

    // Set resource property
    if( !xStringResourceManager.is() || xStringResourceManager->getLocales().getLength() == 0 )
        return;

    ::rtl::OUString aDialogName = pDlgWin->GetName();
    Reference< XStringResourceResolver > xDummyStringResolver;
    implHandleControlResourceProperties
        ( aControlAny, aDialogName, aNewCtrlName, xStringResourceManager,
          xDummyStringResolver, RENAME_CONTROL_IDS );
}


void LocalizationMgr::deleteControlResourceIDsForDeletedEditorObject( DlgEditor* pEditor,
    Any aControlAny, const ::rtl::OUString& aCtrlName )
{
    // Get library for DlgEditor
    DialogWindow* pDlgWin = FindDialogWindowForEditor( pEditor );
    if( !pDlgWin )
        return;
    ScriptDocument aDocument( pDlgWin->GetDocument() );
    DBG_ASSERT( aDocument.isValid(), "LocalizationMgr::deleteControlResourceIDsForDeletedEditorObject: invalid document!" );
    if ( !aDocument.isValid() )
        return;
    const String& rLibName = pDlgWin->GetLibName();
    Reference< container::XNameContainer > xDialogLib( aDocument.getLibrary( E_DIALOGS, rLibName, true ) );
    Reference< XStringResourceManager > xStringResourceManager =
        LocalizationMgr::getStringResourceFromDialogLibrary( xDialogLib );

    ::rtl::OUString aDialogName = pDlgWin->GetName();
    Reference< XStringResourceResolver > xDummyStringResolver;
    sal_Int32 nChangedCount = implHandleControlResourceProperties
        ( aControlAny, aDialogName, aCtrlName, xStringResourceManager,
          xDummyStringResolver, REMOVE_IDS_FROM_RESOURCE );

    if( nChangedCount )
        MarkDocumentModified( aDocument );
}

void LocalizationMgr::setStringResourceAtDialog( const ScriptDocument& rDocument, const ::rtl::OUString& aLibName,
    const ::rtl::OUString& aDlgName, Reference< container::XNameContainer > xDialogModel )
{
    static ::rtl::OUString aResourceResolverPropName( RTL_CONSTASCII_USTRINGPARAM( "ResourceResolver" ));

    // Get library
    Reference< container::XNameContainer > xDialogLib( rDocument.getLibrary( E_DIALOGS, aLibName, true ) );
    Reference< XStringResourceManager > xStringResourceManager =
        LocalizationMgr::getStringResourceFromDialogLibrary( xDialogLib );

    // Set resource property
    if( xStringResourceManager.is() )
    {
        // Not very elegant as dialog may or may not be localized yet
        // TODO: Find better place, where dialog is created
        if( xStringResourceManager->getLocales().getLength() > 0 )
        {
            Any aDialogCtrl;
            aDialogCtrl <<= xDialogModel;
            Reference< XStringResourceResolver > xDummyStringResolver;
            implHandleControlResourceProperties( aDialogCtrl, aDlgName,
                ::rtl::OUString(), xStringResourceManager,
                xDummyStringResolver, SET_IDS );
        }

        Reference< beans::XPropertySet > xDlgPSet( xDialogModel, UNO_QUERY );
        Any aStringResourceManagerAny;
        aStringResourceManagerAny <<= xStringResourceManager;
        xDlgPSet->setPropertyValue( aResourceResolverPropName, aStringResourceManagerAny );
    }
}

void LocalizationMgr::renameStringResourceIDs( const ScriptDocument& rDocument, const ::rtl::OUString& aLibName,
    const ::rtl::OUString& aDlgName, Reference< container::XNameContainer > xDialogModel )
{
    // Get library
    Reference< container::XNameContainer > xDialogLib( rDocument.getLibrary( E_DIALOGS, aLibName, true ) );
    Reference< XStringResourceManager > xStringResourceManager =
        LocalizationMgr::getStringResourceFromDialogLibrary( xDialogLib );
    if( !xStringResourceManager.is() )
        return;

    Any aDialogCtrl;
    aDialogCtrl <<= xDialogModel;
    Reference< XStringResourceResolver > xDummyStringResolver;
    implHandleControlResourceProperties( aDialogCtrl, aDlgName,
        ::rtl::OUString(), xStringResourceManager,
        xDummyStringResolver, RENAME_DIALOG_IDS );

    // Handle all controls
    Sequence< ::rtl::OUString > aNames = xDialogModel->getElementNames();
    const ::rtl::OUString* pNames = aNames.getConstArray();
    sal_Int32 nCtrls = aNames.getLength();
    for( sal_Int32 i = 0 ; i < nCtrls ; ++i )
    {
        ::rtl::OUString aCtrlName( pNames[i] );
        Any aCtrl = xDialogModel->getByName( aCtrlName );
        implHandleControlResourceProperties( aCtrl, aDlgName,
            aCtrlName, xStringResourceManager,
            xDummyStringResolver, RENAME_DIALOG_IDS );
    }
}

void LocalizationMgr::removeResourceForDialog( const ScriptDocument& rDocument, const ::rtl::OUString& aLibName,
    const ::rtl::OUString& aDlgName, Reference< container::XNameContainer > xDialogModel )
{
    // Get library
    Reference< container::XNameContainer > xDialogLib( rDocument.getLibrary( E_DIALOGS, aLibName, true ) );
    Reference< XStringResourceManager > xStringResourceManager =
        LocalizationMgr::getStringResourceFromDialogLibrary( xDialogLib );
    if( !xStringResourceManager.is() )
        return;

    Any aDialogCtrl;
    aDialogCtrl <<= xDialogModel;
    Reference< XStringResourceResolver > xDummyStringResolver;
    implHandleControlResourceProperties( aDialogCtrl, aDlgName,
        ::rtl::OUString(), xStringResourceManager,
        xDummyStringResolver, REMOVE_IDS_FROM_RESOURCE );

    // Handle all controls
    Sequence< ::rtl::OUString > aNames = xDialogModel->getElementNames();
    const ::rtl::OUString* pNames = aNames.getConstArray();
    sal_Int32 nCtrls = aNames.getLength();
    for( sal_Int32 i = 0 ; i < nCtrls ; ++i )
    {
        ::rtl::OUString aCtrlName( pNames[i] );
        Any aCtrl = xDialogModel->getByName( aCtrlName );
        implHandleControlResourceProperties( aCtrl, aDlgName,
            aCtrlName, xStringResourceManager,
            xDummyStringResolver, REMOVE_IDS_FROM_RESOURCE );
    }
}

void LocalizationMgr::resetResourceForDialog( Reference< container::XNameContainer > xDialogModel,
    Reference< XStringResourceManager > xStringResourceManager )
{
    if( !xStringResourceManager.is() )
        return;

    // Dialog as control
    ::rtl::OUString aDummyName;
    Any aDialogCtrl;
    aDialogCtrl <<= xDialogModel;
    Reference< XStringResourceResolver > xDummyStringResolver;
    implHandleControlResourceProperties( aDialogCtrl, aDummyName,
        aDummyName, xStringResourceManager, xDummyStringResolver, RESET_IDS );

    // Handle all controls
    Sequence< ::rtl::OUString > aNames = xDialogModel->getElementNames();
    const ::rtl::OUString* pNames = aNames.getConstArray();
    sal_Int32 nCtrls = aNames.getLength();
    for( sal_Int32 i = 0 ; i < nCtrls ; ++i )
    {
        ::rtl::OUString aCtrlName( pNames[i] );
        Any aCtrl = xDialogModel->getByName( aCtrlName );
        implHandleControlResourceProperties( aCtrl, aDummyName,
            aCtrlName, xStringResourceManager, xDummyStringResolver, RESET_IDS );
    }
}

void LocalizationMgr::setResourceIDsForDialog( Reference< container::XNameContainer > xDialogModel,
    Reference< XStringResourceManager > xStringResourceManager )
{
    if( !xStringResourceManager.is() )
        return;

    // Dialog as control
    ::rtl::OUString aDummyName;
    Any aDialogCtrl;
    aDialogCtrl <<= xDialogModel;
    Reference< XStringResourceResolver > xDummyStringResolver;
    implHandleControlResourceProperties( aDialogCtrl, aDummyName,
        aDummyName, xStringResourceManager, xDummyStringResolver, SET_IDS );

    // Handle all controls
    Sequence< ::rtl::OUString > aNames = xDialogModel->getElementNames();
    const ::rtl::OUString* pNames = aNames.getConstArray();
    sal_Int32 nCtrls = aNames.getLength();
    for( sal_Int32 i = 0 ; i < nCtrls ; ++i )
    {
        ::rtl::OUString aCtrlName( pNames[i] );
        Any aCtrl = xDialogModel->getByName( aCtrlName );
        implHandleControlResourceProperties( aCtrl, aDummyName,
            aCtrlName, xStringResourceManager, xDummyStringResolver, SET_IDS );
    }
}

void LocalizationMgr::copyResourcesForPastedEditorObject( DlgEditor* pEditor,
    Any aControlAny, const ::rtl::OUString& aCtrlName,
    Reference< XStringResourceResolver > xSourceStringResolver )
{
    // Get library for DlgEditor
    DialogWindow* pDlgWin = FindDialogWindowForEditor( pEditor );
    if( !pDlgWin )
        return;
    ScriptDocument aDocument( pDlgWin->GetDocument() );
    DBG_ASSERT( aDocument.isValid(), "LocalizationMgr::copyResourcesForPastedEditorObject: invalid document!" );
    if ( !aDocument.isValid() )
        return;
    const String& rLibName = pDlgWin->GetLibName();
    Reference< container::XNameContainer > xDialogLib( aDocument.getLibrary( E_DIALOGS, rLibName, true ) );
    Reference< XStringResourceManager > xStringResourceManager =
        LocalizationMgr::getStringResourceFromDialogLibrary( xDialogLib );

    // Set resource property
    if( !xStringResourceManager.is() || xStringResourceManager->getLocales().getLength() == 0 )
        return;

    ::rtl::OUString aDialogName = pDlgWin->GetName();
    implHandleControlResourceProperties
        ( aControlAny, aDialogName, aCtrlName, xStringResourceManager,
          xSourceStringResolver, MOVE_RESOURCES );
}

void LocalizationMgr::copyResourceForDroppedDialog( Reference< container::XNameContainer > xDialogModel,
    const ::rtl::OUString& aDialogName, Reference< XStringResourceManager > xStringResourceManager,
    Reference< XStringResourceResolver > xSourceStringResolver )
{
    if( !xStringResourceManager.is() )
        return;

    // Dialog as control
    ::rtl::OUString aDummyName;
    Any aDialogCtrl;
    aDialogCtrl <<= xDialogModel;
    implHandleControlResourceProperties( aDialogCtrl, aDialogName,
        aDummyName, xStringResourceManager, xSourceStringResolver, MOVE_RESOURCES );

    // Handle all controls
    Sequence< ::rtl::OUString > aNames = xDialogModel->getElementNames();
    const ::rtl::OUString* pNames = aNames.getConstArray();
    sal_Int32 nCtrls = aNames.getLength();
    for( sal_Int32 i = 0 ; i < nCtrls ; ++i )
    {
        ::rtl::OUString aCtrlName( pNames[i] );
        Any aCtrl = xDialogModel->getByName( aCtrlName );
        implHandleControlResourceProperties( aCtrl, aDialogName,
            aCtrlName, xStringResourceManager, xSourceStringResolver, MOVE_RESOURCES );
    }
}

void LocalizationMgr::copyResourceForDialog(
    const Reference< container::XNameContainer >& xDialogModel,
    const Reference< XStringResourceResolver >& xSourceStringResolver,
    const Reference< XStringResourceManager >& xTargetStringResourceManager )
{
    if( !xDialogModel.is() || !xSourceStringResolver.is() || !xTargetStringResourceManager.is() )
        return;

    ::rtl::OUString aDummyName;
    Any aDialogCtrl;
    aDialogCtrl <<= xDialogModel;
    implHandleControlResourceProperties
        ( aDialogCtrl, aDummyName, aDummyName, xTargetStringResourceManager,
          xSourceStringResolver, COPY_RESOURCES );

    // Handle all controls
    Sequence< ::rtl::OUString > aNames = xDialogModel->getElementNames();
    const ::rtl::OUString* pNames = aNames.getConstArray();
    sal_Int32 nCtrls = aNames.getLength();
    for( sal_Int32 i = 0 ; i < nCtrls ; ++i )
    {
        ::rtl::OUString aCtrlName( pNames[i] );
        Any aCtrl = xDialogModel->getByName( aCtrlName );
        implHandleControlResourceProperties( aCtrl, aDummyName, aDummyName,
            xTargetStringResourceManager, xSourceStringResolver, COPY_RESOURCES );
    }
}

Reference< XStringResourceManager > LocalizationMgr::getStringResourceFromDialogLibrary
    ( Reference< container::XNameContainer > xDialogLib )
{
    Reference< XStringResourceManager > xStringResourceManager;
    if( xDialogLib.is() )
    {
        Reference< resource::XStringResourceSupplier > xStringResourceSupplier( xDialogLib, UNO_QUERY );
        if( xStringResourceSupplier.is() )
        {
            Reference< resource::XStringResourceResolver >
                xStringResourceResolver = xStringResourceSupplier->getStringResource();

            xStringResourceManager =
                Reference< resource::XStringResourceManager >( xStringResourceResolver, UNO_QUERY );
        }
    }
    return xStringResourceManager;
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
