/*************************************************************************
 *
 *  $RCSfile: view0.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 10:16:36 $
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

#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _SVX_GALBRWS_HXX_
#include <svx/galbrws.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SVX_SRCHDLG_HXX //autogen
#include <svx/srchdlg.hxx>
#endif
#ifndef _SFX_TEMPLDLG_HXX //autogen
#include <sfx2/templdlg.hxx>
#endif
#ifndef _UIVWIMP_HXX
#include <uivwimp.hxx>
#endif
#ifndef _AVMEDIA_MEDIAPPLAYER_HXX
#include <avmedia/mediaplayer.hxx>
#endif

#ifndef _NAVIPI_HXX //autogen
#include <navipi.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#include "view.hxx"
#include "basesh.hxx"
#include "docsh.hxx"
#include "globals.hrc"
#include "cmdid.h"          // FN_       ...
#include "globdoc.hxx"
#include "wview.hxx"
#include "shells.hrc"

#define OLEObjects
#define SwView
#define SearchAttributes
#define ReplaceAttributes
#define SearchSettings
#define _ExecSearch ExecSearch
#define _StateSearch StateSearch
#define Frames
#define Graphics
#define Tables
#define Controls
#define GlobalContents
#define Text
#define Frame
#define Graphic
#define Object
#define Draw
#define TextDrawText
#define TextInTable
#define ListInText
#define ListInTable
#define WebTextInTable
#define WebListInText
#define WebListInTable
#define TextPage
#include "itemdef.hxx"
#include <svx/svxslots.hxx>
#include "swslots.hxx"

using namespace ::com::sun::star;
using namespace ::rtl;

#include <svtools/moduleoptions.hxx>

#define C2S(cChar) UniString::CreateFromAscii(cChar)

SFX_IMPL_VIEWFACTORY(SwView, SW_RES(STR_NONAME))
{
    if ( SvtModuleOptions().IsWriter() )
    {
        SFX_VIEW_REGISTRATION(SwDocShell);
        SFX_VIEW_REGISTRATION(SwGlobalDocShell);
    }
}

SFX_IMPL_INTERFACE( SwView, SfxViewShell, SW_RES(RID_TOOLS_TOOLBOX) )
{
    SFX_CHILDWINDOW_CONTEXT_REGISTRATION(SID_NAVIGATOR);
    SFX_CHILDWINDOW_REGISTRATION(SfxTemplateDialogWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(SvxSearchDialogWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(FN_REDLINE_ACCEPT);
    SFX_CHILDWINDOW_REGISTRATION(SID_HYPERLINK_DIALOG);
    SFX_CHILDWINDOW_REGISTRATION(GalleryChildWindow::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(::avmedia::MediaPlayer::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(FN_INSERT_FIELD_DATA_ONLY);

        SFX_FEATURED_CHILDWINDOW_REGISTRATION(FN_SYNC_LABELS, 1);
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_TOOLS|
                                SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_SERVER,
                                SW_RES(RID_TOOLS_TOOLBOX) );
}

TYPEINIT1(SwView,SfxViewShell)

/*-----------------13.12.97 11:06-------------------

--------------------------------------------------*/
ShellModes  SwView::GetShellMode()
{
    return pViewImpl->GetShellMode();
}

/*-----------------13.12.97 11:28-------------------

--------------------------------------------------*/
view::XSelectionSupplier* SwView::GetUNOObject()
{
    return pViewImpl->GetUNOObject();
}
/* -----------------------------06.05.2002 13:18------------------------------

 ---------------------------------------------------------------------------*/
void SwView::ApplyAccessiblityOptions(SvtAccessibilityOptions& rAccessibilityOptions)
{
    pWrtShell->ApplyAccessiblityOptions(rAccessibilityOptions);
    //to enable the right state of the selection cursor in readonly documents
    if(GetDocShell()->IsReadOnly())
        pWrtShell->ShowCrsr();

}
