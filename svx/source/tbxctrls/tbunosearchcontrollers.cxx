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
#include "precompiled_svx.hxx"

#include "tbunosearchcontrollers.hxx"
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/util/URL.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

namespace svx
{

static const ::rtl::OUString SEARCHITEM_SEARCHSTRING( RTL_CONSTASCII_USTRINGPARAM( "SearchItem.SearchString" ) );
static const ::rtl::OUString SEARCHITEM_SEARCHBACKWARD( RTL_CONSTASCII_USTRINGPARAM( "SearchItem.Backward" ) );

static const ::rtl::OUString COMMAND_EXECUTESEARCH( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExecuteSearch" ) );
static const ::rtl::OUString COMMAND_FINDTEXT( RTL_CONSTASCII_USTRINGPARAM( ".uno:FindText" ) );
static const ::rtl::OUString COMMAND_DOWNSEARCH( RTL_CONSTASCII_USTRINGPARAM(".uno:DownSearch") );
static const ::rtl::OUString COMMAND_UPSEARCH( RTL_CONSTASCII_USTRINGPARAM(".uno:UpSearch") );
static const ::rtl::OUString COMMAND_APPENDSEARCHHISTORY( RTL_CONSTASCII_USTRINGPARAM( "AppendSearchHistory") );

static const ::rtl::OUString SERVICENAME_URLTRANSFORMER( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer") );
static const sal_Int32       REMEMBER_SIZE = 10;

void impl_executeSearch( const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr, const css::uno::Reference< css::frame::XFrame >& xFrame, const css::uno::Sequence< css::beans::PropertyValue >& lArgs )
{
    css::uno::Reference< css::util::XURLTransformer > xURLTransformer( rSMgr->createInstance(SERVICENAME_URLTRANSFORMER), css::uno::UNO_QUERY );
    if ( xURLTransformer.is() )
    {
        css::util::URL aURL;
        aURL.Complete = COMMAND_EXECUTESEARCH;
        xURLTransformer->parseStrict(aURL);

        css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider(xFrame, css::uno::UNO_QUERY);
        if ( xDispatchProvider.is() )
        {
            css::uno::Reference< css::frame::XDispatch > xDispatch = xDispatchProvider->queryDispatch( aURL, ::rtl::OUString(), 0 );
            if ( xDispatch.is() && aURL.Complete.getLength() > 0 )
                xDispatch->dispatch( aURL, lArgs );
        }
    }
}

FindTextFieldControl::FindTextFieldControl( Window* pParent, WinBits nStyle,
    css::uno::Reference< css::frame::XFrame >& xFrame,
    css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager) :
    ComboBox( pParent, nStyle ),
    m_xFrame(xFrame),
    m_xServiceManager(xServiceManager),
    m_bToClearTextField(sal_True)
{
    InitControls_Impl();
}

FindTextFieldControl::~FindTextFieldControl()
{
}

void FindTextFieldControl::InitControls_Impl()
{
    SetText( SVX_RESSTR( RID_SVXSTR_FINDBAR_FIND ) );
    SetControlForeground(GetSettings().GetStyleSettings().GetDisableColor());

    EnableAutocomplete(TRUE, TRUE);
}

void FindTextFieldControl::Remember_Impl(const String& rStr)
{
    USHORT nCount = GetEntryCount();

    for (USHORT i=0; i<nCount; ++i)
    {
        if ( rStr == GetEntry(i))
            return;
    }

    if (nCount == REMEMBER_SIZE)
        RemoveEntry(REMEMBER_SIZE-1);

    InsertEntry(rStr, 0);
}

void FindTextFieldControl::Modify()
{
    ComboBox::Modify();

    SetControlForeground( GetSettings().GetStyleSettings().GetWindowTextColor() );
}

long FindTextFieldControl::PreNotify( NotifyEvent& rNEvt )
{
    long nRet= ComboBox::PreNotify( rNEvt );

    switch ( rNEvt.GetType() )
    {
        case EVENT_KEYINPUT:
        {
            const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
            sal_Bool bShift = pKeyEvent->GetKeyCode().IsShift();
            sal_uInt16 nCode = pKeyEvent->GetKeyCode().GetCode();

            if ( KEY_ESCAPE == nCode )
            {
                nRet = 1;
                GrabFocusToDocument();
            }

            if ( KEY_RETURN == nCode )
            {
                Remember_Impl(GetText());

                ::rtl::OUString sFindText = GetText();
                css::uno::Sequence< css::beans::PropertyValue > lArgs(2);

                lArgs[0].Name = SEARCHITEM_SEARCHSTRING;
                lArgs[0].Value <<= sFindText;

                lArgs[1].Name = SEARCHITEM_SEARCHBACKWARD;
                if (bShift)
                    lArgs[1].Value <<= sal_True;
                else
                    lArgs[1].Value <<= sal_False;

                impl_executeSearch(m_xServiceManager, m_xFrame, lArgs);
                nRet = 1;
            }
            break;
        }

        case EVENT_GETFOCUS:
            if ( m_bToClearTextField )
            {
                SetText( String() );
                m_bToClearTextField = sal_False;
            }
            SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );
            break;

        case EVENT_LOSEFOCUS:
            if ( GetText().Len() == 0 )
            {
                SetText( SVX_RESSTR( RID_SVXSTR_FINDBAR_FIND ) );
                SetControlForeground(GetSettings().GetStyleSettings().GetDisableColor());
                m_bToClearTextField = sal_True;
            }
            break;
    }

    return nRet;
}


//-----------------------------------------------------------------------------------------------------------
// SearchToolbarControllersManager

SearchToolbarControllersManager* SearchToolbarControllersManager::m_pInstance = 0;

SearchToolbarControllersManager::SearchToolbarControllersManager()
{
}

SearchToolbarControllersManager::~SearchToolbarControllersManager()
{
}

SearchToolbarControllersManager* SearchToolbarControllersManager::createControllersManager()
{
    if (!m_pInstance)
        m_pInstance = new SearchToolbarControllersManager();

    return m_pInstance;
}

void SearchToolbarControllersManager::registryController( const css::uno::Reference< css::frame::XFrame >& xFrame, const css::uno::Reference< css::frame::XStatusListener >& xStatusListener, const ::rtl::OUString& sCommandURL )
{
    SearchToolbarControllersMap::iterator pIt = aSearchToolbarControllersMap.find(xFrame);
    if (pIt == aSearchToolbarControllersMap.end())
    {
        SearchToolbarControllersVec lControllers(1);
        lControllers[0].Name = sCommandURL;
        lControllers[0].Value <<= xStatusListener;
        aSearchToolbarControllersMap.insert(SearchToolbarControllersMap::value_type(xFrame, lControllers));
    }
    else
    {
        sal_Int32 nSize = pIt->second.size();
        for (sal_Int32 i=0; i<nSize; ++i)
        {
            if (pIt->second[i].Name.equals(sCommandURL))
                return;
        }

        pIt->second.resize(nSize+1);
        pIt->second[nSize].Name = sCommandURL;
        pIt->second[nSize].Value <<= xStatusListener;
    }
}

void SearchToolbarControllersManager::freeController( const css::uno::Reference< css::frame::XFrame >& xFrame, const css::uno::Reference< css::frame::XStatusListener >& /*xStatusListener*/, const ::rtl::OUString& sCommandURL )
{
    SearchToolbarControllersMap::iterator pIt = aSearchToolbarControllersMap.find(xFrame);
    if (pIt != aSearchToolbarControllersMap.end())
    {
        for (SearchToolbarControllersVec::iterator pItCtrl=pIt->second.begin(); pItCtrl!=pIt->second.end(); ++pItCtrl)
        {
            if (pItCtrl->Name.equals(sCommandURL))
            {
                pIt->second.erase(pItCtrl);
                break;
            }
        }

        if (pIt->second.empty())
            aSearchToolbarControllersMap.erase(pIt);
    }
}

css::uno::Reference< css::frame::XStatusListener > SearchToolbarControllersManager::findController( const css::uno::Reference< css::frame::XFrame >& xFrame, const ::rtl::OUString& sCommandURL )
{
    css::uno::Reference< css::frame::XStatusListener > xStatusListener;

    SearchToolbarControllersMap::iterator pIt = aSearchToolbarControllersMap.find(xFrame);
    if (pIt != aSearchToolbarControllersMap.end())
    {
        for (SearchToolbarControllersVec::iterator pItCtrl =pIt->second.begin(); pItCtrl != pIt->second.end(); ++pItCtrl)
        {
            if (pItCtrl->Name.equals(sCommandURL))
            {
                pItCtrl->Value >>= xStatusListener;
                break;
            }
        }
    }

    return xStatusListener;
}

//-----------------------------------------------------------------------------------------------------------
// FindTextToolbarController

FindTextToolbarController::FindTextToolbarController( const css::uno::Reference< css::lang::XMultiServiceFactory >& rServiceManager )
    :svt::ToolboxController( rServiceManager,
    css::uno::Reference< css::frame::XFrame >(),
    COMMAND_FINDTEXT )
{
}

FindTextToolbarController::~FindTextToolbarController()
{
}

// XInterface
css::uno::Any SAL_CALL FindTextToolbarController::queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException )
{
    css::uno::Any a = ToolboxController::queryInterface( aType );
    if ( a.hasValue() )
        return a;

    return ::cppu::queryInterface( aType, static_cast< css::lang::XServiceInfo* >( this ) );
}

void SAL_CALL FindTextToolbarController::acquire() throw ()
{
    ToolboxController::acquire();
}

void SAL_CALL FindTextToolbarController::release() throw ()
{
    ToolboxController::release();
}

// XServiceInfo
::rtl::OUString SAL_CALL FindTextToolbarController::getImplementationName() throw( css::uno::RuntimeException )
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL FindTextToolbarController::supportsService( const ::rtl::OUString& ServiceName ) throw( css::uno::RuntimeException )
{
    const css::uno::Sequence< ::rtl::OUString > aSNL( getSupportedServiceNames() );
    const ::rtl::OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return true;

    return false;
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL FindTextToolbarController::getSupportedServiceNames() throw( css::uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

css::uno::Sequence< ::rtl::OUString >  FindTextToolbarController::getSupportedServiceNames_Static() throw()
{
    css::uno::Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ToolbarController" ) );
    return aSNS;
}

// XComponent
void SAL_CALL FindTextToolbarController::dispose() throw ( css::uno::RuntimeException )
{
    SolarMutexGuard aSolarMutexGuard;

    SearchToolbarControllersManager::createControllersManager()->freeController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);

    svt::ToolboxController::dispose();
    delete m_pFindTextFieldControl;
    m_pFindTextFieldControl = 0;
}

// XInitialization
void SAL_CALL FindTextToolbarController::initialize( const css::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException)
{
    svt::ToolboxController::initialize(aArguments);

    Window* pWindow = VCLUnoHelper::GetWindow( getParent() );
    ToolBox* pToolBox = (ToolBox*)pWindow;
    if ( pToolBox )
    {
        USHORT nItemCount = pToolBox->GetItemCount();
        for ( USHORT i=0; i<nItemCount; ++i )
        {
            ::rtl::OUString sItemCommand = pToolBox->GetItemCommand(i);
            if ( sItemCommand.equals( COMMAND_DOWNSEARCH ) )
                m_nDownSearchId = i;
            else if (sItemCommand.equals( COMMAND_UPSEARCH ))
                m_nUpSearchId = i;
        }
    }

    SearchToolbarControllersManager::createControllersManager()->registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);
}

// XToolbarController
void SAL_CALL FindTextToolbarController::execute( sal_Int16 /*KeyModifier*/ ) throw ( css::uno::RuntimeException )
{
}

css::uno::Reference< css::awt::XWindow > SAL_CALL FindTextToolbarController::createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) throw ( css::uno::RuntimeException )
{
    css::uno::Reference< css::awt::XWindow > xItemWindow;

    css::uno::Reference< css::awt::XWindow > xParent( Parent );
    Window* pParent = VCLUnoHelper::GetWindow( xParent );
    if ( pParent )
    {
        ToolBox* pToolbar =  ( ToolBox* )pParent;
        m_pFindTextFieldControl = new FindTextFieldControl( pToolbar, WinBits( WB_DROPDOWN | WB_VSCROLL), m_xFrame, m_xServiceManager );

        Size aSize(250, m_pFindTextFieldControl->GetTextHeight() + 200);
        m_pFindTextFieldControl->SetSizePixel( aSize );
        m_pFindTextFieldControl->SetModifyHdl(LINK(this, FindTextToolbarController, EditModifyHdl));
    }
    xItemWindow = VCLUnoHelper::GetInterface( m_pFindTextFieldControl );

    return xItemWindow;
}

