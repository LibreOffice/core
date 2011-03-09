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
#include "richtextvclcontrol.hxx"
#include "richtextimplcontrol.hxx"
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/languageoptions.hxx>
#if OSL_DEBUG_LEVEL > 0
    #ifndef _TOOLS_TEMPFILE_HXX
    #include <tools/tempfile.hxx>
    #endif
    #ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
    #include <unotools/ucbstreamhelper.hxx>
    #endif
    #ifndef _SV_MSGBOX_HXX
    #include <vcl/msgbox.hxx>
    #endif
    #ifndef _FILEDLGHELPER_HXX
    #include <sfx2/filedlghelper.hxx>
    #endif
    #ifndef _URLOBJ_HXX
    #include <tools/urlobj.hxx>
    #endif
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
#include <memory>

//........................................................................
namespace frm
{
//........................................................................

    using namespace ::com::sun::star::awt;

    //====================================================================
    //= RichTextControl
    //====================================================================
    //--------------------------------------------------------------------
    RichTextControl::RichTextControl( RichTextEngine* _pEngine, Window* _pParent, WinBits _nStyle,
        ITextAttributeListener* _pTextAttribListener, ITextSelectionListener* _pSelectionListener )
        :Control( _pParent, implInitStyle( _nStyle ) )
        ,m_pImpl( NULL )
    {
        implInit( _pEngine, _pTextAttribListener, _pSelectionListener );
    }

    //--------------------------------------------------------------------
    void RichTextControl::implInit( RichTextEngine* _pEngine, ITextAttributeListener* _pTextAttribListener, ITextSelectionListener* _pSelectionListener )
    {
        m_pImpl = new RichTextControlImpl( this, _pEngine, _pTextAttribListener, _pSelectionListener );
        SetCompoundControl( sal_True );
    }

    //--------------------------------------------------------------------
    RichTextControl::~RichTextControl( )
    {
        delete m_pImpl;
    }

    //--------------------------------------------------------------------
    AttributeState RichTextControl::getState( AttributeId _nAttributeId ) const
    {
        return m_pImpl->getAttributeState( _nAttributeId );
    }

    //--------------------------------------------------------------------
    void RichTextControl::executeAttribute( AttributeId _nAttributeId, const SfxPoolItem* _pArgument )
    {
        SfxItemSet aToApplyAttributes( getView().GetEmptyItemSet() );
        if ( !m_pImpl->executeAttribute( getView().GetAttribs(), aToApplyAttributes, _nAttributeId, _pArgument, m_pImpl->getSelectedScriptType() ) )
        {
            OSL_ENSURE( sal_False, "RichTextControl::executeAttribute: cannot handle the given attribute!" );
            return;
        }

        applyAttributes( aToApplyAttributes );
    }

    //--------------------------------------------------------------------
    void RichTextControl::applyAttributes( const SfxItemSet& _rAttributesToApply )
    {
        // apply
        if ( HasChildPathFocus() )
            getView().HideCursor();

        sal_Bool bOldUpdateMode = getEngine().GetUpdateMode();   // TODO: guard?
        getEngine().SetUpdateMode( sal_False );

        getView().SetAttribs( _rAttributesToApply );

        getEngine().SetUpdateMode( bOldUpdateMode );
        getView().Invalidate();

        if ( HasChildPathFocus() )
            getView().ShowCursor();

        m_pImpl->updateAllAttributes();
            // TODO: maybe we should have a list of attributes which need to be updated
            // (the handler for the just executed attribute should know)
    }

    //--------------------------------------------------------------------
    void RichTextControl::enableAttributeNotification( AttributeId _nAttributeId, ITextAttributeListener* _pListener )
    {
        m_pImpl->enableAttributeNotification( _nAttributeId, _pListener );
    }

    //--------------------------------------------------------------------
    void RichTextControl::disableAttributeNotification( AttributeId _nAttributeId )
    {
        m_pImpl->disableAttributeNotification( _nAttributeId );
    }

    //--------------------------------------------------------------------
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

    //--------------------------------------------------------------------
    void RichTextControl::Resize()
    {
        m_pImpl->layoutWindow();
        Invalidate();
    }

    //--------------------------------------------------------------------
    void RichTextControl::GetFocus()
    {
        getViewport().GrabFocus();
    }

    //--------------------------------------------------------------------
    WinBits RichTextControl::implInitStyle( WinBits nStyle )
    {
        if ( !( nStyle & WB_NOTABSTOP ) )
            nStyle |= WB_TABSTOP;
        return nStyle;
    }

    //--------------------------------------------------------------------
    void RichTextControl::StateChanged( StateChangedType _nStateChange )
    {
        if ( _nStateChange == STATE_CHANGE_STYLE )
        {
            SetStyle( implInitStyle( GetStyle() ) );
            m_pImpl->notifyStyleChanged();
        }
        else if ( _nStateChange == STATE_CHANGE_ZOOM )
        {
            m_pImpl->notifyZoomChanged();
        }
        else if ( _nStateChange == STATE_CHANGE_INITSHOW )
        {
            m_pImpl->notifyInitShow();
        }
        Control::StateChanged( _nStateChange );
    }

