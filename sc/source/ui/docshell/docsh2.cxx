/*************************************************************************
 *
 *  $RCSfile: docsh2.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: sab $ $Date: 2001-08-01 07:32:36 $
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


#ifdef PCH
#include "ui_pch.hxx"
#endif

#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif

#pragma hdrstop


#include "scitems.hxx"
#include <tools/gen.hxx>
#include <sfx2/interno.hxx>
#include <so3/svstor.hxx>
#include <svtools/ctrltool.hxx>
#include <svx/flstitem.hxx>
#include <svx/drawitem.hxx>
#include <sfx2/printer.hxx>
#include <svtools/smplhint.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <vcl/svapp.hxx>
#include <offmgr/app.hxx>
#include <svx/asiancfg.hxx>
#include <svx/forbiddencharacterstable.hxx>
#include <svx/unolingu.hxx>
#include <rtl/logfile.hxx>



// INCLUDE ---------------------------------------------------------------
/*
#include <svdrwetc.hxx>
#include <svdrwobx.hxx>
#include <sostor.hxx>
*/
#include "drwlayer.hxx"
#include "stlpool.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "sc.hrc"

using namespace com::sun::star;

//------------------------------------------------------------------

BOOL __EXPORT ScDocShell::InitNew( SvStorage * pStor )
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::InitNew" );

    BOOL bRet = SfxInPlaceObject::InitNew( pStor );

    aDocument.MakeTable(0);
    //  zusaetzliche Tabellen werden von der ersten View angelegt,
    //  wenn bIsEmpty dann noch TRUE ist

    if( bRet )
    {
        Size aSize( (long) ( STD_COL_WIDTH           * HMM_PER_TWIPS * OLE_STD_CELLS_X ),
                    (long) ( ScGlobal::nStdRowHeight * HMM_PER_TWIPS * OLE_STD_CELLS_Y ) );
        // hier muss auch der Start angepasst werden
        SetVisAreaOrSize( Rectangle( Point(), aSize ), TRUE );
    }

    aDocument.GetStyleSheetPool()->CreateStandardStyles();
    aDocument.UpdStlShtPtrsFrmNms();
    InitOptions(); // CLOOK

    //  SetDocumentModified ist in Load/InitNew nicht mehr erlaubt!

    InitItems();
    CalcOutputFactor();

    return bRet;
}

//------------------------------------------------------------------

BOOL ScDocShell::IsEmpty() const
{
    return bIsEmpty;
}


void ScDocShell::ResetEmpty()
{
    bIsEmpty = FALSE;
}

//------------------------------------------------------------------

