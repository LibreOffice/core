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

#include <memory>

#include <rtl/ref.hxx>
#include <vcl/dockwin.hxx>

namespace com :: sun :: star :: frame { class XFrame; }
namespace com :: sun :: star :: frame { struct FeatureStateEvent; }
namespace svt { class FrameStatusListener; }

class Control;
class ValueSet;

namespace svtools {

class ToolbarMenuEntry;
struct ToolbarMenu_Impl;

class SVT_DLLPUBLIC ToolbarPopupBase
{
    friend class ToolbarPopupStatusListener;
public:
    ToolbarPopupBase(const css::uno::Reference<css::frame::XFrame>& rFrame);
    virtual ~ToolbarPopupBase();

protected:
    void AddStatusListener( const OUString& rCommandURL );

    // Forwarded from XStatusListener (subclasses must override this one to get the status updates):
    /// @throws css::uno::RuntimeException
    virtual void statusChanged(const css::frame::FeatureStateEvent& Event );

    css::uno::Reference<css::frame::XFrame>  mxFrame;
    rtl::Reference<svt::FrameStatusListener> mxStatusListener;
};

class SVT_DLLPUBLIC ToolbarPopup : public DockingWindow, public ToolbarPopupBase
{
public:
    ToolbarPopup(const css::uno::Reference<css::frame::XFrame>& rFrame,
                 vcl::Window* pParentWindow,
                 WinBits nBits );
    ToolbarPopup(const css::uno::Reference<css::frame::XFrame>& rFrame,
                 vcl::Window* pParentWindow,
                 const OString& rID, const OUString& rUIXMLDescription );
    virtual ~ToolbarPopup() override;
    virtual void dispose() override;

protected:
    bool IsInPopupMode();
    void EndPopupMode();

private:
    void init();
};

class SVT_DLLPUBLIC ToolbarMenu : public ToolbarPopup
{
    friend struct ToolbarMenu_Impl;
public:
    ToolbarMenu(const css::uno::Reference<css::frame::XFrame>& rFrame,
                vcl::Window* pParentWindow,
                WinBits nBits );

    virtual ~ToolbarMenu() override;
    virtual void dispose() override;

    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    KeyInput( const KeyEvent& rKEvent ) override;
    virtual void    Command( const CommandEvent& rCEvt ) override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;

    void            appendEntry( int nEntryId, const OUString& rStr, MenuItemBits nItemBits = MenuItemBits::NONE );
    void            appendEntry( int nEntryId, const OUString& rStr, const Image& rImage );
    void            appendEntry( int nEntryId, Control* pControl );
    void            appendSeparator();

    /** creates an empty ValueSet that is initialized and can be inserted with appendEntry. */
    VclPtr<ValueSet> createEmptyValueSetControl();

    void            checkEntry( int nEntryId, bool bCheck );

    void            enableEntry( int nEntryId, bool bEnable );

    void            setEntryText( int nEntryId, const OUString& rStr );

    void            setEntryImage( int nEntryId, const Image& rImage );

    const Size&     getMenuSize() const;

    void            SetSelectHdl( const Link<ToolbarMenu*,void>& rLink );

    int             getSelectedEntryId() const;
    int             getHighlightedEntryId() const;

protected:
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;

    void            StateChanged( StateChangedType nType ) override;
    void            DataChanged( const DataChangedEvent& rDCEvt ) override;

private:
    DECL_LINK( HighlightHdl, ValueSet*, void );

    void            initWindow();

    Size            implCalcSize();

    void            appendEntry(std::unique_ptr<ToolbarMenuEntry> pEntry);

    void            implPaint(vcl::RenderContext& rRenderContext, ToolbarMenuEntry const * pThisOnly = nullptr, bool bHighlight = false);

    void            implHighlightEntry(vcl::RenderContext& rRenderContext, int nHighlightEntry);
    void            implHighlightAtPosition(const MouseEvent& rMEvt);

    void            implChangeHighlightEntry( int nEntry );
    void            implSelectEntry( int nSelectedEntry );

    ToolbarMenuEntry*   implCursorUpDown( bool bUp, bool bHomeEnd );
    ToolbarMenuEntry*   implGetEntry( int nEntry ) const;
    ToolbarMenuEntry*   implSearchEntry( int nEntryId ) const;

    std::unique_ptr<ToolbarMenu_Impl>   mpImpl;
};

} // namespace svtools

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
