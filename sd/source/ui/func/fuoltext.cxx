/*************************************************************************
 *
 *  $RCSfile: fuoltext.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:35 $
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

#ifndef _OUTLINER_HXX
#include <svx/outliner.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#define ITEMID_FIELD    EE_FEATURE_FIELD
#ifndef _SVX_FLDITEM_HXX //autogen
#include <svx/flditem.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif

#include <svx/svxids.hrc>
#include "app.hrc"
#include "fuoltext.hxx"
#include "outlview.hxx"
#include "sdwindow.hxx"
#include "docshell.hxx"
#include "viewshel.hxx"

#include <stdio.h>          // Fuer SlotFilter-Listing

static USHORT SidArray[] = {
                SID_STYLE_FAMILY2,
                SID_STYLE_FAMILY5,
                SID_STYLE_UPDATE_BY_EXAMPLE,
                SID_CUT,
                SID_COPY,
                SID_PASTE,
                SID_SELECTALL,
                SID_ATTR_CHAR_FONT,
                SID_ATTR_CHAR_POSTURE,
                SID_ATTR_CHAR_WEIGHT,
                SID_ATTR_CHAR_UNDERLINE,
                SID_ATTR_CHAR_FONTHEIGHT,
                SID_ATTR_CHAR_COLOR,
                SID_OUTLINE_UP,
                SID_OUTLINE_DOWN,
                SID_OUTLINE_LEFT,
                SID_OUTLINE_RIGHT,
                //SID_OUTLINE_FORMAT,
                SID_OUTLINE_COLLAPSE_ALL,
                //SID_OUTLINE_BULLET,
                SID_OUTLINE_COLLAPSE,
                SID_OUTLINE_EXPAND_ALL,
                SID_OUTLINE_EXPAND,
                SID_SET_SUPER_SCRIPT,
                SID_SET_SUB_SCRIPT,
                SID_HYPERLINK_GETLINK,
                SID_PRESENTATION_TEMPLATES,
                SID_STATUS_PAGE,
                SID_STATUS_LAYOUT,
                SID_EXPAND_PAGE,
                SID_SUMMARY_PAGE,
                SID_PARASPACE_INCREASE,
                SID_PARASPACE_DECREASE,
                0 };

TYPEINIT1( FuOutlineText, FuOutline );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuOutlineText::FuOutlineText(SdViewShell* pViewShell, SdWindow* pWindow,
                             SdView* pView, SdDrawDocument* pDoc,
                             SfxRequest& rReq)
       : FuOutline(pViewShell, pWindow, pView, pDoc, rReq)
{
//    ERSTELLT SLOTFILTER-LISTING
//    FILE* pFile = fopen("menu.dat", "w");
//    fprintf(pFile, "SID_STYLE_FAMILY2,               %6d\n", SID_STYLE_FAMILY2);
//    fprintf(pFile, "SID_STYLE_FAMILY5,               %6d\n", SID_STYLE_FAMILY5);
//    fprintf(pFile, "SID_STYLE_UPDATE_BY_EXAMPLE,     %6d\n", SID_STYLE_UPDATE_BY_EXAMPLE);
//    fprintf(pFile, "SID_CUT,                            %6d\n", SID_CUT);
//    fprintf(pFile, "SID_COPY,                           %6d\n", SID_COPY);
//    fprintf(pFile, "SID_PASTE,                          %6d\n", SID_PASTE);
//    fprintf(pFile, "SID_SELECTALL,                      %6d\n", SID_SELECTALL);
//    fprintf(pFile, "SID_ATTR_CHAR_FONT,                 %6d\n", SID_ATTR_CHAR_FONT);
//    fprintf(pFile, "SID_ATTR_CHAR_POSTURE,              %6d\n", SID_ATTR_CHAR_POSTURE);
//    fprintf(pFile, "SID_ATTR_CHAR_WEIGHT,               %6d\n", SID_ATTR_CHAR_WEIGHT);
//    fprintf(pFile, "SID_ATTR_CHAR_UNDERLINE,            %6d\n", SID_ATTR_CHAR_UNDERLINE);
//    fprintf(pFile, "SID_ATTR_CHAR_FONTHEIGHT,           %6d\n", SID_ATTR_CHAR_FONTHEIGHT);
//    fprintf(pFile, "SID_ATTR_CHAR_COLOR,                %6d\n", SID_ATTR_CHAR_COLOR);
//    fprintf(pFile, "SID_OUTLINE_UP,                     %6d\n", SID_OUTLINE_UP);
//    fprintf(pFile, "SID_OUTLINE_DOWN,                   %6d\n", SID_OUTLINE_DOWN);
//    fprintf(pFile, "SID_OUTLINE_LEFT,                   %6d\n", SID_OUTLINE_LEFT);
//    fprintf(pFile, "SID_OUTLINE_RIGHT,                  %6d\n", SID_OUTLINE_RIGHT);
//    fprintf(pFile, "SID_OUTLINE_COLLAPSE_ALL,           %6d\n", SID_OUTLINE_COLLAPSE_ALL);
//    fprintf(pFile, "SID_OUTLINE_COLLAPSE,               %6d\n", SID_OUTLINE_COLLAPSE);
//    fprintf(pFile, "SID_OUTLINE_EXPAND_ALL,             %6d\n", SID_OUTLINE_EXPAND_ALL);
//    fprintf(pFile, "SID_OUTLINE_EXPAND,                 %6d\n", SID_OUTLINE_EXPAND);
//    fprintf(pFile, "SID_SET_SUPER_SCRIPT,               %6d\n", SID_SET_SUPER_SCRIPT);
//    fprintf(pFile, "SID_SET_SUB_SCRIPT,                 %6d\n", SID_SET_SUB_SCRIPT);
//    fprintf(pFile, "SID_PRESENTATION_TEMPLATES,         %6d\n", SID_PRESENTATION_TEMPLATES);
//    fprintf(pFile, "SID_STATUS_PAGE,                    %6d\n", SID_STATUS_PAGE);
//    fprintf(pFile, "SID_STATUS_LAYOUT,                  %6d\n", SID_STATUS_LAYOUT);
//    fprintf(pFile, "SID_HYPERLINK_GETLINK,              %6d\n", SID_HYPERLINK_GETLINK);
//    fclose(pFile);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuOutlineText::~FuOutlineText()
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL FuOutlineText::MouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;

    pWindow->GrabFocus();

    bReturn = pOutlineView->GetViewByWindow(pWindow)->MouseButtonDown(rMEvt);

    if (bReturn)
    {
        // Attributierung der akt. Textstelle kann jetzt anders sein
        SFX_BINDINGS().Invalidate( SidArray );
    }
    else
    {
        bReturn = FuOutline::MouseButtonDown(rMEvt);
    }

    return (bReturn);
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL FuOutlineText::MouseMove(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;

    bReturn = pOutlineView->GetViewByWindow(pWindow)->MouseMove(rMEvt);

    if (!bReturn)
    {
        bReturn = FuOutline::MouseMove(rMEvt);
    }

    const SvxFieldItem* pFieldItem = pOutlineView->GetViewByWindow( pWindow )->
                                        GetFieldUnderMousePointer();
    const SvxFieldData* pField = NULL;
    if( pFieldItem )
        pField = pFieldItem->GetField();

    if( pField && pField->ISA( SvxURLField ) )
    {
       pWindow->SetPointer( Pointer( POINTER_REFHAND ) );
    }
    else
       pWindow->SetPointer( Pointer( POINTER_TEXT ) );

    return (bReturn);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuOutlineText::MouseButtonUp(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;

    bReturn = pOutlineView->GetViewByWindow(pWindow)->MouseButtonUp(rMEvt);

    if (bReturn)
    {
        // Attributierung der akt. Textstelle kann jetzt anders sein
        SFX_BINDINGS().Invalidate( SidArray );
    }
    else
    {
        const SvxFieldItem* pFieldItem = pOutlineView->GetViewByWindow( pWindow )->GetFieldUnderMousePointer();
        if( pFieldItem )
        {
            const SvxFieldData* pField = pFieldItem->GetField();

            if( pField && pField->ISA( SvxURLField ) )
            {
                bReturn = TRUE;
                pWindow->ReleaseMouse();
                SfxStringItem aStrItem( SID_FILE_NAME, ( (SvxURLField*) pField)->GetURL() );
                SfxStringItem aReferer( SID_REFERER, pDocSh->GetMedium()->GetName() );
                SfxBoolItem aBrowseItem( SID_BROWSING, TRUE );
                SfxViewFrame* pFrame = pViewShell->GetViewFrame();

                if ( rMEvt.IsMod1() )
                {
                    // Im neuen Frame oeffnen
                    pFrame->GetDispatcher()->Execute(SID_OPENDOC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                &aStrItem, &aBrowseItem, &aReferer, 0L);
                }
                else
                {
                    // Im aktuellen Frame oeffnen
                    SfxFrameItem aFrameItem( SID_DOCFRAME, pFrame );
                    pFrame->GetDispatcher()->Execute(SID_OPENDOC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                &aStrItem, &aFrameItem, &aBrowseItem, &aReferer, 0L);
                }
            }
        }
    }

    if( !bReturn )
        bReturn = FuOutline::MouseButtonUp(rMEvt);

    return (bReturn);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL FuOutlineText::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FALSE;

    if( !pDocSh->IsReadOnly() ||
        rKEvt.GetKeyCode().GetGroup() == KEYGROUP_CURSOR )
    {
        pWindow->GrabFocus();

        bReturn = pOutlineView->GetViewByWindow(pWindow)->PostKeyEvent(rKEvt);

        if (bReturn)
        {
            // Attributierung der akt. Textstelle kann jetzt anders sein
            SFX_BINDINGS().Invalidate( SidArray );
        }
        else
        {
            bReturn = FuOutline::KeyInput(rKEvt);
        }
    }

    return (bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuOutlineText::Activate()
{
    FuOutline::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuOutlineText::Deactivate()
{
    FuOutline::Deactivate();
}

/*************************************************************************
|*
|* Cut object to clipboard
|*
\************************************************************************/

void FuOutlineText::DoCut()
{
    pOutlineView->GetViewByWindow(pWindow)->Cut();
}

/*************************************************************************
|*
|* Copy object to clipboard
|*
\************************************************************************/

void FuOutlineText::DoCopy()
{
    pOutlineView->GetViewByWindow(pWindow)->Copy();
}

/*************************************************************************
|*
|* Paste object from clipboard
|*
\************************************************************************/

void FuOutlineText::DoPaste()
{
    pOutlineView->GetViewByWindow(pWindow)->PasteSpecial();
}




