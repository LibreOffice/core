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

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif

#include <wrtsh.hxx>
#include <fldbas.hxx>
#include <fldmgr.hxx>
#include <vcl/msgbox.hxx>
#include <DropDownFieldDialog.hxx>
#include <flddropdown.hxx>
#include <fldui.hrc>
#include <DropDownFieldDialog.hrc>

using namespace ::com::sun::star;


/*--------------------------------------------------------------------
    Beschreibung: Feldeinfuegen bearbeiten
 --------------------------------------------------------------------*/

sw::DropDownFieldDialog::DropDownFieldDialog( Window *pParent, SwWrtShell &rS,
                              SwField* pField, sal_Bool bNextButton ) :

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

    sal_Bool bEnable = !rSh.IsCrsrReadonly();
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

            ::std::auto_ptr<SwDropDownField> const pCopy(
                static_cast<SwDropDownField *>( pDropField->CopyField() ) );

            pCopy->SetPar1(sSelect);
            rSh.SwEditShell::UpdateFlds(*pCopy);

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

