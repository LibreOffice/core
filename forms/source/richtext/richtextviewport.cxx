/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
