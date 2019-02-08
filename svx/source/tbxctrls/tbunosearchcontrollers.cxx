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

#include <map>
#include <vector>

#include <config_features.h>

#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>

#include <comphelper/propertysequence.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/SearchAlgorithms.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>

#include <svl/ctloptions.hxx>
#include <svl/srchitem.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/svapp.hxx>
#include <rtl/ref.hxx>
#include <rtl/instance.hxx>
#include <svx/srchdlg.hxx>
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/window.hxx>

using namespace css;

namespace {

static const char COMMAND_FINDTEXT[] = ".uno:FindText";
static const char COMMAND_DOWNSEARCH[] = ".uno:DownSearch";
static const char COMMAND_UPSEARCH[] = ".uno:UpSearch";
static const char COMMAND_FINDALL[] = ".uno:FindAll";
static const char COMMAND_MATCHCASE[] = ".uno:MatchCase";
static const char COMMAND_SEARCHFORMATTED[] = ".uno:SearchFormattedDisplayString";

static const sal_Int32       REMEMBER_SIZE = 10;

void impl_executeSearch( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                         const css::uno::Reference< css::frame::XFrame >& xFrame,
                         const ToolBox* pToolBox,
                         const bool aSearchBackwards,
                         const bool aFindAll = false )
{
    css::uno::Reference< css::util::XURLTransformer > xURLTransformer( css::util::URLTransformer::create( rxContext ) );
    css::util::URL aURL;
    aURL.Complete = ".uno:ExecuteSearch";
    xURLTransformer->parseStrict(aURL);

    OUString sFindText;
    bool aMatchCase = false;
    bool bSearchFormatted = false;
    if ( pToolBox )
    {
        ToolBox::ImplToolItems::size_type nItemCount = pToolBox->GetItemCount();
        for ( ToolBox::ImplToolItems::size_type i=0; i<nItemCount; ++i )
        {
            sal_uInt16 id = pToolBox->GetItemId(i);
            OUString sItemCommand = pToolBox->GetItemCommand(id);
            if ( sItemCommand == COMMAND_FINDTEXT )
            {
                vcl::Window* pItemWin = pToolBox->GetItemWindow(id);
                if (pItemWin)
                    sFindText = pItemWin->GetText();
            } else if ( sItemCommand == COMMAND_MATCHCASE )
            {
                CheckBox* pItemWin = static_cast<CheckBox*>( pToolBox->GetItemWindow(id) );
                if (pItemWin)
                    aMatchCase = pItemWin->IsChecked();
            } else if ( sItemCommand == COMMAND_SEARCHFORMATTED )
            {
                CheckBox* pItemWin = static_cast<CheckBox*>( pToolBox->GetItemWindow(id) );
                if (pItemWin)
                    bSearchFormatted = pItemWin->IsChecked();
            }
        }
    }

    SvtCTLOptions aCTLOptions;
    TransliterationFlags nFlags = TransliterationFlags::NONE;
    if (!aMatchCase)
        nFlags |= TransliterationFlags::IGNORE_CASE;
    if (aCTLOptions.IsCTLFontEnabled())
        nFlags |= TransliterationFlags::IGNORE_DIACRITICS_CTL;
    if (aCTLOptions.IsCTLFontEnabled())
        nFlags |= TransliterationFlags::IGNORE_KASHIDA_CTL;

    auto aArgs( comphelper::InitPropertySequence( {
        { "SearchItem.SearchString", css::uno::makeAny( sFindText ) },
        { "SearchItem.Backward", css::uno::makeAny( aSearchBackwards ) },
        { "SearchItem.SearchFlags", css::uno::makeAny( sal_Int32(0) ) },
        { "SearchItem.TransliterateFlags", css::uno::makeAny( static_cast<sal_Int32>(nFlags) ) },
        { "SearchItem.Command", css::uno::makeAny( static_cast<sal_Int16>(aFindAll ?SvxSearchCmd::FIND_ALL : SvxSearchCmd::FIND ) ) },
        { "SearchItem.AlgorithmType", css::uno::makeAny( sal_Int16(css::util::SearchAlgorithms_ABSOLUTE) ) },
        { "SearchItem.AlgorithmType2", css::uno::makeAny( sal_Int16(css::util::SearchAlgorithms2::ABSOLUTE) ) },
        { "SearchItem.SearchFormatted", css::uno::makeAny( bSearchFormatted ) }
    } ) );

    css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider(xFrame, css::uno::UNO_QUERY);
    if ( xDispatchProvider.is() )
    {
        css::uno::Reference< css::frame::XDispatch > xDispatch = xDispatchProvider->queryDispatch( aURL, OUString(), 0 );
        if ( xDispatch.is() && !aURL.Complete.isEmpty() )
            xDispatch->dispatch( aURL, aArgs );
    }
}

class FindTextFieldControl : public ComboBox
{
public:
    FindTextFieldControl( vcl::Window* pParent, WinBits nStyle,
        css::uno::Reference< css::frame::XFrame > const & xFrame,
        const css::uno::Reference< css::uno::XComponentContext >& xContext );

