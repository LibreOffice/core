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


#include <inettbc.hxx>

#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <comphelper/propertyvalue.hxx>
#include <svl/stritem.hxx>
#include <unotools/historyoptions.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/svapp.hxx>
#include <osl/file.hxx>
#include <rtl/ustring.hxx>

#include <svtools/inettbc.hxx>

#include <vcl/InterimItemWindow.hxx>
#include <sfx2/sfxsids.hrc>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;


// SfxURLToolBoxControl_Impl


SFX_IMPL_TOOLBOX_CONTROL(SfxURLToolBoxControl_Impl,SfxStringItem)

SfxURLToolBoxControl_Impl::SfxURLToolBoxControl_Impl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rBox )
    : SfxToolBoxControl( nSlotId, nId, rBox )
    , m_bModified(false)
{
    addStatusListener( ".uno:CurrentURL");
}

SfxURLToolBoxControl_Impl::~SfxURLToolBoxControl_Impl()
{
}

class URLBoxItemWindow final : public InterimItemWindow
{
private:
    std::unique_ptr<SvtURLBox> m_xWidget;

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
public:
    URLBoxItemWindow(vcl::Window* pParent)
        : InterimItemWindow(pParent, "sfx/ui/urlbox.ui", "URLBox")
        , m_xWidget(new SvtURLBox(m_xBuilder->weld_combo_box("urlbox")))
    {
        InitControlBase(m_xWidget->getWidget());

        m_xWidget->connect_key_press(LINK(this, URLBoxItemWindow, KeyInputHdl));

        int nWidth = GetDesktopRectPixel().GetWidth() > 800 ? 300 : 225;
        SetSizePixel(Size(nWidth, m_xWidget->get_preferred_size().Height()));
    }

    SvtURLBox* GetURLBox()
    {
        return m_xWidget.get();
    }

    virtual void dispose() override
    {
        m_xWidget.reset();
        InterimItemWindow::dispose();
    }

    void set_sensitive(bool bSensitive)
    {
        Enable(bSensitive);
        m_xWidget->set_sensitive(bSensitive);
    }

    virtual ~URLBoxItemWindow() override
    {
        disposeOnce();
    }
};

IMPL_LINK(URLBoxItemWindow, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return ChildKeyInput(rKEvt);
}

URLBoxItemWindow* SfxURLToolBoxControl_Impl::GetURLBoxItemWindow() const
{
    return static_cast<URLBoxItemWindow*>(GetToolBox().GetItemWindow(GetId()));
}

SvtURLBox* SfxURLToolBoxControl_Impl::GetURLBox() const
{
    return GetURLBoxItemWindow()->GetURLBox();
}

void SfxURLToolBoxControl_Impl::OpenURL( const OUString& rName ) const
{
    OUString aName;

    INetURLObject aObj( rName );
    if ( aObj.GetProtocol() == INetProtocol::NotValid )
    {
        aName = SvtURLBox::ParseSmart( rName, "" );
    }
    else
        aName = rName;

    if ( aName.isEmpty() )
        return;

    Reference< XDispatchProvider > xDispatchProvider( getFrameInterface(), UNO_QUERY );
    if ( !xDispatchProvider.is() )
        return;

    URL             aTargetURL;
    aTargetURL.Complete = aName;

    getURLTransformer()->parseStrict( aTargetURL );
    Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch( aTargetURL, "_default", 0 );
    if ( !xDispatch.is() )
        return;

    SfxURLToolBoxControl_Impl::ExecuteInfo* pExecuteInfo = new SfxURLToolBoxControl_Impl::ExecuteInfo;
    pExecuteInfo->xDispatch     = xDispatch;
    pExecuteInfo->aTargetURL    = aTargetURL;
    pExecuteInfo->aArgs         = {
        comphelper::makePropertyValue("Referer", OUString( "private:user" )),
        comphelper::makePropertyValue("FileName", aName)
    };

    Application::PostUserEvent( LINK( nullptr, SfxURLToolBoxControl_Impl, ExecuteHdl_Impl), pExecuteInfo );
}


