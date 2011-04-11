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

#include "DrawDocShell.hxx"

#include "app.hrc"

#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>

#include <svx/ofaitem.hxx>
#include <svx/svxerr.hxx>
#include <svx/dialmgr.hxx>
#include <svl/srchitem.hxx>
#include <svx/srchdlg.hxx>
#ifdef _OUTLINER_HXX
#include <editeng/outliner.hxx>
#endif
#include <sfx2/request.hxx>
#include <svl/style.hxx>
#include <svx/drawitem.hxx>
#include <editeng/unolingu.hxx>
#include <com/sun/star/i18n/TextConversionOption.hpp>


#include "strings.hrc"
#include "glob.hrc"
#include "res_bmp.hrc"

#include "app.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "sdattr.hxx"
#include "fusearch.hxx"
#include "ViewShell.hxx"
#include "View.hxx"
#include "slideshow.hxx"
#include "fuhhconv.hxx"
#include "slideshow.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

namespace sd {

#define POOL_BUFFER_SIZE        (sal_uInt16)32768
#define BASIC_BUFFER_SIZE       (sal_uInt16)8192
#define DOCUMENT_BUFFER_SIZE    (sal_uInt16)32768

/*************************************************************************
|*
|* SFX-Requests bearbeiten
|*
\************************************************************************/

void DrawDocShell::Execute( SfxRequest& rReq )
{
    if(mpViewShell && SlideShow::IsRunning( mpViewShell->GetViewShellBase() ))
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
                (const SvxSearchItem*) &pReqArgs->Get(SID_SEARCH_ITEM);

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
                // End Search&Replace in all docshells
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

                if( !xFuSearch.is() && mpViewShell )
                {
                    ::sd::View* pView = mpViewShell->GetView();
                    SetDocShellFunction( FuSearch::Create( mpViewShell, mpViewShell->GetActiveWindow(), pView, mpDoc, rReq ) );
                    xFuSearch.set( dynamic_cast< FuSearch* >( GetDocShellFunction().get() ) );
                }

                if( xFuSearch.is() )
                {
                    const SvxSearchItem* pSearchItem =
                    (const SvxSearchItem*) &pReqArgs->Get(SID_SEARCH_ITEM);

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
            const sal_uLong nOldSwapMode = mpDoc->GetSwapGraphicsMode();

            mpDoc->SetSwapGraphicsMode( SDR_SWAPGRAPHICSMODE_TEMP );
            ExecuteSlot( rReq, SfxObjectShell::GetStaticInterface() );
            mpDoc->SetSwapGraphicsMode( nOldSwapMode );
        }
        break;

        case SID_HANGUL_HANJA_CONVERSION:
        {
            if( mpViewShell )
            {
                FunctionReference aFunc( FuHangulHanjaConversion::Create( mpViewShell, mpViewShell->GetActiveWindow(), mpViewShell->GetView(), mpDoc, rReq ) );
                static_cast< FuHangulHanjaConversion* >( aFunc.get() )->StartConversion( LANGUAGE_KOREAN, LANGUAGE_KOREAN, NULL, i18n::TextConversionOption::CHARACTER_BY_CHARACTER, sal_True );
            }
        }
        break;

        case SID_CHINESE_CONVERSION:
        {
            if( mpViewShell )
            {
                FunctionReference aFunc( FuHangulHanjaConversion::Create( mpViewShell, mpViewShell->GetActiveWindow(), mpViewShell->GetView(), mpDoc, rReq ) );
                static_cast< FuHangulHanjaConversion* >( aFunc.get() )->StartChineseConversion();
            }
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
    pBasePool->SetSearchMask(SD_STYLE_FAMILY_GRAPHICS, SFXSTYLEBIT_USERDEF | SFXSTYLEBIT_USED);
}


void DrawDocShell::SetDocShellFunction( const ::sd::FunctionReference& xFunction )
{
    if( mxDocShellFunction.is() )
        mxDocShellFunction->Dispose();

    mxDocShellFunction = xFunction;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