    virtual bool PreNotify( NotifyEvent& rNEvt ) override;

    void Remember_Impl(const OUString& rStr);
    void SetTextToSelected_Impl();

private:

    css::uno::Reference< css::frame::XFrame > m_xFrame;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    std::unique_ptr<svt::AcceleratorExecute> m_pAcc;
};

FindTextFieldControl::FindTextFieldControl( vcl::Window* pParent, WinBits nStyle,
    css::uno::Reference< css::frame::XFrame > const & xFrame,
    const css::uno::Reference< css::uno::XComponentContext >& xContext) :
    ComboBox( pParent, nStyle ),
    m_xFrame(xFrame),
    m_xContext(xContext),
    m_pAcc(svt::AcceleratorExecute::createAcceleratorHelper())
{
    SetPlaceholderText(SvxResId(RID_SVXSTR_FINDBAR_FIND));
    EnableAutocomplete(true, true);
    m_pAcc->init(m_xContext, m_xFrame);
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
    if (isDisposed())
        return true;
    bool bRet= ComboBox::PreNotify( rNEvt );

    switch ( rNEvt.GetType() )
    {
        case MouseNotifyEvent::KEYINPUT:
        {
            // Clear SearchLabel when altering the search string
            #if HAVE_FEATURE_DESKTOP
            SvxSearchDialogWrapper::SetSearchLabel(SearchLabel::Empty);
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
            else if ( bMod1 && nCode == KEY_F )
                SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );

            // Execute the search when Return, Ctrl-G or F3 pressed
            else if ( KEY_RETURN == nCode || (bMod1 && (KEY_G == nCode)) || (KEY_F3 == nCode) )
            {
                Remember_Impl(GetText());

                vcl::Window* pWindow = GetParent();
                ToolBox* pToolBox = static_cast<ToolBox*>(pWindow);

                impl_executeSearch( m_xContext, m_xFrame, pToolBox, bShift);
                bRet = true;
            }
            else
            {
                auto awtKey = svt::AcceleratorExecute::st_VCLKey2AWTKey(pKeyEvent->GetKeyCode());
                const OUString aCommand(m_pAcc->findCommand(awtKey));
                if (aCommand == ".uno:SearchDialog")
                    bRet = m_pAcc->execute(awtKey);
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

class SearchToolbarControllersManager
{
public:

    SearchToolbarControllersManager();

    static SearchToolbarControllersManager& createControllersManager();

    void registryController( const css::uno::Reference< css::frame::XFrame >& xFrame, const css::uno::Reference< css::frame::XStatusListener >& xStatusListener, const OUString& sCommandURL );
    void freeController ( const css::uno::Reference< css::frame::XFrame >& xFrame, const OUString& sCommandURL );
    css::uno::Reference< css::frame::XStatusListener > findController( const css::uno::Reference< css::frame::XFrame >& xFrame, const OUString& sCommandURL );

    void saveSearchHistory(const FindTextFieldControl* m_pFindTextFieldControl);
    void loadSearchHistory(FindTextFieldControl* m_pFindTextFieldControl);

private:

    typedef ::std::vector< css::beans::PropertyValue > SearchToolbarControllersVec;
    typedef ::std::map< css::uno::Reference< css::frame::XFrame >, SearchToolbarControllersVec > SearchToolbarControllersMap;
    SearchToolbarControllersMap aSearchToolbarControllersMap;
    std::vector<OUString> m_aSearchStrings;

};

SearchToolbarControllersManager::SearchToolbarControllersManager()
{
}

class theSearchToolbarControllersManager
    : public rtl::Static<SearchToolbarControllersManager,
        theSearchToolbarControllersManager>
{
};

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
        aSearchToolbarControllersMap.emplace(xFrame, lControllers);
    }
    else
    {
        sal_Int32 nSize = pIt->second.size();
        for (sal_Int32 i=0; i<nSize; ++i)
        {
            if (pIt->second[i].Name == sCommandURL)
                return;
        }

        pIt->second.resize(nSize+1);
        pIt->second[nSize].Name = sCommandURL;
        pIt->second[nSize].Value <<= xStatusListener;
    }
}

void SearchToolbarControllersManager::freeController( const css::uno::Reference< css::frame::XFrame >& xFrame, const OUString& sCommandURL )
{
    SearchToolbarControllersMap::iterator pIt = aSearchToolbarControllersMap.find(xFrame);
    if (pIt != aSearchToolbarControllersMap.end())
    {
        auto pItCtrl = std::find_if(pIt->second.begin(), pIt->second.end(),
            [&sCommandURL](const css::beans::PropertyValue& rCtrl) { return rCtrl.Name == sCommandURL; });
        if (pItCtrl != pIt->second.end())
            pIt->second.erase(pItCtrl);

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
        auto pItCtrl = std::find_if(pIt->second.begin(), pIt->second.end(),
            [&sCommandURL](const css::beans::PropertyValue& rCtrl) { return rCtrl.Name == sCommandURL; });
        if (pItCtrl != pIt->second.end())
            pItCtrl->Value >>= xStatusListener;
    }

    return xStatusListener;
}

class FindTextToolbarController : public svt::ToolboxController,
                                  public css::lang::XServiceInfo
{
public:

    FindTextToolbarController( const css::uno::Reference< css::uno::XComponentContext > & rxContext );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XToolbarController
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

    DECL_LINK(EditModifyHdl, Edit&, void);

private:

    void textfieldChanged();

    VclPtr<FindTextFieldControl> m_pFindTextFieldControl;

    sal_uInt16 m_nDownSearchId;
    sal_uInt16 m_nUpSearchId;
    sal_uInt16 m_nFindAllId;

};

FindTextToolbarController::FindTextToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext )
    : svt::ToolboxController(rxContext, css::uno::Reference< css::frame::XFrame >(), COMMAND_FINDTEXT)
    , m_pFindTextFieldControl(nullptr)
    , m_nDownSearchId(0)
    , m_nUpSearchId(0)
    , m_nFindAllId(0)
{
}

// XInterface
css::uno::Any SAL_CALL FindTextToolbarController::queryInterface( const css::uno::Type& aType )
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
OUString SAL_CALL FindTextToolbarController::getImplementationName()
{
    return OUString("com.sun.star.svx.FindTextToolboxController");
}

sal_Bool SAL_CALL FindTextToolbarController::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL FindTextToolbarController::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

// XComponent
void SAL_CALL FindTextToolbarController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    SearchToolbarControllersManager::createControllersManager().freeController(m_xFrame, m_aCommandURL);

