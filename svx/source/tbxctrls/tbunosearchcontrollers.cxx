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

#include <config_feature_desktop.h>

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

#include <vcl/InterimItemWindow.hxx>
#include <svl/ctloptions.hxx>
#include <svl/srchitem.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/svapp.hxx>
#include <svx/labelitemwindow.hxx>
#include <svx/srchdlg.hxx>
#include <vcl/event.hxx>

#include <findtextfield.hxx>

using namespace css;

namespace {

constexpr OUStringLiteral COMMAND_FINDTEXT = u".uno:FindText";
constexpr OUStringLiteral COMMAND_DOWNSEARCH = u".uno:DownSearch";
constexpr OUStringLiteral COMMAND_UPSEARCH = u".uno:UpSearch";
constexpr OUStringLiteral COMMAND_FINDALL = u".uno:FindAll";
constexpr OUStringLiteral COMMAND_MATCHCASE = u".uno:MatchCase";
constexpr OUStringLiteral COMMAND_SEARCHFORMATTED = u".uno:SearchFormattedDisplayString";

const sal_Int32       REMEMBER_SIZE = 10;

class CheckButtonItemWindow final : public InterimItemWindow
{
public:
    CheckButtonItemWindow(vcl::Window* pParent, const OUString& rLabel)
        : InterimItemWindow(pParent, "svx/ui/checkbuttonbox.ui", "CheckButtonBox")
        , m_xWidget(m_xBuilder->weld_check_button("checkbutton"))
    {
        InitControlBase(m_xWidget.get());

        m_xWidget->connect_key_press(LINK(this, CheckButtonItemWindow, KeyInputHdl));
        m_xWidget->set_label(rLabel);
        SetSizePixel(m_xWidget->get_preferred_size());
    }

    bool get_active() const
    {
        return m_xWidget->get_active();
    }

    virtual void dispose() override
    {
        m_xWidget.reset();
        InterimItemWindow::dispose();
    }

