/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: richtextimplcontrol.cxx,v $
 * $Revision: 1.8 $
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
#include "richtextimplcontrol.hxx"
#include "textattributelistener.hxx"
#include "richtextengine.hxx"
#include <svx/editeng.hxx>
#include <svx/editview.hxx>
#include <svx/eeitem.hxx>
#include <svx/editstat.hxx>
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <svx/scripttypeitem.hxx>

#include <svx/editobj.hxx>
#include <svtools/itempool.hxx>
#include <svtools/itemset.hxx>
#include <vcl/mapunit.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>

#include <memory>

#define EMPTY_PAPER_SIZE    0x7FFFFFFF

//........................................................................
namespace frm
{
//........................................................................
    //====================================================================
    //= RichTextControlImpl
    //====================================================================
    //--------------------------------------------------------------------
    RichTextControlImpl::RichTextControlImpl( Control* _pAntiImpl, RichTextEngine* _pEngine, ITextAttributeListener* _pTextAttrListener, ITextSelectionListener* _pSelectionListener )
        :m_pAntiImpl            ( _pAntiImpl          )
        ,m_pViewport            ( NULL                )
        ,m_pHScroll             ( NULL                )
        ,m_pVScroll             ( NULL                )
        ,m_pScrollCorner        ( NULL                )
        ,m_pEngine              ( _pEngine            )
        ,m_pView                ( NULL                )
        ,m_pTextAttrListener    ( _pTextAttrListener  )
        ,m_pSelectionListener   ( _pSelectionListener )
        ,m_bHasEverBeenShown    ( false               )
    {
        OSL_ENSURE( m_pAntiImpl, "RichTextControlImpl::RichTextControlImpl: invalid window!" );
        OSL_ENSURE( m_pEngine,   "RichTextControlImpl::RichTextControlImpl: invalid edit engine! This will *definately* crash!" );

        m_pViewport = new RichTextViewPort( m_pAntiImpl );
        m_pViewport->setAttributeInvalidationHandler( LINK( this, RichTextControlImpl, OnInvalidateAllAttributes ) );
        m_pViewport->Show();

        // ensure that both the window and the reference device have the same map unit
        MapMode aRefDeviceMapMode( m_pEngine->GetRefDevice()->GetMapMode() );
        m_pAntiImpl->SetMapMode( aRefDeviceMapMode );
        m_pViewport->SetMapMode( aRefDeviceMapMode );

        m_pView = new EditView( m_pEngine, m_pViewport );
        m_pEngine->InsertView( m_pView );
        m_pViewport->setView( *m_pView );

        m_pEngine->registerEngineStatusListener( this );

        {
            ULONG nViewControlWord = m_pView->GetControlWord();
            nViewControlWord |= EV_CNTRL_AUTOSCROLL;
            m_pView->SetControlWord( nViewControlWord );
        }

        // ensure that it's initially scrolled to the upper left
        m_pView->SetVisArea( Rectangle( Point( ), m_pViewport->GetOutputSize() ) );

        ensureScrollbars();

        m_pAntiImpl->SetBackground( Wallpaper( m_pAntiImpl->GetSettings().GetStyleSettings().GetFieldColor() ) );
    }