// XStatusListener
void SAL_CALL FindTextToolbarController::statusChanged( const css::frame::FeatureStateEvent& rEvent ) throw ( css::uno::RuntimeException )
{
    SolarMutexGuard aSolarMutexGuard;
    if ( m_bDisposed )
        return;

    ::rtl::OUString aFeatureURL = rEvent.FeatureURL.Complete;
    if (aFeatureURL.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("AppendSearchHistory")))
    {
        m_pFindTextFieldControl->Remember_Impl(m_pFindTextFieldControl->GetText());
    }
}

IMPL_LINK( FindTextToolbarController, EditModifyHdl, void *, EMPTYARG )
{
    // enable or disable item DownSearch/UpSearch of findbar
    Window* pWindow = VCLUnoHelper::GetWindow( getParent() );
    ToolBox* pToolBox = (ToolBox*)pWindow;
    if ( pToolBox && m_pFindTextFieldControl )
    {
        if (m_pFindTextFieldControl->GetText().Len()>0)
        {
            if ( !pToolBox->IsItemEnabled(m_nDownSearchId) )
                pToolBox->EnableItem(m_nDownSearchId, sal_True);
            if ( !pToolBox->IsItemEnabled(m_nUpSearchId) )
                pToolBox->EnableItem(m_nUpSearchId, sal_True);
        }
        else
        {
            if ( pToolBox->IsItemEnabled(m_nDownSearchId) )
                pToolBox->EnableItem(m_nDownSearchId, sal_False);
            if ( pToolBox->IsItemEnabled(m_nUpSearchId) )
                pToolBox->EnableItem(m_nUpSearchId, sal_False);
        }
    }

    return 0;
}

