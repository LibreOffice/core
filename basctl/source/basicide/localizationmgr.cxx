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

#include "precompiled_basctl.hxx"
#include <ide_pch.hxx>

#include <basidesh.hxx>
#include <baside3.hxx>
#include <basobj.hxx>
#include <iderdll.hxx>
#include "dlged.hxx"

#include <localizationmgr.hxx>
#include <com/sun/star/resource/XStringResourceSupplier.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::resource;

static ::rtl::OUString aDot  = ::rtl::OUString::createFromAscii( "." );
static ::rtl::OUString aEsc  = ::rtl::OUString::createFromAscii( "&" );
static ::rtl::OUString aSemi = ::rtl::OUString::createFromAscii( ";" );


LocalizationMgr::LocalizationMgr( BasicIDEShell* pIDEShell,
    const ScriptDocument& rDocument, String aLibName,
    const Reference< XStringResourceManager >& xStringResourceManager )
        : m_xStringResourceManager( xStringResourceManager )
        , m_pIDEShell( pIDEShell )
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
    static ::rtl::OUString aLayoutManagerName = ::rtl::OUString::createFromAscii( "LayoutManager" );
    static ::rtl::OUString aToolBarResName =
        ::rtl::OUString::createFromAscii( "private:resource/toolbar/translationbar" );

    Reference< beans::XPropertySet > xFrameProps
        ( m_pIDEShell->GetViewFrame()->GetFrame().GetFrameInterface(), uno::UNO_QUERY );
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


//============================================
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
//============================================

void LocalizationMgr::implEnableDisableResourceForAllLibraryDialogs( HandleResourceMode eMode )
{
    Sequence< ::rtl::OUString > aDlgNames = m_aDocument.getObjectNames( E_DIALOGS, m_aLibName );
    sal_Int32 nDlgCount = aDlgNames.getLength();
    const ::rtl::OUString* pDlgNames = aDlgNames.getConstArray();

    Reference< XStringResourceResolver > xDummyStringResolver;
    for( sal_Int32 i = 0 ; i < nDlgCount ; i++ )
    {
        String aDlgName = pDlgNames[ i ];
        DialogWindow* pWin = m_pIDEShell->FindDlgWin( m_aDocument, m_aLibName, aDlgName, sal_False );
        if( pWin && pWin->IsA( TYPE( DialogWindow ) ) )
        {
            DialogWindow* pDialogWin = static_cast< DialogWindow* >( pWin );
            Reference< container::XNameContainer > xDialog = pDialogWin->GetDialog();
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
    if( aCtrlName.getLength() )
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
                        bool bEscAlreadyExisting = (aPropStr.getLength() && aPropStr.getStr()[0] == '&' );
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
                        //aPropIdStr += aSemi;
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
                            catch(MissingResourceException&)
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
                                catch(MissingResourceException&)
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
                            catch(MissingResourceException&)
                            {}
                        }

                        ::rtl::OUString aPropIdStr = aEsc;
                        aPropIdStr += aPureIdStr;
                        // TODO?: Change here and in toolkit
                        //aPropIdStr += aSemi;
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
                            catch(MissingResourceException&)
                            {
                                aResStr = xSourceStringResolver->resolveStringForLocale
                                    ( aPureSourceIdStr, rDefaultLocale );
                            }
                            xStringResourceManager->setStringForLocale( aPureIdStr, aResStr, rLocale );
                        }

                        ::rtl::OUString aPropIdStr = aEsc;
                        aPropIdStr += aPureIdStr;
                        // TODO?: Change here and in toolkit
                        //aPropIdStr += aSemi;
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
                            catch(MissingResourceException&)
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
                            bool bEscAlreadyExisting = (aPropStr.getLength() && aPropStr.getStr()[0] == '&' );
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
                                catch(MissingResourceException&)
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
                                    catch(MissingResourceException&)
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
                                catch(MissingResourceException&)
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
                                catch(MissingResourceException&)
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
                                catch(MissingResourceException&)
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

/*
void TEST_simulateDialogAddRemoveLocale( bool bAdd )
{
    Sequence< Locale > aLocaleSeq( 1 );
    Locale* pLocales = aLocaleSeq.getArray();

    ::com::sun::star::lang::Locale aLocale_en;
    aLocale_en.Language = ::rtl::OUString::createFromAscii( "en" );
    aLocale_en.Country = ::rtl::OUString::createFromAscii( "US" );

    ::com::sun::star::lang::Locale aLocale_de;
    aLocale_de.Language = ::rtl::OUString::createFromAscii( "de" );
    aLocale_de.Country = ::rtl::OUString::createFromAscii( "DE" );

    ::com::sun::star::lang::Locale aLocale_fr;
    aLocale_fr.Language = ::rtl::OUString::createFromAscii( "fr" );
    aLocale_fr.Country = ::rtl::OUString::createFromAscii( "FR" );

    int n = 0;
    if( n == 0 )
        pLocales[0] = aLocale_en;
    else if( n == 1 )
        pLocales[0] = aLocale_de;
    else if( n == 2 )
        pLocales[0] = aLocale_fr;

    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    LocalizationMgr* pMgr = pIDEShell->GetCurLocalizationMgr();
    if( bAdd )
        pMgr->handleAddLocales( aLocaleSeq );
    else
        pMgr->handleRemoveLocales( aLocaleSeq );
}

void TEST_simulateDialogAddLocale( void )
{
    TEST_simulateDialogAddRemoveLocale( true );
}

void TEST_simulateDialogRemoveLocale( void )
{
    TEST_simulateDialogAddRemoveLocale( false );
}
*/

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

    BasicIDE::MarkDocumentModified( m_aDocument );

    // update locale toolbar
    SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
    if ( pBindings )
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
            catch(IllegalArgumentException&)
            {
                bConsistant = false;
            }
        }
    }
    if( bModified )
    {
        BasicIDE::MarkDocumentModified( m_aDocument );

        // update slots
        SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
        if ( pBindings )
        {
            pBindings->Invalidate( SID_BASICIDE_CURRENT_LANG );
            pBindings->Invalidate( SID_BASICIDE_MANAGE_LANG );
        }

        handleTranslationbar();
    }

    DBG_ASSERT( bConsistant,
        "LocalizationMgr::handleRemoveLocales(): sequence contains unsupported locales" );
}