void ScDocShell::InitItems()
{
    // AllItemSet fuer Controller mit benoetigten Items fuellen:

    if ( pFontList )
        delete pFontList;

    //  Druck-Optionen werden beim Drucken und evtl. in GetPrinter gesetzt

    pFontList = new FontList( GetPrinter(), Application::GetDefaultDevice() );
    PutItem( SvxFontListItem( pFontList, SID_ATTR_CHAR_FONTLIST ) );

    ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
    if (pDrawLayer)
    {
        PutItem( SvxColorTableItem  ( pDrawLayer->GetColorTable() ) );
        PutItem( SvxGradientListItem( pDrawLayer->GetGradientList() ) );
        PutItem( SvxHatchListItem   ( pDrawLayer->GetHatchList() ) );
        PutItem( SvxBitmapListItem  ( pDrawLayer->GetBitmapList() ) );
        PutItem( SvxDashListItem    ( pDrawLayer->GetDashList() ) );
        PutItem( SvxLineEndListItem ( pDrawLayer->GetLineEndList() ) );

            //  andere Anpassungen nach dem Anlegen des DrawLayers

        pDrawLayer->SetNotifyUndoActionHdl( LINK( pDocFunc, ScDocFunc, NotifyDrawUndo ) );

        //if (SfxObjectShell::HasSbxObject())
        pDrawLayer->UpdateBasic();          // DocShell-Basic in DrawPages setzen
    }
    else
    {
        //  always use global color table instead of local copy

        PutItem( SvxColorTableItem( OFF_APP()->GetStdColorTable() ) );
    }

    if ( !aDocument.GetForbiddenCharacters().isValid() )
    {
        // set forbidden characters if necessary
        SvxAsianConfig aAsian;
        uno::Sequence<lang::Locale> aLocales = aAsian.GetStartEndCharLocales();
        if (aLocales.getLength())
        {
            vos::ORef<SvxForbiddenCharactersTable> xForbiddenTable =
                    new SvxForbiddenCharactersTable( aDocument.GetServiceManager() );

            const lang::Locale* pLocales = aLocales.getConstArray();
            for (sal_Int32 i = 0; i < aLocales.getLength(); i++)
            {
                i18n::ForbiddenCharacters aForbidden;
                aAsian.GetStartEndChars( pLocales[i], aForbidden.beginLine, aForbidden.endLine );
                LanguageType eLang = SvxLocaleToLanguage(pLocales[i]);
                //pDoc->SetForbiddenCharacters( eLang, aForbidden );

                xForbiddenTable->SetForbiddenCharacters( eLang, aForbidden );
            }

            aDocument.SetForbiddenCharacters( xForbiddenTable );
        }
    }

    if ( !aDocument.IsValidAsianCompression() )
    {
        // set compression mode from configuration if not already set (e.g. XML import)
        SvxAsianConfig aAsian( sal_False );     //! share with forbidden characters
        aDocument.SetAsianCompression( aAsian.GetCharDistanceCompression() );
    }
}

//------------------------------------------------------------------

void ScDocShell::ResetDrawObjectShell()
{
    ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
    if (pDrawLayer)
        pDrawLayer->SetObjectShell( NULL );
}

//------------------------------------------------------------------

void __EXPORT ScDocShell::Activate()
{
}


void __EXPORT ScDocShell::Deactivate()
{
}

//------------------------------------------------------------------


ScDrawLayer* ScDocShell::MakeDrawLayer()
{
    ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
    if (!pDrawLayer)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::MakeDrawLayer" );

        aDocument.InitDrawLayer(this);
        pDrawLayer = aDocument.GetDrawLayer();
        InitItems();                                            // incl. Undo und Basic
        Broadcast( SfxSimpleHint( SC_HINT_DRWLAYER_NEW ) );
        if (nDocumentLock)
            pDrawLayer->setLock(TRUE);
    }
    return pDrawLayer;
}

//------------------------------------------------------------------


void ScDocShell::RemoveUnknownObjects()
{
    //  OLE-Objekte loeschen, wenn kein Drawing-Objekt dazu existiert
    //  Loeschen wie in SvPersist::CleanUp

    ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
    const SvInfoObjectMemberList* pChildList = GetObjectList();

    if( pChildList && pChildList->Count() )
    {
        for( ULONG i=0; i<pChildList->Count(); )
        {
            SvInfoObjectRef pEle = pChildList->GetObject(i);
            String aObjName = pEle->GetObjName();
            BOOL bFound = FALSE;
            if ( pDrawLayer )
            {
                USHORT nTabCount = pDrawLayer->GetPageCount();
                for (USHORT nTab=0; nTab<nTabCount && !bFound; nTab++)
                {
                    SdrPage* pPage = pDrawLayer->GetPage(nTab);
                    DBG_ASSERT(pPage,"Page ?");
                    if (pPage)
                    {
                        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
                        SdrObject* pObject = aIter.Next();
                        while (pObject && !bFound)
                        {
                            if ( pObject->ISA(SdrOle2Obj) && pObject->GetName() == aObjName )
                                bFound = TRUE;
                            pObject = aIter.Next();
                        }
                    }
                }
            }

            if (!bFound)
            {
                DBG_ASSERT(pEle->GetRefCount()==2, "Loeschen von referenziertem Storage");
                String aStorName(pEle->GetStorageName());
                SvPersist::Remove(pEle);
                GetStorage()->Remove(aStorName);
            }
            else
                i++;
        }
    }
}


