/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#include "futhes.hxx"

#include <editeng/outliner.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <editeng/eeitem.hxx>

#include <svx/dialogs.hrc>
#include <svx/svxerr.hxx>
#include <svx/dialmgr.hxx>
#include <editeng/unolingu.hxx>
#include <comphelper/processfactory.hxx>
#include "app.hrc"
#include "strings.hrc"
#include "drawdoc.hxx"
#include "app.hxx"
#include "View.hxx"
#include "Outliner.hxx"
#include "DrawViewShell.hxx"
#include "OutlineViewShell.hxx"
#include "Window.hxx"
#include "sdresid.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

class SfxRequest;

namespace sd {

TYPEINIT1( FuThesaurus, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuThesaurus::FuThesaurus( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
                  SdDrawDocument* pDoc, SfxRequest& rReq )
       : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuThesaurus::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuThesaurus( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuThesaurus::DoExecute( SfxRequest& )
{
    SfxErrorContext aContext(ERRCTX_SVX_LINGU_THESAURUS, String(),
                             mpWindow, RID_SVXERRCTX, &DIALOG_MGR() );

    if( mpViewShell && mpViewShell->ISA(DrawViewShell) )
    {
        SdrTextObj* pTextObj = NULL;

        if ( mpView->AreObjectsMarked() )
        {
            const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

            if ( rMarkList.GetMarkCount() == 1 )
            {
                SdrMark* pMark = rMarkList.GetMark(0);
                SdrObject* pObj = pMark->GetMarkedSdrObj();

                if ( pObj->ISA(SdrTextObj) )
                {
                    pTextObj = (SdrTextObj*) pObj;
                }
            }
        }

        ::Outliner* pOutliner = mpView->GetTextEditOutliner();
        const OutlinerView* pOutlView = mpView->GetTextEditOutlinerView();

        if ( pTextObj && pOutliner && pOutlView )
        {
            if ( !pOutliner->GetSpeller().is() )
            {
                Reference< XSpellChecker1 > xSpellChecker( LinguMgr::GetSpellChecker() );
                if ( xSpellChecker.is() )
                    pOutliner->SetSpeller( xSpellChecker );

                Reference< XHyphenator > xHyphenator( LinguMgr::GetHyphenator() );
                if( xHyphenator.is() )
                    pOutliner->SetHyphenator( xHyphenator );

                pOutliner->SetDefaultLanguage( mpDoc->GetLanguage( EE_CHAR_LANGUAGE ) );
            }

            EESpellState eState = ( (OutlinerView*) pOutlView)->StartThesaurus();
            DBG_ASSERT(eState != EE_SPELL_NOSPELLER, "No SpellChecker");

            if (eState == EE_SPELL_NOLANGUAGE)
            {
                ErrorBox(mpWindow, WB_OK, String(SdResId(STR_NOLANGUAGE))).Execute();
            }
        }
    }
    else if ( mpViewShell->ISA(OutlineViewShell) )
    {
        Outliner* pOutliner = mpDoc->GetOutliner();
        OutlinerView* pOutlView = pOutliner->GetView(0);

        if ( !pOutliner->GetSpeller().is() )
        {
            Reference< XSpellChecker1 > xSpellChecker( LinguMgr::GetSpellChecker() );
            if ( xSpellChecker.is() )
                pOutliner->SetSpeller( xSpellChecker );

            Reference< XHyphenator > xHyphenator( LinguMgr::GetHyphenator() );
            if( xHyphenator.is() )
                pOutliner->SetHyphenator( xHyphenator );

            pOutliner->SetDefaultLanguage( mpDoc->GetLanguage( EE_CHAR_LANGUAGE ) );
        }

        EESpellState eState = pOutlView->StartThesaurus();
        DBG_ASSERT(eState != EE_SPELL_NOSPELLER, "No SpellChecker");

        if (eState == EE_SPELL_NOLANGUAGE)
        {
            ErrorBox(mpWindow, WB_OK, String(SdResId(STR_NOLANGUAGE))).Execute();
        }
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
