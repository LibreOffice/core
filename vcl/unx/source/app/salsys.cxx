/*************************************************************************
 *
 *  $RCSfile: salsys.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:42:59 $
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

#include <stacktrace.hxx>
#include <salunx.h>
#include <salsys.hxx>
#include <dtint.hxx>
#include <msgbox.hxx>
#include <button.hxx>
#include <svdata.hxx>
#include <salinst.h>
#include <saldisp.hxx>
#include <cstdio>


class X11SalSystem : public SalSystem
{
public:
    X11SalSystem() {}
    virtual ~X11SalSystem();

    // overload pure virtual methods
    virtual String GetSalSummarySystemInfos( ULONG nFlags );
    virtual bool GetSalSystemDisplayInfo( System::DisplayInfo& rInfo );
    virtual int ShowNativeDialog( const String& rTitle,
                                  const String& rMessage,
                                  const std::list< String >& rButtons,
                                  int nDefButton );
    virtual int ShowNativeMessageBox( const String& rTitle,
                                      const String& rMessage,
                                      int nButtonCombination,
                                      int nDefaultButton);
};

SalSystem* X11SalInstance::CreateSalSystem()
{
    return new X11SalSystem();
}


// -----------------------------------------------------------------------

X11SalSystem::~X11SalSystem()
{
}

String X11SalSystem::GetSalSummarySystemInfos( ULONG nFlags )
{
    sal_PostMortem aPostMortem;

    /*
     *  unimplemented flags:
     *  SALSYSTEM_GETSYSTEMINFO_MODULES
     *  SALSYSTEM_GETSYSTEMINFO_MOUSEINFO
     *  SALSYSTEM_GETSYSTEMINFO_SYSTEMDIRS
     *  SALSYSTEM_GETSYSTEMINFO_LOCALVOLUMES
     */

    ByteString aRet;
    if( nFlags & SALSYSTEM_GETSYSTEMINFO_SYSTEMVERSION )
        aRet += aPostMortem.getSystemInfo();
    if( nFlags & SALSYSTEM_GETSYSTEMINFO_CPUTYPE )
        aRet += aPostMortem.getProcessorInfo();
    if( nFlags & SALSYSTEM_GETSYSTEMINFO_MEMORYINFO )
        aRet += aPostMortem.getMemoryInfo();
    if( nFlags & SALSYSTEM_GETSYSTEMINFO_STACK )
        aRet += aPostMortem.getStackTrace();
    if( nFlags & SALSYSTEM_GETSYSTEMINFO_GRAPHICSSYSTEM )
        aRet += aPostMortem.getGraphicsSystem();

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "SalSystem::GetSummarySystemInfos() =\n%s", aRet.GetBuffer() );
#endif
    return String( aRet, RTL_TEXTENCODING_ISO_8859_1 );
}

bool X11SalSystem::GetSalSystemDisplayInfo( System::DisplayInfo& rInfo )
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

    DtIntegrator* pIntegrator = DtIntegrator::CreateDtIntegrator();
    if( pIntegrator->GetDtType() == DtGNOME )
    {
        ByteString aCmdLine( "msgbox-gnome ");
        int nButton = 0;
        for( std::list< String >::const_iterator it = rButtons.begin(); it != rButtons.end(); ++it )
        {
            if( nButton == nDefButton )
                aCmdLine.Append( "-defaultbutton" );
            else
                aCmdLine.Append( "-button" );
            nButton++;
            aCmdLine.Append( " \"" );
            aCmdLine.Append( ByteString( *it, RTL_TEXTENCODING_UTF8 ) );
            aCmdLine.Append( "\" " );
        }
        aCmdLine.Append( " \"" );
        aCmdLine.Append( ByteString( rTitle, RTL_TEXTENCODING_UTF8 ) );
        aCmdLine.Append( "\" \"" );
        aCmdLine.Append( ByteString( rMessage, RTL_TEXTENCODING_UTF8 ) );
        aCmdLine.Append( "\" 2>/dev/null" );

        FILE* fp = popen( aCmdLine.GetBuffer(), "r" );
        if( fp )
        {
            ByteString aAnswer;
            char buf[16];
            while( fgets( buf, sizeof( buf ), fp ) )
            {
                aAnswer.Append( buf );
            }
            pclose( fp );
            nRet = aAnswer.ToInt32();
        }
    }
    else // default to a VCL dialogue since we do not have a native implementation
    {
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
    }

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
