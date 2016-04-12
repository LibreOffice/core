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

#include <tbunosearchcontrollers.hxx>

#include <config_features.h>

#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <svl/ctloptions.hxx>
#include <svl/srchitem.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <rtl/instance.hxx>
#include <svx/srchdlg.hxx>

#include <vcl/fixed.hxx>

using namespace css;

namespace {

static const char SEARCHITEM_COMMAND[] = "SearchItem.Command";
static const char SEARCHITEM_SEARCHSTRING[] = "SearchItem.SearchString";
static const char SEARCHITEM_SEARCHBACKWARD[] = "SearchItem.Backward";
static const char SEARCHITEM_SEARCHFORMATTED[] = "SearchItem.SearchFormatted";
static const char SEARCHITEM_SEARCHFLAGS[] = "SearchItem.SearchFlags";
static const char SEARCHITEM_TRANSLITERATEFLAGS[] = "SearchItem.TransliterateFlags";
static const char SEARCHITEM_ALGORITHMTYPE[] = "SearchItem.AlgorithmType";

static const char COMMAND_EXECUTESEARCH[] = ".uno:ExecuteSearch";
static const char COMMAND_FINDTEXT[] = ".uno:FindText";
static const char COMMAND_DOWNSEARCH[] = ".uno:DownSearch";
static const char COMMAND_UPSEARCH[] = ".uno:UpSearch";
static const char COMMAND_EXITSEARCH[] = ".uno:ExitSearch";
static const char COMMAND_MATCHCASE[] = ".uno:MatchCase";
static const char COMMAND_SEARCHFORMATTED[] = ".uno:SearchFormattedDisplayString";
static const char COMMAND_APPENDSEARCHHISTORY[] = "AppendSearchHistory";

static const sal_Int32       REMEMBER_SIZE = 10;

void impl_executeSearch( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                         const css::uno::Reference< css::frame::XFrame >& xFrame,
                         const ToolBox* pToolBox,
                         const bool aSearchBackwards = false,
                         const bool aFindAll = false )
{
    css::uno::Reference< css::util::XURLTransformer > xURLTransformer( css::util::URLTransformer::create( rxContext ) );
    css::util::URL aURL;
    aURL.Complete = COMMAND_EXECUTESEARCH;
    xURLTransformer->parseStrict(aURL);

    OUString sFindText;
    bool aMatchCase = false;
    bool bSearchFormatted = false;
    if ( pToolBox )
    {
        sal_uInt16 nItemCount = pToolBox->GetItemCount();
        for ( sal_uInt16 i=0; i<nItemCount; ++i )
        {
            OUString sItemCommand = pToolBox->GetItemCommand(i);
            if ( sItemCommand == COMMAND_FINDTEXT )
            {
                vcl::Window* pItemWin = pToolBox->GetItemWindow(i);
                if (pItemWin)
                    sFindText = pItemWin->GetText();
            } else if ( sItemCommand == COMMAND_MATCHCASE )
            {
                CheckBox* pItemWin = static_cast<CheckBox*>( pToolBox->GetItemWindow(i) );
                if (pItemWin)
                    aMatchCase = pItemWin->IsChecked();
            } else if ( sItemCommand == COMMAND_SEARCHFORMATTED )
            {
                CheckBox* pItemWin = static_cast<CheckBox*>( pToolBox->GetItemWindow(i) );
                if (pItemWin)
                    bSearchFormatted = pItemWin->IsChecked();
            }
        }
    }

    css::uno::Sequence< css::beans::PropertyValue > lArgs(7);
    lArgs[0].Name = SEARCHITEM_SEARCHSTRING;
    lArgs[0].Value <<= sFindText;
    lArgs[1].Name = SEARCHITEM_SEARCHBACKWARD;
    lArgs[1].Value <<= aSearchBackwards;
    lArgs[2].Name = SEARCHITEM_SEARCHFLAGS;
    lArgs[2].Value <<= (sal_Int32)0;
    lArgs[3].Name = SEARCHITEM_TRANSLITERATEFLAGS;
    SvtCTLOptions aCTLOptions;
    sal_Int32 nFlags = 0;
    nFlags |= (!aMatchCase ? static_cast<int>(css::i18n::TransliterationModules_IGNORE_CASE) : 0);
    nFlags |= (aCTLOptions.IsCTLFontEnabled() ? css::i18n::TransliterationModulesExtra::IGNORE_DIACRITICS_CTL:0 );
    nFlags |= (aCTLOptions.IsCTLFontEnabled() ? css::i18n::TransliterationModulesExtra::IGNORE_KASHIDA_CTL:0 );
    lArgs[3].Value <<= nFlags;
    lArgs[4].Name = SEARCHITEM_COMMAND;
    lArgs[4].Value <<= (sal_Int16)(aFindAll ?
        SvxSearchCmd::FIND_ALL : SvxSearchCmd::FIND );
    lArgs[5].Name = SEARCHITEM_ALGORITHMTYPE;
    lArgs[5].Value <<= (sal_Int16)0;  // 0 == SearchAlgorithms_ABSOLUTE
    lArgs[6].Name = SEARCHITEM_SEARCHFORMATTED;
    lArgs[6].Value <<= bSearchFormatted;

    css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider(xFrame, css::uno::UNO_QUERY);
    if ( xDispatchProvider.is() )
    {
        css::uno::Reference< css::frame::XDispatch > xDispatch = xDispatchProvider->queryDispatch( aURL, OUString(), 0 );
        if ( xDispatch.is() && !aURL.Complete.isEmpty() )
            xDispatch->dispatch( aURL, lArgs );
    }
}

FindTextFieldControl::FindTextFieldControl( vcl::Window* pParent, WinBits nStyle,
    css::uno::Reference< css::frame::XFrame >& xFrame,
    const css::uno::Reference< css::uno::XComponentContext >& xContext) :
    ComboBox( pParent, nStyle ),
    m_xFrame(xFrame),
    m_xContext(xContext)
{
    SetPlaceholderText(SVX_RESSTR(RID_SVXSTR_FINDBAR_FIND));
    EnableAutocomplete(true, true);
}

void FindTextFieldControl::Remember_Impl(const OUString& rStr)
{
    const sal_Int32 nCount = GetEntryCount();

    for (sal_Int32 i=0; i<nCount; ++i)
    {
        if ( rStr == GetEntry(i))
            return;
    }

    if (nCount == REMEMBER_SIZE)
        RemoveEntryAt(REMEMBER_SIZE-1);

    InsertEntry(rStr, 0);
}

void FindTextFieldControl::SetTextToSelected_Impl()
{
    OUString aString;

    try
    {
        css::uno::Reference<css::frame::XController> xController(m_xFrame->getController(), css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::frame::XModel> xModel(xController->getModel(), css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::container::XIndexAccess> xIndexAccess(xModel->getCurrentSelection(), css::uno::UNO_QUERY_THROW);
        if (xIndexAccess->getCount() > 0)
        {
            css::uno::Reference<css::text::XTextRange> xTextRange(xIndexAccess->getByIndex(0), css::uno::UNO_QUERY_THROW);
            aString = xTextRange->getString();
        }
    }
    catch ( ... )
    {
    }

    if ( !aString.isEmpty() )
    {
        // If something is selected in the document, prepopulate with this
        SetText( aString );
        GetModifyHdl().Call(*this); // FIXME why SetText doesn't trigger this?
    }
    else if (GetEntryCount() > 0)
    {
        // Else, prepopulate with last search word (fdo#84256)
        SetText(GetEntry(0));
    }
}

bool FindTextFieldControl::PreNotify( NotifyEvent& rNEvt )
{
    bool bRet= ComboBox::PreNotify( rNEvt );

    switch ( rNEvt.GetType() )
    {
        case MouseNotifyEvent::KEYINPUT:
        {
            // Clear SearchLabel when altering the search string
            #if HAVE_FEATURE_DESKTOP
                SvxSearchDialogWrapper::SetSearchLabel(SL_Empty);
            #endif

            const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
            bool bShift = pKeyEvent->GetKeyCode().IsShift();
            bool bMod1 = pKeyEvent->GetKeyCode().IsMod1();
            sal_uInt16 nCode = pKeyEvent->GetKeyCode().GetCode();

            // Close the search bar on Escape
            if ( KEY_ESCAPE == nCode )
            {
                bRet = true;
                GrabFocusToDocument();

                // hide the findbar
                css::uno::Reference< css::beans::XPropertySet > xPropSet(m_xFrame, css::uno::UNO_QUERY);
                if (xPropSet.is())
                {
                    css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
                    css::uno::Any aValue = xPropSet->getPropertyValue("LayoutManager");
                    aValue >>= xLayoutManager;
                    if (xLayoutManager.is())
                    {
                        const OUString sResourceURL( "private:resource/toolbar/findbar" );
                        xLayoutManager->hideElement( sResourceURL );
                        xLayoutManager->destroyElement( sResourceURL );
                    }
                }
            }
            // Select text in the search box when Ctrl-F pressed
            if ( bMod1 && nCode == KEY_F )
                SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );

            // Execute the search when Return, Ctrl-G or F3 pressed
            if ( KEY_RETURN == nCode || (bMod1 && (KEY_G == nCode)) || (KEY_F3 == nCode) )
            {
                Remember_Impl(GetText());

                vcl::Window* pWindow = GetParent();
                ToolBox* pToolBox = static_cast<ToolBox*>(pWindow);

                impl_executeSearch( m_xContext, m_xFrame, pToolBox, bShift);
                bRet = true;
            }
            break;
        }

        case MouseNotifyEvent::GETFOCUS:
            SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );
            break;

        default:
            break;
    }

    return bRet;
}

SearchToolbarControllersManager::SearchToolbarControllersManager()
{
}

SearchToolbarControllersManager::~SearchToolbarControllersManager()
{
}

namespace
{
    class theSearchToolbarControllersManager
        : public rtl::Static<SearchToolbarControllersManager,
            theSearchToolbarControllersManager>
    {
    };
}

SearchToolbarControllersManager& SearchToolbarControllersManager::createControllersManager()
{
    return theSearchToolbarControllersManager::get();
}

void SearchToolbarControllersManager::saveSearchHistory(const FindTextFieldControl* pFindTextFieldControl)
{
    const sal_Int32 nECount( pFindTextFieldControl->GetEntryCount() );
    m_aSearchStrings.resize( nECount );
    for( sal_Int32 i=0; i<nECount; ++i )
    {
        m_aSearchStrings[i] = pFindTextFieldControl->GetEntry(i);
    }
}

void SearchToolbarControllersManager::loadSearchHistory(FindTextFieldControl* pFindTextFieldControl)
{
    for( size_t i=0; i<m_aSearchStrings.size(); ++i )
    {
        pFindTextFieldControl->InsertEntry(m_aSearchStrings[i],i);
    }
}

void SearchToolbarControllersManager::registryController( const css::uno::Reference< css::frame::XFrame >& xFrame, const css::uno::Reference< css::frame::XStatusListener >& xStatusListener, const OUString& sCommandURL )
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

void SearchToolbarControllersManager::freeController( const css::uno::Reference< css::frame::XFrame >& xFrame, const css::uno::Reference< css::frame::XStatusListener >& /*xStatusListener*/, const OUString& sCommandURL )
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

css::uno::Reference< css::frame::XStatusListener > SearchToolbarControllersManager::findController( const css::uno::Reference< css::frame::XFrame >& xFrame, const OUString& sCommandURL )
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

FindTextToolbarController::FindTextToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext )
    : svt::ToolboxController(rxContext, css::uno::Reference< css::frame::XFrame >(), OUString(COMMAND_FINDTEXT))
    , m_pFindTextFieldControl(nullptr)
    , m_nDownSearchId(0)
    , m_nUpSearchId(0)
{
}

