/*************************************************************************
 *
 *  $RCSfile: fontworkbar.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:51:35 $
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

#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif
#ifndef _SFXOBJFACE_HXX
#include <sfx2/objface.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif
#ifndef _SVDOASHP_HXX
#include "svdoashp.hxx"
#endif
#ifndef _SVX_DIALOGS_HRC
#include "dialogs.hrc"
#endif
#ifndef _SVDVIEW_HXX
#include "svdview.hxx"
#endif

#include "fontworkbar.hxx"
#include "fontworkgallery.hxx"

using namespace ::svx;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

/*************************************************************************
|*
|* Standardinterface deklarieren (Die Slotmap darf nicht leer sein, also
|* tragen wir etwas ein, was hier (hoffentlich) nie vorkommt).
|*
\************************************************************************/

#define ShellClass FontworkBar

SFX_SLOTMAP(FontworkBar)
{
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

SFX_IMPL_INTERFACE(FontworkBar, SfxShell, SVX_RES(RID_SVX_FONTWORK_BAR))
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT, SVX_RES(RID_SVX_FONTWORK_BAR) );
}

TYPEINIT1( FontworkBar, SfxShell );


/*************************************************************************
|*
|* Standard-Konstruktor
|*
\************************************************************************/

FontworkBar::FontworkBar(SfxViewShell* pViewShell )
: SfxShell(pViewShell)
{
//  SdDrawDocShell* pDocShell = pSdViewShell->GetDocSh();
//  SetPool(&pDocShell->GetPool());
//  SetUndoManager(pDocShell->GetUndoManager());
//  SetRepeatTarget(pSdView);
    SetHelpId( SVX_INTERFACE_FONTWORK_BAR );
    SetName( String( SVX_RES( RID_SVX_FONTWORK_BAR) ));
}


/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FontworkBar::~FontworkBar()
{
    SetRepeatTarget(NULL);
}

static Window* ImpGetViewWin(SdrView* pView)
{
    if( pView )
    {
        USHORT nAnz=pView->GetWinCount();
        for (USHORT nNum=0; nNum<nAnz; nNum++) {
            OutputDevice* pOut=pView->GetWin(nNum);
            if (pOut->GetOutDevType()==OUTDEV_WINDOW) {
                return (Window*)pOut;
            }
        }
    }
    return 0L;
}

#include "gallery.hxx"
#include "fmmodel.hxx"
#include "fmpage.hxx"
#include <svtools/itempool.hxx>

void FontworkBar::execute( SdrView* pSdrView, SfxRequest& rReq )
{
    execute( pSdrView, rReq.GetSlot() );
}

void FontworkBar::execute( SdrView* pSdrView, sal_uInt16 nSID )
{
    switch( nSID )
    {
        case SID_DRAW_FONTWORK:
        case SID_DRAW_FONTWORK_VERTICAL:
        case SID_FONTWORK_GALLERY_FLOATER:
            {
                FontWorkGalleryDialog aDlg( pSdrView, ImpGetViewWin(pSdrView), nSID );
                aDlg.Execute();
            }
            break;
        case SID_FONTWORK_SHAPE_FLOATER:
            {
                // Model
                FmFormModel* pModel = new FmFormModel();
                pModel->GetItemPool().FreezeIdRanges();

                // Page
                FmFormPage* pFmPage = new FmFormPage( *pModel, NULL );
                pModel->InsertPage( pFmPage, 0 );

                SdrObject* pSingleObj = NULL;
                if (pSdrView->GetMarkedObjectList().GetMarkCount()==1)
                    pSingleObj = pSdrView->GetMarkedObjectList().GetMark(0)->GetObj();

                if( pSingleObj )
                {
                    SdrObject* pNewObj = pSingleObj->Clone();
                    pFmPage->InsertObject( pNewObj );

                    GalleryExplorer::InsertSdrObj( 36, *pModel );
                }

                delete pModel;
                break;
            }
            break;
        case SID_FONTWORK_SHAPE:
        case SID_FONTWORK_ALIGNMENT:
        case SID_FONTWORK_CHARACTER_SPACING:
        case SID_FONTWORK_KERN_CHARACTER_PAIRS:
            break;
    };
}

void FontworkBar::getState( SdrView* pSdrView, SfxItemSet& rSet )
{
/*
SID_FONTWORK_SHAPE
SID_FONTWORK_ALIGNMENT
SID_FONTWORK_CHARACTER_SPACING
SID_FONTWORK_KERN_CHARACTER_PAIRS
*/
}