//-----------------------------------------------------------------------------------------------------------
// class DownSearchToolboxController

DownSearchToolboxController::DownSearchToolboxController(const css::uno::Reference< css::lang::XMultiServiceFactory >& rServiceManager )
    : svt::ToolboxController( rServiceManager,
    css::uno::Reference< css::frame::XFrame >(),
    COMMAND_DOWNSEARCH )
{
}

DownSearchToolboxController::~DownSearchToolboxController()
{
}

// XInterface
css::uno::Any SAL_CALL DownSearchToolboxController::queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException )
{
    css::uno::Any a = ToolboxController::queryInterface( aType );
    if ( a.hasValue() )
        return a;

    return ::cppu::queryInterface( aType, static_cast< css::lang::XServiceInfo* >( this ) );
}

void SAL_CALL DownSearchToolboxController::acquire() throw ()
{
    ToolboxController::acquire();
}

void SAL_CALL DownSearchToolboxController::release() throw ()
{
    ToolboxController::release();
}

// XServiceInfo
::rtl::OUString SAL_CALL DownSearchToolboxController::getImplementationName() throw( css::uno::RuntimeException )
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL DownSearchToolboxController::supportsService( const ::rtl::OUString& ServiceName ) throw( css::uno::RuntimeException )
{
    const css::uno::Sequence< ::rtl::OUString > aSNL( getSupportedServiceNames() );
    const ::rtl::OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return true;

    return false;
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL DownSearchToolboxController::getSupportedServiceNames() throw( css::uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

css::uno::Sequence< ::rtl::OUString >  DownSearchToolboxController::getSupportedServiceNames_Static() throw()
{
    css::uno::Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ToolbarController" ));
    return aSNS;
}

// XComponent
void SAL_CALL DownSearchToolboxController::dispose() throw ( css::uno::RuntimeException )
{
    SolarMutexGuard aSolarMutexGuard;

    SearchToolbarControllersManager::createControllersManager()->freeController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);

    svt::ToolboxController::dispose();
}

// XInitialization
void SAL_CALL DownSearchToolboxController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException )
{
    svt::ToolboxController::initialize( aArguments );
    SearchToolbarControllersManager::createControllersManager()->registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);
}

