/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: richtextviewport.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 00:01:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"

#ifndef FORMS_SOURCE_RICHTEXT_RICHTEXTVIEWPORT_HXX
#include "richtextviewport.hxx"
#endif

#ifndef _MyEDITVIEW_HXX
#include <svx/editview.hxx>
#endif

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
        m_pView->ShowCursor( TRUE );
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