FindTextToolbarController::~FindTextToolbarController()
{
}

// XInterface
css::uno::Any SAL_CALL FindTextToolbarController::queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException, std::exception )
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
OUString SAL_CALL FindTextToolbarController::getImplementationName() throw( css::uno::RuntimeException, std::exception )
{
    return OUString("com.sun.star.svx.FindTextToolboxController");
}

sal_Bool SAL_CALL FindTextToolbarController::supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL FindTextToolbarController::getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception )
{
    css::uno::Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}

// XComponent
void SAL_CALL FindTextToolbarController::dispose() throw ( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarMutexGuard;

    SearchToolbarControllersManager::createControllersManager().freeController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);

    svt::ToolboxController::dispose();
    if (m_pFindTextFieldControl != nullptr) {
        SearchToolbarControllersManager::createControllersManager()
            .saveSearchHistory(m_pFindTextFieldControl);
        m_pFindTextFieldControl.disposeAndClear();
    }
}

// XInitialization
void SAL_CALL FindTextToolbarController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException, std::exception)
{
    svt::ToolboxController::initialize(aArguments);

    vcl::Window* pWindow = VCLUnoHelper::GetWindow( getParent() );
    ToolBox* pToolBox = static_cast<ToolBox*>(pWindow);
    if ( pToolBox )
    {
        sal_uInt16 nItemCount = pToolBox->GetItemCount();
        for ( sal_uInt16 i=0; i<nItemCount; ++i )
        {
            OUString sItemCommand = pToolBox->GetItemCommand(i);
            if ( sItemCommand == COMMAND_DOWNSEARCH )
            {
                pToolBox->EnableItem(i, false);
                m_nDownSearchId = i;
            }
            else if ( sItemCommand == COMMAND_UPSEARCH )
            {
                pToolBox->EnableItem(i, false);
                m_nUpSearchId = i;
            }
        }
    }

    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);
}