    //--------------------------------------------------------------------
    long RichTextControl::PreNotify( NotifyEvent& _rNEvt )
    {
        if ( IsWindowOrChild( _rNEvt.GetWindow() ) )
        {
            if ( EVENT_KEYINPUT == _rNEvt.GetType() )
            {
                const ::KeyEvent* pKeyEvent = _rNEvt.GetKeyEvent();

                sal_uInt16 nCode = pKeyEvent->GetKeyCode().GetCode();
                sal_Bool   bShift = pKeyEvent->GetKeyCode().IsShift();
                sal_Bool   bCtrl = pKeyEvent->GetKeyCode().IsMod1();
                sal_Bool   bAlt = pKeyEvent->GetKeyCode().IsMod2();
                if ( ( KEY_TAB == nCode ) && bCtrl && !bAlt )
                {
                    // Ctrl-Tab is used to step out of the control
                    // -> build a new key event without the Ctrl-key, and let the very base class handle it
                    KeyCode aNewCode( KEY_TAB, bShift, sal_False, sal_False, sal_False );
                    ::KeyEvent aNewEvent( pKeyEvent->GetCharCode(), aNewCode );
                    Control::KeyInput( aNewEvent );
                    return 1;   // handled
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

                    ::sfx2::FileDialogHelper aFP( bLoad ? com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE : com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION, 0, this );

                    for ( size_t i = 0; i < SAL_N_ELEMENTS( aExportFormats ); ++i )
                    {
                        aFP.AddFilter(
                            String::CreateFromAscii( aExportFormats[i].pDescription ),
                            String::CreateFromAscii( aExportFormats[i].pExtension ) );
                    }
                    ErrCode nResult = aFP.Execute();
                    if ( nResult == 0 )
                    {
                        String sFileName = aFP.GetPath();
                        SvStream* pStream = ::utl::UcbStreamHelper::CreateStream(
                            sFileName, ( bLoad ? STREAM_READ : STREAM_WRITE | STREAM_TRUNC ) | STREAM_SHARE_DENYALL
                        );
                        if ( pStream )
                        {
                            EETextFormat eFormat = EE_FORMAT_XML;
                            String sFilter = aFP.GetCurrentFilter();
                            for ( size_t i = 0; i < SAL_N_ELEMENTS( aExportFormats ); ++i )
                            {
                                if ( sFilter.EqualsAscii( aExportFormats[i].pDescription ) )
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
                    return 1;   // handled
                }
#endif
            }
        }
        return Control::PreNotify( _rNEvt );
    }

    //--------------------------------------------------------------------
    long RichTextControl::Notify( NotifyEvent& _rNEvt )
    {
        long nDone = 0;
        if ( _rNEvt.GetType() == EVENT_COMMAND )
        {
            const CommandEvent& rEvent = *_rNEvt.GetCommandEvent();
            nDone = m_pImpl->HandleCommand( rEvent );
        }
        return nDone ? nDone : Control::Notify( _rNEvt );
    }

    //--------------------------------------------------------------------
    void RichTextControl::Draw( OutputDevice* _pDev, const Point& _rPos, const Size& _rSize, sal_uLong _nFlags )
    {
        m_pImpl->Draw( _pDev, _rPos, _rSize, _nFlags );
    }

    //--------------------------------------------------------------------
    EditView& RichTextControl::getView()
    {
        return *m_pImpl->getView( RichTextControlImpl::GrantAccess() );
    }

    //--------------------------------------------------------------------
    const EditView& RichTextControl::getView() const
    {
        return *m_pImpl->getView( RichTextControlImpl::GrantAccess() );
    }

    //--------------------------------------------------------------------
    EditEngine& RichTextControl::getEngine() const
    {
        return *m_pImpl->getEngine( RichTextControlImpl::GrantAccess() );
    }

    //--------------------------------------------------------------------
    Window& RichTextControl::getViewport() const
    {
        return *m_pImpl->getViewport( RichTextControlImpl::GrantAccess() );
    }

    //--------------------------------------------------------------------
    void RichTextControl::SetReadOnly( bool _bReadOnly )
    {
        m_pImpl->SetReadOnly( _bReadOnly );
    }

    //--------------------------------------------------------------------
    bool RichTextControl::IsReadOnly() const
    {
        return m_pImpl->IsReadOnly();
    }

    //--------------------------------------------------------------------
    void RichTextControl::SetBackgroundColor( )
    {
        m_pImpl->SetBackgroundColor( );
    }

    //--------------------------------------------------------------------
    void RichTextControl::SetBackgroundColor( const Color& _rColor )
    {
        m_pImpl->SetBackgroundColor( _rColor );
    }

    //--------------------------------------------------------------------
    void RichTextControl::SetHideInactiveSelection( bool _bHide )
    {
        m_pImpl->SetHideInactiveSelection( _bHide );
    }

    //--------------------------------------------------------------------
    bool RichTextControl::GetHideInactiveSelection() const
    {
        return m_pImpl->GetHideInactiveSelection( );
    }

//........................................................................
}   // namespace frm
//........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
