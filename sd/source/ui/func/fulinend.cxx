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
#include "precompiled_sd.hxx"


#include "fulinend.hxx"
#include <svx/xtable.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svx/svdobj.hxx>
#include <svx/svdopath.hxx>
#include <vcl/msgbox.hxx>

#include "strings.hrc"
#include "ViewShell.hxx"
#include "helpids.h"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "View.hxx"
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif

namespace sd {

#define BITMAP_WIDTH  32
#define BITMAP_HEIGHT 12

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuLineEnd::FuLineEnd(ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
                    SdDrawDocument* pDoc, SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuLineEnd::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuLineEnd( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuLineEnd::DoExecute( SfxRequest& )
{
    const SdrObject* pSelected = mpView->getSelectedIfSingle();

    if( pSelected )
    {
        const SdrObject* pNewObj;
        SdrObject* pConvPolyObj = NULL;

        if( dynamic_cast< const SdrPathObj* >(pSelected) )
        {
            pNewObj = pSelected;
        }
        else
        {
            SdrObjTransformInfoRec aInfoRec;
            pSelected->TakeObjInfo( aInfoRec );

            if( aInfoRec.mbCanConvToPath &&
                pSelected->GetObjInventor() == SdrInventor &&
                pSelected->GetObjIdentifier() != OBJ_GRUP )
                // mbCanConvToPath ist bei Gruppenobjekten true,
                // stuerzt aber bei ConvertToPathObj() ab !
            {
                pNewObj = pConvPolyObj = pSelected->ConvertToPolyObj( true, false );

                if( !pNewObj || !dynamic_cast< const SdrPathObj* >(pNewObj) )
                    return; // Abbruch, zusaetzliche Sicherheit, die bei
                            // Gruppenobjekten aber nichts bringt.
            }
            else return; // Abbruch
        }

        const ::basegfx::B2DPolyPolygon aPolyPolygon = ( (SdrPathObj*) pNewObj )->getB2DPolyPolygonInObjectCoordinates();

        // Loeschen des angelegten PolyObjektes
        deleteSdrObjectSafeAndClearPointer( pConvPolyObj );

        XLineEndList* pLineEndList = mpDoc->GetLineEndList();
        XLineEndEntry* pEntry;

        String aNewName( SdResId( STR_LINEEND ) );
        String aDesc( SdResId( STR_DESC_LINEEND ) );
        String aName;

        long nCount = pLineEndList->Count();
        long j = 1;
        bool bDifferent = false;

        while( !bDifferent )
        {
            aName = aNewName;
            aName.Append( sal_Unicode(' ') );
            aName.Append( UniString::CreateFromInt32( j++ ) );
            bDifferent = true;
            for( long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == pLineEndList->GetLineEnd( i )->GetName() )
                    bDifferent = false;
            }
        }

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        AbstractSvxNameDialog* pDlg = pFact ? pFact->CreateSvxNameDialog( NULL, aName, aDesc ) : 0;

        if( pDlg )
        {
            pDlg->SetEditHelpId( HID_SD_NAMEDIALOG_LINEEND );

            if( pDlg->Execute() == RET_OK )
            {
                pDlg->GetName( aName );
                bDifferent = true;

                for( long i = 0; i < nCount && bDifferent; i++ )
                {
                    if( aName == pLineEndList->GetLineEnd( i )->GetName() )
                        bDifferent = false;
                }

                if( bDifferent )
                {
                    pEntry = new XLineEndEntry( aPolyPolygon, aName );
                    pLineEndList->Insert( pEntry, LIST_APPEND);
                }
                else
                {
                    String aStr(SdResId( STR_WARN_NAME_DUPLICATE ));
                    WarningBox aWarningBox( mpWindow, WinBits( WB_OK ),
                         aStr );
                    aWarningBox.Execute();
                }
            }
        }
        delete pDlg;
    }
}

void FuLineEnd::Activate()
{
}

void FuLineEnd::Deactivate()
{
}

} // end of namespace sd