css::uno::Reference< css::awt::XWindow > SAL_CALL FindTextToolbarController::createItemWindow( const css::uno::Reference< css::awt::XWindow >& xParent ) throw ( css::uno::RuntimeException, std::exception )
{
    css::uno::Reference< css::awt::XWindow > xItemWindow;

    vcl::Window* pParent = VCLUnoHelper::GetWindow( xParent );
    if ( pParent )
    {
        ToolBox* pToolbar = static_cast<ToolBox*>(pParent);
        m_pFindTextFieldControl = VclPtr<FindTextFieldControl>::Create( pToolbar, WinBits( WB_DROPDOWN | WB_VSCROLL), m_xFrame, m_xContext  );

        Size aSize(250, m_pFindTextFieldControl->GetTextHeight() + 200);
        m_pFindTextFieldControl->SetSizePixel( aSize );
        m_pFindTextFieldControl->SetModifyHdl(LINK(this, FindTextToolbarController, EditModifyHdl));
        SearchToolbarControllersManager::createControllersManager().loadSearchHistory(m_pFindTextFieldControl);
    }
    xItemWindow = VCLUnoHelper::GetInterface( m_pFindTextFieldControl );

    return xItemWindow;
}

// XStatusListener
void SAL_CALL FindTextToolbarController::statusChanged( const css::frame::FeatureStateEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarMutexGuard;
    if ( m_bDisposed )
        return;

    OUString aFeatureURL = rEvent.FeatureURL.Complete;
    if ( aFeatureURL == "AppendSearchHistory" )
    {
        m_pFindTextFieldControl->Remember_Impl(m_pFindTextFieldControl->GetText());
    }
}