    svt::ToolboxController::dispose();
    if (m_pFindTextFieldControl != nullptr) {
        SearchToolbarControllersManager::createControllersManager()
            .saveSearchHistory(m_pFindTextFieldControl);
        m_pFindTextFieldControl.disposeAndClear();
    }
}

// XInitialization
void SAL_CALL FindTextToolbarController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::ToolboxController::initialize(aArguments);

    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( getParent() );
    ToolBox* pToolBox = static_cast<ToolBox*>(pWindow.get());
    if ( pToolBox )
    {
        m_nDownSearchId = pToolBox->GetItemId(COMMAND_DOWNSEARCH);
        m_nUpSearchId = pToolBox->GetItemId(COMMAND_UPSEARCH);
        m_nFindAllId = pToolBox->GetItemId(COMMAND_FINDALL);
    }

    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);
}

css::uno::Reference< css::awt::XWindow > SAL_CALL FindTextToolbarController::createItemWindow( const css::uno::Reference< css::awt::XWindow >& xParent )
{
    css::uno::Reference< css::awt::XWindow > xItemWindow;

    VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow( xParent );
    if ( pParent )
    {
        ToolBox* pToolbar = static_cast<ToolBox*>(pParent.get());
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
void SAL_CALL FindTextToolbarController::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    SolarMutexGuard aSolarMutexGuard;
    if ( m_bDisposed )
        return;

    OUString aFeatureURL = rEvent.FeatureURL.Complete;
    if ( aFeatureURL == "AppendSearchHistory" )
    {
        m_pFindTextFieldControl->Remember_Impl(m_pFindTextFieldControl->GetText());
    }
    // enable up/down buttons in case there is already text (from the search history)
    textfieldChanged();
}

IMPL_LINK_NOARG(FindTextToolbarController, EditModifyHdl, Edit&, void)
{
    textfieldChanged();
}

void FindTextToolbarController::textfieldChanged() {
    // enable or disable item DownSearch/UpSearch/FindAll of findbar
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( getParent() );
    ToolBox* pToolBox = static_cast<ToolBox*>(pWindow.get());
    if ( pToolBox && m_pFindTextFieldControl )
    {
        bool enableButtons = !m_pFindTextFieldControl->GetText().isEmpty();
        pToolBox->EnableItem(m_nDownSearchId, enableButtons);
        pToolBox->EnableItem(m_nUpSearchId, enableButtons);
        pToolBox->EnableItem(m_nFindAllId, enableButtons);
    }
}

class UpDownSearchToolboxController : public svt::ToolboxController,
                                      public css::lang::XServiceInfo
{
public:
    enum Type { UP, DOWN };

    UpDownSearchToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext, Type eType );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XToolbarController
    virtual void SAL_CALL execute( sal_Int16 KeyModifier ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

private:
    Type const meType;
};

UpDownSearchToolboxController::UpDownSearchToolboxController( const css::uno::Reference< css::uno::XComponentContext > & rxContext, Type eType )
    : svt::ToolboxController( rxContext,
            css::uno::Reference< css::frame::XFrame >(),
            (eType == UP) ? OUString( COMMAND_UPSEARCH ):  OUString( COMMAND_DOWNSEARCH ) ),
      meType( eType )
{
}

// XInterface
css::uno::Any SAL_CALL UpDownSearchToolboxController::queryInterface( const css::uno::Type& aType )
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
OUString SAL_CALL UpDownSearchToolboxController::getImplementationName()
{
    return meType == UpDownSearchToolboxController::UP?
        OUString( "com.sun.star.svx.UpSearchToolboxController" ) :
        OUString( "com.sun.star.svx.DownSearchToolboxController" );
}

sal_Bool SAL_CALL UpDownSearchToolboxController::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL UpDownSearchToolboxController::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

// XComponent
void SAL_CALL UpDownSearchToolboxController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    SearchToolbarControllersManager::createControllersManager().freeController(m_xFrame, m_aCommandURL);

    svt::ToolboxController::dispose();
}

