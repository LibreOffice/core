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

#include "richtextvclcontrol.hxx"
#include "richtextimplcontrol.hxx"
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/languageoptions.hxx>
#if OSL_DEBUG_LEVEL > 0
    #include <unotools/ucbstreamhelper.hxx>
    #include <vcl/msgbox.hxx>
    #include <sfx2/filedlghelper.hxx>
    #include <tools/urlobj.hxx>
    #include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#endif
#include <editeng/scripttypeitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/editids.hrc>
#include <svx/svxids.hrc>

namespace frm
{

    RichTextControl::RichTextControl( RichTextEngine* _pEngine, vcl::Window* _pParent, WinBits _nStyle,
        ITextAttributeListener* _pTextAttribListener, ITextSelectionListener* _pSelectionListener )
        :Control( _pParent, implInitStyle( _nStyle ) )
        ,m_pImpl( nullptr )
    {
        implInit( _pEngine, _pTextAttribListener, _pSelectionListener );
    }


    void RichTextControl::implInit( RichTextEngine* _pEngine, ITextAttributeListener* _pTextAttribListener, ITextSelectionListener* _pSelectionListener )
    {
        m_pImpl = new RichTextControlImpl( this, _pEngine, _pTextAttribListener, _pSelectionListener );
        SetCompoundControl( true );
    }


    RichTextControl::~RichTextControl( )
    {
        disposeOnce();
    }

    void RichTextControl::dispose()
    {
        delete m_pImpl;
        Control::dispose();
    }


    AttributeState RichTextControl::getState( AttributeId _nAttributeId ) const
    {
        return m_pImpl->getAttributeState( _nAttributeId );
    }


    void RichTextControl::executeAttribute( AttributeId _nAttributeId, const SfxPoolItem* _pArgument )
    {
        SfxItemSet aToApplyAttributes( getView().GetEmptyItemSet() );
        if ( !m_pImpl->executeAttribute( getView().GetAttribs(), aToApplyAttributes, _nAttributeId, _pArgument, m_pImpl->getSelectedScriptType() ) )
        {
            OSL_FAIL( "RichTextControl::executeAttribute: cannot handle the given attribute!" );
            return;
        }

        applyAttributes( aToApplyAttributes );
    }


    void RichTextControl::applyAttributes( const SfxItemSet& _rAttributesToApply )
    {
        // apply
        if ( HasChildPathFocus() )
            getView().HideCursor();

        bool bOldUpdateMode = getEngine().GetUpdateMode();   // TODO: guard?
        getEngine().SetUpdateMode( false );

        getView().SetAttribs( _rAttributesToApply );

        getEngine().SetUpdateMode( bOldUpdateMode );
        getView().Invalidate();

        if ( HasChildPathFocus() )
            getView().ShowCursor();

        m_pImpl->updateAllAttributes();
            // TODO: maybe we should have a list of attributes which need to be updated
            // (the handler for the just executed attribute should know)
    }


    void RichTextControl::enableAttributeNotification( AttributeId _nAttributeId, ITextAttributeListener* _pListener )
    {
        m_pImpl->enableAttributeNotification( _nAttributeId, _pListener );
    }


    void RichTextControl::disableAttributeNotification( AttributeId _nAttributeId )
    {
        m_pImpl->disableAttributeNotification( _nAttributeId );
    }


    bool RichTextControl::isMappableSlot( SfxSlotId _nSlotId )
    {
        switch ( _nSlotId )
        {
            case SID_ATTR_PARA_ADJUST_LEFT:
            case SID_ATTR_PARA_ADJUST_CENTER:
            case SID_ATTR_PARA_ADJUST_RIGHT:
            case SID_ATTR_PARA_ADJUST_BLOCK:
            case SID_SET_SUPER_SCRIPT:
            case SID_SET_SUB_SCRIPT:
            case SID_ATTR_PARA_LINESPACE_10:
            case SID_ATTR_PARA_LINESPACE_15:
            case SID_ATTR_PARA_LINESPACE_20:
            case SID_ATTR_PARA_LEFT_TO_RIGHT:
            case SID_ATTR_PARA_RIGHT_TO_LEFT:
            case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
            case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
            case SID_ATTR_CHAR_LATIN_FONT:
            case SID_ATTR_CHAR_LATIN_FONTHEIGHT:
            case SID_ATTR_CHAR_LATIN_LANGUAGE:
            case SID_ATTR_CHAR_LATIN_POSTURE:
            case SID_ATTR_CHAR_LATIN_WEIGHT:
                return true;
        }
        return false;
    }


    void RichTextControl::Resize()
    {
        m_pImpl->layoutWindow();
        Invalidate();
    }


    void RichTextControl::GetFocus()
    {
        getViewport().GrabFocus();
    }


    WinBits RichTextControl::implInitStyle( WinBits nStyle )
    {
        if ( !( nStyle & WB_NOTABSTOP ) )
            nStyle |= WB_TABSTOP;
        return nStyle;
    }


    void RichTextControl::StateChanged( StateChangedType _nStateChange )
    {
        if ( _nStateChange == StateChangedType::Style )
        {
            SetStyle( implInitStyle( GetStyle() ) );
            m_pImpl->notifyStyleChanged();
        }
        else if ( _nStateChange == StateChangedType::Zoom )
        {
            m_pImpl->notifyZoomChanged();
        }
        else if ( _nStateChange == StateChangedType::InitShow )
        {
            m_pImpl->notifyInitShow();
        }
        Control::StateChanged( _nStateChange );
    }