IMPL_LINK_NOARG_TYPED(FindTextToolbarController, EditModifyHdl, Edit&, void)
{
    // enable or disable item DownSearch/UpSearch of findbar
    vcl::Window* pWindow = VCLUnoHelper::GetWindow( getParent() );
    ToolBox* pToolBox = static_cast<ToolBox*>(pWindow);
    if ( pToolBox && m_pFindTextFieldControl )
    {
        if (!m_pFindTextFieldControl->GetText().isEmpty())
        {
            if ( !pToolBox->IsItemEnabled(m_nDownSearchId) )
                pToolBox->EnableItem(m_nDownSearchId);
            if ( !pToolBox->IsItemEnabled(m_nUpSearchId) )
                pToolBox->EnableItem(m_nUpSearchId);
        }
        else
        {
            if ( pToolBox->IsItemEnabled(m_nDownSearchId) )
                pToolBox->EnableItem(m_nDownSearchId, false);
            if ( pToolBox->IsItemEnabled(m_nUpSearchId) )
                pToolBox->EnableItem(m_nUpSearchId, false);
        }
    }
}

UpDownSearchToolboxController::UpDownSearchToolboxController( const css::uno::Reference< css::uno::XComponentContext > & rxContext, Type eType )
    : svt::ToolboxController( rxContext,
            css::uno::Reference< css::frame::XFrame >(),
            (eType == UP) ? OUString( COMMAND_UPSEARCH ):  OUString( COMMAND_DOWNSEARCH ) ),
      meType( eType )
{
}

UpDownSearchToolboxController::~UpDownSearchToolboxController()
{
}

// XInterface
css::uno::Any SAL_CALL UpDownSearchToolboxController::queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException, std::exception )
{
    css::uno::Any a = ToolboxController::queryInterface( aType );
    if ( a.hasValue() )
        return a;

    return ::cppu::queryInterface( aType, static_cast< css::lang::XServiceInfo* >( this ) );
}

void SAL_CALL UpDownSearchToolboxController::acquire() throw ()
{
    ToolboxController::acquire();
}

void SAL_CALL UpDownSearchToolboxController::release() throw ()
{
    ToolboxController::release();
}

// XServiceInfo
OUString SAL_CALL UpDownSearchToolboxController::getImplementationName() throw( css::uno::RuntimeException, std::exception )
{
    return meType == UpDownSearchToolboxController::UP?
        OUString( "com.sun.star.svx.UpSearchToolboxController" ) :
        OUString( "com.sun.star.svx.DownSearchToolboxController" );
}

sal_Bool SAL_CALL UpDownSearchToolboxController::supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL UpDownSearchToolboxController::getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception )
{
    css::uno::Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}

// XComponent
void SAL_CALL UpDownSearchToolboxController::dispose() throw ( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarMutexGuard;

    SearchToolbarControllersManager::createControllersManager().freeController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);

    svt::ToolboxController::dispose();
}

// XInitialization
void SAL_CALL UpDownSearchToolboxController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException, std::exception )
{
    svt::ToolboxController::initialize( aArguments );
    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);
}

// XToolbarController
void SAL_CALL UpDownSearchToolboxController::execute( sal_Int16 /*KeyModifier*/ ) throw ( css::uno::RuntimeException, std::exception )
{
    if ( m_bDisposed )
        throw css::lang::DisposedException();

    vcl::Window* pWindow = VCLUnoHelper::GetWindow( getParent() );
    ToolBox* pToolBox = static_cast<ToolBox*>(pWindow);

    impl_executeSearch(m_xContext, m_xFrame, pToolBox, meType == UP );

    css::frame::FeatureStateEvent aEvent;
    aEvent.FeatureURL.Complete = COMMAND_APPENDSEARCHHISTORY;
    css::uno::Reference< css::frame::XStatusListener > xStatusListener = SearchToolbarControllersManager::createControllersManager().findController(m_xFrame, COMMAND_FINDTEXT);
    if (xStatusListener.is())
        xStatusListener->statusChanged( aEvent );
}

