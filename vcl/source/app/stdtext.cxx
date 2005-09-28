/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stdtext.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-28 14:36:18 $
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

#ifndef _SV_SVIDS_HRC
#include <svids.hrc>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <msgbox.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <stdtext.hxx>
#endif



// =======================================================================

XubString GetStandardText( USHORT nStdText )
{
    ResMgr* pResMgr = ImplGetResMgr();
    XubString aText;
    if( pResMgr )
        aText = XubString( ResId( nStdText-STANDARD_TEXT_FIRST+SV_STDTEXT_FIRST, pResMgr ) );
    return aText;
}

// =======================================================================

void ShowServiceNotAvailableError( Window* pParent,
                                   const XubString& rServiceName, BOOL bError )
{
    XubString aText( GetStandardText( STANDARD_TEXT_SERVICE_NOT_AVAILABLE ) );
    aText.SearchAndReplaceAscii( "%s", rServiceName );
    if ( bError )
    {
        ErrorBox aBox( pParent, WB_OK | WB_DEF_OK, aText );
        aBox.Execute();
    }
    else
    {
        WarningBox aBox( pParent, WB_OK | WB_DEF_OK, aText );
        aBox.Execute();
    }
}