    virtual ~CheckButtonItemWindow() override
    {
        disposeOnce();
    }

private:
    std::unique_ptr<weld::CheckButton> m_xWidget;

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
};

IMPL_LINK(CheckButtonItemWindow, KeyInputHdl, const KeyEvent&, rKeyEvent, bool)
{
    return ChildKeyInput(rKeyEvent);
}

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
            ToolBoxItemId id = pToolBox->GetItemId(i);
            OUString sItemCommand = pToolBox->GetItemCommand(id);
            if ( sItemCommand == COMMAND_FINDTEXT )
            {
                FindTextFieldControl* pItemWin = static_cast<FindTextFieldControl*>(pToolBox->GetItemWindow(id));
                if (pItemWin)
                {
                    sFindText = pItemWin->get_active_text();
                    if (aFindAll && !pItemWin->ControlHasFocus())
                        pItemWin->GetFocus();
                }
            } else if ( sItemCommand == COMMAND_MATCHCASE )
            {
                CheckButtonItemWindow* pItemWin = static_cast<CheckButtonItemWindow*>(pToolBox->GetItemWindow(id));
                if (pItemWin)
                    aMatchCase = pItemWin->get_active();
            } else if ( sItemCommand == COMMAND_SEARCHFORMATTED )
            {
                CheckButtonItemWindow* pItemWin = static_cast<CheckButtonItemWindow*>(pToolBox->GetItemWindow(id));
                if (pItemWin)
                    bSearchFormatted = pItemWin->get_active();
            }
        }
    }

    SvtCTLOptions aCTLOptions;
    TransliterationFlags nFlags = TransliterationFlags::NONE;
    if (!aMatchCase)
        nFlags |= TransliterationFlags::IGNORE_CASE;
    if (aCTLOptions.IsCTLFontEnabled())
        nFlags |= TransliterationFlags::IGNORE_DIACRITICS_CTL
                  | TransliterationFlags::IGNORE_KASHIDA_CTL;

    auto aArgs( comphelper::InitPropertySequence( {
        { "SearchItem.SearchString", css::uno::makeAny( sFindText ) },
        // Related tdf#102506: make Find Bar Ctrl+F searching by value by default
        { "SearchItem.CellType", css::uno::makeAny( sal_Int16(SvxSearchCellType::VALUE) ) },
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

}

FindTextFieldControl::FindTextFieldControl( vcl::Window* pParent,
    css::uno::Reference< css::frame::XFrame > const & xFrame,
    const css::uno::Reference< css::uno::XComponentContext >& xContext) :
    InterimItemWindow(pParent, "svx/ui/findbox.ui", "FindBox"),
    m_nAsyncGetFocusId(nullptr),
    m_xWidget(m_xBuilder->weld_combo_box("find")),
    m_xFrame(xFrame),
    m_xContext(xContext),
    m_pAcc(svt::AcceleratorExecute::createAcceleratorHelper())
{
    InitControlBase(m_xWidget.get());

    m_xWidget->set_entry_placeholder_text(SvxResId(RID_SVXSTR_FINDBAR_FIND));
    m_xWidget->set_entry_completion(true, true);
    m_pAcc->init(m_xContext, m_xFrame);

    m_xWidget->connect_focus_in(LINK(this, FindTextFieldControl, FocusInHdl));
    m_xWidget->connect_key_press(LINK(this, FindTextFieldControl, KeyInputHdl));
    m_xWidget->connect_entry_activate(LINK(this, FindTextFieldControl, ActivateHdl));

    m_xWidget->set_size_request(250, -1);
    SetSizePixel(m_xWidget->get_preferred_size());
}

void FindTextFieldControl::Remember_Impl(const OUString& rStr)
{
    const sal_Int32 nCount = m_xWidget->get_count();

    for (sal_Int32 i=0; i<nCount; ++i)
    {
        if (rStr == m_xWidget->get_text(i))
            return;
    }

    if (nCount == REMEMBER_SIZE)
        m_xWidget->remove(REMEMBER_SIZE-1);

    m_xWidget->insert_text(0, rStr);
}

void FindTextFieldControl::SetTextToSelected_Impl()
{
    OUString aString;

    try
    {
        css::uno::Reference<css::frame::XController> xController(m_xFrame->getController(), css::uno::UNO_SET_THROW);
        css::uno::Reference<css::frame::XModel> xModel(xController->getModel(), css::uno::UNO_SET_THROW);
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
        m_xWidget->set_entry_text(aString);
        m_aChangeHdl.Call(*m_xWidget);
    }
    else if (get_count() > 0)
    {
        // Else, prepopulate with last search word (fdo#84256)
        m_xWidget->set_entry_text(m_xWidget->get_text(0));
    }
}

IMPL_LINK(FindTextFieldControl, KeyInputHdl, const KeyEvent&, rKeyEvent, bool)
{
    if (isDisposed())
        return true;

    bool bRet = false;

    bool bShift = rKeyEvent.GetKeyCode().IsShift();
    bool bMod1 = rKeyEvent.GetKeyCode().IsMod1();
    sal_uInt16 nCode = rKeyEvent.GetKeyCode().GetCode();

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
                static const OUStringLiteral sResourceURL( u"private:resource/toolbar/findbar" );
                xLayoutManager->hideElement( sResourceURL );
                xLayoutManager->destroyElement( sResourceURL );
            }
        }
    }
    // Select text in the search box when Ctrl-F pressed
    else if ( bMod1 && nCode == KEY_F )
        m_xWidget->select_entry_region(0, -1);

    // Execute the search when Ctrl-G, F3 and Shift-RETURN pressed (in addition to ActivateHdl condition which handles bare RETURN)
    else if ( (bMod1 && KEY_G == nCode) || (bShift && KEY_RETURN == nCode) || (KEY_F3 == nCode) )
    {
        ActivateFind(bShift);
        bRet = true;
    }
    else
    {
        auto awtKey = svt::AcceleratorExecute::st_VCLKey2AWTKey(rKeyEvent.GetKeyCode());
        const OUString aCommand(m_pAcc->findCommand(awtKey));
        if (aCommand == ".uno:SearchDialog")
            bRet = m_pAcc->execute(awtKey);
    }

    return bRet || ChildKeyInput(rKeyEvent);
}

void FindTextFieldControl::ActivateFind(bool bShift)
{
    Remember_Impl(m_xWidget->get_active_text());

    vcl::Window* pWindow = GetParent();
    ToolBox* pToolBox = static_cast<ToolBox*>(pWindow);

    impl_executeSearch(m_xContext, m_xFrame, pToolBox, bShift);

    m_xWidget->grab_focus();
}

// Execute the search when activated, typically due to "Return"
IMPL_LINK_NOARG(FindTextFieldControl, ActivateHdl, weld::ComboBox&, bool)
{
    if (isDisposed())
        return true;

    ActivateFind(false);

    return true;
}

