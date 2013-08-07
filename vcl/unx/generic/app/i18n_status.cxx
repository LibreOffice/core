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


#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif
#include <sal/alloca.h>

#include <prex.h>
#include <X11/Xlib.h>
#include <unx/XIM.h>
#include <postx.h>

#include <unx/salunx.h>
#include <unx/i18n_status.hxx>
#include <unx/i18n_ic.hxx>
#include <unx/saldisp.hxx>
#include <unx/salframe.h>
#include <unx/saldata.hxx>

#include <vcl/wrkwin.hxx>
#include <vcl/fixed.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>

#include <svdata.hxx>

using namespace vcl;

namespace vcl {

class StatusWindow : public WorkWindow
{
protected:
    StatusWindow( WinBits nWinBits );
public:
    virtual ~StatusWindow();

    virtual void setPosition( SalFrame* );
    virtual void setText( const OUString & ) = 0;
    virtual OUString getText() const = 0;
    virtual void show( bool bShow, I18NStatus::ShowReason eReason ) = 0;
    virtual void toggle( bool bOn ) = 0;
};

}

StatusWindow::StatusWindow( WinBits nWinBits ) :
        WorkWindow( NULL, nWinBits )
{
}

StatusWindow::~StatusWindow() {}

void StatusWindow::setPosition( SalFrame* )
{
}

// --------------------------------------------------------------------------

namespace vcl {

class XIMStatusWindow : public StatusWindow
{
    FixedText               m_aStatusText;
    SalFrame*               m_pLastParent;
    Size                    m_aWindowSize;
    bool                    m_bAnchoredAtRight;
        // true if the right edge (instead of the left edge) should stay at a
        // fixed position when re-sizing the window

    // for delayed showing
    bool                    m_bDelayedShow;
    I18NStatus::ShowReason  m_eDelayedReason;
    sal_uLong                   m_nDelayedEvent;
    // for toggling
    bool                    m_bOn;

    Point updatePosition();
    void layout();
    bool checkLastParent() const;

    DECL_LINK( DelayedShowHdl, void* );
public:
    XIMStatusWindow( bool bOn );
    virtual ~XIMStatusWindow();

    virtual void setPosition( SalFrame* );
    virtual void setText( const OUString & );
    virtual OUString getText() const;
    virtual void show( bool bShow, I18NStatus::ShowReason eReason );
    virtual void toggle( bool bOn );

    // overload WorkWindow::DataChanged
    virtual void DataChanged( const DataChangedEvent& rEvt );
};

}

XIMStatusWindow::XIMStatusWindow( bool bOn ) :
        StatusWindow( WB_BORDER | WB_SYSTEMFLOATWIN | WB_TOOLTIPWIN ),
        m_aStatusText( this, 0 ),
        m_pLastParent( NULL ),
        m_bAnchoredAtRight( false ),
        m_bDelayedShow( false ),
        m_eDelayedReason( I18NStatus::contextmap ),
        m_nDelayedEvent( 0 ),
        m_bOn( bOn )
{
    layout();
}

XIMStatusWindow::~XIMStatusWindow()
{
    if( m_nDelayedEvent )
        Application::RemoveUserEvent( m_nDelayedEvent );
}

void XIMStatusWindow::toggle( bool bOn )
{
    m_bOn = bOn;
    show( bOn, I18NStatus::contextmap );
}