// XStatusListener
void SAL_CALL UpDownSearchToolboxController::statusChanged( const css::frame::FeatureStateEvent& /*rEvent*/ ) throw ( css::uno::RuntimeException, std::exception )
{
}

MatchCaseToolboxController::MatchCaseToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext )
    : svt::ToolboxController( rxContext,
        css::uno::Reference< css::frame::XFrame >(),
        OUString(COMMAND_MATCHCASE) )
    , m_pMatchCaseControl(nullptr)
{
}

MatchCaseToolboxController::~MatchCaseToolboxController()
{
}

// XInterface
css::uno::Any SAL_CALL MatchCaseToolboxController::queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException, std::exception )
{
    css::uno::Any a = ToolboxController::queryInterface( aType );
    if ( a.hasValue() )
        return a;

    return ::cppu::queryInterface( aType, static_cast< css::lang::XServiceInfo* >( this ) );
}

void SAL_CALL MatchCaseToolboxController::acquire() throw ()
{
    ToolboxController::acquire();
}

void SAL_CALL MatchCaseToolboxController::release() throw ()
{
    ToolboxController::release();
}

// XServiceInfo
OUString SAL_CALL MatchCaseToolboxController::getImplementationName() throw( css::uno::RuntimeException, std::exception )
{
    return OUString( "com.sun.star.svx.MatchCaseToolboxController" );
}

sal_Bool SAL_CALL MatchCaseToolboxController::supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL MatchCaseToolboxController::getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception )
{
    css::uno::Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}

// XComponent
void SAL_CALL MatchCaseToolboxController::dispose() throw ( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarMutexGuard;

    SearchToolbarControllersManager::createControllersManager().freeController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);

    svt::ToolboxController::dispose();

    m_pMatchCaseControl.disposeAndClear();
}

// XInitialization
void SAL_CALL MatchCaseToolboxController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException, std::exception)
{
    svt::ToolboxController::initialize(aArguments);

    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);
}

css::uno::Reference< css::awt::XWindow > SAL_CALL MatchCaseToolboxController::createItemWindow( const css::uno::Reference< css::awt::XWindow >& xParent ) throw ( css::uno::RuntimeException, std::exception )
{
    css::uno::Reference< css::awt::XWindow > xItemWindow;

    vcl::Window* pParent = VCLUnoHelper::GetWindow( xParent );
    if ( pParent )
    {
        ToolBox* pToolbar = static_cast<ToolBox*>(pParent);
        m_pMatchCaseControl = VclPtr<CheckBox>::Create( pToolbar, 0 );
        m_pMatchCaseControl->SetText( SVX_RESSTR( RID_SVXSTR_FINDBAR_MATCHCASE ) );
        Size aSize( m_pMatchCaseControl->GetOptimalSize() );
        m_pMatchCaseControl->SetSizePixel( aSize );
    }
    xItemWindow = VCLUnoHelper::GetInterface( m_pMatchCaseControl );

    return xItemWindow;
}

// XStatusListener
void SAL_CALL MatchCaseToolboxController::statusChanged( const css::frame::FeatureStateEvent& ) throw ( css::uno::RuntimeException, std::exception )
{
}

SearchFormattedToolboxController::SearchFormattedToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext )
    : svt::ToolboxController( rxContext,
        css::uno::Reference< css::frame::XFrame >(),
        OUString(COMMAND_SEARCHFORMATTED) )
    , m_pSearchFormattedControl(nullptr)
{
}

SearchFormattedToolboxController::~SearchFormattedToolboxController()
{
}

// XInterface
css::uno::Any SAL_CALL SearchFormattedToolboxController::queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException, std::exception )
{
    css::uno::Any a = ToolboxController::queryInterface( aType );
    if ( a.hasValue() )
        return a;

    return ::cppu::queryInterface( aType, static_cast< css::lang::XServiceInfo* >( this ) );
}

void SAL_CALL SearchFormattedToolboxController::acquire() throw ()
{
    ToolboxController::acquire();
}

void SAL_CALL SearchFormattedToolboxController::release() throw ()
{
    ToolboxController::release();
}

// XServiceInfo
OUString SAL_CALL SearchFormattedToolboxController::getImplementationName() throw( css::uno::RuntimeException, std::exception )
{
    return OUString( "com.sun.star.svx.SearchFormattedToolboxController" );
}

sal_Bool SAL_CALL SearchFormattedToolboxController::supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL SearchFormattedToolboxController::getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception )
{
    css::uno::Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}

// XComponent
void SAL_CALL SearchFormattedToolboxController::dispose() throw ( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarMutexGuard;

    SearchToolbarControllersManager::createControllersManager().freeController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);

    svt::ToolboxController::dispose();

    m_pSearchFormattedControl.disposeAndClear();
}

// XInitialization
void SAL_CALL SearchFormattedToolboxController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException, std::exception)
{
    svt::ToolboxController::initialize(aArguments);

    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);
}