// XToolbarController
void SAL_CALL DownSearchToolboxController::execute( sal_Int16 /*KeyModifier*/ ) throw ( css::uno::RuntimeException)
{
    if ( m_bDisposed )
        throw css::lang::DisposedException();

    ::rtl::OUString sFindText;
    Window* pWindow = VCLUnoHelper::GetWindow( getParent() );
    ToolBox* pToolBox = (ToolBox*)pWindow;
    if ( pToolBox )
    {
        USHORT nItemCount = pToolBox->GetItemCount();
        for ( USHORT i=0; i<nItemCount; ++i )
        {
            ::rtl::OUString sItemCommand = pToolBox->GetItemCommand(i);
            if ( sItemCommand.equals( COMMAND_FINDTEXT ) )
            {
                Window* pItemWin = pToolBox->GetItemWindow(i);
                if (pItemWin)
                    sFindText = pItemWin->GetText();
                break;
            }
        }
    }

    css::uno::Sequence< css::beans::PropertyValue > lArgs(2);
    lArgs[0].Name = SEARCHITEM_SEARCHSTRING;
    lArgs[0].Value <<= sFindText;
    lArgs[1].Name = SEARCHITEM_SEARCHBACKWARD;
    lArgs[1].Value <<= sal_False;

    impl_executeSearch(m_xServiceManager, m_xFrame, lArgs);

    css::frame::FeatureStateEvent aEvent;
    aEvent.FeatureURL.Complete = COMMAND_APPENDSEARCHHISTORY;
    css::uno::Reference< css::frame::XStatusListener > xStatusListener = SearchToolbarControllersManager::createControllersManager()->findController(m_xFrame, COMMAND_FINDTEXT);
    if (xStatusListener.is())
        xStatusListener->statusChanged( aEvent );
}