    bool RichTextControl::PreNotify( NotifyEvent& _rNEvt )
    {
        if ( IsWindowOrChild( _rNEvt.GetWindow() ) )
        {
            if ( MouseNotifyEvent::KEYINPUT == _rNEvt.GetType() )
            {
                const ::KeyEvent* pKeyEvent = _rNEvt.GetKeyEvent();

                sal_uInt16 nCode = pKeyEvent->GetKeyCode().GetCode();
                bool   bShift = pKeyEvent->GetKeyCode().IsShift();
                bool   bCtrl = pKeyEvent->GetKeyCode().IsMod1();
                bool   bAlt = pKeyEvent->GetKeyCode().IsMod2();
                if ( ( KEY_TAB == nCode ) && bCtrl && !bAlt )
                {
                    // Ctrl-Tab is used to step out of the control
                    // -> build a new key event without the Ctrl-key, and let the very base class handle it
                    vcl::KeyCode aNewCode( KEY_TAB, bShift, false, false, false );
                    ::KeyEvent aNewEvent( pKeyEvent->GetCharCode(), aNewCode );
                    Control::KeyInput( aNewEvent );
                    return true;   // handled
                }

#if OSL_DEBUG_LEVEL > 0
                if  (   (   ( KEY_F12 == nCode )
                        ||  ( KEY_F11 == nCode )
                        )
                    &&  bCtrl
                    &&  bAlt
                    )
                {
                    bool bLoad = KEY_F11 == nCode;
                    struct
                    {
                        const sal_Char* pDescription;
                        const sal_Char* pExtension;
                        EETextFormat    eFormat;
                    } aExportFormats[] =
                    {
                        { "OASIS OpenDocument (*.xml)", "*.xml", EE_FORMAT_XML },
                        { "HyperText Markup Language (*.html)", "*.html", EE_FORMAT_HTML },
                        { "Rich Text format (*.rtf)", "*.rtf", EE_FORMAT_RTF },
                        { "Text (*.txt)", "*.txt", EE_FORMAT_TEXT }
                    };

                    ::sfx2::FileDialogHelper aFP( bLoad ? css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE : css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION, 0, this );

                    for ( size_t i = 0; i < SAL_N_ELEMENTS( aExportFormats ); ++i )
                    {
                        aFP.AddFilter(
                            OUString::createFromAscii( aExportFormats[i].pDescription ),
                            OUString::createFromAscii( aExportFormats[i].pExtension ) );
                    }
                    ErrCode nResult = aFP.Execute();
                    if ( nResult == 0 )
                    {
                        OUString sFileName = aFP.GetPath();
                        SvStream* pStream = ::utl::UcbStreamHelper::CreateStream(
                            sFileName, ( bLoad ? StreamMode::READ : StreamMode::WRITE | StreamMode::TRUNC ) | StreamMode::SHARE_DENYALL
                        );
                        if ( pStream )
                        {
                            EETextFormat eFormat = EE_FORMAT_XML;
                            OUString sFilter = aFP.GetCurrentFilter();
                            for ( size_t i = 0; i < SAL_N_ELEMENTS( aExportFormats ); ++i )
                            {
                                if ( sFilter.equalsAscii( aExportFormats[i].pDescription ) )
                                {
                                    eFormat = aExportFormats[i].eFormat;
                                    break;
                                }
                            }
                            if ( bLoad )
                            {
                                INetURLObject aURL( sFileName );
                                aURL.removeSegment();
                                getEngine().Read( *pStream, aURL.GetMainURL( INetURLObject::NO_DECODE ), eFormat );
                            }
                            else
                            {
                                getEngine().Write( *pStream, eFormat );
                            }
                        }
                        DELETEZ( pStream );
                    }
                    return true;   // handled
                }
#endif
            }
        }
        return Control::PreNotify( _rNEvt );
    }


    bool RichTextControl::Notify( NotifyEvent& _rNEvt )
    {
        bool bDone = false;
        if ( _rNEvt.GetType() == MouseNotifyEvent::COMMAND )
        {
            const CommandEvent& rEvent = *_rNEvt.GetCommandEvent();
            bDone = m_pImpl->HandleCommand( rEvent );
        }
        return bDone || Control::Notify( _rNEvt );
    }


    void RichTextControl::Draw( OutputDevice* _pDev, const Point& _rPos, const Size& _rSize, DrawFlags _nFlags )
    {
        m_pImpl->Draw( _pDev, _rPos, _rSize, _nFlags );
    }


    EditView& RichTextControl::getView()
    {
        return *m_pImpl->getView( RichTextControlImpl::GrantAccess() );
    }


    const EditView& RichTextControl::getView() const
    {
        return *m_pImpl->getView( RichTextControlImpl::GrantAccess() );
    }


    EditEngine& RichTextControl::getEngine() const
    {
        return *m_pImpl->getEngine( RichTextControlImpl::GrantAccess() );
    }


    vcl::Window& RichTextControl::getViewport() const
    {
        return *m_pImpl->getViewport( RichTextControlImpl::GrantAccess() );
    }


    void RichTextControl::SetReadOnly( bool _bReadOnly )
    {
        m_pImpl->SetReadOnly( _bReadOnly );
    }


    bool RichTextControl::IsReadOnly() const
    {
        return m_pImpl->IsReadOnly();
    }


    void RichTextControl::SetBackgroundColor( )
    {
        m_pImpl->SetBackgroundColor( );
    }


    void RichTextControl::SetBackgroundColor( const Color& _rColor )
    {
        m_pImpl->SetBackgroundColor( _rColor );
    }


    void RichTextControl::SetHideInactiveSelection( bool _bHide )
    {
        m_pImpl->SetHideInactiveSelection( _bHide );
    }


    bool RichTextControl::GetHideInactiveSelection() const
    {
        return m_pImpl->GetHideInactiveSelection( );
    }


}   // namespace frm



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