css::uno::Reference< css::awt::XWindow > SAL_CALL SearchFormattedToolboxController::createItemWindow( const css::uno::Reference< css::awt::XWindow >& xParent ) throw ( css::uno::RuntimeException, std::exception )
{
    css::uno::Reference< css::awt::XWindow > xItemWindow;

    vcl::Window* pParent = VCLUnoHelper::GetWindow( xParent );
    if ( pParent )
    {
        ToolBox* pToolbar = static_cast<ToolBox*>(pParent);
        m_pSearchFormattedControl = VclPtr<CheckBox>::Create( pToolbar, 0 );
        m_pSearchFormattedControl->SetText( SVX_RESSTR( RID_SVXSTR_FINDBAR_SEARCHFORMATTED ) );
        Size aSize( m_pSearchFormattedControl->GetOptimalSize() );
        m_pSearchFormattedControl->SetSizePixel( aSize );
    }
    xItemWindow = VCLUnoHelper::GetInterface( m_pSearchFormattedControl );

    return xItemWindow;
}

// XStatusListener
void SAL_CALL SearchFormattedToolboxController::statusChanged( const css::frame::FeatureStateEvent& ) throw ( css::uno::RuntimeException, std::exception )
{
}

FindAllToolboxController::FindAllToolboxController( const css::uno::Reference< css::uno::XComponentContext > & rxContext )
    : svt::ToolboxController( rxContext,
            css::uno::Reference< css::frame::XFrame >(),
            ".uno:FindAll" )
{
}

FindAllToolboxController::~FindAllToolboxController()
{
}

// XInterface
css::uno::Any SAL_CALL FindAllToolboxController::queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException, std::exception )
{
    css::uno::Any a = ToolboxController::queryInterface( aType );
    if ( a.hasValue() )
        return a;

    return ::cppu::queryInterface( aType, static_cast< css::lang::XServiceInfo* >( this ) );
}

void SAL_CALL FindAllToolboxController::acquire() throw ()
{
    ToolboxController::acquire();
}

void SAL_CALL FindAllToolboxController::release() throw ()
{
    ToolboxController::release();
}

// XServiceInfo
OUString SAL_CALL FindAllToolboxController::getImplementationName() throw( css::uno::RuntimeException, std::exception )
{
    return OUString( "com.sun.star.svx.FindAllToolboxController" );
}


sal_Bool SAL_CALL FindAllToolboxController::supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL FindAllToolboxController::getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception )
{
    css::uno::Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}

// XComponent
void SAL_CALL FindAllToolboxController::dispose() throw ( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarMutexGuard;

    SearchToolbarControllersManager::createControllersManager().freeController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);

    svt::ToolboxController::dispose();
}

// XInitialization
void SAL_CALL FindAllToolboxController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException, std::exception )
{
    svt::ToolboxController::initialize( aArguments );
    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);
}

// XToolbarController
void SAL_CALL FindAllToolboxController::execute( sal_Int16 /*KeyModifier*/ ) throw ( css::uno::RuntimeException, std::exception )
{
    if ( m_bDisposed )
        throw css::lang::DisposedException();

    vcl::Window* pWindow = VCLUnoHelper::GetWindow( getParent() );
    ToolBox* pToolBox = static_cast<ToolBox*>(pWindow);

    impl_executeSearch(m_xContext, m_xFrame, pToolBox, false, true);
}

// XStatusListener
void SAL_CALL FindAllToolboxController::statusChanged( const css::frame::FeatureStateEvent& /*rEvent*/ ) throw ( css::uno::RuntimeException, std::exception )
{
}

ExitSearchToolboxController::ExitSearchToolboxController( const css::uno::Reference< css::uno::XComponentContext > & rxContext )
    : svt::ToolboxController( rxContext,
            css::uno::Reference< css::frame::XFrame >(),
            OUString( COMMAND_EXITSEARCH ) )
{
}

ExitSearchToolboxController::~ExitSearchToolboxController()
{
}

// XInterface
css::uno::Any SAL_CALL ExitSearchToolboxController::queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException, std::exception )
{
    css::uno::Any a = ToolboxController::queryInterface( aType );
    if ( a.hasValue() )
        return a;

    return ::cppu::queryInterface( aType, static_cast< css::lang::XServiceInfo* >( this ) );
}

void SAL_CALL ExitSearchToolboxController::acquire() throw ()
{
    ToolboxController::acquire();
}

void SAL_CALL ExitSearchToolboxController::release() throw ()
{
    ToolboxController::release();
}

// XServiceInfo
OUString SAL_CALL ExitSearchToolboxController::getImplementationName() throw( css::uno::RuntimeException, std::exception )
{
    return OUString( "com.sun.star.svx.ExitFindbarToolboxController" );
}


sal_Bool SAL_CALL ExitSearchToolboxController::supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ExitSearchToolboxController::getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception )
{
    css::uno::Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}

