/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: localizationmgr.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-01-02 15:50:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef _COM_SUN_STAR_RESOURCE_XSTRINGRESOURCESUPPLIER_HPP_
#include <com/sun/star/resource/XStringResourceSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <com/sun/star/frame/XLayoutManager.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::resource;


LocalizationMgr::LocalizationMgr( BasicIDEShell* pIDEShell,
    SfxObjectShell* pShell, String aLibName,
    const Reference< XStringResourceManager >& xStringResourceManager )
        : m_xStringResourceManager( xStringResourceManager )
        , m_pIDEShell( pIDEShell )
        , m_pShell( pShell )
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
        ( m_pIDEShell->GetViewFrame()->GetFrame()->GetFrameInterface(), uno::UNO_QUERY );
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


// TODO: Iterator class as similar functionality is needed in different contexts
void LocalizationMgr::implEnableDisableResourceForAllLibraryDialogs( HandleResourceMode eMode )
{
    Sequence< ::rtl::OUString > aDlgNames = BasicIDE::GetDialogNames( m_pShell, m_aLibName );
    sal_Int32 nDlgCount = aDlgNames.getLength();
    const ::rtl::OUString* pDlgNames = aDlgNames.getConstArray();

    for( sal_Int32 i = 0 ; i < nDlgCount ; i++ )
    {
        String aDlgName = pDlgNames[ i ];
        DialogWindow* pWin = m_pIDEShell->FindDlgWin( m_pShell, m_aLibName, aDlgName, FALSE );
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
                    ::rtl::OUString(), m_xStringResourceManager, eMode );

                // Handle all controls
                Sequence< ::rtl::OUString > aNames = xDialog->getElementNames();
                const ::rtl::OUString* pNames = aNames.getConstArray();
                sal_Int32 nCtrls = aNames.getLength();
                for( sal_Int32 i = 0 ; i < nCtrls ; ++i )
                {
                    ::rtl::OUString aCtrlName( pNames[i] );
                    Any aCtrl = xDialog->getByName( aCtrlName );
                    implHandleControlResourceProperties( aCtrl, aDlgName,
                        aCtrlName, m_xStringResourceManager, eMode );
                }
            }
        }
    }
}

// Works on xStringResourceManager's current language for SET_IDS/RESET_IDS,
// anyway only one language should exist when calling this method then,
// either the first one for mode SET_IDS or the last one for mode RESET_IDS
sal_Int32 LocalizationMgr::implHandleControlResourceProperties
    ( Any aControlAny, const ::rtl::OUString& aDialogName, const ::rtl::OUString& aCtrlName,
        Reference< XStringResourceManager > xStringResourceManager, HandleResourceMode eMode )
{
    static ::rtl::OUString aDot  = ::rtl::OUString::createFromAscii( "." );
    static ::rtl::OUString aEsc  = ::rtl::OUString::createFromAscii( "&" );
    static ::rtl::OUString aSemi = ::rtl::OUString::createFromAscii( ";" );

    sal_Int32 nChangedCount = 0;

    Reference< XPropertySet > xPropertySet;
    aControlAny >>= xPropertySet;
    if( xPropertySet.is() )
    {
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
                if( rProp.Type.getTypeClass() == TypeClass_STRING &&
                    isLanguageDependentProperty( aPropName ) )
                {
                    Any aPropAny = xPropertySet->getPropertyValue( aPropName );
                    ::rtl::OUString aPropStr;
                    aPropAny >>= aPropStr;

                    if( eMode == SET_IDS )
                    {
                        // Replace string by id, add id+string to StringResource
                        bool bEscAlreadyExisting = (aPropStr.getLength() && aPropStr.getStr()[0] == '&' );
                        if( bEscAlreadyExisting )
                            continue;

                        sal_Int32 nUniqueId = xStringResourceManager->getUniqueNumericId();
                        // TODO?: Catch NoSupportException? Cannot happen with empty resource

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

                        // Set Id for all locales
                        //xStringResourceManager->setString( aPureIdStr, aPropStr );
                        Sequence< Locale > aLocaleSeq = xStringResourceManager->getLocales();
                        const Locale* pLocales = aLocaleSeq.getConstArray();
                        sal_Int32 nLocaleCount = aLocaleSeq.getLength();
                        for( sal_Int32 i = 0 ; i < nLocaleCount ; i++ )
                        {
                            const Locale& rLocale = pLocales[ i ];
                            try
                            {
                                xStringResourceManager->setStringForLocale( aPureIdStr, aPropStr, rLocale );
                            }
                            catch(MissingResourceException&)
                            {
                            }
                        }

                        ::rtl::OUString aPropIdStr = aEsc;
                        aPropIdStr += aPureIdStr;
                        // TODO: Change here and in toolkit
                        //aPropIdStr += aSemi;
                        (void)aSemi;
                        aPropAny <<= aPropIdStr;
                        xPropertySet->setPropertyValue( aPropName, aPropAny );
                    }
                    else if( eMode == RESET_IDS )
                    {
                        // Replace id by string from StringResource
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
                    else if( eMode == REMOVE_IDS_FROM_RESOURCE )
                    {
                        // Remove Id for all locales
                        if( aPropStr.getLength() > 1 )
                        {
                            ::rtl::OUString aPureIdStr = aPropStr.copy( 1 );

                            Sequence< Locale > aLocaleSeq = xStringResourceManager->getLocales();
                            const Locale* pLocales = aLocaleSeq.getConstArray();
                            sal_Int32 nLocaleCount = aLocaleSeq.getLength();
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
                    nChangedCount++;
                }
            }
        }
    }
    return nChangedCount;
}

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

    BasicIDE::MarkDocShellModified( m_pShell );

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
            if( rLocale.Language == rLastResLocale.Language &&
                rLocale.Country  == rLastResLocale.Country &&
                rLocale.Variant  == rLastResLocale.Variant )
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
        BasicIDE::MarkDocShellModified( m_pShell );

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
    DialogWindow* pDlgWin = FindDialogWindowForEditor( pEditor );
    if( !pDlgWin )
        return;

    // Get library
    SfxObjectShell* pShell = pDlgWin->GetShell();
    const String& rLibName = pDlgWin->GetLibName();
    Reference< container::XNameContainer > xDialogLib = BasicIDE::GetDialogLibrary( pShell, rLibName, TRUE );
    Reference< XStringResourceManager > xStringResourceManager =
        LocalizationMgr::getStringResourceFromDialogLibrary( xDialogLib );

    // Set resource property
    if( !xStringResourceManager.is() || xStringResourceManager->getLocales().getLength() == 0 )
        return;

    ::rtl::OUString aDialogName = pDlgWin->GetName();
    sal_Int32 nChangedCount = implHandleControlResourceProperties
        ( aControlAny, aDialogName, aCtrlName, xStringResourceManager, SET_IDS );

    if( nChangedCount )
        BasicIDE::MarkDocShellModified( pShell );
}