    //--------------------------------------------------------------------
    RichTextControlImpl::~RichTextControlImpl( )
    {
        m_pEngine->RemoveView( m_pView );
        m_pEngine->revokeEngineStatusListener( this );
        delete m_pView;
        delete m_pViewport;
        delete m_pHScroll;
        delete m_pVScroll;
        delete m_pScrollCorner;
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::implUpdateAttribute( AttributeHandlerPool::const_iterator _pHandler )
    {
        if  (  ( _pHandler->first == SID_ATTR_CHAR_WEIGHT )
            || ( _pHandler->first == SID_ATTR_CHAR_POSTURE )
            || ( _pHandler->first == SID_ATTR_CHAR_FONT )
            || ( _pHandler->first == SID_ATTR_CHAR_FONTHEIGHT )
            )
        {
            // these are attributes whose value depends on the current script type.
            // I.e., in real, there are *three* items in the ItemSet: One for each script
            // type (Latin, Asian, Complex). However, if we have an observer who is interested
            // in the state of this attribute, we have to kind of *merge* the three attributes
            // to only one.
            // This is usefull in case the observer is for instance a toolbox which contains only
            // an, e.g., "bold" slot, and thus not interested in the particular script type of the
            // current selection.
            SvxScriptSetItem aNormalizedSet( (WhichId)_pHandler->first, *m_pView->GetAttribs().GetPool() );
            normalizeScriptDependentAttribute( aNormalizedSet );

            implCheckUpdateCache( _pHandler->first, _pHandler->second->getState( aNormalizedSet.GetItemSet() ) );
        }
        else
            implCheckUpdateCache( _pHandler->first, _pHandler->second->getState( m_pView->GetAttribs() ) );
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::updateAttribute( AttributeId _nAttribute )
    {
        AttributeHandlerPool::const_iterator pHandler = m_aAttributeHandlers.find( _nAttribute );
        if ( pHandler != m_aAttributeHandlers.end() )
            implUpdateAttribute( pHandler );
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::updateAllAttributes( )
    {
        for (   AttributeHandlerPool::const_iterator pHandler = m_aAttributeHandlers.begin();
                pHandler != m_aAttributeHandlers.end();
                ++pHandler
            )
        {
            implUpdateAttribute( pHandler );
        }

        // notify changes of the selection, if necessary
        if ( m_pSelectionListener && m_pView )
        {
            ESelection aCurrentSelection = m_pView->GetSelection();
            if ( !aCurrentSelection.IsEqual( m_aLastKnownSelection ) )
            {
                m_aLastKnownSelection = aCurrentSelection;
                m_pSelectionListener->onSelectionChanged( m_aLastKnownSelection );
            }
        }
    }

    //--------------------------------------------------------------------
    AttributeState RichTextControlImpl::getAttributeState( AttributeId _nAttributeId ) const
    {
        StateCache::const_iterator aCachedStatePos = m_aLastKnownStates.find( _nAttributeId );
        if ( aCachedStatePos == m_aLastKnownStates.end() )
        {
            OSL_ENSURE( sal_False, "RichTextControlImpl::getAttributeState: Don't ask for the state of an attribute which I never encountered!" );
            return AttributeState( eIndetermined );
        }
        return aCachedStatePos->second;
    }

    //--------------------------------------------------------------------
    bool RichTextControlImpl::executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rAttribs, AttributeId _nAttribute, const SfxPoolItem* _pArgument, ScriptType _nForScriptType )
    {
        // let's see whether we have a handler for this attribute
        AttributeHandlerPool::const_iterator aHandlerPos = m_aAttributeHandlers.find( _nAttribute );
        if ( aHandlerPos != m_aAttributeHandlers.end() )
        {
            aHandlerPos->second->executeAttribute( _rCurrentAttribs, _rAttribs, _pArgument, _nForScriptType );
            return true;
        }
        return false;
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::enableAttributeNotification( AttributeId _nAttributeId, ITextAttributeListener* _pListener )
    {
        AttributeHandlerPool::const_iterator aHandlerPos = m_aAttributeHandlers.find( _nAttributeId  );
        if ( aHandlerPos == m_aAttributeHandlers.end() )
        {
            ::rtl::Reference< IAttributeHandler > aHandler = AttributeHandlerFactory::getHandlerFor( _nAttributeId, *m_pEngine->GetEmptyItemSet().GetPool() );
            OSL_ENSURE( aHandler.is(), "RichTextControlImpl::enableAttributeNotification: no handler available for this attribute!" );
            if ( !aHandler.is() )
                return;
            OSL_POSTCOND( _nAttributeId == aHandler->getAttributeId(), "RichTextControlImpl::enableAttributeNotification: suspicious handler!" );

            aHandlerPos = m_aAttributeHandlers.insert( AttributeHandlerPool::value_type( _nAttributeId , aHandler ) ).first;
        }

        // remember the listener
        if ( _pListener )
            m_aAttributeListeners.insert( AttributeListenerPool::value_type( _nAttributeId, _pListener ) );

        // update (and broadcast) the state of this attribute
        updateAttribute( _nAttributeId );
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::disableAttributeNotification( AttributeId _nAttributeId )
    {
        // forget the handler for this attribute
        AttributeHandlerPool::iterator aHandlerPos = m_aAttributeHandlers.find( _nAttributeId );
        if ( aHandlerPos != m_aAttributeHandlers.end() )
            m_aAttributeHandlers.erase( aHandlerPos );

        // as well as the listener
        AttributeListenerPool::iterator aListenerPos = m_aAttributeListeners.find( _nAttributeId );
        if ( aListenerPos != m_aAttributeListeners.end() )
            m_aAttributeListeners.erase( aListenerPos );
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::normalizeScriptDependentAttribute( SvxScriptSetItem& _rScriptSetItem )
    {
        _rScriptSetItem.GetItemSet().Put( m_pView->GetAttribs(), FALSE );
        const SfxPoolItem* pNormalizedItem = _rScriptSetItem.GetItemOfScript( getSelectedScriptType() );

        WhichId nNormalizedWhichId = _rScriptSetItem.GetItemSet().GetPool()->GetWhich( _rScriptSetItem.Which() );
        if ( pNormalizedItem )
        {
            SfxPoolItem* pProperWhich = pNormalizedItem->Clone();
            pProperWhich->SetWhich( nNormalizedWhichId );
            _rScriptSetItem.GetItemSet().Put( *pProperWhich );
            DELETEZ( pProperWhich );
        }
        else
            _rScriptSetItem.GetItemSet().InvalidateItem( nNormalizedWhichId );
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::implCheckUpdateCache( AttributeId _nAttribute, const AttributeState& _rState )
    {
        StateCache::iterator aCachePos = m_aLastKnownStates.find( _nAttribute );
        if ( aCachePos == m_aLastKnownStates.end() )
        {   // nothing known about this attribute, yet
            m_aLastKnownStates.insert( StateCache::value_type( _nAttribute, _rState ) );
        }
        else
        {
            if ( aCachePos->second == _rState )
            {
                // nothing to do
                return;
            }
            aCachePos->second = _rState;
        }

        // is there a dedicated listener for this particular attribute?
        AttributeListenerPool::const_iterator aListenerPos = m_aAttributeListeners.find( _nAttribute );
        if ( aListenerPos != m_aAttributeListeners.end( ) )
            aListenerPos->second->onAttributeStateChanged( _nAttribute, _rState );

        // call our global listener, if there is one
        if ( m_pTextAttrListener )
            m_pTextAttrListener->onAttributeStateChanged( _nAttribute, _rState );
    }

    //--------------------------------------------------------------------
    ScriptType RichTextControlImpl::getSelectedScriptType() const
    {
        ScriptType nScript = m_pView->GetSelectedScriptType();
        if ( !nScript )
            nScript = SvtLanguageOptions::GetScriptTypeOfLanguage( Application::GetSettings().GetLanguage() );
        return nScript;
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::EditEngineStatusChanged( const EditStatus& _rStatus )
    {
        ULONG nStatusWord( _rStatus.GetStatusWord() );
        if  (   ( nStatusWord & EE_STAT_TEXTWIDTHCHANGED )
            ||  ( nStatusWord & EE_STAT_TEXTHEIGHTCHANGED )
            )
        {
            if ( ( nStatusWord & EE_STAT_TEXTHEIGHTCHANGED ) && windowHasAutomaticLineBreak() )
                m_pEngine->SetPaperSize( Size( m_pEngine->GetPaperSize().Width(), m_pEngine->GetTextHeight() ) );

            updateScrollbars();
        }

        bool bHScroll = 0 != ( nStatusWord & EE_STAT_HSCROLL );
        bool bVScroll = 0 != ( nStatusWord & EE_STAT_VSCROLL );

        // In case of *no* automatic line breaks, we also need to check for the *range* here.
        // Normally, we would do this only after a EE_STAT_TEXTWIDTHCHANGED. However, due to a bug
        // in the EditEngine (I believe so) this is not fired when the engine does not have
        // the AutoPaperSize bits set.
        // So in order to be properly notified, we would need the AutoPaperSize. But, with
        // AutoPaperSize, other things do not work anymore: Either, when we set a MaxAutoPaperSize,
        // then the view does automatic soft line breaks at the paper end - which we definately do
        // want. Or, if we did not set a MaxAutoPaperSize, then the view does not automatically scroll
        // anymore in horizontal direction.
        // So this is some kind of lose-lose situation ... :(
        if ( !windowHasAutomaticLineBreak() && bHScroll )
        {
            updateScrollbars();
            return;
        }

        if ( bHScroll && m_pHScroll )
            m_pHScroll->SetThumbPos( m_pView->GetVisArea().Left() );
        if ( bVScroll && m_pVScroll )
            m_pVScroll->SetThumbPos( m_pView->GetVisArea().Top() );
    }

    //--------------------------------------------------------------------
    IMPL_LINK( RichTextControlImpl, OnInvalidateAllAttributes, void*, /*_pNotInterestedIn*/ )
    {
        updateAllAttributes();
        return 0L;
    }

    //--------------------------------------------------------------------
    IMPL_LINK( RichTextControlImpl, OnHScroll, ScrollBar*, _pScrollbar )
    {
        m_pView->Scroll( -_pScrollbar->GetDelta(), 0, RGCHK_PAPERSZ1 );
        return 0L;
    }

    //--------------------------------------------------------------------
    IMPL_LINK( RichTextControlImpl, OnVScroll, ScrollBar*, _pScrollbar )
    {
        m_pView->Scroll( 0, -_pScrollbar->GetDelta(), RGCHK_PAPERSZ1 );
        return 0L;
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::ensureScrollbars()
    {
        bool bNeedVScroll = 0 != ( m_pAntiImpl->GetStyle() & WB_VSCROLL );
        bool bNeedHScroll = 0 != ( m_pAntiImpl->GetStyle() & WB_HSCROLL );

        if ( ( bNeedVScroll == hasVScrollBar() ) && ( bNeedHScroll == hasHScrollBar( ) ) )
            // nothing to do
            return;

        // create or delete the scrollbars, as necessary
        if ( !bNeedVScroll )
        {
            delete m_pVScroll;
            m_pVScroll = NULL;
        }
        else
        {
            m_pVScroll = new ScrollBar( m_pAntiImpl, WB_VSCROLL | WB_DRAG | WB_REPEAT );
            m_pVScroll->SetScrollHdl ( LINK( this, RichTextControlImpl, OnVScroll ) );
            m_pVScroll->Show();
        }

        if ( !bNeedHScroll )
        {
            delete m_pHScroll;
            m_pHScroll = NULL;
        }
        else
        {
            m_pHScroll = new ScrollBar( m_pAntiImpl, WB_HSCROLL | WB_DRAG | WB_REPEAT );
            m_pHScroll->SetScrollHdl ( LINK( this, RichTextControlImpl, OnHScroll ) );
            m_pHScroll->Show();
        }

        if ( m_pHScroll && m_pVScroll )
        {
            delete m_pScrollCorner;
            m_pScrollCorner = new ScrollBarBox( m_pAntiImpl );
            m_pScrollCorner->Show();
        }
        else
        {
            delete m_pScrollCorner;
            m_pScrollCorner = NULL;
        }

        layoutWindow();
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::ensureLineBreakSetting()
    {
        if ( !windowHasAutomaticLineBreak() )
            m_pEngine->SetPaperSize( Size( EMPTY_PAPER_SIZE, EMPTY_PAPER_SIZE ) );

        layoutWindow();
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::layoutWindow()
    {
        if ( !m_bHasEverBeenShown )
            // no need to do anything. Especially, no need to set the paper size on the
            // EditEngine to anything ....
            return;

        const StyleSettings& rStyleSettings = m_pAntiImpl->GetSettings().GetStyleSettings();

        long nScrollBarWidth = m_pVScroll ? rStyleSettings.GetScrollBarSize() : 0;
        long nScrollBarHeight = m_pHScroll ? rStyleSettings.GetScrollBarSize() : 0;

        if ( m_pAntiImpl->IsZoom() )
        {
            nScrollBarWidth = m_pAntiImpl->CalcZoom( nScrollBarWidth );
            nScrollBarHeight = m_pAntiImpl->CalcZoom( nScrollBarHeight );
        }

        // the overall size we can use
        Size aPlaygroundSizePixel( m_pAntiImpl->GetOutputSizePixel() );

        // the size of the viewport - note that the viewport does *not* occupy all the place
        // which is left when subtracting the scrollbar width/height
        Size aViewportPlaygroundPixel( aPlaygroundSizePixel );
        aViewportPlaygroundPixel.Width() = ::std::max( long( 10 ), long( aViewportPlaygroundPixel.Width() - nScrollBarWidth ) );
        aViewportPlaygroundPixel.Height() = ::std::max( long( 10 ), long( aViewportPlaygroundPixel.Height() - nScrollBarHeight ) );
        Size aViewportPlaygroundLogic( m_pViewport->PixelToLogic( aViewportPlaygroundPixel ) );

        const long nOffset = 2;
        Size aViewportSizePixel( aViewportPlaygroundPixel.Width() - 2 * nOffset, aViewportPlaygroundPixel.Height() - 2 * nOffset );
        Size aViewportSizeLogic( m_pViewport->PixelToLogic( aViewportSizePixel ) );

        // position the viewport
        m_pViewport->SetPosSizePixel( Point( nOffset, nOffset ), aViewportSizePixel );
        // position the scrollbars
        if ( m_pVScroll )
            m_pVScroll->SetPosSizePixel( Point( aViewportPlaygroundPixel.Width(), 0 ), Size( nScrollBarWidth, aViewportPlaygroundPixel.Height() ) );
        if ( m_pHScroll )
            m_pHScroll->SetPosSizePixel( Point( 0, aViewportPlaygroundPixel.Height() ), Size( aViewportPlaygroundPixel.Width(), nScrollBarHeight ) );
        if ( m_pScrollCorner )
            m_pScrollCorner->SetPosSizePixel( Point( aViewportPlaygroundPixel.Width(), aViewportPlaygroundPixel.Height() ), Size( nScrollBarWidth, nScrollBarHeight ) );

        // paper size
        if ( windowHasAutomaticLineBreak() )
            m_pEngine->SetPaperSize( Size( aViewportSizeLogic.Width(), m_pEngine->GetTextHeight() ) );

        // output area of the view
        m_pView->SetOutputArea( Rectangle( Point( ), aViewportSizeLogic ) );
        m_pView->SetVisArea( Rectangle( Point( ), aViewportSizeLogic ) );

        if ( m_pVScroll )
        {
            m_pVScroll->SetVisibleSize( aViewportPlaygroundLogic.Height() );

            // the default height of a text line ....
            long nFontHeight = m_pEngine->GetStandardFont(0).GetSize().Height();
            // ... is the scroll size for the vertical scrollbar
            m_pVScroll->SetLineSize( nFontHeight );
            // the viewport width, minus one line, is the page scroll size
            m_pVScroll->SetPageSize( ::std::max( nFontHeight, aViewportPlaygroundLogic.Height() - nFontHeight ) );
        }

        // the font width
        if ( m_pHScroll )
        {
            m_pHScroll->SetVisibleSize( aViewportPlaygroundLogic.Width() );

            long nFontWidth = m_pEngine->GetStandardFont(0).GetSize().Width();
            if ( !nFontWidth )
            {
                m_pViewport->Push( PUSH_FONT );
                m_pViewport->SetFont( m_pEngine->GetStandardFont(0) );
                nFontWidth = m_pViewport->GetTextWidth( String( RTL_CONSTASCII_USTRINGPARAM( "x" ) ) );
                m_pViewport->Pop();
            }
            // ... is the scroll size for the horizontal scrollbar
            m_pHScroll->SetLineSize( 5 * nFontWidth );
            // the viewport height, minus one character, is the page scroll size
            m_pHScroll->SetPageSize( ::std::max( nFontWidth, aViewportPlaygroundLogic.Width() - nFontWidth ) );
        }

        // update range and position of the scrollbars
        updateScrollbars();
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::updateScrollbars()
    {
        if ( m_pVScroll )
        {
            long nOverallTextHeight = m_pEngine->GetTextHeight();
            m_pVScroll->SetRange( Range( 0, nOverallTextHeight ) );
            m_pVScroll->SetThumbPos( m_pView->GetVisArea().Top() );
        }

        if ( m_pHScroll )
        {
            Size aPaperSize( m_pEngine->GetPaperSize() );
            long nOverallTextWidth = ( aPaperSize.Width() == EMPTY_PAPER_SIZE ) ? m_pEngine->CalcTextWidth() : aPaperSize.Width();
            m_pHScroll->SetRange( Range( 0, nOverallTextWidth ) );
            m_pHScroll->SetThumbPos( m_pView->GetVisArea().Left() );
        }
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::notifyInitShow()
    {
        if ( !m_bHasEverBeenShown )
        {
            m_bHasEverBeenShown = true;
            layoutWindow();
        }
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::notifyStyleChanged()
    {
        ensureScrollbars();
        ensureLineBreakSetting();
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::notifyZoomChanged()
    {
        const Fraction& rZoom = m_pAntiImpl->GetZoom();

        MapMode aMapMode( m_pAntiImpl->GetMapMode() );
        aMapMode.SetScaleX( rZoom );
        aMapMode.SetScaleY( rZoom );
        m_pAntiImpl->SetMapMode( aMapMode );

        m_pViewport->SetZoom( rZoom );
        m_pViewport->SetMapMode( aMapMode );

        layoutWindow();
    }

    //--------------------------------------------------------------------
    bool RichTextControlImpl::windowHasAutomaticLineBreak()
    {
        return ( m_pAntiImpl->GetStyle() & WB_WORDBREAK ) != 0;
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::SetReadOnly( bool _bReadOnly )
    {
        m_pView->SetReadOnly( _bReadOnly );
    }

    //--------------------------------------------------------------------
    bool RichTextControlImpl::IsReadOnly() const
    {
        return m_pView->IsReadOnly( );
    }

    //--------------------------------------------------------------------
    namespace
    {
        static void lcl_inflate( Rectangle& _rRect, long _nInflateX, long _nInflateY )
        {
            _rRect.Left() -= _nInflateX;
            _rRect.Right() += _nInflateX;
            _rRect.Top() -= _nInflateY;
            _rRect.Bottom() += _nInflateY;
        }
    }
    //--------------------------------------------------------------------
    long RichTextControlImpl::HandleCommand( const CommandEvent& _rEvent )
    {
        if (  ( _rEvent.GetCommand() == COMMAND_WHEEL )
           || ( _rEvent.GetCommand() == COMMAND_STARTAUTOSCROLL )
           || ( _rEvent.GetCommand() == COMMAND_AUTOSCROLL )
           )
        {
            m_pAntiImpl->HandleScrollCommand( _rEvent, m_pHScroll, m_pVScroll );
            return 1;
        }
        return 0;
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::Draw( OutputDevice* _pDev, const Point& _rPos, const Size& _rSize, ULONG /*_nFlags*/ )
    {
        // need to normalize the map mode of the device - every paint operation on any device needs
        // to use the same map mode
        _pDev->Push( PUSH_MAPMODE | PUSH_LINECOLOR | PUSH_FILLCOLOR );

        // enforce our "normalize map mode" on the device
        MapMode aRefMapMode( m_pEngine->GetRefDevice()->GetMapMode() );
        MapMode aOriginalMapMode( _pDev->GetMapMode() );
        MapMode aNormalizedMapMode( aRefMapMode.GetMapUnit(), aRefMapMode.GetOrigin(), aOriginalMapMode.GetScaleX(), aOriginalMapMode.GetScaleY() );
        _pDev->SetMapMode( aNormalizedMapMode );

        // translate coordinates
        Point aPos( OutputDevice::LogicToLogic( _rPos, aOriginalMapMode, aNormalizedMapMode ) );
        Size aSize( OutputDevice::LogicToLogic( _rSize, aOriginalMapMode, aNormalizedMapMode ) );

        Rectangle aPlayground( aPos, aSize );
        Size aOnePixel( _pDev->PixelToLogic( Size( 1, 1 ) ) );

        // background
        _pDev->SetLineColor();
        _pDev->DrawRect( Rectangle( aPlayground.TopLeft(), m_pEngine->GetPaperSize()) );

        // possibly with border
        bool bBorder = ( m_pAntiImpl->GetStyle() & WB_BORDER );
        if ( bBorder )
            // let's draw a border
            _pDev->SetLineColor( COL_BLACK );
        else
            _pDev->SetLineColor();
        _pDev->SetFillColor( m_pAntiImpl->GetBackground().GetColor() );
        _pDev->DrawRect( aPlayground );

        if ( bBorder )
            // don't draw the text over the border
            lcl_inflate( aPlayground, -aOnePixel.Width(), -aOnePixel.Height() );

        // leave a space of one pixel between the "surroundings" of the control
        // and the content
        lcl_inflate( aPlayground, -aOnePixel.Width(), -aOnePixel.Height() );

        m_pEngine->Draw( _pDev, aPlayground, Point(), TRUE );

        _pDev->Pop();
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::SetBackgroundColor( )
    {
        SetBackgroundColor( Application::GetSettings().GetStyleSettings().GetFieldColor() );
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::SetBackgroundColor( const Color& _rColor )
    {
        Wallpaper aWallpaper( _rColor );
        m_pAntiImpl->SetBackground( aWallpaper );
        m_pViewport->SetBackground( aWallpaper );
    }

    //--------------------------------------------------------------------
    void RichTextControlImpl::SetHideInactiveSelection( bool _bHide )
    {
        m_pViewport->SetHideInactiveSelection( _bHide );
    }

    //--------------------------------------------------------------------
    bool RichTextControlImpl::GetHideInactiveSelection() const
    {
        return m_pViewport->GetHideInactiveSelection( );
    }

//........................................................................
}   // namespace frm
//........................................................................

