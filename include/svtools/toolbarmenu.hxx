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

#ifndef INCLUDED_SVTOOLS_TOOLBARMENU_HXX
#define INCLUDED_SVTOOLS_TOOLBARMENU_HXX

#include <svtools/svtdllapi.h>

#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <vector>

#include <rtl/ref.hxx>

#include <vcl/ctrl.hxx>
#include <vcl/menu.hxx>
#include <vcl/dockwin.hxx>

class ValueSet;

namespace svtools {

class ToolbarMenuEntry;
struct ToolbarMenu_Impl;

class SVT_DLLPUBLIC ToolbarMenu : public DockingWindow
{
    friend class ToolbarMenuStatusListener;
    friend struct ToolbarMenu_Impl;
public:
    ToolbarMenu(const css::uno::Reference<css::frame::XFrame>& rFrame,
                vcl::Window* pParentWindow,
                WinBits nBits );

    virtual ~ToolbarMenu();
    virtual void dispose() override;

    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    KeyInput( const KeyEvent& rKEvent ) override;
    virtual void    Command( const CommandEvent& rCEvt ) override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;

    void            appendEntry( int nEntryId, const OUString& rStr, MenuItemBits nItemBits = MenuItemBits::NONE );
    void            appendEntry( int nEntryId, const OUString& rStr, const Image& rImage, MenuItemBits nItemBits = MenuItemBits::NONE );
    void            appendEntry( int nEntryId, Control* pControl, MenuItemBits nItemBits = MenuItemBits::NONE );
    void            appendSeparator();

    /** creates an empty ValueSet that is initialized and can be inserted with appendEntry. */
    VclPtr<ValueSet> createEmptyValueSetControl();

    void            checkEntry( int nEntryId, bool bCheck = true );

    void            enableEntry( int nEntryId, bool bEnable = true );

    void            setEntryText( int nEntryId, const OUString& rStr );

    void            setEntryImage( int nEntryId, const Image& rImage );

    const Size&     getMenuSize() const;

    void            SetSelectHdl( const Link<ToolbarMenu*,void>& rLink );

    int             getSelectedEntryId() const;
    int             getHighlightedEntryId() const;

    void            highlightFirstEntry();

protected:
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;

    // todo: move to new base class that will replace SfxPopupWindow
    void AddStatusListener( const OUString& rCommandURL );

    bool IsInPopupMode();
    void EndPopupMode();

    // Forwarded from XStatusListener (subclasses must override this one to get
    // the status updates):
    virtual void statusChanged(const css::frame::FeatureStateEvent& Event ) throw (css::uno::RuntimeException, std::exception);

    void            StateChanged( StateChangedType nType ) override;
    void            DataChanged( const DataChangedEvent& rDCEvt ) override;

private:
    DECL_LINK_TYPED( HighlightHdl, ValueSet*, void );

    void initStatusListener();

    void            implInit(const css::uno::Reference<css::frame::XFrame>& rFrame);
    void            initWindow();

    Size            implCalcSize();

    void            appendEntry(ToolbarMenuEntry* pEntry);

    void            implPaint(vcl::RenderContext& rRenderContext, ToolbarMenuEntry* pThisOnly = nullptr, bool bHighlight = false);

    void            implHighlightEntry(vcl::RenderContext& rRenderContext, int nHighlightEntry, bool bHighlight);
    void            implHighlightAtPosition(const MouseEvent& rMEvt, bool bMBDown);

    void            implChangeHighlightEntry( int nEntry );
    void            implSelectEntry( int nSelectedEntry );

    ToolbarMenuEntry*   implCursorUpDown( bool bUp, bool bHomeEnd );
    ToolbarMenuEntry*   implGetEntry( int nEntry ) const;
    ToolbarMenuEntry*   implSearchEntry( int nEntryId ) const;

    ToolbarMenu_Impl*   mpImpl;
};

} // namespace svtools

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
