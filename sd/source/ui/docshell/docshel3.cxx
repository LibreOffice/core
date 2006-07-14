/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docshel3.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-14 08:25:18 $
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

#include "DrawDocShell.hxx"

#include "app.hrc"

#define ITEMID_FONTLIST         SID_ATTR_CHAR_FONTLIST
#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST    SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST       SID_HATCH_LIST
#define ITEMID_BITMAP_LIST      SID_BITMAP_LIST
#define ITEMID_DASH_LIST        SID_DASH_LIST
#define ITEMID_LINEEND_LIST     SID_LINEEND_LIST
#define ITEMID_SEARCH           SID_SEARCH_ITEM

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif

#include <svx/ofaitem.hxx>

#ifndef _SVXERR_HXX
#include <svx/svxerr.hxx>
#endif
#ifndef _SVX_DIALMGR_HXX
#include <svx/dialmgr.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef _SVX_SRCHDLG_HXX
#include <svx/srchdlg.hxx>
#endif
#ifdef _OUTLINER_HXX
#include <svx/outliner.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _SVX_DRAWITEM_HXX //autogen
#include <svx/drawitem.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_TEXTCONVERSIONOPTION_HPP_
#include <com/sun/star/i18n/TextConversionOption.hpp>
#endif

#pragma hdrstop

#include "strings.hrc"
#include "glob.hrc"
#include "res_bmp.hrc"

#include "app.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "sdattr.hxx"
#ifndef SD_FU_SEARCH_HXX
#include "fusearch.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef _SD_SLIDESHOW_HXX
#include "slideshow.hxx"
#endif
#include "fuhhconv.hxx"


using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

namespace sd {

#define POOL_BUFFER_SIZE        (USHORT)32768
#define BASIC_BUFFER_SIZE       (USHORT)8192
#define DOCUMENT_BUFFER_SIZE    (USHORT)32768

/*************************************************************************
|*
|* SFX-Requests bearbeiten
|*
\************************************************************************/

void DrawDocShell::Execute( SfxRequest& rReq )
{
    if(pViewShell && pViewShell->GetSlideShow())
    {
        // during a running presentation no slot will be executed
        return;
    }

    switch ( rReq.GetSlot() )
    {
        case SID_SEARCH_ITEM:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if (pReqArgs)
            {
                const SvxSearchItem* pSearchItem =
                (const SvxSearchItem*) &pReqArgs->Get(ITEMID_SEARCH);

                // ein Zuweisungsoperator am SearchItem waer nicht schlecht...
                SvxSearchItem* pAppSearchItem = SD_MOD()->GetSearchItem();
                delete pAppSearchItem;
                pAppSearchItem = (SvxSearchItem*) pSearchItem->Clone();
                SD_MOD()->SetSearchItem(pAppSearchItem);
            }

            rReq.Done();
        }
        break;

        case FID_SEARCH_ON:
        {
            // Keine Aktion noetig
            rReq.Done();
        }
        break;

        case FID_SEARCH_OFF:
        {
            if( dynamic_cast< FuSearch* >(mxDocShellFunction.get()) )
            {
                // Suchen&Ersetzen in allen DocShells beenden
                SfxObjectShell* pFirstShell = SfxObjectShell::GetFirst();
                SfxObjectShell* pShell = pFirstShell;

                while (pShell)
                {
                    if (pShell->ISA(DrawDocShell))
                    {
                        ( (DrawDocShell*) pShell)->CancelSearching();
                    }

                    pShell = SfxObjectShell::GetNext(*pShell);

                    if (pShell == pFirstShell)
                    {
                        pShell = NULL;
                    }
                }

                SetDocShellFunction(0);
                Invalidate();
                rReq.Done();
            }
        }
        break;

        case FID_SEARCH_NOW:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if ( pReqArgs )
            {
                rtl::Reference< FuSearch > xFuSearch( dynamic_cast< FuSearch* >( GetDocShellFunction().get() ) );

                if( !xFuSearch.is() )
                {
                    ::sd::View* pView = pViewShell->GetView();
                    SetDocShellFunction( FuSearch::Create( pViewShell, pViewShell->GetActiveWindow(), pView, pDoc, rReq ) );
                    xFuSearch.set( dynamic_cast< FuSearch* >( GetDocShellFunction().get() ) );
                }

                if( xFuSearch.is() )
                {
                    const SvxSearchItem* pSearchItem =
                    (const SvxSearchItem*) &pReqArgs->Get(ITEMID_SEARCH);

                    // ein Zuweisungsoperator am SearchItem waer nicht schlecht...
                    SvxSearchItem* pAppSearchItem = SD_MOD()->GetSearchItem();
                    delete pAppSearchItem;
                    pAppSearchItem = (SvxSearchItem*)pSearchItem->Clone();
                    SD_MOD()->SetSearchItem(pAppSearchItem);
                    xFuSearch->SearchAndReplace(pSearchItem);
                }
            }

            rReq.Done();
        }
        break;

        case SID_CLOSEDOC:
        {
//            SfxObjectShell::DoClose();
            ExecuteSlot(rReq, SfxObjectShell::GetStaticInterface());
        }
        break;

        case SID_GET_COLORTABLE:
        {
            //  passende ColorTable ist per PutItem gesetzt worden
            SvxColorTableItem* pColItem = (SvxColorTableItem*) GetItem( SID_COLOR_TABLE );
            XColorTable* pTable = pColItem->GetColorTable();
            rReq.SetReturnValue( OfaPtrItem( SID_GET_COLORTABLE, pTable ) );
        }
        break;

        case SID_VERSION:
        {
            const ULONG nOldSwapMode = pDoc->GetSwapGraphicsMode();

            pDoc->SetSwapGraphicsMode( SDR_SWAPGRAPHICSMODE_TEMP );
            ExecuteSlot( rReq, SfxObjectShell::GetStaticInterface() );
            pDoc->SetSwapGraphicsMode( nOldSwapMode );
        }
        break;

        case SID_HANGUL_HANJA_CONVERSION:
        {
            FunctionReference aFunc( FuHangulHanjaConversion::Create( pViewShell, pViewShell->GetActiveWindow(), pViewShell->GetView(), pDoc, rReq ) );
            static_cast< FuHangulHanjaConversion* >( aFunc.get() )->StartConversion( LANGUAGE_KOREAN, LANGUAGE_KOREAN, NULL, i18n::TextConversionOption::CHARACTER_BY_CHARACTER, sal_True );
        }
        break;

        case SID_CHINESE_CONVERSION:
        {
            FunctionReference aFunc( FuHangulHanjaConversion::Create( pViewShell, pViewShell->GetActiveWindow(), pViewShell->GetView(), pDoc, rReq ) );
            static_cast< FuHangulHanjaConversion* >( aFunc.get() )->StartChineseConversion();
        }
        break;

        default:
        break;
    }
}

/*************************************************************************
|*
|* Suchmaske fuer Organizer
|*
\************************************************************************/

void DrawDocShell::SetOrganizerSearchMask(SfxStyleSheetBasePool* pBasePool) const
{
    pBasePool->SetSearchMask(SFX_STYLE_FAMILY_PARA,
                             SFXSTYLEBIT_USERDEF | SFXSTYLEBIT_USED);
}


void DrawDocShell::SetDocShellFunction( const ::sd::FunctionReference& xFunction )
{
    if( mxDocShellFunction.is() )
        mxDocShellFunction->Dispose();

    mxDocShellFunction = xFunction;
}

} // end of namespace sd
