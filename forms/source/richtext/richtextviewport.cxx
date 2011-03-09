/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"
#include "richtextviewport.hxx"
#include <editeng/editview.hxx>

//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= RichTextViewPort
    //====================================================================
    //--------------------------------------------------------------------
    RichTextViewPort::RichTextViewPort( Window* _pParent )
        :Control ( _pParent )
        ,m_bHideInactiveSelection( true )
    {
    }

    //--------------------------------------------------------------------
    void RichTextViewPort::setView( EditView& _rView )
    {
        m_pView = &_rView;
        SetPointer( _rView.GetPointer() );
    }

    //--------------------------------------------------------------------
    void RichTextViewPort::Paint( const Rectangle& _rRect )
    {
        m_pView->Paint( _rRect );
    }

    //--------------------------------------------------------------------
    void RichTextViewPort::GetFocus()
    {
        Control::GetFocus();
        m_pView->SetSelectionMode( EE_SELMODE_STD );
        m_pView->ShowCursor( sal_True );
    }

    //--------------------------------------------------------------------
    void RichTextViewPort::LoseFocus()
    {
        m_pView->HideCursor();
        m_pView->SetSelectionMode( m_bHideInactiveSelection ? EE_SELMODE_HIDDEN : EE_SELMODE_STD );
        Control::LoseFocus();
    }

    //--------------------------------------------------------------------
    void RichTextViewPort::KeyInput( const KeyEvent& _rKEvt )
    {
        if ( !m_pView->PostKeyEvent( _rKEvt ) )
            Control::KeyInput( _rKEvt );
        else
            implInvalidateAttributes();
    }

    //--------------------------------------------------------------------
    void RichTextViewPort::MouseMove( const MouseEvent& _rMEvt )
    {
        Control::MouseMove( _rMEvt );
        m_pView->MouseMove( _rMEvt );
    }

    //--------------------------------------------------------------------
    void RichTextViewPort::MouseButtonDown( const MouseEvent& _rMEvt )
    {
        Control::MouseButtonDown( _rMEvt );
        m_pView->MouseButtonDown( _rMEvt );
        GrabFocus();
    }

    //--------------------------------------------------------------------
    void RichTextViewPort::MouseButtonUp( const MouseEvent& _rMEvt )
    {
        Control::MouseButtonUp( _rMEvt );
        m_pView->MouseButtonUp( _rMEvt );
        implInvalidateAttributes();
    }

    //--------------------------------------------------------------------
    void RichTextViewPort::SetHideInactiveSelection( bool _bHide )
    {
        if ( m_bHideInactiveSelection == _bHide )
            return;
        m_bHideInactiveSelection = _bHide;
        if ( !HasFocus() )
            m_pView->SetSelectionMode( m_bHideInactiveSelection ? EE_SELMODE_HIDDEN : EE_SELMODE_STD );
    }

    //--------------------------------------------------------------------
    bool RichTextViewPort::GetHideInactiveSelection() const
    {
        return m_bHideInactiveSelection;
    }

//........................................................................
}   // namespace frm
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
