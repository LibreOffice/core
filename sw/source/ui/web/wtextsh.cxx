/*************************************************************************
 *
 *  $RCSfile: wtextsh.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 13:12:56 $
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


#pragma hdrstop

#include "hintids.hxx"

#ifndef _SFXOBJFACE_HXX //autogen
#include <sfx2/objface.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx> //***
#endif
#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <svtools/sbxvar.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif

#include "swtypes.hxx"
#include "cmdid.h"
#include "view.hxx"
#include "wtextsh.hxx"
#include "basesh.hxx"
#include "globals.hrc"
#include "popup.hrc"
#include "shells.hrc"
#include "web.hrc"

#define C2S(cChar) UniString::CreateFromAscii(cChar)

// STATIC DATA -----------------------------------------------------------
#define SwWebTextShell
#define Paragraph
#define HyphenZone
#define TextFont
#define DropCap
#include "itemdef.hxx"
#include "swslots.hxx"


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SFX_IMPL_INTERFACE(SwWebTextShell, SwBaseShell, SW_RES(STR_SHELLNAME_WEBTEXT))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_WEB_TEXT_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_WEBTEXT_TOOLBOX));
    SFX_CHILDWINDOW_REGISTRATION(FN_EDIT_FORMULA);
    SFX_CHILDWINDOW_REGISTRATION(SID_INSERT_DIAGRAM);
    SFX_CHILDWINDOW_REGISTRATION(FN_INSERT_FIELD);
}

TYPEINIT1(SwWebTextShell, SwTextShell)

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwWebTextShell::SwWebTextShell(SwView &rView) :
    SwTextShell(rView)
{
    SetHelpId(SW_WEBTEXTSHELL);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwWebTextShell::~SwWebTextShell()
{
}



