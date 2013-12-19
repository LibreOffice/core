/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#include <sfx2/app.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/bindings.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdview.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdouno.hxx>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <svx/htmlmode.hxx>
#include <tools/urlobj.hxx>
#include "wrtsh.hxx"
#include "view.hxx"
#include "IMark.hxx"
#include "doc.hxx"
#include "wrtsh.hrc"

#include <unomid.h>


using namespace ::com::sun::star;
using ::rtl::OUString;

extern sal_Bool bNoInterrupt;       // in mainwn.cxx

sal_Bool SwWrtShell::MoveBookMark( BookMarkMove eFuncId, const ::sw::mark::IMark* const pMark)
{
//JP 08.03.96: die Wizards brauchen die Selektion !!
//  EndSelect();
    (this->*fnKillSel)( 0, sal_False );

    sal_Bool bRet = sal_True;
    switch(eFuncId)
    {
        case BOOKMARK_INDEX:bRet = SwCrsrShell::GotoMark( pMark );break;
        case BOOKMARK_NEXT: bRet = SwCrsrShell::GoNextBookmark();break;
        case BOOKMARK_PREV: bRet = SwCrsrShell::GoPrevBookmark();break;
        default:;//prevent warning
    }

    if( bRet && IsSelFrmMode() )
    {
        UnSelectFrm();
        LeaveSelFrmMode();
    }
    if( IsSelection() )
    {
        fnKillSel = &SwWrtShell::ResetSelect;
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
    }
    return bRet;
}

sal_Bool SwWrtShell::GotoField( const SwFmtFld& rFld )
{
    (this->*fnKillSel)( 0, sal_False );

    sal_Bool bRet = SwCrsrShell::GotoFld( rFld );
    if( bRet && IsSelFrmMode() )
    {
        UnSelectFrm();
        LeaveSelFrmMode();
    }

    if( IsSelection() )
    {
        fnKillSel = &SwWrtShell::ResetSelect;
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
    }

    return bRet;
}

bool SwWrtShell::GotoFieldmark(::sw::mark::IFieldmark const * const pMark)
{
    (this->*fnKillSel)( 0, sal_False );
    bool bRet = SwCrsrShell::GotoFieldmark(pMark);
    if( bRet && IsSelFrmMode() )
    {
        UnSelectFrm();
        LeaveSelFrmMode();
    }
    if( IsSelection() )
    {
        fnKillSel = &SwWrtShell::ResetSelect;
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung: FontWork-Slots invalidieren
 --------------------------------------------------------------------*/


void SwWrtShell::DrawSelChanged( )
{
    static sal_uInt16 __READONLY_DATA aInval[] =
    {
        SID_ATTR_FILL_STYLE, SID_ATTR_FILL_COLOR, SID_ATTR_LINE_STYLE,
        SID_ATTR_LINE_WIDTH, SID_ATTR_LINE_COLOR,
        /*AF: these may be needed for the sidebar.
        SID_SVX_AREA_TRANSPARENCY, SID_SVX_AREA_TRANSP_GRADIENT,
        SID_SVX_AREA_TRANS_TYPE,
        */
        0
    };

    GetView().GetViewFrame()->GetBindings().Invalidate(aInval);

    sal_Bool bOldVal = bNoInterrupt;
    bNoInterrupt = sal_True;    // Trick, um AttrChangedNotify ueber Timer auszufuehren
    GetView().AttrChangedNotify(this);
    bNoInterrupt = bOldVal;
}

sal_Bool SwWrtShell::GotoMark( const ::rtl::OUString& rName )
{
    IDocumentMarkAccess::const_iterator_t ppMark = getIDocumentMarkAccess()->findMark( rName );
    if(ppMark == getIDocumentMarkAccess()->getAllMarksEnd()) return false;
    return MoveBookMark( BOOKMARK_INDEX, ppMark->get() );
}


sal_Bool SwWrtShell::GotoMark( const ::sw::mark::IMark* const pMark )
{
    return MoveBookMark( BOOKMARK_INDEX, pMark );
}


sal_Bool SwWrtShell::GoNextBookmark()
{
    return MoveBookMark( BOOKMARK_NEXT );
}


sal_Bool SwWrtShell::GoPrevBookmark()
{
    return MoveBookMark( BOOKMARK_PREV );
}


void SwWrtShell::ExecMacro( const SvxMacro& rMacro, String* pRet, SbxArray* pArgs )
{
    // OD 11.02.2003 #100556# - execute macro, if it is allowed.
    if ( IsMacroExecAllowed() )
    {
        GetDoc()->ExecMacro( rMacro, pRet, pArgs );
    }
}


sal_uInt16 SwWrtShell::CallEvent( sal_uInt16 nEvent, const SwCallMouseEvent& rCallEvent,
                                sal_Bool bChkPtr, SbxArray* pArgs,
                                const Link* pCallBack )
{
    return GetDoc()->CallEvent( nEvent, rCallEvent, bChkPtr, pArgs, pCallBack );
}


    // fall ein util::URL-Button selektiert ist, dessen util::URL returnen, ansonsten
    // einen LeerString
sal_Bool SwWrtShell::GetURLFromButton( String& rURL, String& rDescr ) const
{
    sal_Bool bRet = sal_False;
    const SdrView *pDView = GetDrawView();
    if( pDView )
    {
        // Ein Fly ist genau dann erreichbar, wenn er selektiert ist.
        const SdrMarkList &rMarkList = pDView->GetMarkedObjectList();

        if (rMarkList.GetMark(0))
        {
            SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, rMarkList.GetMark(0)->GetMarkedSdrObj());
            if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
            {
                uno::Reference< awt::XControlModel >  xControlModel = pUnoCtrl->GetUnoControlModel();

                ASSERT( xControlModel.is(), "UNO-Control ohne Model" );
                if( !xControlModel.is() )
                    return bRet;

                uno::Reference< beans::XPropertySet >  xPropSet(xControlModel, uno::UNO_QUERY);

                uno::Any aTmp;

                form::FormButtonType eButtonType = form::FormButtonType_URL;
                uno::Reference< beans::XPropertySetInfo >   xInfo = xPropSet->getPropertySetInfo();
                if(xInfo->hasPropertyByName( C2U("ButtonType") ))
                {
                    aTmp = xPropSet->getPropertyValue( C2U("ButtonType") );
                    form::FormButtonType eTmpButtonType;
                    aTmp >>= eTmpButtonType;
                    if( eButtonType == eTmpButtonType)
                    {
                        // Label
                        aTmp = xPropSet->getPropertyValue( C2U("Label") );
                        OUString uTmp;
                        if( (aTmp >>= uTmp) && uTmp.getLength())
                        {
                            rDescr = String(uTmp);
                        }

                        // util::URL
                        aTmp = xPropSet->getPropertyValue( C2U("TargetURL") );
                        if( (aTmp >>= uTmp) && uTmp.getLength())
                        {
                            rURL = String(uTmp);
                        }
                        bRet = sal_True;
                    }
                }
            }
        }
    }

    return bRet;
}