// XInitialization
void SAL_CALL UpDownSearchToolboxController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::ToolboxController::initialize( aArguments );
    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);
}

// XToolbarController
void SAL_CALL UpDownSearchToolboxController::execute( sal_Int16 /*KeyModifier*/ )
{
    if ( m_bDisposed )
        throw css::lang::DisposedException();

    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( getParent() );
    ToolBox* pToolBox = static_cast<ToolBox*>(pWindow.get());

    impl_executeSearch(m_xContext, m_xFrame, pToolBox, meType == UP );

    css::frame::FeatureStateEvent aEvent;
    aEvent.FeatureURL.Complete = "AppendSearchHistory";
    css::uno::Reference< css::frame::XStatusListener > xStatusListener = SearchToolbarControllersManager::createControllersManager().findController(m_xFrame, COMMAND_FINDTEXT);
    if (xStatusListener.is())
        xStatusListener->statusChanged( aEvent );
}

// XStatusListener
void SAL_CALL UpDownSearchToolboxController::statusChanged( const css::frame::FeatureStateEvent& /*rEvent*/ )
{
}

class MatchCaseToolboxController : public svt::ToolboxController,
                                      public css::lang::XServiceInfo
{
public:
    MatchCaseToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XToolbarController
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

private:
    VclPtr<CheckBox> m_pMatchCaseControl;
};

