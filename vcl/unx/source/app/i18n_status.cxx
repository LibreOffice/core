/*************************************************************************
 *
 *  $RCSfile: i18n_status.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:40:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif
// [ed] 6/15/02 We don't have alloca.h on OS X either.
#if !(defined FREEBSD || defined NETBSD || defined MACOSX)
#include <alloca.h>
#endif

#include <prex.h>
#include <X11/Xlib.h>
#include <XIM.h>
#include <postx.h>

#include <salunx.h>

#include <i18n_status.hxx>
#include <i18n_ic.hxx>

#include <wrkwin.hxx>
#include <fixed.hxx>
#include <menubtn.hxx>
#include <menu.hxx>
#include <svdata.hxx>
#include <svapp.hxx>
#include <saldisp.hxx>
#include <salframe.h>
#include <saldata.hxx>
#include <sysdata.hxx>

using namespace vcl;
using namespace rtl;

namespace vcl {

class StatusWindow : public WorkWindow
{
protected:
    StatusWindow( WinBits nWinBits );
public:
    virtual ~StatusWindow();

    virtual void setPosition( SalFrame* );
    virtual void setText( const String & ) = 0;
    virtual String getText() const = 0;
    virtual void show( bool bShow, I18NStatus::ShowReason eReason ) = 0;
    virtual void toggle( bool bOn );
};

}

StatusWindow::StatusWindow( WinBits nWinBits ) :
        WorkWindow( NULL, nWinBits )
{
}

StatusWindow::~StatusWindow() {}

void StatusWindow::setPosition( SalFrame* pFrame )
{
}

void StatusWindow::toggle( bool bOn )
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
    ULONG                   m_nDelayedEvent;

    Point updatePosition();
    void layout();
    bool checkLastParent() const;

    DECL_LINK( DelayedShowHdl, void* );
public:
    XIMStatusWindow();
    virtual ~XIMStatusWindow();

    virtual void setPosition( SalFrame* );
    virtual void setText( const String & );
    virtual String getText() const;
    virtual void show( bool bShow, I18NStatus::ShowReason eReason );

    // overload WorkWindow::DataChanged
    virtual void DataChanged( const DataChangedEvent& rEvt );
};

}

XIMStatusWindow::XIMStatusWindow() :
        StatusWindow( WB_BORDER | WB_SYSTEMFLOATWIN | WB_TOOLTIPWIN ),
        m_aStatusText( this, 0 ),
        m_bAnchoredAtRight( false ),
        m_pLastParent( NULL ),
        m_bDelayedShow( false ),
        m_eDelayedReason( I18NStatus::contextmap ),
        m_nDelayedEvent( 0 )
{
    layout();
}

XIMStatusWindow::~XIMStatusWindow()
{
    if( m_nDelayedEvent )
        Application::RemoveUserEvent( m_nDelayedEvent );
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
    m_aStatusText.Show( TRUE );

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
        // FIXME: remove X11SalFrame
        X11SalFrame* pFrame = GetSalData()->pFirstFrame_;
        while( pFrame )
        {
            if( pFrame == m_pLastParent )
                return true;
            pFrame = pFrame->GetNextFrame();
        }
    }
    return false;
}

void XIMStatusWindow::DataChanged( const DataChangedEvent& rEvt )
{
    m_aStatusText.SetSettings( GetSettings() );
    layout();
}

Point XIMStatusWindow::updatePosition()
{
    Point aRet;
    if( checkLastParent() )
    {
        const SystemEnvData* pEnvData = GetSystemData();
        const SystemEnvData* pParentEnvData = m_pLastParent->GetSystemData();
        SalFrame* pStatusFrame = (SalFrame*)pEnvData->pSalFrame;

        SalExtTextInputPosEvent aPosEvent;
        m_pLastParent->CallCallback( SALEVENT_EXTTEXTINPUTPOS, (void*)&aPosEvent );
        int x, y;
        XLIB_Window aChild;
        XTranslateCoordinates( (Display*)pParentEnvData->pDisplay,
                               (XLIB_Window)pParentEnvData->aShellWindow,
                               GetSalData()->GetDefDisp()->GetRootWindow(),
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
            setText( String() );
            m_pLastParent = pParent;
            Show( FALSE, SHOW_NOACTIVATE );
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

IMPL_LINK( XIMStatusWindow, DelayedShowHdl, void*, pDummy )
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
    Show( m_bDelayedShow, SHOW_NOACTIVATE );
    if( m_bDelayedShow )
    {
        XRaiseWindow( (Display*)pData->pDisplay,
                      (XLIB_Window)pData->aShellWindow );
    }
    return 0;
}

void XIMStatusWindow::show( bool bShow, I18NStatus::ShowReason eReason )
{
    if( bShow && ! m_aStatusText.GetText().Len() )
        bShow = false;

    m_bDelayedShow = bShow;
    m_eDelayedReason = eReason;
    if( ! m_nDelayedEvent )
        m_nDelayedEvent = Application::PostUserEvent( LINK( this, XIMStatusWindow, DelayedShowHdl ) );
}

void XIMStatusWindow::setText( const String& rText )
{
    m_aStatusText.SetText( rText );
    m_aWindowSize.Width() = m_aStatusText.GetTextWidth( rText )+8;
}

String XIMStatusWindow::getText() const
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

    virtual void setText( const String & );
    virtual String getText() const;
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
    SetText( String( RTL_CONSTASCII_USTRINGPARAM( "IME Status" ) ) );

    layout();

    m_aStatusBtn.SetSelectHdl( LINK( this, IIIMPStatusWindow, SelectHdl ) );
    m_aStatusBtn.SetPopupMenu( &m_aMenu );
    m_aStatusBtn.Show( TRUE );

    const ::std::vector< I18NStatus::ChoiceData >& rChoices( I18NStatus::get().getChoices() );
    int i = 1;
    for( ::std::vector< I18NStatus::ChoiceData >::const_iterator it = rChoices.begin(); it != rChoices.end(); ++it, i++ )
        m_aMenu.InsertItem( i, it->aString );

    if( pParent )
    {
        const SystemEnvData* pEnvData = GetSystemData();
        const SystemEnvData* pParentEnvData = pParent->GetSystemData();

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
    EnableAlwaysOnTop( TRUE );
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

void IIIMPStatusWindow::DataChanged( const DataChangedEvent& rEvt )
{
    m_aStatusBtn.SetSettings( GetSettings() );
    layout();
}

void IIIMPStatusWindow::setText( const String& rText )
{
    m_aStatusBtn.SetText( rText );
}

String IIIMPStatusWindow::getText() const
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
        X11SalFrame* pFrame = GetSalData()->pFirstFrame_;
        while( pFrame && pFrame != m_pResetFocus )
            pFrame = pFrame->GetNextFrame();
        if( pFrame == m_pResetFocus )
        {
            const SystemEnvData* pParentEnvData = m_pResetFocus->GetSystemData();
            SalXLib* pXLib = GetSalData()->GetDefDisp()->GetXLib();
            BOOL bIgnore = pXLib->GetIgnoreXErrors();
            pXLib->SetIgnoreXErrors( TRUE );
            XSetInputFocus( (Display*)pParentEnvData->pDisplay,
                            (XLIB_Window)pParentEnvData->aShellWindow,
                            RevertToNone,
                            CurrentTime
                            );
            XSync( (Display*)pParentEnvData->pDisplay, False );
            pXLib->SetIgnoreXErrors( bIgnore );
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
        int nIndex = m_aStatusBtn.GetCurItemId()-1;
        if( nIndex < rChoices.size() )
        {
            bool bDummy; // should always be false in this case
            XSetICValues( I18NStatus::get().getInputContext( bDummy )->GetContext(),
                          XNUnicodeCharacterSubset,
                          rChoices[nIndex].pData,
                          0);
            // FIXME: get rid of X11SalFrame
            X11SalFrame* pParent = static_cast<X11SalFrame*>(I18NStatus::get().getParent());
            if( pParent && pParent->isMapped() )
            {
                const SystemEnvData* pEnv = pParent->GetSystemData();
                SalXLib* pXLib = GetSalData()->GetDefDisp()->GetXLib();
                BOOL bIgnore = pXLib->GetIgnoreXErrors();
                pXLib->SetIgnoreXErrors( TRUE );
                XSetInputFocus( (Display*)pEnv->pDisplay,
                                (XLIB_Window)pEnv->aShellWindow,
                                RevertToNone,
                                CurrentTime
                                );
                XSync( (Display*)pEnv->pDisplay, False );
                pXLib->SetIgnoreXErrors( bIgnore );
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

void I18NStatus::free()
{
    if( pInstance )
        delete pInstance, pInstance = NULL;
}

// --------------------------------------------------------------------------

I18NStatus::I18NStatus() :
        m_pParent( NULL ),
        m_pStatusWindow( NULL ),
        m_pInputContext( NULL )
{
}

// --------------------------------------------------------------------------

I18NStatus::~I18NStatus()
{
    if( m_pInputContext )
        delete m_pInputContext, m_pInputContext = NULL;
    if( m_pStatusWindow )
        delete m_pStatusWindow, m_pStatusWindow = NULL;
    if( pInstance == this )
        pInstance = NULL;
}

// --------------------------------------------------------------------------

SalI18N_InputContext* I18NStatus::getInputContext( bool& bDeleteAfterUse )
{
    SalI18N_InputContext* pRet = NULL;
    if( m_aChoices.end() != m_aChoices.begin() )
    {
        if( ! m_pInputContext )
            m_pInputContext = new SalI18N_InputContext( m_pParent );
        bDeleteAfterUse = false;
        pRet = m_pInputContext;
    }
    else
    {
        pRet = new SalI18N_InputContext( m_pParent );
        bDeleteAfterUse = true;
    }
    return pRet;
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
            m_pStatusWindow = new XIMStatusWindow();
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

void I18NStatus::setStatusText( const String& rText )
{
    if( m_pStatusWindow )
    {
        /*
         *  #93614# convert fullwidth ASCII forms to ascii
         */
        int nChars = rText.Len()+1;
        sal_Unicode* pBuffer = (sal_Unicode*)alloca( nChars*sizeof( sal_Unicode ) );
        const sal_Unicode* pCopy = rText.GetBuffer();
        for( int i = 0; i < nChars; i++ )
        {
            if( pCopy[i] >=0xff00 && pCopy[i] <= 0xff5f )
                pBuffer[i] = (pCopy[i] & 0xff) + 0x20;
            else
                pBuffer[i] = pCopy[i];
        }
        String aText( pBuffer );
        m_pStatusWindow->setText( aText );
        m_pStatusWindow->setPosition( m_pParent );
        m_pStatusWindow->show( true, contextmap );
    }
}

// --------------------------------------------------------------------------

void I18NStatus::changeIM( const String& rIM )
{
    m_aCurrentIM = rIM;
}

// --------------------------------------------------------------------------

String I18NStatus::getStatusText() const
{
    return m_pStatusWindow ? m_pStatusWindow->getText() :  String();
}

// --------------------------------------------------------------------------

void I18NStatus::clearChoices()
{
    m_aChoices.clear();
}

// --------------------------------------------------------------------------

void I18NStatus::addChoice( const String& rChoice, void* pData )
{
    ChoiceData aData;
    aData.pData     = pData;
    aData.aString   = rChoice;
    m_aChoices.push_back( aData );
}

// --------------------------------------------------------------------------

void I18NStatus::toTop() const
{
    if( m_pStatusWindow )
    {
        const SystemEnvData* pData = m_pStatusWindow->GetSystemData();
        XRaiseWindow( (Display*)pData->pDisplay,
                      (XLIB_Window)pData->aShellWindow );
    }
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
    return m_aChoices.begin() != m_aChoices.end(); // implies IIIMP
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
