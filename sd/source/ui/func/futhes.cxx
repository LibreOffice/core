/*************************************************************************
 *
 *  $RCSfile: futhes.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:36 $
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

#include <tools/pstm.hxx>
#include <svx/outliner.hxx>
#include <offmgr/osplcfg.hxx>
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif

#define ITEMID_LANGUAGE         SID_ATTR_CHAR_LANGUAGE
#include <svx/dialogs.hrc>
#include <svx/svxerr.hxx>
#include <svx/dialmgr.hxx>

#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif

#include "app.hrc"
#include "strings.hrc"
#include "drawdoc.hxx"
#include "app.hxx"
#include "futhes.hxx"
#include "sdview.hxx"
#include "sdoutl.hxx"
#include "drviewsh.hxx"
#include "outlnvsh.hxx"
#include "sdwindow.hxx"
#include "sdresid.hxx"

using namespace ::com::sun::star;

class SfxRequest;

TYPEINIT1( FuThesaurus, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuThesaurus::FuThesaurus( SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
                  SdDrawDocument* pDoc, SfxRequest& rReq )
       : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    SfxErrorContext aContext(ERRCTX_SVX_LINGU_THESAURUS, String(),
                             pWin, RID_SVXERRCTX, DIALOG_MGR() );

    if ( pViewShell->ISA(SdDrawViewShell) )
    {
        SdrTextObj* pTextObj = NULL;

        if ( pView->HasMarkedObj() )
        {
            const SdrMarkList& rMarkList = pView->GetMarkList();

            if ( rMarkList.GetMarkCount() == 1 )
            {
                SdrMark* pMark = rMarkList.GetMark(0);
                SdrObject* pObj = pMark->GetObj();

                if ( pObj->ISA(SdrTextObj) )
                {
                    pTextObj = (SdrTextObj*) pObj;
                }
            }
        }

        Outliner* pOutliner = pView->GetTextEditOutliner();
        const OutlinerView* pOutlView = pView->GetTextEditOutlinerView();

        if ( pTextObj && pOutliner && pOutlView )
        {
            if ( !pOutliner->GetSpeller().is() )
            {
                uno::Reference< linguistic::XSpellChecker1 > xSpellChecker( SvxGetSpellChecker() );
                if ( xSpellChecker.is() )
                    pOutliner->SetSpeller( xSpellChecker );

                uno::Reference< linguistic::XHyphenator > xHyphenator( OFF_APP()->GetHyphenator() );
                if( xHyphenator.is() )
                    pOutliner->SetHyphenator( xHyphenator );

                pOutliner->SetDefaultLanguage( pDoc->GetLanguage() );
            }

            EESpellState eState = ( (OutlinerView*) pOutlView)
                                  ->StartThesaurus( pDoc->GetLanguage() );

            DBG_ASSERT(eState != EE_SPELL_NOSPELLER, "No SpellChecker");

            if (eState == EE_SPELL_NOLANGUAGE)
            {
                ErrorBox(pWindow, WB_OK, String(SdResId(STR_NOLANGUAGE))).Execute();
            }
        }
    }
    else if ( pViewShell->ISA(SdOutlineViewShell) )
    {
        Outliner* pOutliner = pDoc->GetOutliner();
        OutlinerView* pOutlView = pOutliner->GetView(0);

        if ( !pOutliner->GetSpeller().is() )
        {
            uno::Reference< linguistic::XSpellChecker1 > xSpellChecker( SvxGetSpellChecker() );
            if ( xSpellChecker.is() )
                pOutliner->SetSpeller( xSpellChecker );

            uno::Reference< linguistic::XHyphenator > xHyphenator( OFF_APP()->GetHyphenator() );
            if( xHyphenator.is() )
                pOutliner->SetHyphenator( xHyphenator );

            pOutliner->SetDefaultLanguage( pDoc->GetLanguage() );
        }

        EESpellState eState = pOutlView->StartThesaurus( pDoc->GetLanguage() );

        DBG_ASSERT(eState != EE_SPELL_NOSPELLER, "No SpellChecker");

        if (eState == EE_SPELL_NOLANGUAGE)
        {
            ErrorBox(pWindow, WB_OK, String(SdResId(STR_NOLANGUAGE))).Execute();
        }
    }
}



/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/
FuThesaurus::~FuThesaurus()
{
}


