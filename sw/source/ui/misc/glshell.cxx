/*************************************************************************
 *
 *  $RCSfile: glshell.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:36:08 $
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

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXSIDS_HRC //autogen
#include <sfx2/sfxsids.hrc>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef _OFAACCFG_HXX //autogen
#include <offmgr/ofaaccfg.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SFXMACITEM_HXX
#include <svtools/macitem.hxx>
#endif
#ifndef _GLOSHDL_HXX
#include <gloshdl.hxx>
#endif

#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _GLSHELL_HXX
#include <glshell.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _GLOSDOC_HXX
#include <glosdoc.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>                   // fuer ::GetGlossaries()
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _SWERROR_H
#include <swerror.h>
#endif
#ifndef _MISC_HRC
#include <misc.hrc>
#endif


#define SwWebGlosDocShell
#define SwGlosDocShell
#ifndef _ITEMDEF_HXX
#include <itemdef.hxx>
#endif
#ifndef _SWSLOTS_HXX
#include <swslots.hxx>
#endif


SFX_IMPL_INTERFACE( SwGlosDocShell, SwDocShell, SW_RES(0) )
{
}

SFX_IMPL_INTERFACE( SwWebGlosDocShell, SwWebDocShell, SW_RES(0) )
{
}


TYPEINIT1( SwGlosDocShell, SwDocShell );
TYPEINIT1( SwWebGlosDocShell, SwWebDocShell );


void lcl_Execute( SwDocShell& rSh, SfxRequest& rReq )
{
    if ( rReq.GetSlot() == SID_SAVEDOC )
    {
        if( !rSh.HasName() )
        {
            rReq.SetReturnValue( SfxBoolItem( 0, rSh.Save() ) );
        }
        else
        {
            const SfxBoolItem* pRes = ( const SfxBoolItem* )
                                        rSh.ExecuteSlot( rReq,
                                        rSh.SfxObjectShell::GetInterface() );
            if( pRes->GetValue() )
                rSh.GetDoc()->ResetModified();
        }
    }
}


void lcl_GetState( SwDocShell& rSh, SfxItemSet& rSet )
{
    if( SFX_ITEM_AVAILABLE >= rSet.GetItemState( SID_SAVEDOC, FALSE ))
    {
        if( !rSh.GetDoc()->IsModified() )
            rSet.DisableItem( SID_SAVEDOC );
        else
            rSet.Put( SfxStringItem( SID_SAVEDOC, SW_RESSTR(STR_SAVE_GLOSSARY)));
    }
}


BOOL lcl_Save( SwWrtShell& rSh, const String& rGroupName,
                const String& rShortNm, const String& rLongNm )
{
    const OfaAutoCorrCfg* pCfg = OFF_APP()->GetAutoCorrConfig();
    SwTextBlocks * pBlock = ::GetGlossaries()->GetGroupDoc( rGroupName );

    SvxMacro aStart(aEmptyStr, aEmptyStr);
    SvxMacro aEnd(aEmptyStr, aEmptyStr);
    SwGlossaryHdl* pGlosHdl;

    pGlosHdl = rSh.GetView().GetGlosHdl();
    pGlosHdl->GetMacros( rShortNm, aStart, aEnd, pBlock );

    USHORT nRet = rSh.SaveGlossaryDoc( *pBlock, rLongNm, rShortNm,
                                pCfg->IsSaveRelFile(), pCfg->IsSaveRelNet(),
                                pBlock->IsOnlyTextBlock( rShortNm ) );

    if(aStart.GetMacName().Len() || aEnd.GetMacName().Len() )
    {
        SvxMacro* pStart = aStart.GetMacName().Len() ? &aStart : 0;
        SvxMacro* pEnd = aEnd.GetMacName().Len() ? &aEnd : 0;
        pGlosHdl->SetMacros( rShortNm, pStart, pEnd, pBlock );
    }

    rSh.EnterStdMode();
    if( USHRT_MAX != nRet )
        rSh.ResetModified();
    delete pBlock;
    return nRet != USHRT_MAX;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwGlosDocShell::SwGlosDocShell( sal_Bool bNewShow)
    : bShow ( bNewShow )
    , SwDocShell( bShow ? SFX_CREATE_MODE_STANDARD : SFX_CREATE_MODE_INTERNAL )
{
    SetHelpId(SW_GLOSDOCSHELL);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwGlosDocShell::~SwGlosDocShell(  )
{
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwGlosDocShell::Execute( SfxRequest& rReq )
{
    ::lcl_Execute( *this, rReq );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwGlosDocShell::GetState( SfxItemSet& rSet )
{
    ::lcl_GetState( *this, rSet );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


BOOL SwGlosDocShell::Save()
{
    // In case of an API object which holds this document, it is possible that the WrtShell is already
    // dead. For instance, if the doc is modified via this API object, and then, upon office shutdown,
    // the document's view is closed (by the SFX framework) _before_ the API object is release and
    // tries to save the doc, again.
    // 96380 - 2002-03-03 - fs@openoffice.org
    if ( GetWrtShell() )
        return ::lcl_Save( *GetWrtShell(), aGroupName, aShortName, aLongName );
    else
    {
        SetModified( FALSE );
        return FALSE;
    }
}


/**/

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwWebGlosDocShell::SwWebGlosDocShell()
    : SwWebDocShell( SFX_CREATE_MODE_STANDARD )
{
    SetHelpId(SW_WEBGLOSDOCSHELL);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwWebGlosDocShell::~SwWebGlosDocShell(  )
{
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwWebGlosDocShell::Execute( SfxRequest& rReq )
{
    ::lcl_Execute( *this, rReq );
}
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwWebGlosDocShell::GetState( SfxItemSet& rSet )
{
    ::lcl_GetState( *this, rSet );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


BOOL SwWebGlosDocShell::Save()
{
    // same comment as in SwGlosDocShell::Save - see there
    if ( GetWrtShell() )
        return ::lcl_Save( *GetWrtShell(), aGroupName, aShortName, aLongName );
    else
    {
        SetModified( FALSE );
        return FALSE;
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SV_IMPL_REF ( SwDocShell )

SwDocShellRef SwGlossaries::EditGroupDoc( const String& rGroup, const String& rShortName, BOOL bShow )
{
    SwDocShellRef xDocSh;

    SwTextBlocks* pGroup = GetGroupDoc( rGroup );
    if( pGroup && pGroup->GetCount() )
    {
        // erfrage welche View registriert ist. Im WebWriter gibts es keine
        // normale View
        USHORT nViewId = 0 != &SwView::Factory() ? 2 : 6;
        String sLongName = pGroup->GetLongName(pGroup->GetIndex( rShortName ));

        if( 6 == nViewId )
        {
            SwWebGlosDocShell* pDocSh = new SwWebGlosDocShell();
            xDocSh = pDocSh;
            pDocSh->DoInitNew( 0 );
            pDocSh->SetLongName( sLongName );
            pDocSh->SetShortName( rShortName);
            pDocSh->SetGroupName( rGroup );
        }
        else
        {
            SwGlosDocShell* pDocSh = new SwGlosDocShell(bShow);
            xDocSh = pDocSh;
            pDocSh->DoInitNew( 0 );
            pDocSh->SetLongName( sLongName );
            pDocSh->SetShortName( rShortName );
            pDocSh->SetGroupName( rGroup );
        }

        // Dokumenttitel setzen
        SfxViewFrame* pFrame = SFX_APP()->CreateViewFrame( *xDocSh, nViewId, !bShow );
        String aDocTitle(SW_RES( STR_GLOSSARY ));
        aDocTitle += sLongName;

        BOOL bDoesUndo = xDocSh->GetDoc()->DoesUndo();
        xDocSh->GetDoc()->DoUndo( FALSE );

        xDocSh->GetWrtShell()->InsertGlossary( *pGroup, rShortName );
        if( !xDocSh->GetDoc()->GetPrt() )
        {
            // wir erzeugen einen default SfxPrinter.
            // Das ItemSet wird vom Sfx geloescht!
            SfxItemSet *pSet = new SfxItemSet( xDocSh->GetDoc()->GetAttrPool(),
                        FN_PARAM_ADDPRINTER, FN_PARAM_ADDPRINTER,
                        SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                        SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
                        0 );
            SfxPrinter* pPrinter = new SfxPrinter( pSet );

            // und haengen ihn ans Dokument.
            xDocSh->GetDoc()->SetPrt( pPrinter );
        }

        xDocSh->SetTitle( aDocTitle );
        xDocSh->GetDoc()->DoUndo( bDoesUndo );
        xDocSh->GetDoc()->ResetModified();
        if ( bShow )
            pFrame->GetFrame()->Appear();
        xDocSh->GetDoc()->DoUndo ( FALSE );

        delete pGroup;
    }
    return xDocSh;
}


