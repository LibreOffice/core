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

#include <tools/debug.hxx>
#include <sal/types.h>
#include <vcl/lstbox.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/event.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/slstitm.hxx>
#include <svl/stritem.hxx>
#include <svtools/toolbarmenu.hxx>
#include <svx/dialmgr.hxx>
#include <svx/lboxctrl.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/settings.hxx>
#include <tools/urlobj.hxx>

#include <svx/svxids.hrc>
#include <svx/strings.hrc>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;

class SvxPopupWindowListBox final : public svtools::ToolbarPopup
{
    VclPtr<ListBox> m_pListBox;
    rtl::Reference<SvxUndoRedoControl> m_xControl;

    DECL_LINK( SelectHdl, ListBox&, void );

public:
    SvxPopupWindowListBox(SvxUndoRedoControl* pControl, vcl::Window* pParent);
    virtual ~SvxPopupWindowListBox() override;
    virtual void dispose() override;

    ListBox &            GetListBox()    { return *m_pListBox; }

    void                 SetInfo(sal_Int32 nCount);
};

SvxPopupWindowListBox::SvxPopupWindowListBox(SvxUndoRedoControl* pControl, vcl::Window* pParent)
    : ToolbarPopup(pControl->getFrameInterface(), pParent, "FloatingUndoRedo", "svx/ui/floatingundoredo.ui")
    , m_xControl(pControl)
{
    get(m_pListBox, "treeview");
    WinBits nBits(m_pListBox->GetStyle());
    nBits &= ~WB_SIMPLEMODE;
    m_pListBox->SetStyle(nBits);
    Size aSize(LogicToPixel(Size(100, 85), MapMode(MapUnit::MapAppFont)));
    m_pListBox->set_width_request(aSize.Width());
    m_pListBox->set_height_request(aSize.Height());
    m_pListBox->EnableMultiSelection( true, true );
    SetBackground( GetSettings().GetStyleSettings().GetDialogColor() );

    m_pListBox->SetSelectHdl( LINK( this, SvxPopupWindowListBox, SelectHdl ) );
}

SvxPopupWindowListBox::~SvxPopupWindowListBox()
{
    disposeOnce();
}

void SvxPopupWindowListBox::dispose()
{
    m_pListBox.clear();
    ToolbarPopup::dispose();
}

void SvxPopupWindowListBox::SetInfo( sal_Int32 nCount )
{
    const char* pId;
    if (nCount == 1)
        pId = m_xControl->getCommandURL() == ".uno:Undo" ? RID_SVXSTR_NUM_UNDO_ACTION : RID_SVXSTR_NUM_REDO_ACTION;
    else
        pId = m_xControl->getCommandURL() == ".uno:Undo" ? RID_SVXSTR_NUM_UNDO_ACTIONS : RID_SVXSTR_NUM_REDO_ACTIONS;
    OUString aActionStr = SvxResId(pId);
    OUString aText = aActionStr.replaceAll("$(ARG1)", OUString::number(nCount));
    SetText(aText);
}

IMPL_LINK(SvxPopupWindowListBox, SelectHdl, ListBox&, rListBox, void)
{
    if (rListBox.IsTravelSelect())
        SetInfo(rListBox.GetSelectedEntryCount());
    else
    {
        m_xControl->Do(GetListBox().GetSelectedEntryCount());
        EndPopupMode();
    }
}

void SvxUndoRedoControl::Do(sal_Int16 nCount)
{
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
    if ( xDispatchProvider.is() )
    {
        css::util::URL aTargetURL;
        Reference < XURLTransformer > xTrans( URLTransformer::create(::comphelper::getProcessComponentContext()) );
        aTargetURL.Complete = m_aCommandURL;
        xTrans->parseStrict( aTargetURL );

        Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
        if ( xDispatch.is() )
        {
            INetURLObject aObj( m_aCommandURL );
            Sequence< PropertyValue > aArgs( 1 );
            aArgs[0].Name = aObj.GetURLPath();
            aArgs[0].Value <<= nCount;
            xDispatch->dispatch(aTargetURL, aArgs);
        }
    }
}

SvxUndoRedoControl::SvxUndoRedoControl(const css::uno::Reference<css::uno::XComponentContext>& rContext)
    : PopupWindowController(rContext, nullptr, OUString())
{
}

void SvxUndoRedoControl::initialize( const css::uno::Sequence< css::uno::Any >& rArguments )
{
    PopupWindowController::initialize(rArguments);

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if (getToolboxId(nId, &pToolBox))
    {
        pToolBox->SetItemBits(nId, ToolBoxItemBits::DROPDOWN | pToolBox->GetItemBits(nId));
        aDefaultTooltip = pToolBox->GetQuickHelpText(nId);
    }
}

SvxUndoRedoControl::~SvxUndoRedoControl()
{
}

// XStatusListener
void SAL_CALL SvxUndoRedoControl::statusChanged(const css::frame::FeatureStateEvent& rEvent)
{
    if (rEvent.FeatureURL.Main == ".uno:GetUndoStrings" || rEvent.FeatureURL.Main == ".uno:GetRedoStrings")
    {
        css::uno::Sequence<OUString> aStrings;
        rEvent.State >>= aStrings;
        aUndoRedoList = comphelper::sequenceToContainer<std::vector<OUString>>(aStrings);
        return;
    }

    PopupWindowController::statusChanged(rEvent);

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if (!getToolboxId(nId, &pToolBox))
        return;

    if (!rEvent.IsEnabled)
    {
        pToolBox->SetQuickHelpText(nId, aDefaultTooltip);
        return;
    }

    OUString aQuickHelpText;
    if (rEvent.State >>= aQuickHelpText)
        pToolBox->SetQuickHelpText(nId, aQuickHelpText);
}

VclPtr<vcl::Window> SvxUndoRedoControl::createVclPopupWindow(vcl::Window* pParent)
{
    if ( m_aCommandURL == ".uno:Undo" )
        updateStatus( ".uno:GetUndoStrings");
    else
        updateStatus( ".uno:GetRedoStrings");

    auto xPopupWin = VclPtr<SvxPopupWindowListBox>::Create(this, pParent);

    ListBox &rListBox = xPopupWin->GetListBox();

    for(const OUString & s : aUndoRedoList)
        rListBox.InsertEntry( s );

    rListBox.SelectEntryPos(0);
    xPopupWin->SetInfo(rListBox.GetSelectedEntryCount());

    return xPopupWin;
}

OUString SvxUndoRedoControl::getImplementationName()
{
    return "com.sun.star.comp.svx.UndoRedoToolBoxControl";
}

css::uno::Sequence<OUString> SvxUndoRedoControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_UndoRedoToolBoxControl_get_implementation(
    css::uno::XComponentContext* rContext,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire(new SvxUndoRedoControl(rContext));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