// XStatusListener
void SAL_CALL DownSearchToolboxController::statusChanged( const css::frame::FeatureStateEvent& /*rEvent*/ ) throw ( css::uno::RuntimeException )
{
    SolarMutexGuard aSolarMutexGuard;
    if ( m_bDisposed )
        return;
}

//-----------------------------------------------------------------------------------------------------------
// class UpSearchToolboxController

UpSearchToolboxController::UpSearchToolboxController( const css::uno::Reference< css::lang::XMultiServiceFactory > & rServiceManager )
    :svt::ToolboxController( rServiceManager,
    css::uno::Reference< css::frame::XFrame >(),
    COMMAND_UPSEARCH )
{
}

UpSearchToolboxController::~UpSearchToolboxController()
{
}

// XInterface
css::uno::Any SAL_CALL UpSearchToolboxController::queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException )
{
    css::uno::Any a = ToolboxController::queryInterface( aType );
    if ( a.hasValue() )
        return a;

    return ::cppu::queryInterface( aType, static_cast< css::lang::XServiceInfo* >( this ) );
}

void SAL_CALL UpSearchToolboxController::acquire() throw ()
{
    ToolboxController::acquire();
}

void SAL_CALL UpSearchToolboxController::release() throw ()
{
    ToolboxController::release();
}

// XServiceInfo
::rtl::OUString SAL_CALL UpSearchToolboxController::getImplementationName() throw( css::uno::RuntimeException )
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL UpSearchToolboxController::supportsService( const ::rtl::OUString& ServiceName ) throw( css::uno::RuntimeException )
{
    const css::uno::Sequence< ::rtl::OUString > aSNL( getSupportedServiceNames() );
    const ::rtl::OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return true;

    return false;
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL UpSearchToolboxController::getSupportedServiceNames() throw( css::uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

css::uno::Sequence< ::rtl::OUString >  UpSearchToolboxController::getSupportedServiceNames_Static() throw()
{
    css::uno::Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ToolbarController" ) );
    return aSNS;
}

// XComponent
void SAL_CALL UpSearchToolboxController::dispose() throw ( css::uno::RuntimeException )
{
    SolarMutexGuard aSolarMutexGuard;

    SearchToolbarControllersManager::createControllersManager()->freeController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);

    svt::ToolboxController::dispose();
}

// XInitialization
void SAL_CALL UpSearchToolboxController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException )
{
    svt::ToolboxController::initialize( aArguments );
    SearchToolbarControllersManager::createControllersManager()->registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);
}