MatchCaseToolboxController::MatchCaseToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext )
    : svt::ToolboxController( rxContext,
        css::uno::Reference< css::frame::XFrame >(),
        COMMAND_MATCHCASE )
    , m_pMatchCaseControl(nullptr)
{
}

// XInterface
css::uno::Any SAL_CALL MatchCaseToolboxController::queryInterface( const css::uno::Type& aType )
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
OUString SAL_CALL MatchCaseToolboxController::getImplementationName()
{
    return OUString( "com.sun.star.svx.MatchCaseToolboxController" );
}

sal_Bool SAL_CALL MatchCaseToolboxController::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL MatchCaseToolboxController::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

// XComponent
void SAL_CALL MatchCaseToolboxController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    SearchToolbarControllersManager::createControllersManager().freeController(m_xFrame, m_aCommandURL);

    svt::ToolboxController::dispose();

    m_pMatchCaseControl.disposeAndClear();
}

// XInitialization
void SAL_CALL MatchCaseToolboxController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::ToolboxController::initialize(aArguments);

    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);
}

css::uno::Reference< css::awt::XWindow > SAL_CALL MatchCaseToolboxController::createItemWindow( const css::uno::Reference< css::awt::XWindow >& xParent )
{
    css::uno::Reference< css::awt::XWindow > xItemWindow;

    VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow( xParent );
    if ( pParent )
    {
        ToolBox* pToolbar = static_cast<ToolBox*>(pParent.get());
        m_pMatchCaseControl = VclPtr<CheckBox>::Create( pToolbar, 0 );
        m_pMatchCaseControl->SetText( SvxResId( RID_SVXSTR_FINDBAR_MATCHCASE ) );
        Size aSize( m_pMatchCaseControl->GetOptimalSize() );
        m_pMatchCaseControl->SetSizePixel( aSize );
    }
    xItemWindow = VCLUnoHelper::GetInterface( m_pMatchCaseControl );

    return xItemWindow;
}

// XStatusListener
void SAL_CALL MatchCaseToolboxController::statusChanged( const css::frame::FeatureStateEvent& )
{
}

class SearchFormattedToolboxController : public svt::ToolboxController,
                                      public css::lang::XServiceInfo
{
public:
    SearchFormattedToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XToolbarController
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

private:
    VclPtr<CheckBox> m_pSearchFormattedControl;
};

SearchFormattedToolboxController::SearchFormattedToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext )
    : svt::ToolboxController( rxContext,
        css::uno::Reference< css::frame::XFrame >(),
        COMMAND_SEARCHFORMATTED )
    , m_pSearchFormattedControl(nullptr)
{
}

// XInterface
css::uno::Any SAL_CALL SearchFormattedToolboxController::queryInterface( const css::uno::Type& aType )
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
OUString SAL_CALL SearchFormattedToolboxController::getImplementationName()
{
    return OUString( "com.sun.star.svx.SearchFormattedToolboxController" );
}

sal_Bool SAL_CALL SearchFormattedToolboxController::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL SearchFormattedToolboxController::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

// XComponent
void SAL_CALL SearchFormattedToolboxController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    SearchToolbarControllersManager::createControllersManager().freeController(m_xFrame, m_aCommandURL);

    svt::ToolboxController::dispose();

    m_pSearchFormattedControl.disposeAndClear();
}

