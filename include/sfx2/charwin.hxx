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

class SFX2_DLLPUBLIC SvxCharView : public Control
{
public:
    SvxCharView(vcl::Window* pParent);

    void            SetFont( const vcl::Font& rFont );
    void            SetText( const OUString& rText ) override;
    void            SetHasInsert( bool bInsert );
    void            InsertCharToDoc();

    void            createContextMenu();

    virtual void    Resize() override;

    virtual Size    GetOptimalSize() const override;

    void setMouseClickHdl(const Link<SvxCharView*,void> &rLink);
    void setClearClickHdl(const Link<SvxCharView*,void> &rLink);
    void setClearAllClickHdl(const Link<SvxCharView*,void> &rLink);

    DECL_LINK(ContextMenuSelectHdl, Menu*, bool);

protected:
    virtual void    Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle&) override;

    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;

    virtual void KeyInput( const KeyEvent& rKEvt ) override;

private:
    long            mnY;
    Point           maPosition;
    vcl::Font       maFont;
    bool            maHasInsert;

    Link<SvxCharView*, void> maMouseClickHdl;
    Link<SvxCharView*, void> maClearClickHdl;
    Link<SvxCharView*, void> maClearAllClickHdl;
};

#endif
