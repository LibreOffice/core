/*************************************************************************
 *
 *  $RCSfile: sdrhhcwrap.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-04-27 15:40:22 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVDITER_HXX //autogen
#include <svx/svditer.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _MyEDITDATA_HXX //autogen
#include <svx/editdata.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVDOGRP_HXX //autogen
#include <svx/svdogrp.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _LINGUISTIC_LNGPROPS_HHX_
#include <linguistic/lngprops.hxx>
#endif
#ifndef _SFX_SFXUNO_HXX
#include <sfx2/sfxuno.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

#ifndef _SDRHHCWRAP_HXX_
#include <sdrhhcwrap.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _SDRSPELL_HXX
#include <sdrspell.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>        //     "     "     "
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>        //  MakeVisible
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>     // Spelling von DrawObj
#endif
#ifndef _DOC_HXX
#include <doc.hxx>        //     "     "     "
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif

class SfxStyleSheetPool;

using namespace ::com::sun::star;
using namespace ::rtl;

//////////////////////////////////////////////////////////////////////

SdrHHCWrapper::SdrHHCWrapper( SwView* pVw, INT16 nLanguage ) :
    SdrOutliner(pVw->GetDocShell()->GetDoc()->GetDrawModel()->
                             GetDrawOutliner().GetEmptyItemSet().GetPool(),
                OUTLINERMODE_TEXTOBJECT ),
    pView( pVw ),
    pTextObj( NULL ),
    pOutlView( NULL ),
    pListIter( NULL ),
    nDocIndex( 0 ),
    nLang( nLanguage )
{
    SetRefDevice( pView->GetDocShell()->GetDoc()->GetPrt() );

    MapMode aMapMode (MAP_TWIP);
    SetRefMapMode(aMapMode);

     Size aSize( 1, 1 );
    SetPaperSize( aSize );

    pOutlView = new OutlinerView( this, &(pView->GetEditWin()) );
    pOutlView->GetOutliner()->SetRefDevice(pView->GetWrtShell().GetPrt());

    // Hack: Es sollten alle SdrTextObj-Attribute an die EditEngine
    //       uebertragen werden.
    pOutlView->SetBackgroundColor( Color( COL_WHITE ) );


    InsertView( pOutlView );
    Point aPoint( 0, 0 );
     Rectangle aRect( aPoint, aSize );
    pOutlView->SetOutputArea( aRect );
//  SetText( NULL );
    ClearModifyFlag();
}

SdrHHCWrapper::~SdrHHCWrapper()
{
    if (pTextObj)
    {
        SdrView *pSdrView = pView->GetWrtShell().GetDrawView();
        ASSERT( pSdrView, "SdrHHCWrapper without DrawView?" );
        pSdrView->EndTextEdit( sal_True );
        SetUpdateMode(sal_False);
        pOutlView->SetOutputArea( Rectangle( Point(), Size(1, 1) ) );
//      SetPaperSize( Size(1, 1) );
//      SetText(NULL);
//      pTextObj = NULL;
    }
    RemoveView( pOutlView );
    delete pOutlView;
}


void SdrHHCWrapper::StartTextConversion()
{
    pOutlView->StartTextConversion( nLang, sal_True );
}


sal_Bool SdrHHCWrapper::ConvertNextDocument()
{
    sal_Bool bNextDoc = sal_False;

    if ( pTextObj )
    {
        SdrView *pSdrView = pView->GetWrtShell().GetDrawView();
        ASSERT( pSdrView, "SdrHHCWrapper without DrawView?" );
        pSdrView->EndTextEdit( sal_True );
        SetUpdateMode(sal_False);
        pOutlView->SetOutputArea( Rectangle( Point(), Size(1, 1) ) );
        SetPaperSize( Size(1, 1) );
        Clear();
        pTextObj = NULL;
    }

    sal_uInt16 n = nDocIndex;

    while( !bNextDoc && ( pListIter ||
         n < pView->GetDocShell()->GetDoc()->GetSpzFrmFmts()->Count() ) )
    {
        while( !pTextObj && pListIter )
        {
            if( pListIter->IsMore() )
            {
                SdrObject* pSdrO = pListIter->Next();
                if( pSdrO && pSdrO->IsA( TYPE(SdrTextObj) ) &&
                    ( (SdrTextObj*) pSdrO )->HasText() )
                    pTextObj = (SdrTextObj*) pSdrO;
            }
            else
            {
                delete pListIter;
                pListIter = NULL;
            }
        }

        if ( !pTextObj &&
             n < pView->GetDocShell()->GetDoc()->GetSpzFrmFmts()->Count() )
        {
            SwFrmFmt* pFly = (*pView->GetDocShell()->GetDoc()->GetSpzFrmFmts())[ n ];
            if( pFly->IsA( TYPE(SwDrawFrmFmt) ) )
            {
                SwClientIter aIter( (SwFmt&) *pFly );
                if( aIter.First( TYPE(SwDrawContact) ) )
                {
                    SdrObject* pSdrO = ((SwDrawContact*)aIter())->GetMaster();
                    if ( pSdrO )
                    {
                        if ( pSdrO->IsA( TYPE(SdrObjGroup) ) )
                            pListIter = new SdrObjListIter( *pSdrO, IM_DEEPNOGROUPS );
                        else if( pSdrO->IsA( TYPE(SdrTextObj) ) &&
                                ( (SdrTextObj*) pSdrO )->HasText() )
                            pTextObj = (SdrTextObj*) pSdrO;
                    }
                }
            }
            ++n;
        }
        if ( pTextObj )
        {
            OutlinerParaObject* pParaObj = pTextObj->GetOutlinerParaObject();

            if ( pParaObj )
            {
                SetPaperSize( pTextObj->GetLogicRect().GetSize() );
                SetText( *pParaObj );

                ClearModifyFlag();

                if (HasConvertibleTextPortion( nLang ))
                {
                    SdrView *pSdrView = pView->GetWrtShell().GetDrawView();
                    ASSERT( pSdrView, "SdrHHCWrapper without DrawView?" );
                    SdrPageView* pPV = pSdrView->GetPageViewPvNum(0);
                    nDocIndex = n;
                    bNextDoc = sal_True;
                    pOutlView->SetOutputArea( Rectangle( Point(), Size(1,1)));
                    SetPaperSize( pTextObj->GetLogicRect().GetSize() );
                    SetUpdateMode(sal_True);
                    pView->GetWrtShell().MakeVisible(pTextObj->GetLogicRect());
                    pSdrView->BegTextEdit( pTextObj, pPV, &pView->GetEditWin(),
                                           sal_False, this, pOutlView, sal_True, sal_True);
                }
            }
            if ( !bNextDoc )
                pTextObj = NULL;
        }
    }

    ClearModifyFlag();

    return bNextDoc;
}

//////////////////////////////////////////////////////////////////////