void LocalizationMgr::handleSetDefaultLocale( Locale aLocale )
{
    if( m_xStringResourceManager.is() )
    {
        try
        {
            m_xStringResourceManager->setDefaultLocale( aLocale );
        }
        catch(IllegalArgumentException&)
        {
            DBG_ERROR( "LocalizationMgr::handleSetDefaultLocale: Invalid locale" );
        }

        // update locale toolbar
        SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
        if ( pBindings )
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
        catch(IllegalArgumentException&)
        {
            DBG_ERROR( "LocalizationMgr::handleSetCurrentLocale: Invalid locale" );
        }

        // update locale toolbar
        SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
        if ( pBindings )
            pBindings->Invalidate( SID_BASICIDE_CURRENT_LANG );

        IDEBaseWindow* pCurWin = m_pIDEShell->GetCurWindow();
        if ( pCurWin && !pCurWin->IsSuspended() && pCurWin->IsA( TYPE( DialogWindow ) ) )
        {
            DialogWindow* pDlgWin = (DialogWindow*)pCurWin;
            DlgEditor* pWinEditor = pDlgWin->GetEditor();
            if( pWinEditor )
                pWinEditor->UpdatePropertyBrowserDelayed();
        }
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
    catch(IllegalArgumentException&)
    {
    }
}


DialogWindow* FindDialogWindowForEditor( DlgEditor* pEditor )
{
    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    IDEWindowTable& aIDEWindowTable = pIDEShell->GetIDEWindowTable();
    IDEBaseWindow* pWin = aIDEWindowTable.First();
    DialogWindow* pFoundDlgWin = NULL;
    while( pWin )
    {
        if ( !pWin->IsSuspended() && pWin->IsA( TYPE( DialogWindow ) ) )
        {
            DialogWindow* pDlgWin = (DialogWindow*)pWin;
            DlgEditor* pWinEditor = pDlgWin->GetEditor();
            if( pWinEditor == pEditor )
            {
                pFoundDlgWin = pDlgWin;
                break;
            }
        }
        pWin = aIDEWindowTable.Next();
    }
    return pFoundDlgWin;
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
    Reference< container::XNameContainer > xDialogLib( aDocument.getLibrary( E_DIALOGS, rLibName, sal_True ) );
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
        BasicIDE::MarkDocumentModified( aDocument );
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
    Reference< container::XNameContainer > xDialogLib( aDocument.getLibrary( E_DIALOGS, rLibName, sal_True ) );
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
    Reference< container::XNameContainer > xDialogLib( aDocument.getLibrary( E_DIALOGS, rLibName, sal_True ) );
    Reference< XStringResourceManager > xStringResourceManager =
        LocalizationMgr::getStringResourceFromDialogLibrary( xDialogLib );

    ::rtl::OUString aDialogName = pDlgWin->GetName();
    Reference< XStringResourceResolver > xDummyStringResolver;
    sal_Int32 nChangedCount = implHandleControlResourceProperties
        ( aControlAny, aDialogName, aCtrlName, xStringResourceManager,
          xDummyStringResolver, REMOVE_IDS_FROM_RESOURCE );

    if( nChangedCount )
        BasicIDE::MarkDocumentModified( aDocument );
}

void LocalizationMgr::setStringResourceAtDialog( const ScriptDocument& rDocument, const String& aLibName,
    const String& aDlgName, Reference< container::XNameContainer > xDialogModel )
{
    static ::rtl::OUString aResourceResolverPropName = ::rtl::OUString::createFromAscii( "ResourceResolver" );

    // Get library
    Reference< container::XNameContainer > xDialogLib( rDocument.getLibrary( E_DIALOGS, aLibName, sal_True ) );
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

void LocalizationMgr::renameStringResourceIDs( const ScriptDocument& rDocument, const String& aLibName,
    const String& aDlgName, Reference< container::XNameContainer > xDialogModel )
{
    // Get library
    Reference< container::XNameContainer > xDialogLib( rDocument.getLibrary( E_DIALOGS, aLibName, sal_True ) );
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

void LocalizationMgr::removeResourceForDialog( const ScriptDocument& rDocument, const String& aLibName,
    const String& aDlgName, Reference< container::XNameContainer > xDialogModel )
{
    // Get library
    Reference< container::XNameContainer > xDialogLib( rDocument.getLibrary( E_DIALOGS, aLibName, sal_True ) );
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
    Reference< container::XNameContainer > xDialogLib( aDocument.getLibrary( E_DIALOGS, rLibName, sal_True ) );
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

