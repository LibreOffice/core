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

#include "sdmod.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "sdattr.hxx"
#include "fusearch.hxx"
#include "ViewShell.hxx"
#include "View.hxx"
#include "slideshow.hxx"
#include "fuhhconv.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

namespace sd {

/**
 * Handles SFX-Requests
 */
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

                // would be nice to have an an assign operation at SearchItem
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
            // no action needed
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

                    // would be nice to have an an assign operation at SearchItem
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

        case SID_GET_COLORLIST:
        {
            SvxColorListItem* pColItem = (SvxColorListItem*) GetItem( SID_COLOR_TABLE );
            XColorListRef pList = pColItem->GetColorList();
            rReq.SetReturnValue( OfaRefItem<XColorList>( SID_GET_COLORLIST, pList ) );
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

void DrawDocShell::SetDocShellFunction( const ::sd::FunctionReference& xFunction )
{
    if( mxDocShellFunction.is() )
        mxDocShellFunction->Dispose();

    mxDocShellFunction = xFunction;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
