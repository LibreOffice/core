/*************************************************************************
 *
 *  $RCSfile: drtxtob.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:43 $
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

#ifndef _EEITEMID_HXX
#include <svx/eeitemid.hxx>
#endif
#ifndef _ULSPITEM_HXX
#include <svx/ulspitem.hxx>
#endif
#ifndef _LSPCITEM_HXX
#include <svx/lspcitem.hxx>
#endif
#ifndef _ADJITEM_HXX
#include <svx/adjitem.hxx>
#endif
#ifndef _SV_FONTTYPE_HXX //autogen
#include <vcl/fonttype.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _SFX_TPLPITEM_HXX //autogen
#include <sfx2/tplpitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVDOUTL_HXX //autogen
#include <svx/svdoutl.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif

#pragma hdrstop


#include "app.hrc"
#include "glob.hrc"
#include "res_bmp.hrc"

#include "eetext.hxx"
#include "drtxtob.hxx"
#include "drawdoc.hxx"
#include "drviewsh.hxx"
#include "outlnvsh.hxx"
#include "futempl.hxx"
#include "sdresid.hxx"
#include "sdwindow.hxx"
#include "outlview.hxx"



/*************************************************************************
|*
|* Standardinterface deklarieren (Die Slotmap darf nicht leer sein, also
|* tragen wir etwas ein, was hier (hoffentlich) nie vorkommt).
|*
\************************************************************************/

#define SdDrawTextObjectBar
#define FEATURE_DRAW_TEXT_OBJECTBAR     1L
#define FEATURE_GRAPHIC_TEXT_OBJECTBAR  2L

SFX_DECL_TYPE(13);

#include "sdslots.hxx"

SFX_IMPL_INTERFACE( SdDrawTextObjectBar, SfxShell, SdResId(STR_TEXTOBJECTBARSHELL) )
{
//    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT, SdResId(RID_DRAW_TEXT_TOOLBOX) );
    SFX_FEATURED_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT, SdResId(RID_DRAW_TEXT_TOOLBOX),
                                         FEATURE_DRAW_TEXT_OBJECTBAR );
    SFX_FEATURED_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT, SdResId(RID_GRAPHIC_TEXT_TOOLBOX),
                                         FEATURE_GRAPHIC_TEXT_OBJECTBAR );
}

TYPEINIT1( SdDrawTextObjectBar, SfxShell );

/*************************************************************************
|*
|* Standard-Konstruktor
|*
\************************************************************************/