IMPL_LINK_NOARG(FindTextFieldControl, OnAsyncGetFocus, void*, void)
{
    m_nAsyncGetFocusId = nullptr;
    m_xWidget->select_entry_region(0, -1);
}

void FindTextFieldControl::FocusIn()
{
    if (m_nAsyncGetFocusId || !m_xWidget)
        return;

    // do it async to defeat entry in combobox having its own ideas about the focus
    m_nAsyncGetFocusId = Application::PostUserEvent(LINK(this, FindTextFieldControl, OnAsyncGetFocus));

    GrabFocus(); // tdf#137993 ensure the toplevel vcl::Window is activated so SfxViewFrame::Current is valid
}

IMPL_LINK_NOARG(FindTextFieldControl, FocusInHdl, weld::Widget&, void)
{
    FocusIn();
}

void FindTextFieldControl::dispose()
{
    if (m_nAsyncGetFocusId)
    {
        Application::RemoveUserEvent(m_nAsyncGetFocusId);
        m_nAsyncGetFocusId = nullptr;
    }
    m_xWidget.reset();
    InterimItemWindow::dispose();
}

FindTextFieldControl::~FindTextFieldControl()
{
    disposeOnce();
}

void FindTextFieldControl::connect_changed(const Link<weld::ComboBox&, void>& rLink)
{
    m_aChangeHdl = rLink;
    m_xWidget->connect_changed(rLink);
}

int FindTextFieldControl::get_count() const
{
    return m_xWidget->get_count();
}

OUString FindTextFieldControl::get_text(int nIndex) const
{
    return m_xWidget->get_text(nIndex);
}

OUString FindTextFieldControl::get_active_text() const
{
    return m_xWidget->get_active_text();
}

void FindTextFieldControl::append_text(const OUString& rText)
{
    m_xWidget->append_text(rText);
}

void FindTextFieldControl::set_entry_message_type(weld::EntryMessageType eType)
{
    m_xWidget->set_entry_message_type(eType);
}

namespace {

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

SearchToolbarControllersManager& SearchToolbarControllersManager::createControllersManager()
{
    static SearchToolbarControllersManager theSearchToolbarControllersManager;
    return theSearchToolbarControllersManager;
}

void SearchToolbarControllersManager::saveSearchHistory(const FindTextFieldControl* pFindTextFieldControl)
{
    const sal_Int32 nECount( pFindTextFieldControl->get_count() );
    m_aSearchStrings.resize( nECount );
    for( sal_Int32 i=0; i<nECount; ++i )
    {
        m_aSearchStrings[i] = pFindTextFieldControl->get_text(i);
    }
}

void SearchToolbarControllersManager::loadSearchHistory(FindTextFieldControl* pFindTextFieldControl)
{
    for( size_t i=0; i<m_aSearchStrings.size(); ++i )
    {
        pFindTextFieldControl->append_text(m_aSearchStrings[i]);
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
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

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

    DECL_LINK(EditModifyHdl, weld::ComboBox&, void);

private:

    void textfieldChanged();

    VclPtr<FindTextFieldControl> m_pFindTextFieldControl;

    ToolBoxItemId m_nDownSearchId;
    ToolBoxItemId m_nUpSearchId;
    ToolBoxItemId m_nFindAllId;

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

void SAL_CALL FindTextToolbarController::acquire() noexcept
{
    ToolboxController::acquire();
}

void SAL_CALL FindTextToolbarController::release() noexcept
{
    ToolboxController::release();
}

// XServiceInfo
OUString SAL_CALL FindTextToolbarController::getImplementationName()
{
    return "com.sun.star.svx.FindTextToolboxController";
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

    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(this), m_aCommandURL);
}

css::uno::Reference< css::awt::XWindow > SAL_CALL FindTextToolbarController::createItemWindow( const css::uno::Reference< css::awt::XWindow >& xParent )
{
    css::uno::Reference< css::awt::XWindow > xItemWindow;

    VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow( xParent );
    if ( pParent )
    {
        ToolBox* pToolbar = static_cast<ToolBox*>(pParent.get());
        m_pFindTextFieldControl = VclPtr<FindTextFieldControl>::Create(pToolbar, m_xFrame, m_xContext);

        m_pFindTextFieldControl->connect_changed(LINK(this, FindTextToolbarController, EditModifyHdl));
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
        m_pFindTextFieldControl->Remember_Impl(m_pFindTextFieldControl->get_active_text());
    }
    // enable up/down buttons in case there is already text (from the search history)
    textfieldChanged();
}

IMPL_LINK_NOARG(FindTextToolbarController, EditModifyHdl, weld::ComboBox&, void)
{
    // Clear SearchLabel when search string altered
    #if HAVE_FEATURE_DESKTOP
    SvxSearchDialogWrapper::SetSearchLabel(SearchLabel::Empty);
    #endif

    textfieldChanged();
}

void FindTextToolbarController::textfieldChanged() {
    // enable or disable item DownSearch/UpSearch/FindAll of findbar
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( getParent() );
    ToolBox* pToolBox = static_cast<ToolBox*>(pWindow.get());
    if ( pToolBox && m_pFindTextFieldControl )
    {
        bool enableButtons = !m_pFindTextFieldControl->get_active_text().isEmpty();
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
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

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
    Type meType;
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

void SAL_CALL UpDownSearchToolboxController::acquire() noexcept
{
    ToolboxController::acquire();
}

void SAL_CALL UpDownSearchToolboxController::release() noexcept
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
    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(this), m_aCommandURL);
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
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

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
    VclPtr<CheckButtonItemWindow> m_xMatchCaseControl;
};

MatchCaseToolboxController::MatchCaseToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext )
    : svt::ToolboxController( rxContext,
        css::uno::Reference< css::frame::XFrame >(),
        COMMAND_MATCHCASE )
    , m_xMatchCaseControl(nullptr)
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

void SAL_CALL MatchCaseToolboxController::acquire() noexcept
{
    ToolboxController::acquire();
}

void SAL_CALL MatchCaseToolboxController::release() noexcept
{
    ToolboxController::release();
}

// XServiceInfo
OUString SAL_CALL MatchCaseToolboxController::getImplementationName()
{
    return "com.sun.star.svx.MatchCaseToolboxController";
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

    m_xMatchCaseControl.disposeAndClear();
}

// XInitialization
void SAL_CALL MatchCaseToolboxController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::ToolboxController::initialize(aArguments);

    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(this), m_aCommandURL);
}