// XComponent
void SAL_CALL ExitSearchToolboxController::dispose() throw ( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarMutexGuard;

    SearchToolbarControllersManager::createControllersManager().freeController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);

    svt::ToolboxController::dispose();
}

// XInitialization
void SAL_CALL ExitSearchToolboxController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException, std::exception )
{
    svt::ToolboxController::initialize( aArguments );
    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);
}

// XToolbarController
void SAL_CALL ExitSearchToolboxController::execute( sal_Int16 /*KeyModifier*/ ) throw ( css::uno::RuntimeException, std::exception )
{
    vcl::Window *pFocusWindow = Application::GetFocusWindow();
    if ( pFocusWindow )
        pFocusWindow->GrabFocusToDocument();

    // hide the findbar
    css::uno::Reference< css::beans::XPropertySet > xPropSet(m_xFrame, css::uno::UNO_QUERY);
    if (xPropSet.is())
    {
        css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
        css::uno::Any aValue = xPropSet->getPropertyValue("LayoutManager");
        aValue >>= xLayoutManager;
        if (xLayoutManager.is())
        {
            const OUString sResourceURL( "private:resource/toolbar/findbar" );
            xLayoutManager->hideElement( sResourceURL );
            xLayoutManager->destroyElement( sResourceURL );
        }
    }
}

// XStatusListener
void SAL_CALL ExitSearchToolboxController::statusChanged( const css::frame::FeatureStateEvent& /*rEvent*/ ) throw ( css::uno::RuntimeException, std::exception )
{
}

SearchLabelToolboxController::SearchLabelToolboxController( const css::uno::Reference< css::uno::XComponentContext > & rxContext )
    : svt::ToolboxController( rxContext,
            css::uno::Reference< css::frame::XFrame >(),
            OUString( ".uno:SearchLabel" ) )
{
}

SearchLabelToolboxController::~SearchLabelToolboxController()
{
}

// XInterface
css::uno::Any SAL_CALL SearchLabelToolboxController::queryInterface( const css::uno::Type& aType ) throw ( css::uno::RuntimeException, std::exception )
{
    css::uno::Any a = ToolboxController::queryInterface( aType );
    if ( a.hasValue() )
        return a;

    return ::cppu::queryInterface( aType, static_cast< css::lang::XServiceInfo* >( this ) );
}

void SAL_CALL SearchLabelToolboxController::acquire() throw ()
{
    ToolboxController::acquire();
}

void SAL_CALL SearchLabelToolboxController::release() throw ()
{
    ToolboxController::release();
}

// XServiceInfo
OUString SAL_CALL SearchLabelToolboxController::getImplementationName() throw( css::uno::RuntimeException, std::exception )
{
    return OUString( "com.sun.star.svx.SearchLabelToolboxController" );
}


sal_Bool SAL_CALL SearchLabelToolboxController::supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL SearchLabelToolboxController::getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception )
{
    css::uno::Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}

// XComponent
void SAL_CALL SearchLabelToolboxController::dispose() throw ( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarMutexGuard;

    SearchToolbarControllersManager::createControllersManager().freeController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);

    svt::ToolboxController::dispose();
}

// XInitialization
void SAL_CALL SearchLabelToolboxController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException, std::exception )
{
    svt::ToolboxController::initialize( aArguments );
    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);
}

// XStatusListener
void SAL_CALL SearchLabelToolboxController::statusChanged( const css::frame::FeatureStateEvent& ) throw ( css::uno::RuntimeException, std::exception )
{
}

css::uno::Reference< css::awt::XWindow > SAL_CALL SearchLabelToolboxController::createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) throw ( css::uno::RuntimeException, std::exception )
{
    VclPtr<vcl::Window> pSL = VclPtr<FixedText>::Create(VCLUnoHelper::GetWindow( Parent ));
    pSL->SetSizePixel(Size(250, 25));
    return VCLUnoHelper::GetInterface(pSL);
}

FindbarDispatcher::FindbarDispatcher()
{
}

FindbarDispatcher::~FindbarDispatcher()
{
    m_xFrame = nullptr;
}

// XInterface
css::uno::Any SAL_CALL FindbarDispatcher::queryInterface( const css::uno::Type& aType ) throw( css::uno::RuntimeException, std::exception )
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
OUString SAL_CALL FindbarDispatcher::getImplementationName() throw( css::uno::RuntimeException, std::exception )
{
    return OUString("com.sun.star.comp.svx.Impl.FindbarDispatcher");
}

sal_Bool SAL_CALL FindbarDispatcher::supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL FindbarDispatcher::getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception )
{
    css::uno::Sequence< OUString > aSNS( 2 );
    aSNS[0] = "com.sun.star.comp.svx.FindbarDispatcher";
    aSNS[1] = "com.sun.star.frame.ProtocolHandler";
    return aSNS;
}