IMPL_STATIC_LINK( SfxURLToolBoxControl_Impl, ExecuteHdl_Impl, void*, p, void )
{
    ExecuteInfo* pExecuteInfo = static_cast<ExecuteInfo*>(p);
    try
    {
        // Asynchronous execution as this can lead to our own destruction!
        // Framework can recycle our current frame and the layout manager disposes all user interface
        // elements if a component gets detached from its frame!
        pExecuteInfo->xDispatch->dispatch( pExecuteInfo->aTargetURL, pExecuteInfo->aArgs );
    }
    catch ( Exception& )
    {
    }

    delete pExecuteInfo;
}

VclPtr<InterimItemWindow> SfxURLToolBoxControl_Impl::CreateItemWindow( vcl::Window* pParent )
{
    VclPtrInstance<URLBoxItemWindow> xURLBox(pParent);
    SvtURLBox* pURLBox = xURLBox->GetURLBox();
    pURLBox->connect_changed(LINK(this, SfxURLToolBoxControl_Impl, SelectHdl));
    pURLBox->connect_entry_activate(LINK(this, SfxURLToolBoxControl_Impl, OpenHdl));
    xURLBox->Show();
    return xURLBox;
}

IMPL_LINK(SfxURLToolBoxControl_Impl, SelectHdl, weld::ComboBox&, rComboBox, void)
{
    m_bModified = true;

    SvtURLBox* pURLBox = GetURLBox();
    OUString aName( pURLBox->GetURL() );

    if (rComboBox.changed_by_direct_pick() && !aName.isEmpty())
        OpenURL( aName );
}

IMPL_LINK_NOARG(SfxURLToolBoxControl_Impl, OpenHdl, weld::ComboBox&, bool)
{
    SvtURLBox* pURLBox = GetURLBox();
    OpenURL( pURLBox->GetURL() );

    Reference< XDesktop2 > xDesktop = Desktop::create( m_xContext );
    Reference< XFrame > xFrame = xDesktop->getActiveFrame();
    if (!xFrame.is())
        return true;

    auto xWin = xFrame->getContainerWindow();
    if (!xWin)
        return true;
    xWin->setFocus();
    Reference<css::awt::XTopWindow> xTop(xWin, UNO_QUERY);
    if (!xTop)
        return true;
    xTop->toFront();
    return true;
}

void SfxURLToolBoxControl_Impl::StateChangedAtToolBoxControl
(
    sal_uInt16          nSID,
    SfxItemState        eState,
    const SfxPoolItem*  pState
)
{
    if ( nSID == SID_OPENURL )
    {
        // Disable URL box if command is disabled
        GetURLBoxItemWindow()->set_sensitive( SfxItemState::DISABLED != eState );
    }

    if ( !GetURLBoxItemWindow()->IsEnabled() )
        return;

    if( nSID == SID_FOCUSURLBOX )
    {
        if ( GetURLBoxItemWindow()->IsVisible() )
            GetURLBoxItemWindow()->GrabFocus();
    }
    else if ( !m_bModified && SfxItemState::DEFAULT == eState )
    {
        SvtURLBox* pURLBox = GetURLBox();
        pURLBox->clear();

        const std::vector< SvtHistoryOptions::HistoryItem > lList = SvtHistoryOptions::GetList(EHistoryType::PickList);
        for (const SvtHistoryOptions::HistoryItem& lProps : lList)
        {
            if (!lProps.sURL.isEmpty())
            {
                INetURLObject aURL    ( lProps.sURL );
                OUString      sMainURL( aURL.GetMainURL( INetURLObject::DecodeMechanism::WithCharset ) );
                OUString      sFile;

                if (osl::FileBase::getSystemPathFromFileURL(sMainURL, sFile) == osl::FileBase::E_None)
                    pURLBox->append_text(sFile);
                else
                    pURLBox->append_text(sMainURL);
            }
        }

        const SfxStringItem *pURL = dynamic_cast< const SfxStringItem* >(pState);
        assert(pURL);
        INetURLObject aURL( pURL->GetValue() );
        INetProtocol eProt = aURL.GetProtocol();
        if ( eProt == INetProtocol::File )
        {
            pURLBox->set_entry_text( aURL.PathToFileName() );
        }
        else
            pURLBox->set_entry_text( aURL.GetURLNoPass() );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
