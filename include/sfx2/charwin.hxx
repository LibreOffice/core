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

#ifndef INCLUDED_SFX2_INC_CHARWIN_HXX
#define INCLUDED_SFX2_INC_CHARWIN_HXX

#include <sfx2/tbxctrl.hxx>
#include <sfx2/dllapi.h>
#include <vcl/weld.hxx>

class SFX2_DLLPUBLIC SvxCharView
{
private:
    VclPtr<VirtualDevice> mxVirDev;
    std::unique_ptr<weld::DrawingArea> mxDrawingArea;
    Size m_aSize;
    long            mnY;
    Point           maPosition;
    vcl::Font       maFont;
    bool            maHasInsert;
    OUString        m_sText;

    Link<SvxCharView*, void> maMouseClickHdl;
    Link<SvxCharView*, void> maClearClickHdl;
    Link<SvxCharView*, void> maClearAllClickHdl;


    DECL_LINK(DoPaint, weld::DrawingArea::draw_args, void);
    DECL_LINK(DoResize, const Size& rSize, void);
    DECL_LINK(DoMouseButtonDown, const MouseEvent&, void);
    DECL_LINK(DoKeyDown, const KeyEvent&, bool);
    DECL_LINK(DoGetFocus, weld::Widget&, void);
    DECL_LINK(DoLoseFocus, weld::Widget&, void);
public:
    SvxCharView(weld::Builder& rBuilder, const OString& rId, const VclPtr<VirtualDevice>& rVirDev);

    void            SetFont( const vcl::Font& rFont );
    vcl::Font       GetFont() const { return maFont; }
    void            SetText( const OUString& rText );
    OUString        GetText() const { return m_sText; }
    void            Show() { mxDrawingArea->show(); }
    void            Hide() { mxDrawingArea->hide(); }
    void            SetHasInsert( bool bInsert );
    void            InsertCharToDoc();

    void            createContextMenu();

    void            grab_focus() { mxDrawingArea->grab_focus(); }
    void            queue_draw() { mxDrawingArea->queue_draw(); }
    Size            get_preferred_size() const { return mxDrawingArea->get_preferred_size(); }

    void            connect_focus_in(const Link<weld::Widget&, void>& rLink) { mxDrawingArea->connect_focus_in(rLink); }
    void            connect_focus_out(const Link<weld::Widget&, void>& rLink) { mxDrawingArea->connect_focus_out(rLink); }


    void setMouseClickHdl(const Link<SvxCharView*,void> &rLink);
    void setClearClickHdl(const Link<SvxCharView*,void> &rLink);
    void setClearAllClickHdl(const Link<SvxCharView*,void> &rLink);

    void ContextMenuSelect(const OString& rIdent);
};

class SFX2_DLLPUBLIC SvxCharViewControl : public Control
{
public:
    SvxCharViewControl(vcl::Window* pParent);

    void            SetFont( const vcl::Font& rFont );
    void            SetText( const OUString& rText ) override;
    void            InsertCharToDoc();

    void            createContextMenu();

    virtual void    Resize() override;

    virtual Size    GetOptimalSize() const override;

    void setMouseClickHdl(const Link<SvxCharViewControl*,void> &rLink);

protected:
    virtual void    Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle&) override;

    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;

    virtual void KeyInput( const KeyEvent& rKEvt ) override;

private:
    long            mnY;
    Point           maPosition;
    vcl::Font       maFont;

    Link<SvxCharViewControl*, void> maMouseClickHdl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
