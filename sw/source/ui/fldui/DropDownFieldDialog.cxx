/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DropDownFieldDialog.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:45:32 $
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

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif



#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _FLDMGR_HXX
#include <fldmgr.hxx>
#endif
#ifndef _MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SW_DROPDOWNFIELDDIALOG_HXX
#include <DropDownFieldDialog.hxx>
#endif
#ifndef _FLDDROPDOWN_HXX
#include <flddropdown.hxx>
#endif
#ifndef _FLDUI_HRC
#include <fldui.hrc>
#endif
#ifndef _SW_DROPDOWNFIELDDIALOG_HRC
#include <DropDownFieldDialog.hrc>
#endif

using namespace ::com::sun::star;


/*--------------------------------------------------------------------
    Beschreibung: Feldeinfuegen bearbeiten
 --------------------------------------------------------------------*/

sw::DropDownFieldDialog::DropDownFieldDialog( Window *pParent, SwWrtShell &rS,
                              SwField* pField, BOOL bNextButton ) :

    SvxStandardDialog(pParent,  SW_RES(DLG_FLD_DROPDOWN)),
    aItemsFL(       this, SW_RES( FL_ITEMS       )),
    aListItemsLB(   this, SW_RES( LB_LISTITEMS   )),

    aOKPB(          this, SW_RES( PB_OK          )),
    aCancelPB(      this, SW_RES( PB_CANCEL      )),
    aNextPB(        this, SW_RES( PB_NEXT        )),
    aHelpPB(        this, SW_RES( PB_HELP        )),

    aEditPB(        this, SW_RES( PB_EDIT        )),

    rSh( rS ),
    pDropField(0)
{
    Link aButtonLk = LINK(this, DropDownFieldDialog, ButtonHdl);
    aEditPB.SetClickHdl(aButtonLk);
    if( bNextButton )
    {
        aNextPB.Show();
        aNextPB.SetClickHdl(aButtonLk);
    }
    else
    {
        long nDiff = aCancelPB.GetPosPixel().Y() - aOKPB.GetPosPixel().Y();
        Point aPos = aHelpPB.GetPosPixel();
        aPos.Y() -= nDiff;
        aHelpPB.SetPosPixel(aPos);
    }
    if( RES_DROPDOWN == pField->GetTyp()->Which() )
    {
        //
        pDropField = (SwDropDownField*)pField;
        String sTitle = GetText();
        sTitle += pDropField->GetPar2();
        SetText(sTitle);
        uno::Sequence< rtl::OUString > aItems = pDropField->GetItemSequence();
        const rtl::OUString* pArray = aItems.getConstArray();
        for(sal_Int32 i = 0; i < aItems.getLength(); i++)
            aListItemsLB.InsertEntry(pArray[i]);
        aListItemsLB.SelectEntry(pDropField->GetSelectedItem());
    }

    BOOL bEnable = !rSh.IsCrsrReadonly();
    aOKPB.Enable( bEnable );

    aListItemsLB.GrabFocus();
    FreeResource();
}

sw::DropDownFieldDialog::~DropDownFieldDialog()
{
}

/*--------------------------------------------------------------------

 --------------------------------------------------------------------*/

void sw::DropDownFieldDialog::Apply()
{
    if(pDropField)
    {
        String sSelect = aListItemsLB.GetSelectEntry();
        if(pDropField->GetPar1() != sSelect)
        {
            rSh.StartAllAction();

            SwDropDownField * pCopy = (SwDropDownField *) pDropField->Copy();

            pCopy->SetPar1(sSelect);
            rSh.SwEditShell::UpdateFlds(*pCopy);

            delete pCopy;

            rSh.SetUndoNoResetModified();
            rSh.EndAllAction();
        }
    }
}
/* -----------------17.06.2003 10:50-----------------

 --------------------------------------------------*/
IMPL_LINK(sw::DropDownFieldDialog, ButtonHdl, PushButton*, pButton)
{
    EndDialog(&aNextPB == pButton ? RET_OK : RET_YES );
    return 0;
}