css::uno::Reference< css::awt::XWindow > SAL_CALL MatchCaseToolboxController::createItemWindow( const css::uno::Reference< css::awt::XWindow >& xParent )
{
    css::uno::Reference< css::awt::XWindow > xItemWindow;

    VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow( xParent );
    if ( pParent )
    {
        ToolBox* pToolbar = static_cast<ToolBox*>(pParent.get());
        m_xMatchCaseControl = VclPtr<CheckButtonItemWindow>::Create(pToolbar, SvxResId(RID_SVXSTR_FINDBAR_MATCHCASE));
    }
    xItemWindow = VCLUnoHelper::GetInterface(m_xMatchCaseControl);

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
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

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
    VclPtr<CheckButtonItemWindow> m_xSearchFormattedControl;
};

SearchFormattedToolboxController::SearchFormattedToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext )
    : svt::ToolboxController( rxContext,
        css::uno::Reference< css::frame::XFrame >(),
        COMMAND_SEARCHFORMATTED )
    , m_xSearchFormattedControl(nullptr)
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

void SAL_CALL SearchFormattedToolboxController::acquire() noexcept
{
    ToolboxController::acquire();
}

void SAL_CALL SearchFormattedToolboxController::release() noexcept
{
    ToolboxController::release();
}

// XServiceInfo
OUString SAL_CALL SearchFormattedToolboxController::getImplementationName()
{
    return "com.sun.star.svx.SearchFormattedToolboxController";
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

    m_xSearchFormattedControl.disposeAndClear();
}

// XInitialization
void SAL_CALL SearchFormattedToolboxController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::ToolboxController::initialize(aArguments);

    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(this), m_aCommandURL);
}

