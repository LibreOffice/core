/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrhhcwrap.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:52:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

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

using namespace ::com::sun::star;
using ::rtl::OUString;

//////////////////////////////////////////////////////////////////////

SdrHHCWrapper::SdrHHCWrapper( SwView* pVw,
       LanguageType nSourceLanguage, LanguageType nTargetLanguage,
       const Font* pTargetFnt,
       sal_Int32 nConvOptions,
       sal_Bool bInteractive ) :
    SdrOutliner(pVw->GetDocShell()->GetDoc()->GetDrawModel()->
                             GetDrawOutliner().GetEmptyItemSet().GetPool(),
                OUTLINERMODE_TEXTOBJECT ),
    pView( pVw ),
    pTextObj( NULL ),
    pOutlView( NULL ),
    pListIter( NULL ),
    nOptions( nConvOptions ),
    nDocIndex( 0 ),
    nSourceLang( nSourceLanguage ),
    nTargetLang( nTargetLanguage ),
    pTargetFont( pTargetFnt ),
    bIsInteractive( bInteractive )
{
    SetRefDevice( pView->GetDocShell()->GetDoc()->getPrinter( false ) );

    MapMode aMapMode (MAP_TWIP);
    SetRefMapMode(aMapMode);

     Size aSize( 1, 1 );
    SetPaperSize( aSize );

    pOutlView = new OutlinerView( this, &(pView->GetEditWin()) );
    pOutlView->GetOutliner()->SetRefDevice(pView->GetWrtShell().getIDocumentDeviceAccess()->getPrinter( false ));

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
        pSdrView->SdrEndTextEdit( sal_True );
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
    pOutlView->StartTextConversion( nSourceLang, nTargetLang, pTargetFont, nOptions, bIsInteractive, sal_True );
}


sal_Bool SdrHHCWrapper::ConvertNextDocument()
{
    sal_Bool bNextDoc = sal_False;

    if ( pTextObj )
    {
        SdrView *pSdrView = pView->GetWrtShell().GetDrawView();
        ASSERT( pSdrView, "SdrHHCWrapper without DrawView?" );
        pSdrView->SdrEndTextEdit( sal_True );
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

                //!! update mode needs to be set to true otherwise
                //!! the call to 'HasConvertibleTextPortion' will not always
                //!! work correctly because the document may not be properly
                //!! formatted when some information is accessed, and thus
                //!! incorrect results get returned.
                SetUpdateMode(sal_True);
                if (HasConvertibleTextPortion( nSourceLang ))
                {
                    SdrView *pSdrView = pView->GetWrtShell().GetDrawView();
                    ASSERT( pSdrView, "SdrHHCWrapper without DrawView?" );
                    SdrPageView* pPV = pSdrView->GetSdrPageView();
                    nDocIndex = n;
                    bNextDoc = sal_True;
                    pOutlView->SetOutputArea( Rectangle( Point(), Size(1,1)));
                    SetPaperSize( pTextObj->GetLogicRect().GetSize() );
                    SetUpdateMode(sal_True);
                    pView->GetWrtShell().MakeVisible(pTextObj->GetLogicRect());

                    pSdrView->SdrBeginTextEdit(pTextObj, pPV, &pView->GetEditWin(), sal_False, this, pOutlView, sal_True, sal_True);
                }
                else
                    SetUpdateMode(sal_False);
            }
            if ( !bNextDoc )
                pTextObj = NULL;
        }
    }

    ClearModifyFlag();

    return bNextDoc;
}

//////////////////////////////////////////////////////////////////////