void LocalizationMgr::deleteControlResourceIDsForDeletedEditorObject( DlgEditor* pEditor,
    Any aControlAny, const ::rtl::OUString& aCtrlName )
{
    DialogWindow* pDlgWin = FindDialogWindowForEditor( pEditor );
    if( !pDlgWin )
        return;

    // Get library
    SfxObjectShell* pShell = pDlgWin->GetShell();
    const String& rLibName = pDlgWin->GetLibName();
    Reference< container::XNameContainer > xDialogLib = BasicIDE::GetDialogLibrary( pShell, rLibName, TRUE );
    Reference< XStringResourceManager > xStringResourceManager =
        LocalizationMgr::getStringResourceFromDialogLibrary( xDialogLib );

    ::rtl::OUString aDialogName = pDlgWin->GetName();
    sal_Int32 nChangedCount = implHandleControlResourceProperties
        ( aControlAny, aDialogName, aCtrlName, xStringResourceManager, REMOVE_IDS_FROM_RESOURCE );

    if( nChangedCount )
        BasicIDE::MarkDocShellModified( pShell );
}

void LocalizationMgr::setStringResourceAtDialog( SfxObjectShell* pShell, const String& aLibName,
    const String& aDlgName, Reference< container::XNameContainer > xDialogModel )
{
    static ::rtl::OUString aResourceResolverPropName = ::rtl::OUString::createFromAscii( "ResourceResolver" );

    // Get library
    Reference< container::XNameContainer > xDialogLib = BasicIDE::GetDialogLibrary( pShell, aLibName, TRUE );
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
            implHandleControlResourceProperties( aDialogCtrl, aDlgName,
                ::rtl::OUString(), xStringResourceManager, SET_IDS );
        }

        Reference< beans::XPropertySet > xDlgPSet( xDialogModel, UNO_QUERY );
        Any aStringResourceManagerAny;
        aStringResourceManagerAny <<= xStringResourceManager;
        xDlgPSet->setPropertyValue( aResourceResolverPropName, aStringResourceManagerAny );
    }
}

void LocalizationMgr::removeResourceForDialog( SfxObjectShell* pShell, const String& aLibName,
    const String& aDlgName, Reference< container::XNameContainer > xDialogModel )
{
    // Get library
    Reference< container::XNameContainer > xDialogLib = BasicIDE::GetDialogLibrary( pShell, aLibName, TRUE );
    Reference< XStringResourceManager > xStringResourceManager =
        LocalizationMgr::getStringResourceFromDialogLibrary( xDialogLib );

    // Set resource property
    if( xStringResourceManager.is() )
    {
        Any aDialogCtrl;
        aDialogCtrl <<= xDialogModel;
        implHandleControlResourceProperties( aDialogCtrl, aDlgName,
            ::rtl::OUString(), xStringResourceManager, REMOVE_IDS_FROM_RESOURCE );

        // TODO: Remove for all controls

        // Handle all controls
        Sequence< ::rtl::OUString > aNames = xDialogModel->getElementNames();
        const ::rtl::OUString* pNames = aNames.getConstArray();
        sal_Int32 nCtrls = aNames.getLength();
        for( sal_Int32 i = 0 ; i < nCtrls ; ++i )
        {
            ::rtl::OUString aCtrlName( pNames[i] );
            Any aCtrl = xDialogModel->getByName( aCtrlName );
            implHandleControlResourceProperties( aCtrl, aDlgName,
                aCtrlName, xStringResourceManager, REMOVE_IDS_FROM_RESOURCE );
        }
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

