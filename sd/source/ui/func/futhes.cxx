/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <futhes.hxx>

#include <editeng/outliner.hxx>
#include <sfx2/request.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <editeng/eeitem.hxx>
#include <tools/debug.hxx>

#include <svx/svxerr.hxx>
#include <svx/dialmgr.hxx>
#include <editeng/unolingu.hxx>
#include <vcl/weld.hxx>
#include <drawdoc.hxx>
#include <View.hxx>
#include <Outliner.hxx>
#include <DrawViewShell.hxx>
#include <OutlineViewShell.hxx>
#include <Window.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

class SfxRequest;

namespace sd {


FuThesaurus::FuThesaurus( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
                  SdDrawDocument* pDoc, SfxRequest& rReq )
       : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuThesaurus::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuThesaurus( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuThesaurus::DoExecute(SfxRequest& rReq)
{
    SfxErrorContext aContext(ERRCTX_SVX_LINGU_THESAURUS, OUString(),
                             mpWindow->GetFrameWeld(), RID_SVXERRCTX, SvxResLocale());

    if (dynamic_cast< DrawViewShell *>( mpViewShell ))
    {
        SdrTextObj* pTextObj = nullptr;

        if ( mpView->AreObjectsMarked() )
        {
            const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

            if ( rMarkList.GetMarkCount() == 1 )
            {
                SdrMark* pMark = rMarkList.GetMark(0);
                SdrObject* pObj = pMark->GetMarkedSdrObj();

                pTextObj = dynamic_cast<SdrTextObj *>( pObj );
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

            EESpellState eState = const_cast<OutlinerView*>(pOutlView)->StartThesaurus(rReq.GetFrameWeld());
            DBG_ASSERT(eState != EESpellState::NoSpeller, "No SpellChecker");
        }
    }
    else if (dynamic_cast< OutlineViewShell *>( mpViewShell ))
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

        EESpellState eState = pOutlView->StartThesaurus(rReq.GetFrameWeld());
        DBG_ASSERT(eState != EESpellState::NoSpeller, "No SpellChecker");
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
