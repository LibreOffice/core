/*************************************************************************
 *
 *  $RCSfile: dlgchar.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 15:43:20 $
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

#define ITEMID_FONTLIST     SID_ATTR_CHAR_FONTLIST

#include <svx/dialogs.hrc>
#include <svx/flstitem.hxx>
//CHINA001 #ifndef _SVX_CHARDLG_HXX //autogen
//CHINA001 #include <svx/chardlg.hxx>
//CHINA001 #endif
#include <svx/flagsdef.hxx> //CHINA001
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif

#include "sdresid.hxx"
#include "dlg_char.hxx"
#include <svx/svxids.hrc> //CHINA001
#ifndef _SFXINTITEM_HXX //CHINA001
#include <svtools/intitem.hxx> //CHINA001
#endif //CHINA001

/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

SdCharDlg::SdCharDlg( Window* pParent, const SfxItemSet* pAttr,
                    const SfxObjectShell* pDocShell ) :
        SfxTabDialog        ( pParent, SdResId( TAB_CHAR ), pAttr ),
        rOutAttrs           ( *pAttr ),
        rDocShell           ( *pDocShell )
{
    FreeResource();

    AddTabPage( RID_SVXPAGE_CHAR_NAME ); //CHINA001 AddTabPage( RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, 0 );
    AddTabPage( RID_SVXPAGE_CHAR_EFFECTS ); //CHINA001 AddTabPage( RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_CHAR_POSITION ); //CHINA001 AddTabPage( RID_SVXPAGE_CHAR_POSITION, SvxCharPositionPage::Create, 0 );
}

// -----------------------------------------------------------------------

void SdCharDlg::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool())); //CHINA001
    switch( nId )
    {
        case RID_SVXPAGE_CHAR_NAME:
        {
            SvxFontListItem aItem(*( (const SvxFontListItem*)
                ( rDocShell.GetItem( SID_ATTR_CHAR_FONTLIST) ) ) );

            //CHINA001 ( (SvxCharNamePage&) rPage ).SetFontList( aItem );
            aSet.Put (SvxFontListItem( aItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
            rPage.PageCreated(aSet);
        }
        break;

        case RID_SVXPAGE_CHAR_EFFECTS:
            //CHINA001 ( (SvxCharEffectsPage&) rPage ).DisableControls( DISABLE_CASEMAP );
            aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP)); //CHINA001
            rPage.PageCreated(aSet);
            break;

        default:
        break;
    }
}



