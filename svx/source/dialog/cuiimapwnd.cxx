/*************************************************************************
 *
 *  $RCSfile: cuiimapwnd.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 16:32:36 $
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

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_HELP_HXX //autogen
#include <vcl/help.hxx>
#endif
#pragma hdrstop

#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>     // SID_ATTR_MACROITEM
#endif
#define _ANIMATION
#define ITEMID_MACRO SID_ATTR_MACROITEM
#ifndef _MACROPG_HXX //autogen
#include <sfx2/macropg.hxx>
#endif
#ifndef _GOODIES_IMAPRECT_HXX //autogen
#include <svtools/imaprect.hxx>
#endif
#ifndef _GOODIES_IMAPCIRC_HXX //autogen
#include <svtools/imapcirc.hxx>
#endif
#ifndef _GOODIES_IMAPPOLY_HXX //autogen
#include <svtools/imappoly.hxx>
#endif
#ifndef _URLBMK_HXX //autogen
#include <svtools/urlbmk.hxx>
#endif

#include <xoutbmp.hxx>
#include <dialmgr.hxx>
#include <dialogs.hrc>
#include <svxids.hrc>
#include <imapdlg.hrc>
#include <imapwnd.hxx>
#include "svdpage.hxx"
#include "svdorect.hxx"
#include "svdocirc.hxx"
#include "svdopath.hxx"
#include "xfltrit.hxx"
#include "svdpagv.hxx"

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SVX_FILLITEM_HXX //autogen
#include <xfillit.hxx>
#endif
#ifndef _SVX_XLINIIT_HXX //autogen
#include <xlineit.hxx>
#endif

#include <sot/formats.hxx>

#include "cuiimapwnd.hxx" //CHINA001

#ifdef MAC
#define TRANSCOL Color( COL_LIGHTGRAY )
#else
#define TRANSCOL Color( COL_WHITE )
#endif

/*************************************************************************
|*
|*  URLDlg
|*
\************************************************************************/

URLDlg::URLDlg( Window* pWindow, const String& rURL,
                const String& rDescription, const String& rTarget,
                const String& rName, TargetList& rTargetList ) :

    ModalDialog         ( pWindow, SVX_RES( RID_SVXDLG_IMAPURL ) ),

    aFlURL              ( this, ResId( FL_URL ) ),
    aBtnOk              ( this, ResId( BTN_OK ) ),
    aBtnCancel          ( this, ResId( BTN_CANCEL ) ),
    aFtURL1             ( this, ResId( FT_URL1 ) ),
    aEdtURL             ( this, ResId( EDT_URL ) ),
    aFtURLDescription   ( this, ResId( FT_URLDESCRIPTION ) ),
    aEdtURLDescription  ( this, ResId( EDT_URLDESCRIPTION ) ),
    aFtTarget           ( this, ResId( FT_TARGET ) ),
    aCbbTargets         ( this, ResId( CBB_TARGETS ) ),
    aFtName             ( this, ResId( FT_NAME ) ),
    aEdtName            ( this, ResId( EDT_NAME ) )

{
    FreeResource();

    aEdtURL.SetText( rURL );
    aEdtURLDescription.SetText( rDescription );
    aEdtName.SetText( rName );

    for( String* pStr = rTargetList.First(); pStr; pStr = rTargetList.Next() )
        aCbbTargets.InsertEntry( *pStr );

    if( !rTarget.Len() )
        aCbbTargets.SetText( String::CreateFromAscii( "_self" ) );
    else
        aCbbTargets.SetText( rTarget );
}