SdDrawTextObjectBar::SdDrawTextObjectBar( SdViewShell* pSdViewSh,
                SfxItemPool& rItemPool, SdView* pSdView ) :
    SfxShell(pSdViewSh),
    rPool( rItemPool ),
    pViewShell( pSdViewSh ),
    pView( pSdView )
{
    SetPool(&rItemPool);

    if (pSdViewSh->ISA(SdOutlineViewShell))
    {
        SfxUndoManager& rUndoMgr = ((SdOutlineView*)pSdView)->
                                    GetOutliner()->GetUndoManager();
        SetUndoManager(&rUndoMgr);
    }
    else
    {
        SdDrawDocument* pDoc      = pView->GetDoc();
        SdDrawDocShell* pDocShell = pDoc->GetDocSh();
        SfxUndoManager* pUndoMgr  = pDocShell->GetUndoManager();
        SetUndoManager(pUndoMgr);
        if ( pSdViewSh->ISA(SdDrawViewShell) )
            SetRepeatTarget(pSdView);
    }

    SetName( String( RTL_CONSTASCII_USTRINGPARAM( "SdDrawTextObjectBar" )));

    // SetHelpId( SD_IF_SDDRAWTEXTOBJECTBAR );
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SdDrawTextObjectBar::~SdDrawTextObjectBar()
{
    SetRepeatTarget(NULL);
}

/*************************************************************************
|*
|* Status der Attribut-Items
|*
\************************************************************************/

void SdDrawTextObjectBar::GetAttrState( SfxItemSet& rSet )
{
    SfxWhichIter    aIter( rSet );
    USHORT          nWhich = aIter.FirstWhich();
    BOOL            bTemplate = FALSE;

    while ( nWhich )
    {
        USHORT nSlotId = SfxItemPool::IsWhich(nWhich)
            ? GetPool().GetSlotId(nWhich)
            : nWhich;

        switch ( nSlotId )
        {
            case SID_STYLE_APPLY:
            case SID_STYLE_FAMILY2:
            {
                SfxStyleSheet* pStyleSheet = pView->GetStyleSheetFromMarked();
                if( pStyleSheet )
                    rSet.Put( SfxTemplateItem( nWhich, pStyleSheet->GetName() ) );
                else
                {
                    rSet.Put( SfxTemplateItem( nWhich, String() ) );
                }
                bTemplate = TRUE;
            }
            break;

            case SID_OUTLINE_LEFT:
            case SID_OUTLINE_RIGHT:
            case SID_OUTLINE_UP:
            case SID_OUTLINE_DOWN:
            {
                BOOL bDisableLeft     = TRUE;
                BOOL bDisableRight    = TRUE;
                BOOL bDisableUp       = TRUE;
                BOOL bDisableDown     = TRUE;
                OutlinerView* pOLV = pView->GetTextEditOutlinerView();

                if (pView->ISA(SdOutlineView))
                {
                    pOLV = ((SdOutlineView*) pView)->GetViewByWindow(
                                                 pViewShell->GetActiveWindow());
                }

                BOOL bOutlineViewSh = pViewShell->ISA(SdOutlineViewShell);

                if (pOLV &&
                    ( pOLV->GetOutliner()->GetMode() == OUTLINERMODE_OUTLINEOBJECT || bOutlineViewSh ) )
                {
                    // Outliner im Gliederungsmodus
                    Outliner* pOutl = pOLV->GetOutliner();
                    List* pList = pOLV->CreateSelectionList();
                    Paragraph* pPara = (Paragraph*) pList->First();

                    if ( pOutl->GetAbsPos(pPara) > 0 && pView->GetDoc()->GetSdPageCount( PK_STANDARD ) > 1 )
                    {
                        // Nicht ganz oben
                        bDisableUp = FALSE;
                    }

                    USHORT nMinDepth = 0;

                    if (pViewShell->ISA(SdDrawViewShell))
                    {
                        nMinDepth = 1;
                    }

                    while (pPara)
                    {
                        USHORT nDepth = pOutl->GetDepth( pOutl->GetAbsPos( pPara ) );

                        if (nDepth > nMinDepth)
                        {
                            // Nicht minimale Tiefe
                            bDisableLeft = FALSE;
                        }

                        if (nDepth < 9 &&
                            ( pOutl->GetAbsPos(pPara) != 0 || !bOutlineViewSh ) )
                        {
                            // Nicht maximale Tiefe und nicht ganz oben
                            bDisableRight = FALSE;
                        }

                        pPara = (Paragraph*) pList->Next();
                    }

                    if ( ( pOutl->GetAbsPos((Paragraph*) pList->Last()) < pOutl->GetParagraphCount() - 1 ) &&
                         ( pOutl->GetParagraphCount() > 1 || !bOutlineViewSh) )
                    {
                        // Nicht letzter Absatz
                        bDisableDown = FALSE;
                    }

                    delete pList;
                }

                if (bDisableLeft)
                    rSet.DisableItem(SID_OUTLINE_LEFT);
                if (bDisableRight)
                    rSet.DisableItem(SID_OUTLINE_RIGHT);
                if (bDisableUp)
                    rSet.DisableItem(SID_OUTLINE_UP);
                if (bDisableDown)
                    rSet.DisableItem(SID_OUTLINE_DOWN);
            }
            break;

            default:
            break;
        }

        nWhich = aIter.NextWhich();
    }

    SfxItemSet aAttrSet( pView->GetDoc()->GetPool() );
    pView->GetAttributes( aAttrSet );

    rSet.Put( aAttrSet, FALSE ); // <- FALSE, damit DontCare-Status uebernommen wird

/*
    FontUnderline eFU = ( (const SvxUnderlineItem&) rAttrSet.Get( ITEMID_UNDERLINE ) ).GetUnderline();
    rSet.Put( SvxUnderlineItem( SID_ATTR_CHAR_UNDERLINE, eFU == UNDERLINE_SINGLE ? TRUE : FALSE ) );
*/

    // die sind im Gliederungsmodus disabled
    if (!pViewShell->ISA(SdDrawViewShell))
    {
        rSet.DisableItem( SID_ATTR_PARA_ADJUST_LEFT );
        rSet.DisableItem( SID_ATTR_PARA_ADJUST_RIGHT );
        rSet.DisableItem( SID_ATTR_PARA_ADJUST_CENTER );
        rSet.DisableItem( SID_ATTR_PARA_ADJUST_BLOCK );
        rSet.DisableItem( SID_ATTR_PARA_LINESPACE_10 );
        rSet.DisableItem( SID_ATTR_PARA_LINESPACE_15 );
        rSet.DisableItem( SID_ATTR_PARA_LINESPACE_20 );
        rSet.DisableItem( SID_PARASPACE_INCREASE );
        rSet.DisableItem( SID_PARASPACE_DECREASE );
    }
    else
    {
        // Absatzabstand
        OutlinerView* pOLV = pView->GetTextEditOutlinerView();
        if( pOLV )
        {
            ESelection aSel = pOLV->GetSelection();
            aSel.Adjust();
            ULONG nStartPara = aSel.nStartPara;
            ULONG nEndPara = aSel.nEndPara;
            if( !aSel.HasRange() )
            {
                nStartPara = 0;
                nEndPara = pOLV->GetOutliner()->GetParagraphCount() - 1;
            }
            long nUpper = 0L;
            for( ULONG nPara = nStartPara; nPara <= nEndPara; nPara++ )
            {
                const SfxItemSet& rItems = pOLV->GetOutliner()->GetParaAttribs( nPara );
                const SvxULSpaceItem& rItem = (const SvxULSpaceItem&) rItems.Get( ITEMID_ULSPACE );
                nUpper = Max( nUpper, (long)rItem.GetUpper() );
            }
            if( nUpper == 0L )
                rSet.DisableItem( SID_PARASPACE_DECREASE );
        }
        else
        {
            // Wird zur Zeit nie disabled !
            //rSet.DisableItem( SID_PARASPACE_INCREASE );
            //rSet.DisableItem( SID_PARASPACE_DECREASE );
        }

        // Absatzausrichtung
        SvxAdjust eAdj = ( (const SvxAdjustItem&) aAttrSet.Get( ITEMID_ADJUST ) ).GetAdjust();
        switch( eAdj )
        {
            case SVX_ADJUST_LEFT:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_LEFT, TRUE ) );
            break;
            case SVX_ADJUST_CENTER:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_CENTER, TRUE ) );
            break;
            case SVX_ADJUST_RIGHT:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_RIGHT, TRUE ) );
            break;
            case SVX_ADJUST_BLOCK:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_ADJUST_BLOCK, TRUE ) );
            break;
        }

        if (aAttrSet.GetItemState(EE_PARA_BULLETSTATE) == SFX_ITEM_ON)
        {
            SfxUInt16Item aBulletState((const SfxUInt16Item&) aAttrSet.Get(EE_PARA_BULLETSTATE));

            if (aBulletState.GetValue() != 0)
            {
                rSet.Put(SfxBoolItem(FN_NUM_BULLET_ON, TRUE));
            }
            else
            {
                rSet.Put(SfxBoolItem(FN_NUM_BULLET_ON, FALSE));
            }
        }

        USHORT nLineSpace = (USHORT) ( (const SvxLineSpacingItem&) aAttrSet.
                            Get( ITEMID_LINESPACING ) ).GetPropLineSpace();
        switch( nLineSpace )
        {
            case 100:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_LINESPACE_10, TRUE ) );
            break;
            case 150:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_LINESPACE_15, TRUE ) );
            break;
            case 200:
                rSet.Put( SfxBoolItem( SID_ATTR_PARA_LINESPACE_20, TRUE ) );
            break;
        }
    }

    // Ausrichtung (hoch/tief) wird auch im Gliederungsmodus gebraucht
    SvxEscapement eEsc = (SvxEscapement ) ( (const SvxEscapementItem&)
                    aAttrSet.Get( ITEMID_ESCAPEMENT ) ).GetEnumValue();

    if( eEsc == SVX_ESCAPEMENT_SUPERSCRIPT )
        rSet.Put( SfxBoolItem( SID_SET_SUPER_SCRIPT, TRUE ) );
    else if( eEsc == SVX_ESCAPEMENT_SUBSCRIPT )
        rSet.Put( SfxBoolItem( SID_SET_SUB_SCRIPT, TRUE ) );
}

/*************************************************************************
|*
|* Command event
|*
\************************************************************************/

void __EXPORT SdDrawTextObjectBar::Command( const CommandEvent& rCEvt )
{
}

/*************************************************************************
|*
|* Unterstuetzte Features erfragen
|*
\************************************************************************/

BOOL SdDrawTextObjectBar::HasUIFeature( ULONG nFeature )
{
    BOOL bRet = FALSE;
    DocumentType eDocType = pViewShell->GetDoc()->GetDocumentType();

    if (nFeature == FEATURE_DRAW_TEXT_OBJECTBAR &&
        eDocType == DOCUMENT_TYPE_IMPRESS)
    {
        // Impress-Objektleiste
        bRet = TRUE;
    }
    else if (nFeature == FEATURE_GRAPHIC_TEXT_OBJECTBAR &&
             eDocType == DOCUMENT_TYPE_DRAW)
    {
        // Draw-Objektleiste
        bRet = TRUE;
    }

    return bRet;
}



