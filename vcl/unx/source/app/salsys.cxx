/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salsys.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:02:11 $
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

#include <salunx.h>
#include <salsys.hxx>
#include <dtint.hxx>
#include <msgbox.hxx>
#include <button.hxx>
#include <svdata.hxx>
#include <salinst.h>
#include <saldisp.hxx>
#include <salsys.h>
#include <cstdio>


SalSystem* X11SalInstance::CreateSalSystem()
{
    return new X11SalSystem();
}

// -----------------------------------------------------------------------

X11SalSystem::~X11SalSystem()
{
}

bool X11SalSystem::GetSalSystemDisplayInfo( DisplayInfo& rInfo )
{
    bool bSuccess = false;
    Display* pDisplay = XOpenDisplay( NULL );
    if( pDisplay )
    {
        int nScreen = DefaultScreen( pDisplay );
        XVisualInfo aVI;
        /*  note: SalDisplay::BestVisual does not
         *  access saldata or any other data available
         *  only after InitVCL; nor does SalOpenGL:MakeVisualWeights
         *  which gets called by SalDisplay::BestVisual.
         *  this is crucial since GetSalSystemDisplayInfo
         *  gets called BEFORE Init.
         */
        SalDisplay::BestVisual( pDisplay, nScreen, aVI );
        rInfo.nDepth    = aVI.depth;
        rInfo.nWidth    = DisplayWidth( pDisplay, nScreen );
        rInfo.nHeight   = DisplayHeight( pDisplay, nScreen );
        XCloseDisplay( pDisplay );
        bSuccess = true;
    }
    return bSuccess;
}

int X11SalSystem::ShowNativeDialog( const String& rTitle, const String& rMessage, const std::list< String >& rButtons, int nDefButton )
{
    int nRet = -1;

    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->mpIntroWindow )
        pSVData->mpIntroWindow->Hide();

    WarningBox aWarn( NULL, WB_STDWORK, rMessage );
    aWarn.SetText( rTitle );
    aWarn.Clear();

    USHORT nButton = 0;
    for( std::list< String >::const_iterator it = rButtons.begin(); it != rButtons.end(); ++it )
    {
            aWarn.AddButton( *it, nButton+1, nButton == (USHORT)nDefButton ? BUTTONDIALOG_DEFBUTTON : 0 );
            nButton++;
    }
    aWarn.SetFocusButton( (USHORT)nDefButton+1 );

    nRet = ((int)aWarn.Execute()) - 1;

    // normalize behaviour, actually this should never happen
    if( nRet < -1 || nRet >= rButtons.size() )
        nRet = -1;

    return nRet;
}

int X11SalSystem::ShowNativeMessageBox(const String& rTitle, const String& rMessage, int nButtonCombination, int nDefaultButton)
{
    int nDefButton = 0;
    std::list< String > aButtons;
    int nButtonIds[5], nBut = 0;

    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL )
    {
        aButtons.push_back( Button::GetStandardText( BUTTON_OK ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK;
    }
    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO )
    {
        aButtons.push_back( Button::GetStandardText( BUTTON_YES ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_YES;
        aButtons.push_back( Button::GetStandardText( BUTTON_NO ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO;
        if( nDefaultButton == SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO )
            nDefButton = 1;
    }
    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL )
    {
        if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL )
        {
            aButtons.push_back( Button::GetStandardText( BUTTON_RETRY ) );
            nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY;
        }
        aButtons.push_back( Button::GetStandardText( BUTTON_CANCEL ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL;
        if( nDefaultButton == SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL )
            nDefButton = aButtons.size()-1;
    }
    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_ABORT_RETRY_IGNORE )
    {
        aButtons.push_back( Button::GetStandardText( BUTTON_ABORT ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_ABORT;
        aButtons.push_back( Button::GetStandardText( BUTTON_RETRY ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY;
        aButtons.push_back( Button::GetStandardText( BUTTON_IGNORE ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_IGNORE;
        switch( nDefaultButton )
        {
            case SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY: nDefButton = 1;break;
            case SALSYSTEM_SHOWNATIVEMSGBOX_BTN_IGNORE: nDefButton = 2;break;
        }
    }
    int nResult = ShowNativeDialog( rTitle, rMessage, aButtons, nDefButton );

    return nResult != -1 ? nButtonIds[ nResult ] : 0;
}