// XToolbarController
void SAL_CALL UpSearchToolboxController::execute( sal_Int16 /*KeyModifier*/ ) throw ( css::uno::RuntimeException )
{
    if ( m_bDisposed )
        throw css::lang::DisposedException();

    ::rtl::OUString sFindText;
    Window* pWindow = VCLUnoHelper::GetWindow( getParent() );
    ToolBox* pToolBox = (ToolBox*)pWindow;
    if ( pToolBox )
    {
        USHORT nItemCount = pToolBox->GetItemCount();
        for ( USHORT i=0; i<nItemCount; ++i )
        {
            ::rtl::OUString sItemCommand = pToolBox->GetItemCommand(i);
            if ( sItemCommand.equals( COMMAND_FINDTEXT ) )
            {
                Window* pItemWin = pToolBox->GetItemWindow(i);
                if (pItemWin)
                    sFindText = pItemWin->GetText();
                break;
            }
        }
    }

    css::uno::Sequence< css::beans::PropertyValue > lArgs(2);
    lArgs[0].Name = SEARCHITEM_SEARCHSTRING;
    lArgs[0].Value <<= sFindText;
    lArgs[1].Name =  SEARCHITEM_SEARCHBACKWARD;
    lArgs[1].Value <<= sal_True;

    impl_executeSearch(m_xServiceManager, m_xFrame, lArgs);

    css::frame::FeatureStateEvent aEvent;
    aEvent.FeatureURL.Complete = COMMAND_APPENDSEARCHHISTORY;
    css::uno::Reference< css::frame::XStatusListener > xStatusListener = SearchToolbarControllersManager::createControllersManager()->findController(m_xFrame, COMMAND_FINDTEXT);
    if (xStatusListener.is())
        xStatusListener->statusChanged( aEvent );
}

// XStatusListener
void SAL_CALL UpSearchToolboxController::statusChanged( const css::frame::FeatureStateEvent& /*rEvent*/ ) throw ( css::uno::RuntimeException )
{
    SolarMutexGuard aSolarMutexGuard;
    if ( m_bDisposed )
        return;
}

//-----------------------------------------------------------------------------------------------------------
// class FindbarDispatcher

FindbarDispatcher::FindbarDispatcher(const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory)
    : m_xFactory( xFactory )
{
}

FindbarDispatcher::~FindbarDispatcher()
{
    m_xFactory = NULL;
    m_xFrame = NULL;
}

// XInterface
css::uno::Any SAL_CALL FindbarDispatcher::queryInterface( const css::uno::Type& aType ) throw( css::uno::RuntimeException )
{
    css::uno::Any aReturn( ::cppu::queryInterface( aType,
        static_cast< css::lang::XServiceInfo* >(this),
        static_cast< css::lang::XInitialization* >(this),
        static_cast< css::frame::XDispatchProvider* >(this),
        static_cast< css::frame::XDispatch* >(this)) );

    if ( aReturn.hasValue() )
        return aReturn;

    return OWeakObject::queryInterface( aType );
}

void SAL_CALL FindbarDispatcher::acquire() throw()
{
    OWeakObject::acquire();
}

void SAL_CALL FindbarDispatcher::release() throw()
{
    OWeakObject::release();
}