void XIMStatusWindow::layout()
{
    m_aWindowSize.Width() = m_aStatusText.GetTextWidth( m_aStatusText.GetText() )+8;
    Font aFont( m_aStatusText.GetFont() );
    m_aWindowSize.Height() = aFont.GetHeight()+10;
    m_aWindowSize = LogicToPixel( m_aWindowSize );

    Size aControlSize( m_aWindowSize );
    aControlSize.Width()  -= 4;
    aControlSize.Height() -= 4;

    m_aStatusText.SetPosSizePixel( Point( 1, 1 ), aControlSize );
    m_aStatusText.SetFont( aFont );
    m_aStatusText.Show( sal_True );

    if (m_bAnchoredAtRight && IsVisible())
    {
        SalFrame* pFrame = (SalFrame*)GetSystemData()->pSalFrame;
        long nDelta = pFrame->maGeometry.nWidth - m_aWindowSize.Width();
        pFrame->SetPosSize( pFrame->maGeometry.nX + nDelta,
                            pFrame->maGeometry.nY,
                            m_aWindowSize.Width(),
                            m_aWindowSize.Height(),
                            SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
    }
    else
        SetOutputSizePixel( m_aWindowSize );
}

bool XIMStatusWindow::checkLastParent() const
{
    if( m_pLastParent )
    {
        const std::list< SalFrame* >& rFrames = GetGenericData()->GetSalDisplay()->getFrames();
        for( std::list< SalFrame* >::const_iterator it = rFrames.begin(); it != rFrames.end(); ++it )
        {
            if( *it == m_pLastParent )
                return true;
        }
    }
    return false;
}

void XIMStatusWindow::DataChanged( const DataChangedEvent& )
{
    m_aStatusText.SetSettings( GetSettings() );
    layout();
}

Point XIMStatusWindow::updatePosition()
{
    Point aRet;
    if( checkLastParent() )
    {
        const SystemEnvData* pParentEnvData = m_pLastParent->GetSystemData();

        SalExtTextInputPosEvent aPosEvent;
        m_pLastParent->CallCallback( SALEVENT_EXTTEXTINPUTPOS, (void*)&aPosEvent );
        int x, y;
        XLIB_Window aChild;
        XTranslateCoordinates( (Display*)pParentEnvData->pDisplay,
                               (XLIB_Window)pParentEnvData->aShellWindow,
                               GetGenericData()->GetSalDisplay()->GetRootWindow( GetGenericData()->GetSalDisplay()->GetDefaultXScreen() ),
                               0, 0,
                               &x, &y,
                               &aChild );

        // TODO:  Currently, place the status window to the (physical) left of
        // the cursor iff in vertical mode (assuming that the columns in
        // vertical mode are always written from right to left, this causes the
        // status window to keep out of the text already written).  This
        // heuristic would break if there is ever a vertical mode in which the
        // columns are written from left to right.  Also, more elaborate
        // positioning for (both horizontal and vertical) left-to-right and
        // right-to-left text would be possible.
        bool bLeft = aPosEvent.mbVertical;
            // true if status window is to the left of the cursor

        int const nGap = 4; // between cursor and status window
        if (aPosEvent.mbVertical)
        {
            aRet.X() = x + aPosEvent.mnX + (bLeft
                                            ? -m_aWindowSize.Width() - nGap
                                            : aPosEvent.mnHeight + nGap);
            aRet.Y() = y + aPosEvent.mnY;
        }
        else
        {
            aRet.X() = x + aPosEvent.mnX + (bLeft ? -m_aWindowSize.Width() : 0);
            aRet.Y() = y + aPosEvent.mnY+aPosEvent.mnHeight + nGap;
        }

        m_bAnchoredAtRight = bLeft;
    }
    return aRet;
}

void XIMStatusWindow::setPosition( SalFrame* pParent )
{
    if( pParent )
    {
        if( pParent != m_pLastParent )
        {
            setText( OUString() );
            m_pLastParent = pParent;
            Show( sal_False, SHOW_NOACTIVATE );
        }
        if( IsVisible() )
        {
            const SystemEnvData* pEnvData = GetSystemData();
            SalFrame* pStatusFrame = (SalFrame*)pEnvData->pSalFrame;
            Point aPoint = updatePosition();
            pStatusFrame->SetPosSize( aPoint.X(), aPoint.Y(), m_aWindowSize.Width(), m_aWindowSize.Height(), SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
        }
    }
}

IMPL_LINK_NOARG(XIMStatusWindow, DelayedShowHdl)
{
    m_nDelayedEvent = 0;
    const SystemEnvData* pData = GetSystemData();
    SalFrame* pStatusFrame = (SalFrame*)pData->pSalFrame;
    if( m_bDelayedShow )
    {
        Size aControlSize( m_aWindowSize.Width()-4, m_aWindowSize.Height()-4 );
        m_aStatusText.SetPosSizePixel( Point( 1, 1 ), aControlSize );
        Point aPoint = updatePosition();
        pStatusFrame->SetPosSize( aPoint.X(), aPoint.Y(), m_aWindowSize.Width(), m_aWindowSize.Height(), SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
    }
    Show( m_bDelayedShow && m_bOn, SHOW_NOACTIVATE );
    if( m_bDelayedShow )
    {
        XRaiseWindow( (Display*)pData->pDisplay,
                      (XLIB_Window)pData->aShellWindow );
    }
    return 0;
}

void XIMStatusWindow::show( bool bShow, I18NStatus::ShowReason eReason )
{
    if( bShow && m_aStatusText.GetText().isEmpty() )
        bShow = false;

    m_bDelayedShow = bShow;
    m_eDelayedReason = eReason;
    if( ! m_nDelayedEvent )
        m_nDelayedEvent = Application::PostUserEvent( LINK( this, XIMStatusWindow, DelayedShowHdl ) );
}

void XIMStatusWindow::setText( const OUString& rText )
{
    m_aStatusText.SetText( rText );
    m_aWindowSize.Width() = m_aStatusText.GetTextWidth( rText )+8;
}

OUString XIMStatusWindow::getText() const
{
    return m_aStatusText.GetText();
}

// --------------------------------------------------------------------------

namespace vcl {

class IIIMPStatusWindow : public StatusWindow
{
    MenuButton              m_aStatusBtn;
    PopupMenu               m_aMenu;
    SalFrame*               m_pResetFocus;
    bool                    m_bShow;
    bool                    m_bOn;

    DECL_LINK( SelectHdl, MenuButton* );

    void show();

public:
    IIIMPStatusWindow( SalFrame* pParent, bool bOn ); // for initial position
    virtual ~IIIMPStatusWindow();

    virtual void setText( const OUString & );
    virtual OUString getText() const;
    virtual void show( bool bShow, I18NStatus::ShowReason eReason );
    virtual void toggle( bool bOn );
    void layout();

    // overload Window focus handler
    virtual void        GetFocus();
    // overload WorkWindow::DataChanged
    virtual void DataChanged( const DataChangedEvent& rEvt );
};

}

IIIMPStatusWindow::IIIMPStatusWindow( SalFrame* pParent, bool bOn ) :
        StatusWindow( WB_MOVEABLE ),
        m_aStatusBtn( this, WB_BORDER ),
        m_pResetFocus( pParent ),
        m_bShow( true ),
        m_bOn( bOn )
{
    SetText( OUString( "IME Status" ) );

    layout();

    m_aStatusBtn.SetSelectHdl( LINK( this, IIIMPStatusWindow, SelectHdl ) );
    m_aStatusBtn.SetPopupMenu( &m_aMenu );
    m_aStatusBtn.Show( sal_True );

    const ::std::vector< I18NStatus::ChoiceData >& rChoices( I18NStatus::get().getChoices() );
    int i = 1;
    for( ::std::vector< I18NStatus::ChoiceData >::const_iterator it = rChoices.begin(); it != rChoices.end(); ++it, i++ )
        m_aMenu.InsertItem( i, it->aString );

    if( pParent )
    {
        const SystemEnvData* pEnvData = GetSystemData();

        const SalFrameGeometry& rGeom( pParent->GetUnmirroredGeometry() );
        int nDistance = rGeom.nTopDecoration;
        if( nDistance < 20 )
            nDistance = 20;
        XMoveWindow( (Display*)pEnvData->pDisplay,
                     (XLIB_Window)pEnvData->aShellWindow,
                     rGeom.nX,
                     rGeom.nY + rGeom.nHeight + nDistance
                     );
    }
#if OSL_DEBUG_LEVEL > 1
    else
        fprintf( stderr, "Warning: could not reposition status window since no frame\n" );
#endif
    EnableAlwaysOnTop( sal_True );
}

IIIMPStatusWindow::~IIIMPStatusWindow()
{
}

void IIIMPStatusWindow::layout()
{
    Font aFont( m_aStatusBtn.GetFont() );
    Size aSize( 15*aFont.GetHeight(), aFont.GetHeight()+14 );
    aSize = m_aStatusBtn.LogicToPixel( aSize );

    m_aStatusBtn.SetPosSizePixel( Point( 0, 0 ), aSize );
    SetOutputSizePixel( aSize );
    if( IsVisible() )
        Invalidate();
}

void IIIMPStatusWindow::DataChanged( const DataChangedEvent& )
{
    m_aStatusBtn.SetSettings( GetSettings() );
    layout();
}

void IIIMPStatusWindow::setText( const OUString& rText )
{
    m_aStatusBtn.SetText( rText );
}

OUString IIIMPStatusWindow::getText() const
{
    return m_aStatusBtn.GetText();
}

void IIIMPStatusWindow::show( bool bShow, I18NStatus::ShowReason eReason )
{
    // hide IIIMPStatusWindow only in presentations
    if( ! bShow
        && eReason != I18NStatus::presentation
        )
        return;

    m_bShow = bShow;
    show();
}

void IIIMPStatusWindow::toggle( bool bOn )
{
    if (bOn != m_bOn)
    {
        m_bOn = bOn;
        show();
    }
}

void IIIMPStatusWindow::show()
{
    if (m_bOn && m_bShow && !IsVisible())
        m_pResetFocus = I18NStatus::get().getParent();
    Show(m_bOn && m_bShow);
}

void IIIMPStatusWindow::GetFocus()
{
    /*
     *  this is here just to put the focus back to the application
     *  window at startup on clickToFocus WMs
     */
    WorkWindow::GetFocus();
    if( m_pResetFocus )
    {
        /*
         *  look if reset focus still exists
         *  since reset focus really is an internal hack there should
         *  not be a method to be called in SalFrame destructor
         */
        const std::list< SalFrame* >& rFrames = GetGenericData()->GetSalDisplay()->getFrames();
        std::list< SalFrame* >::const_iterator it;
        for( it = rFrames.begin(); it != rFrames.end() && *it != m_pResetFocus; ++it )
            ;
        if( it != rFrames.end() )
        {
            const SystemEnvData* pParentEnvData = m_pResetFocus->GetSystemData();
            GetGenericData()->ErrorTrapPush();
            XSetInputFocus( (Display*)pParentEnvData->pDisplay,
                            (XLIB_Window)pParentEnvData->aShellWindow,
                            RevertToNone,
                            CurrentTime
                            );
            XSync( (Display*)pParentEnvData->pDisplay, False );
            GetGenericData()->ErrorTrapPop();
        }
        m_pResetFocus = NULL;
    }
}

// --------------------------------------------------------------------------

IMPL_LINK( IIIMPStatusWindow, SelectHdl, MenuButton*, pBtn )
{
    if( pBtn == & m_aStatusBtn )
    {
        const ::std::vector< I18NStatus::ChoiceData >& rChoices( I18NStatus::get().getChoices() );
        unsigned int nIndex = m_aStatusBtn.GetCurItemId()-1;
        if( nIndex < rChoices.size() )
        {
            XSetICValues( static_cast<X11SalFrame*>(I18NStatus::get().getParent())->getInputContext()->GetContext(),
                          XNUnicodeCharacterSubset,
                          rChoices[nIndex].pData,
                          NULL);
            // FIXME: get rid of X11SalFrame
            X11SalFrame* pParent = static_cast<X11SalFrame*>(I18NStatus::get().getParent());
            if( pParent && pParent->isMapped() )
            {
                const SystemEnvData* pEnv = pParent->GetSystemData();
                GetGenericData()->ErrorTrapPush();
                XSetInputFocus( (Display*)pEnv->pDisplay,
                                (XLIB_Window)pEnv->aShellWindow,
                                RevertToNone,
                                CurrentTime
                                );
                XSync( (Display*)pEnv->pDisplay, False );
                GetGenericData()->ErrorTrapPop();
            }
        }
    }
    return 0;
}

/*
 *  I18NStatus
 */

I18NStatus* I18NStatus::pInstance = NULL;

I18NStatus& I18NStatus::get()
{
    if( ! pInstance )
        pInstance = new I18NStatus();
    return *pInstance;
}

// --------------------------------------------------------------------------

bool I18NStatus::exists()
{
    return pInstance != NULL;
}

// --------------------------------------------------------------------------

void I18NStatus::free()
{
    if( pInstance )
        delete pInstance, pInstance = NULL;
}

// --------------------------------------------------------------------------

I18NStatus::I18NStatus() :
        m_pParent( NULL ),
        m_pStatusWindow( NULL )
{
}

// --------------------------------------------------------------------------

I18NStatus::~I18NStatus()
{
    if( m_pStatusWindow )
        delete m_pStatusWindow, m_pStatusWindow = NULL;
    if( pInstance == this )
        pInstance = NULL;
}

// --------------------------------------------------------------------------

void I18NStatus::setParent( SalFrame* pParent )
{
    m_pParent = pParent;
    if( ! m_pStatusWindow )
    {
        bool bIIIMPmode = m_aChoices.begin() != m_aChoices.end();
        if( bIIIMPmode )
            m_pStatusWindow = new IIIMPStatusWindow( pParent,
                                                     getStatusWindowMode() );
        else
            m_pStatusWindow = new XIMStatusWindow( getStatusWindowMode() );
        setStatusText( m_aCurrentIM );
    }
    m_pStatusWindow->setPosition( m_pParent );
}

// --------------------------------------------------------------------------

void I18NStatus::show( bool bShow, ShowReason eReason )
{
    if( m_pStatusWindow )
    {
        m_pStatusWindow->setPosition( m_pParent );
        m_pStatusWindow->show( bShow, eReason );
    }
}

// --------------------------------------------------------------------------

void I18NStatus::setStatusText( const OUString& rText )
{
    if( m_pStatusWindow )
    {
        /*
         *  #93614# convert fullwidth ASCII forms to ascii
         */
        int nChars = rText.getLength()+1;
        sal_Unicode* pBuffer = (sal_Unicode*)alloca( nChars*sizeof( sal_Unicode ) );
        for( int i = 0; i < nChars; i++ )
        {
            if( rText[i] >=0xff00 && rText[i] <= 0xff5f )
                pBuffer[i] = (rText[i] & 0xff) + 0x20;
            else
                pBuffer[i] = rText[i];
        }
        OUString aText( pBuffer );
        m_pStatusWindow->setText( aText );
        m_pStatusWindow->setPosition( m_pParent );

        bool bVisible = true;
        if( m_pParent )
        {
            long w, h;
            m_pParent->GetClientSize( w, h );
            if( w == 0 || h == 0 )
            {
                bVisible = false;
            }
        }

        m_pStatusWindow->show( bVisible, contextmap );
    }
}

// --------------------------------------------------------------------------

void I18NStatus::changeIM( const OUString& rIM )
{
    m_aCurrentIM = rIM;
}

// --------------------------------------------------------------------------

SalFrame* I18NStatus::getStatusFrame() const
{
    SalFrame* pRet = NULL;
    if( m_pStatusWindow )
    {
        const SystemEnvData* pData = m_pStatusWindow->GetSystemData();
        pRet = (SalFrame*)pData->pSalFrame;
    }
    return pRet;
}

bool I18NStatus::canToggleStatusWindow() const
{
    return true;
}

void I18NStatus::toggleStatusWindow()
{
    if (m_pStatusWindow != 0)
        m_pStatusWindow->toggle(getStatusWindowMode());
}

bool I18NStatus::getStatusWindowMode()
{
    switch (ImplGetSVData()->maAppData.meShowImeStatusWindow)
    {
    default: // ImplSVAppData::ImeStatusWindowMode_UNKNOWN
        return Application::GetShowImeStatusWindowDefault();
    case ImplSVAppData::ImeStatusWindowMode_HIDE:
        return false;
    case ImplSVAppData::ImeStatusWindowMode_SHOW:
        return true;
    }
}

/*
 * X11ImeStatus
 */
X11ImeStatus::~X11ImeStatus()
{
    vcl::I18NStatus::free();
}

bool X11ImeStatus::canToggle()
{
    return vcl::I18NStatus::get().canToggleStatusWindow();
}

void X11ImeStatus::toggle()
{
    vcl::I18NStatus::get().toggleStatusWindow();
}

SalI18NImeStatus* X11SalInstance::CreateI18NImeStatus()
{
    return new X11ImeStatus();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