// XInitialization
void SAL_CALL FindbarDispatcher::initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException, std::exception )
{
    if ( aArguments.getLength() )
        aArguments[0] >>= m_xFrame;
}

// XDispatchProvider
css::uno::Reference< css::frame::XDispatch > SAL_CALL FindbarDispatcher::queryDispatch( const css::util::URL& aURL, const OUString& /*sTargetFrameName*/, sal_Int32 /*nSearchFlags*/ ) throw( css::uno::RuntimeException, std::exception )
{
    css::uno::Reference< css::frame::XDispatch > xDispatch;

    if ( aURL.Protocol == "vnd.sun.star.findbar:" )
        xDispatch = this;

    return xDispatch;
}

css::uno::Sequence < css::uno::Reference< css::frame::XDispatch > > SAL_CALL FindbarDispatcher::queryDispatches( const css::uno::Sequence < css::frame::DispatchDescriptor >& seqDescripts ) throw( css::uno::RuntimeException, std::exception )
{
    sal_Int32 nCount = seqDescripts.getLength();
    css::uno::Sequence < css::uno::Reference < XDispatch > > lDispatcher( nCount );

    for( sal_Int32 i=0; i<nCount; ++i )
        lDispatcher[i] = queryDispatch( seqDescripts[i].FeatureURL, seqDescripts[i].FrameName, seqDescripts[i].SearchFlags );

    return lDispatcher;
}

// XDispatch
void SAL_CALL FindbarDispatcher::dispatch( const css::util::URL& aURL, const css::uno::Sequence < css::beans::PropertyValue >& /*lArgs*/ ) throw( css::uno::RuntimeException, std::exception )
{
    //vnd.sun.star.findbar:FocusToFindbar  - set cursor to the FindTextFieldControl of the findbar
    if ( aURL.Path == "FocusToFindbar" )
    {
        css::uno::Reference< css::beans::XPropertySet > xPropSet(m_xFrame, css::uno::UNO_QUERY);
        if(!xPropSet.is())
            return;

        css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
        css::uno::Any aValue = xPropSet->getPropertyValue("LayoutManager");
        aValue >>= xLayoutManager;
        if (!xLayoutManager.is())
            return;

        const OUString sResourceURL( "private:resource/toolbar/findbar" );
        css::uno::Reference< css::ui::XUIElement > xUIElement = xLayoutManager->getElement(sResourceURL);
        if (!xUIElement.is())
        {
            // show the findbar if necessary
            xLayoutManager->createElement( sResourceURL );
            xLayoutManager->showElement( sResourceURL );
            xUIElement = xLayoutManager->getElement( sResourceURL );
            if ( !xUIElement.is() )
                return;
        }

        css::uno::Reference< css::awt::XWindow > xWindow(xUIElement->getRealInterface(), css::uno::UNO_QUERY);
        vcl::Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        ToolBox* pToolBox = static_cast<ToolBox*>(pWindow);
        if ( pToolBox )
        {
            sal_uInt16 nItemCount = pToolBox->GetItemCount();
            for ( sal_uInt16 i=0; i<nItemCount; ++i )
            {
                OUString sItemCommand = pToolBox->GetItemCommand(i);
                if ( sItemCommand == COMMAND_FINDTEXT )
                {
                    vcl::Window* pItemWin = pToolBox->GetItemWindow( i );
                    if ( pItemWin )
                    {
                        FindTextFieldControl* pFindTextFieldControl = dynamic_cast<FindTextFieldControl*>(pItemWin);
                        if ( pFindTextFieldControl )
                            pFindTextFieldControl->SetTextToSelected_Impl();
                        SolarMutexGuard aSolarMutexGuard;
                        pItemWin->GrabFocus();
                        return;
                    }
                }
            }
        }
    }
}

void SAL_CALL FindbarDispatcher::addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*xControl*/, const css::util::URL& /*aURL*/ ) throw ( css::uno::RuntimeException, std::exception )
{
}

void SAL_CALL FindbarDispatcher::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*xControl*/, const css::util::URL& /*aURL*/ ) throw ( css::uno::RuntimeException, std::exception )
{
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_svx_FindTextToolboxController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new FindTextToolbarController(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_svx_ExitFindbarToolboxController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ExitSearchToolboxController(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_svx_UpSearchToolboxController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UpDownSearchToolboxController(context, UpDownSearchToolboxController::UP));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_svx_DownSearchToolboxController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UpDownSearchToolboxController(context, UpDownSearchToolboxController::DOWN));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_svx_MatchCaseToolboxController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new MatchCaseToolboxController(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_svx_SearchFormattedToolboxController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SearchFormattedToolboxController(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_svx_FindAllToolboxController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new FindAllToolboxController(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_svx_SearchLabelToolboxController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SearchLabelToolboxController(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_svx_Impl_FindbarDispatcher_get_implementation(
    SAL_UNUSED_PARAMETER css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new FindbarDispatcher);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