// XServiceInfo
::rtl::OUString SAL_CALL FindbarDispatcher::getImplementationName() throw( css::uno::RuntimeException )
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL FindbarDispatcher::supportsService( const ::rtl::OUString& ServiceName ) throw( css::uno::RuntimeException )
{
    return (
        ServiceName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("com.sun.star.comp.svx.FindbarDispatcher")) ||
        ServiceName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("com.sun.star.frame.ProtocolHandler"))
        );
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL FindbarDispatcher::getSupportedServiceNames() throw( css::uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

css::uno::Sequence< ::rtl::OUString >  FindbarDispatcher::getSupportedServiceNames_Static() throw()
{
    css::uno::Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.svx.FindbarDispatcher" ));
    aSNS.getArray()[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ProtocolHandler" ));
    return aSNS;
}

// XInitialization
void SAL_CALL FindbarDispatcher::initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException )
{
    if ( aArguments.getLength() )
        aArguments[0] >>= m_xFrame;
}

// XDispatchProvider
css::uno::Reference< css::frame::XDispatch > SAL_CALL FindbarDispatcher::queryDispatch( const css::util::URL& aURL, const ::rtl::OUString& /*sTargetFrameName*/, sal_Int32 /*nSearchFlags*/ ) throw( css::uno::RuntimeException )
{
    css::uno::Reference< css::frame::XDispatch > xDispatch;

    if ( aURL.Protocol.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.findbar:")) )
        xDispatch = this;

    return xDispatch;
}

css::uno::Sequence < css::uno::Reference< css::frame::XDispatch > > SAL_CALL FindbarDispatcher::queryDispatches( const css::uno::Sequence < css::frame::DispatchDescriptor >& seqDescripts ) throw( css::uno::RuntimeException )
{
    sal_Int32 nCount = seqDescripts.getLength();
    css::uno::Sequence < css::uno::Reference < XDispatch > > lDispatcher( nCount );

    for( sal_Int32 i=0; i<nCount; ++i )
        lDispatcher[i] = queryDispatch( seqDescripts[i].FeatureURL, seqDescripts[i].FrameName, seqDescripts[i].SearchFlags );

    return lDispatcher;
}

// XDispatch
void SAL_CALL FindbarDispatcher::dispatch( const css::util::URL& aURL, const css::uno::Sequence < css::beans::PropertyValue >& /*lArgs*/ ) throw( css::uno::RuntimeException )
{
    //vnd.sun.star.findbar:FocusToFindbar  - set cursor to the FindTextFieldControl of the findbar
    if ( aURL.Path.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("FocusToFindbar")) )
    {
        css::uno::Reference< css::beans::XPropertySet > xPropSet(m_xFrame, css::uno::UNO_QUERY);
        if(!xPropSet.is())
            return;

        css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
        css::uno::Any aValue = xPropSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" )) );
        aValue >>= xLayoutManager;
        if (!xLayoutManager.is())
            return;

        const ::rtl::OUString sResourceURL = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/findbar" ));
        css::uno::Reference< css::ui::XUIElement > xUIElement = xLayoutManager->getElement(sResourceURL);
        if (!xUIElement.is())
            return;

        css::uno::Reference< css::awt::XWindow > xWindow(xUIElement->getRealInterface(), css::uno::UNO_QUERY);
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        ToolBox* pToolBox = (ToolBox*)pWindow;
        if ( pToolBox )
        {
            USHORT nItemCount = pToolBox->GetItemCount();
            for ( USHORT i=0; i<nItemCount; ++i )
            {
                ::rtl::OUString sItemCommand = pToolBox->GetItemCommand(i);
                if ( sItemCommand.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(".uno:FindText")) )
                {
                    Window* pItemWin = pToolBox->GetItemWindow( i );
                    if ( pItemWin )
                    {
                        pItemWin->GrabFocus();
                        return;
                    }
                }
            }
        }

    }
}

void SAL_CALL FindbarDispatcher::addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*xControl*/, const css::util::URL& /*aURL*/ ) throw ( css::uno::RuntimeException )
{
}

void SAL_CALL FindbarDispatcher::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*xControl*/, const css::util::URL& /*aURL*/ ) throw ( css::uno::RuntimeException )
{
}

//-----------------------------------------------------------------------------------------------------------
// create Instance

css::uno::Reference< css::uno::XInterface > SAL_CALL FindTextToolbarController_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr )
{
    return *new FindTextToolbarController( rSMgr );
}

css::uno::Reference< css::uno::XInterface > SAL_CALL DownSearchToolboxController_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr )
{
    return *new DownSearchToolboxController( rSMgr );
}

css::uno::Reference< css::uno::XInterface > SAL_CALL UpSearchToolboxController_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr )
{
    return *new UpSearchToolboxController( rSMgr );
}

css::uno::Reference< css::uno::XInterface > SAL_CALL FindbarDispatcher_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr )
{
    return *new FindbarDispatcher( rSMgr );
}

//-----------------------------------------------------------------------------------------------------------
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