css::uno::Reference< css::awt::XWindow > SAL_CALL SearchFormattedToolboxController::createItemWindow( const css::uno::Reference< css::awt::XWindow >& xParent )
{
    css::uno::Reference< css::awt::XWindow > xItemWindow;

    VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow( xParent );
    if ( pParent )
    {
        ToolBox* pToolbar = static_cast<ToolBox*>(pParent.get());
        m_xSearchFormattedControl = VclPtr<CheckButtonItemWindow>::Create(pToolbar, SvxResId(RID_SVXSTR_FINDBAR_SEARCHFORMATTED));
    }
    xItemWindow = VCLUnoHelper::GetInterface(m_xSearchFormattedControl);

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
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

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

void SAL_CALL FindAllToolboxController::acquire() noexcept
{
    ToolboxController::acquire();
}

void SAL_CALL FindAllToolboxController::release() noexcept
{
    ToolboxController::release();
}

// XServiceInfo
OUString SAL_CALL FindAllToolboxController::getImplementationName()
{
    return "com.sun.star.svx.FindAllToolboxController";
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
    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(this), m_aCommandURL);
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
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

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

void SAL_CALL ExitSearchToolboxController::acquire() noexcept
{
    ToolboxController::acquire();
}

void SAL_CALL ExitSearchToolboxController::release() noexcept
{
    ToolboxController::release();
}

// XServiceInfo
OUString SAL_CALL ExitSearchToolboxController::getImplementationName()
{
    return "com.sun.star.svx.ExitFindbarToolboxController";
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
    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(this), m_aCommandURL);
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
            static const OUStringLiteral sResourceURL( u"private:resource/toolbar/findbar" );
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
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

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
    VclPtr<LabelItemWindow> m_xSL;
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

void SAL_CALL SearchLabelToolboxController::acquire() noexcept
{
    ToolboxController::acquire();
}

void SAL_CALL SearchLabelToolboxController::release() noexcept
{
    ToolboxController::release();
}

// XServiceInfo
OUString SAL_CALL SearchLabelToolboxController::getImplementationName()
{
    return "com.sun.star.svx.SearchLabelToolboxController";
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
    m_xSL.disposeAndClear();
}

// XInitialization
void SAL_CALL SearchLabelToolboxController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::ToolboxController::initialize( aArguments );
    SearchToolbarControllersManager::createControllersManager().registryController(m_xFrame, css::uno::Reference< css::frame::XStatusListener >(this), m_aCommandURL);
}

// XStatusListener
void SAL_CALL SearchLabelToolboxController::statusChanged( const css::frame::FeatureStateEvent& )
{
    if (m_xSL)
    {
        OUString aStr = SvxSearchDialogWrapper::GetSearchLabel();
        m_xSL->set_label(aStr);
        m_xSL->SetOptimalSize();
        Size aSize(m_xSL->GetSizePixel());
        tools::Long nWidth = !aStr.isEmpty() ? aSize.getWidth() : 16;
        m_xSL->SetSizePixel(Size(nWidth, aSize.Height()));
    }
}

css::uno::Reference< css::awt::XWindow > SAL_CALL SearchLabelToolboxController::createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent )
{
    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if (getToolboxId(nId, &pToolBox))
        pToolBox->SetItemWindowNonInteractive(nId, true);

    m_xSL = VclPtr<LabelItemWindow>::Create(VCLUnoHelper::GetWindow(Parent), "");
    m_xSL->SetSizePixel(Size(16, m_xSL->GetSizePixel().Height()));
    return VCLUnoHelper::GetInterface(m_xSL);
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
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

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

void SAL_CALL FindbarDispatcher::acquire() noexcept
{
    OWeakObject::acquire();
}

void SAL_CALL FindbarDispatcher::release() noexcept
{
    OWeakObject::release();
}

// XServiceInfo
OUString SAL_CALL FindbarDispatcher::getImplementationName()
{
    return "com.sun.star.comp.svx.Impl.FindbarDispatcher";
}

sal_Bool SAL_CALL FindbarDispatcher::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL FindbarDispatcher::getSupportedServiceNames()
{
    return { "com.sun.star.comp.svx.FindbarDispatcher", "com.sun.star.frame.ProtocolHandler" };
}

// XInitialization
void SAL_CALL FindbarDispatcher::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    if ( aArguments.hasElements() )
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

    std::transform(seqDescripts.begin(), seqDescripts.end(), lDispatcher.getArray(),
        [this](const css::frame::DispatchDescriptor& rDescript) -> css::uno::Reference < XDispatch > {
            return queryDispatch( rDescript.FeatureURL, rDescript.FrameName, rDescript.SearchFlags ); });

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

    static const OUStringLiteral sResourceURL( u"private:resource/toolbar/findbar" );
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
    pToolBox->set_id("FindBar");
    if ( !pToolBox )
        return;

    ToolBox::ImplToolItems::size_type nItemCount = pToolBox->GetItemCount();
    for ( ToolBox::ImplToolItems::size_type i=0; i<nItemCount; ++i )
    {
        ToolBoxItemId id = pToolBox->GetItemId(i);
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