// XInitialization
void SAL_CALL SearchFormattedToolboxController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::ToolboxController::initialize(aArguments);

    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);
}

css::uno::Reference< css::awt::XWindow > SAL_CALL SearchFormattedToolboxController::createItemWindow( const css::uno::Reference< css::awt::XWindow >& xParent )
{
    css::uno::Reference< css::awt::XWindow > xItemWindow;

    VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow( xParent );
    if ( pParent )
    {
        ToolBox* pToolbar = static_cast<ToolBox*>(pParent.get());
        m_pSearchFormattedControl = VclPtr<CheckBox>::Create( pToolbar, 0 );
        m_pSearchFormattedControl->SetText( SvxResId( RID_SVXSTR_FINDBAR_SEARCHFORMATTED ) );
        Size aSize( m_pSearchFormattedControl->GetOptimalSize() );
        m_pSearchFormattedControl->SetSizePixel( aSize );
    }
    xItemWindow = VCLUnoHelper::GetInterface( m_pSearchFormattedControl );

    return xItemWindow;
}

// XStatusListener
void SAL_CALL SearchFormattedToolboxController::statusChanged( const css::frame::FeatureStateEvent& )
{
}

class FindAllToolboxController   : public svt::ToolboxController,
                                      public css::lang::XServiceInfo
{
public:
    FindAllToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XToolbarController
    virtual void SAL_CALL execute( sal_Int16 KeyModifier ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;
};

FindAllToolboxController::FindAllToolboxController( const css::uno::Reference< css::uno::XComponentContext > & rxContext )
    : svt::ToolboxController( rxContext,
            css::uno::Reference< css::frame::XFrame >(),
            ".uno:FindAll" )
{
}

// XInterface
css::uno::Any SAL_CALL FindAllToolboxController::queryInterface( const css::uno::Type& aType )
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
OUString SAL_CALL FindAllToolboxController::getImplementationName()
{
    return OUString( "com.sun.star.svx.FindAllToolboxController" );
}


sal_Bool SAL_CALL FindAllToolboxController::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL FindAllToolboxController::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

// XComponent
void SAL_CALL FindAllToolboxController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    SearchToolbarControllersManager::createControllersManager().freeController(m_xFrame, m_aCommandURL);

    svt::ToolboxController::dispose();
}

// XInitialization
void SAL_CALL FindAllToolboxController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::ToolboxController::initialize( aArguments );
    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);
}

// XToolbarController
void SAL_CALL FindAllToolboxController::execute( sal_Int16 /*KeyModifier*/ )
{
    if ( m_bDisposed )
        throw css::lang::DisposedException();

    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( getParent() );
    ToolBox* pToolBox = static_cast<ToolBox*>(pWindow.get());

    impl_executeSearch(m_xContext, m_xFrame, pToolBox, false, true);
}

// XStatusListener
void SAL_CALL FindAllToolboxController::statusChanged( const css::frame::FeatureStateEvent& /*rEvent*/ )
{
}

class ExitSearchToolboxController   : public svt::ToolboxController,
                                      public css::lang::XServiceInfo
{
public:
    ExitSearchToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XToolbarController
    virtual void SAL_CALL execute( sal_Int16 KeyModifier ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;
};

ExitSearchToolboxController::ExitSearchToolboxController( const css::uno::Reference< css::uno::XComponentContext > & rxContext )
    : svt::ToolboxController( rxContext,
            css::uno::Reference< css::frame::XFrame >(),
            ".uno:ExitSearch" )
{
}

// XInterface
css::uno::Any SAL_CALL ExitSearchToolboxController::queryInterface( const css::uno::Type& aType )
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
OUString SAL_CALL ExitSearchToolboxController::getImplementationName()
{
    return OUString( "com.sun.star.svx.ExitFindbarToolboxController" );
}


sal_Bool SAL_CALL ExitSearchToolboxController::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ExitSearchToolboxController::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

// XComponent
void SAL_CALL ExitSearchToolboxController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    SearchToolbarControllersManager::createControllersManager().freeController(m_xFrame, m_aCommandURL);

    svt::ToolboxController::dispose();
}

