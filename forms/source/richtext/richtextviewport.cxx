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

#include "richtextviewport.hxx"
#include <editeng/editview.hxx>

namespace frm
{
    RichTextViewPort::RichTextViewPort( vcl::Window* _pParent )
        : Control ( _pParent )
        , m_pView(nullptr)
        , m_bHideInactiveSelection( true )
    {
    }

    void RichTextViewPort::setView( EditView& _rView )
    {
        m_pView = &_rView;
        SetPointer( _rView.GetPointer() );
    }

    void RichTextViewPort::Paint( vcl::RenderContext& rRenderContext, const Rectangle& _rRect )
    {
        m_pView->Paint(_rRect, &rRenderContext);
    }

    void RichTextViewPort::GetFocus()
    {
        Control::GetFocus();
        if (m_pView)
        {
            m_pView->SetSelectionMode( EE_SELMODE_STD );
            m_pView->ShowCursor();
        }
    }

    void RichTextViewPort::LoseFocus()
    {
        if (m_pView)
        {
            m_pView->HideCursor();
            m_pView->SetSelectionMode( m_bHideInactiveSelection ? EE_SELMODE_HIDDEN : EE_SELMODE_STD );
        }
        Control::LoseFocus();
    }

    void RichTextViewPort::KeyInput( const KeyEvent& _rKEvt )
    {
        if ( !m_pView->PostKeyEvent( _rKEvt ) )
            Control::KeyInput( _rKEvt );
        else
            implInvalidateAttributes();
    }

    void RichTextViewPort::MouseMove( const MouseEvent& _rMEvt )
    {
        Control::MouseMove( _rMEvt );
        m_pView->MouseMove( _rMEvt );
    }

    void RichTextViewPort::MouseButtonDown( const MouseEvent& _rMEvt )
    {
        Control::MouseButtonDown( _rMEvt );
        m_pView->MouseButtonDown( _rMEvt );
        GrabFocus();
    }

    void RichTextViewPort::MouseButtonUp( const MouseEvent& _rMEvt )
    {
        Control::MouseButtonUp( _rMEvt );
        m_pView->MouseButtonUp( _rMEvt );
        implInvalidateAttributes();
    }

    void RichTextViewPort::SetHideInactiveSelection( bool _bHide )
    {
        if ( m_bHideInactiveSelection == _bHide )
            return;
        m_bHideInactiveSelection = _bHide;
        if ( !HasFocus() )
            m_pView->SetSelectionMode( m_bHideInactiveSelection ? EE_SELMODE_HIDDEN : EE_SELMODE_STD );
    }

}   // namespace frm

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