// XInitialization
void SAL_CALL ExitSearchToolboxController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::ToolboxController::initialize( aArguments );
    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);
}

// XToolbarController
void SAL_CALL ExitSearchToolboxController::execute( sal_Int16 /*KeyModifier*/ )
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
void SAL_CALL ExitSearchToolboxController::statusChanged( const css::frame::FeatureStateEvent& /*rEvent*/ )
{
}

class SearchLabelToolboxController : public svt::ToolboxController,
                                     public css::lang::XServiceInfo
{
public:
    SearchLabelToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XToolbarController
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

private:
    VclPtr<vcl::Window> m_pSL;
};

SearchLabelToolboxController::SearchLabelToolboxController( const css::uno::Reference< css::uno::XComponentContext > & rxContext )
    : svt::ToolboxController( rxContext,
            css::uno::Reference< css::frame::XFrame >(),
            ".uno:SearchLabel" )
{
}

// XInterface
css::uno::Any SAL_CALL SearchLabelToolboxController::queryInterface( const css::uno::Type& aType )
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
OUString SAL_CALL SearchLabelToolboxController::getImplementationName()
{
    return OUString( "com.sun.star.svx.SearchLabelToolboxController" );
}


sal_Bool SAL_CALL SearchLabelToolboxController::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL SearchLabelToolboxController::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

// XComponent
void SAL_CALL SearchLabelToolboxController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    SearchToolbarControllersManager::createControllersManager().freeController(m_xFrame, m_aCommandURL);

    svt::ToolboxController::dispose();
    m_pSL.disposeAndClear();
}

// XInitialization
void SAL_CALL SearchLabelToolboxController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::ToolboxController::initialize( aArguments );
    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY), m_aCommandURL);
}

// XStatusListener
void SAL_CALL SearchLabelToolboxController::statusChanged( const css::frame::FeatureStateEvent& )
{
    if (m_pSL)
    {
        OUString aStr = SvxSearchDialogWrapper::GetSearchLabel();
        m_pSL->SetText(aStr);
        long aWidth = !aStr.isEmpty() ? m_pSL->get_preferred_size().getWidth() : 16;
        m_pSL->SetSizePixel(Size(aWidth, m_pSL->get_preferred_size().getHeight()));
    }
}

css::uno::Reference< css::awt::XWindow > SAL_CALL SearchLabelToolboxController::createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent )
{
    m_pSL = VclPtr<FixedText>::Create(VCLUnoHelper::GetWindow( Parent ));
    m_pSL->SetSizePixel(Size(16, 25));
    return VCLUnoHelper::GetInterface(m_pSL);
}

// protocol handler for "vnd.sun.star.findbar:*" URLs
// The dispatch object will be used for shortcut commands for findbar
class FindbarDispatcher : public css::lang::XServiceInfo,
                          public css::lang::XInitialization,
                          public css::frame::XDispatchProvider,
                          public css::frame::XDispatch,
                          public ::cppu::OWeakObject
{
public:

    FindbarDispatcher();
    virtual ~FindbarDispatcher() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch( const css::util::URL& aURL, const OUString& sTargetFrameName , sal_Int32 nSearchFlags ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptions    ) override;

    // XDispatch
    virtual void SAL_CALL dispatch( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) override;
    virtual void SAL_CALL addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener, const css::util::URL& aURL ) override;
    virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener, const css::util::URL& aURL ) override;

private:

    css::uno::Reference< css::frame::XFrame > m_xFrame;

};

FindbarDispatcher::FindbarDispatcher()
{
}

FindbarDispatcher::~FindbarDispatcher()
{
    m_xFrame = nullptr;
}

// XInterface
css::uno::Any SAL_CALL FindbarDispatcher::queryInterface( const css::uno::Type& aType )
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
OUString SAL_CALL FindbarDispatcher::getImplementationName()
{
    return OUString("com.sun.star.comp.svx.Impl.FindbarDispatcher");
}

sal_Bool SAL_CALL FindbarDispatcher::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL FindbarDispatcher::getSupportedServiceNames()
{
    css::uno::Sequence< OUString > aSNS( 2 );
    aSNS[0] = "com.sun.star.comp.svx.FindbarDispatcher";
    aSNS[1] = "com.sun.star.frame.ProtocolHandler";
    return aSNS;
}

// XInitialization
void SAL_CALL FindbarDispatcher::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    if ( aArguments.getLength() )
        aArguments[0] >>= m_xFrame;
}

// XDispatchProvider
css::uno::Reference< css::frame::XDispatch > SAL_CALL FindbarDispatcher::queryDispatch( const css::util::URL& aURL, const OUString& /*sTargetFrameName*/, sal_Int32 /*nSearchFlags*/ )
{
    css::uno::Reference< css::frame::XDispatch > xDispatch;

    if ( aURL.Protocol == "vnd.sun.star.findbar:" )
        xDispatch = this;

    return xDispatch;
}

css::uno::Sequence < css::uno::Reference< css::frame::XDispatch > > SAL_CALL FindbarDispatcher::queryDispatches( const css::uno::Sequence < css::frame::DispatchDescriptor >& seqDescripts )
{
    sal_Int32 nCount = seqDescripts.getLength();
    css::uno::Sequence < css::uno::Reference < XDispatch > > lDispatcher( nCount );

    for( sal_Int32 i=0; i<nCount; ++i )
        lDispatcher[i] = queryDispatch( seqDescripts[i].FeatureURL, seqDescripts[i].FrameName, seqDescripts[i].SearchFlags );

    return lDispatcher;
}

// XDispatch
void SAL_CALL FindbarDispatcher::dispatch( const css::util::URL& aURL, const css::uno::Sequence < css::beans::PropertyValue >& /*lArgs*/ )
{
    //vnd.sun.star.findbar:FocusToFindbar  - set cursor to the FindTextFieldControl of the findbar
    if ( aURL.Path != "FocusToFindbar" )
        return;

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
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
    ToolBox* pToolBox = static_cast<ToolBox*>(pWindow.get());
    if ( pToolBox )
    {
        ToolBox::ImplToolItems::size_type nItemCount = pToolBox->GetItemCount();
        for ( ToolBox::ImplToolItems::size_type i=0; i<nItemCount; ++i )
        {
            sal_uInt16 id = pToolBox->GetItemId(i);
            OUString sItemCommand = pToolBox->GetItemCommand(id);
            if ( sItemCommand == COMMAND_FINDTEXT )
            {
                vcl::Window* pItemWin = pToolBox->GetItemWindow( id );
                if ( pItemWin )
                {
                    SolarMutexGuard aSolarMutexGuard;
                    FindTextFieldControl* pFindTextFieldControl = dynamic_cast<FindTextFieldControl*>(pItemWin);
                    if ( pFindTextFieldControl )
                        pFindTextFieldControl->SetTextToSelected_Impl();
                    pItemWin->GrabFocus();
                    return;
                }
            }
        }
    }
}

void SAL_CALL FindbarDispatcher::addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*xControl*/, const css::util::URL& /*aURL*/ )
{
}

void SAL_CALL FindbarDispatcher::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*xControl*/, const css::util::URL& /*aURL*/ )
{
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_svx_FindTextToolboxController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new FindTextToolbarController(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_svx_ExitFindbarToolboxController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ExitSearchToolboxController(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_svx_UpSearchToolboxController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UpDownSearchToolboxController(context, UpDownSearchToolboxController::UP));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_svx_DownSearchToolboxController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UpDownSearchToolboxController(context, UpDownSearchToolboxController::DOWN));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_svx_MatchCaseToolboxController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new MatchCaseToolboxController(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_svx_SearchFormattedToolboxController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SearchFormattedToolboxController(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_svx_FindAllToolboxController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new FindAllToolboxController(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_svx_SearchLabelToolboxController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SearchLabelToolboxController(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_Impl_FindbarDispatcher_get_implementation(
    SAL_UNUSED_PARAMETER css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new FindbarDispatcher);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
