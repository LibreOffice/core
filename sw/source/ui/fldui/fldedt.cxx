/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fldedt.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:24:21 $
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

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SVX_OPTGENRL_HXX //autogen
#include <svx/optgenrl.hxx>
#endif

#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _DBFLD_HXX
#include <dbfld.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _FLDDB_HXX
#include <flddb.hxx>
#endif
#ifndef _FLDDINF_HXX
#include <flddinf.hxx>
#endif
#ifndef _FLDVAR_HXX
#include <fldvar.hxx>
#endif
#ifndef _FLDDOK_HXX
#include <flddok.hxx>
#endif
#ifndef _FLDFUNC_HXX
#include <fldfunc.hxx>
#endif
#ifndef _FLDREF_HXX
#include <fldref.hxx>
#endif
#ifndef _FLDEDT_HXX
#include <fldedt.hxx>
#endif
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
#endif


#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _FLDUI_HRC
#include <fldui.hrc>
#endif
#include "swabstdlg.hxx"
#include "dialog.hrc"

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>


namespace swui
{
    SwAbstractDialogFactory * GetFactory();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFldEditDlg::SwFldEditDlg(SwView& rVw) :
    SfxSingleTabDialog(&rVw.GetViewFrame()->GetWindow(), 0, 0),
    pSh         (rVw.GetWrtShellPtr()),
    aPrevBT     (this, SW_RES(BTN_FLDEDT_PREV)),
    aNextBT     (this, SW_RES(BTN_FLDEDT_NEXT)),
    aAddressBT  (this, SW_RES(PB_FLDEDT_ADDRESS))
{
    SwFldMgr aMgr(pSh);

    SwField *pCurFld = aMgr.GetCurFld();
    if(!pCurFld)
        return;

    pSh->SetCareWin(this);

    /* #108536# Only create selection if there is none
        already. Normalize PaM instead of swapping. */
    if ( ! pSh->HasSelection() )
        pSh->Right(CRSR_SKIP_CHARS, TRUE, 1, FALSE );

    pSh->NormalizePam();

    USHORT nGroup = aMgr.GetGroup(FALSE, pCurFld->GetTypeId(), pCurFld->GetSubType());

    CreatePage(nGroup);

    GetOKButton()->SetClickHdl(LINK(this, SwFldEditDlg, OKHdl));

    // Buttons selbst positionieren, da sie sonst bei unterschiedlichen
    // Fontgroessen im Wald stehen, und da PB im SingleTabDlg feste Pixelgroessen
    // fuer seine Buttons und die Dialogbreite verwendet.
    aPrevBT.SetPosPixel(Point(GetOKButton()->GetPosPixel().X(), aPrevBT.GetPosPixel().Y()));
    USHORT nWidth = static_cast< USHORT >(GetOKButton()->GetOutputSize().Width() / 2 - 3);
    Size aNewSize(LogicToPixel(Size(nWidth, GetOKButton()->GetOutputSize().Height())));
    aPrevBT.SetSizePixel(aNewSize);

    aNextBT.SetSizePixel(aPrevBT.GetSizePixel());

    long nXPos = GetOKButton()->GetPosPixel().X() + GetOKButton()->GetSizePixel().Width()
                    - aNextBT.GetSizePixel().Width() - 1;
    aNextBT.SetPosPixel(Point(nXPos, aNextBT.GetPosPixel().Y()));

    aAddressBT.SetPosPixel(Point(GetOKButton()->GetPosPixel().X(), aAddressBT.GetPosPixel().Y()));
    aAddressBT.SetSizePixel(GetOKButton()->GetSizePixel());

    aPrevBT.SetClickHdl(LINK(this, SwFldEditDlg, NextPrevHdl));
    aNextBT.SetClickHdl(LINK(this, SwFldEditDlg, NextPrevHdl));

    aAddressBT.SetClickHdl(LINK(this, SwFldEditDlg, AddressHdl));
    aAddressBT.SetHelpId(HID_FLDEDT_ADDRESS);

    Init();
}

/*--------------------------------------------------------------------
    Beschreibung: Controlls initialisieren
 --------------------------------------------------------------------*/

void SwFldEditDlg::Init()
{
    SwFldPage* pTabPage = (SwFldPage*)GetTabPage();

    if( pTabPage )
    {
        SwFldMgr& rMgr = pTabPage->GetFldMgr();

        SwField *pCurFld = rMgr.GetCurFld();

        if(!pCurFld)
            return;

        // Traveling nur bei mehr als einem Feld
        pSh->StartAction();
        pSh->CreateCrsr();

        BOOL bMove = rMgr.GoNext();
        if( bMove )
            rMgr.GoPrev();
        aNextBT.Enable(bMove);

        if( 0 != ( bMove = rMgr.GoPrev() ) )
            rMgr.GoNext();
        aPrevBT.Enable( bMove );

        if (pCurFld->GetTypeId() == TYP_EXTUSERFLD)
            aAddressBT.Show();

        pSh->DestroyCrsr();
        pSh->EndAction();
    }

    GetOKButton()->Enable( !pSh->IsReadOnlyAvailable() ||
                           !pSh->HasReadonlySel() );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxTabPage* SwFldEditDlg::CreatePage(USHORT nGroup)
{
    // TabPage erzeugen
    SfxTabPage* pTabPage = 0;
    USHORT nHelpId = 0;

    switch (nGroup)
    {
        case GRP_DOC:
            pTabPage = SwFldDokPage::Create(this, *(SfxItemSet*)0);
            nHelpId = HID_EDIT_FLD_DOK;
            break;
        case GRP_FKT:
            pTabPage = SwFldFuncPage::Create(this, *(SfxItemSet*)0);
            nHelpId = HID_EDIT_FLD_FUNC;
            break;
        case GRP_REF:
            pTabPage = SwFldRefPage::Create(this, *(SfxItemSet*)0);
            nHelpId = HID_EDIT_FLD_REF;
            break;
        case GRP_REG:
            {
                SfxObjectShell* pDocSh = SfxObjectShell::Current();
                SfxItemSet* pSet = new SfxItemSet( pDocSh->GetPool(), SID_DOCINFO, SID_DOCINFO );
                using namespace ::com::sun::star;
                uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                    pDocSh->GetModel(), uno::UNO_QUERY_THROW);
                uno::Reference<document::XDocumentProperties> xDocProps
                    = xDPS->getDocumentProperties();
                uno::Reference< beans::XPropertySet > xUDProps(
                    xDocProps->getUserDefinedProperties(),
                    uno::UNO_QUERY_THROW);
                uno::Reference< beans::XPropertySetInfo > xSetInfo
                    = xUDProps->getPropertySetInfo();
                const uno::Sequence< beans::Property > props
                    = xSetInfo->getProperties();
                uno::Sequence< ::rtl::OUString > names(props.getLength());
                for (sal_Int32 i = 0; i < props.getLength(); ++i) {
                    names[i] = props[i].Name;
                }
                pSet->Put( SfxUnoAnyItem( SID_DOCINFO, uno::makeAny(names) ) );
                pTabPage = SwFldDokInfPage::Create(this, *pSet);
                nHelpId = HID_EDIT_FLD_DOKINF;
                break;
            }
        case GRP_DB:
            pTabPage = SwFldDBPage::Create(this, *(SfxItemSet*)0);
            static_cast<SwFldDBPage*>(pTabPage)->SetWrtShell(*pSh);
            nHelpId = HID_EDIT_FLD_DB;
            break;
        case GRP_VAR:
            pTabPage = SwFldVarPage::Create(this, *(SfxItemSet*)0);
            nHelpId = HID_EDIT_FLD_VAR;
            break;

    }
    pTabPage->SetHelpId(nHelpId);
    static_cast<SwFldPage*>(pTabPage)->SetWrtShell(pSh);

    SetTabPage(pTabPage);

    String sTitle(GetText());
    sTitle.Insert(String::CreateFromAscii(": "), 0);
    sTitle.Insert(SW_RESSTR(STR_FLD_EDIT_DLG), 0);
    SetText(sTitle);

    return pTabPage;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFldEditDlg::~SwFldEditDlg()
{
    pSh->SetCareWin(NULL);
    pSh->EnterStdMode();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldEditDlg::EnableInsert(BOOL bEnable)
{
    if( bEnable && pSh->IsReadOnlyAvailable() && pSh->HasReadonlySel() )
        bEnable = FALSE;
    GetOKButton()->Enable( bEnable );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldEditDlg::InsertHdl()
{
    GetOKButton()->Click();
}

/*--------------------------------------------------------------------
     Beschreibung: Aendern des Feldes anstossen
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldEditDlg, OKHdl, Button *, EMPTYARG )
{
    if (GetOKButton()->IsEnabled())
    {
        SfxTabPage* pTabPage = GetTabPage();
        if (pTabPage)
        {
            pTabPage->FillItemSet(*(SfxItemSet*)0);

        }
        EndDialog( RET_OK );
    }

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

short SwFldEditDlg::Execute()
{
    // Ohne TabPage kein Dialog
    return GetTabPage() ? Dialog::Execute() : RET_CANCEL;
}

/*--------------------------------------------------------------------
    Beschreibung: Traveling zwishen Feldern gleichen Typs
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldEditDlg, NextPrevHdl, Button *, pButton )
{
    BOOL bNext = pButton == &aNextBT;

    pSh->EnterStdMode();

    SwFieldType *pOldTyp = 0;
    SwFldPage* pTabPage = (SwFldPage*)GetTabPage();

    //#112462# FillItemSet may delete the current field
    //that's why it has to be called before accessing the current field
    if( GetOKButton()->IsEnabled() )
        pTabPage->FillItemSet(*(SfxItemSet*)0);

    SwFldMgr& rMgr = pTabPage->GetFldMgr();
    SwField *pCurFld = rMgr.GetCurFld();
    if (pCurFld->GetTypeId() == TYP_DBFLD)
        pOldTyp = (SwDBFieldType*)pCurFld->GetTyp();

    rMgr.GoNextPrev( bNext, pOldTyp );
    pCurFld = rMgr.GetCurFld();

    /* #108536# Only create selection if there is none
        already. Normalize PaM instead of swapping. */
    if ( ! pSh->HasSelection() )
        pSh->Right(CRSR_SKIP_CHARS, TRUE, 1, FALSE );

    pSh->NormalizePam();

    USHORT nGroup = rMgr.GetGroup(FALSE, pCurFld->GetTypeId(), pCurFld->GetSubType());

    if (nGroup != pTabPage->GetGroup())
        pTabPage = (SwFldPage*)CreatePage(nGroup);

    pTabPage->EditNewField();

    Init();

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldEditDlg, AddressHdl, PushButton *, EMPTYARG )
{
    SwFldPage* pTabPage = (SwFldPage*)GetTabPage();
    SwFldMgr& rMgr = pTabPage->GetFldMgr();
    SwField *pCurFld = rMgr.GetCurFld();

    SfxItemSet aSet( pSh->GetAttrPool(),
                        SID_FIELD_GRABFOCUS, SID_FIELD_GRABFOCUS,
                        0L );

    USHORT nEditPos = UNKNOWN_EDIT;

    switch(pCurFld->GetSubType())
    {
        case EU_FIRSTNAME:  nEditPos = FIRSTNAME_EDIT;  break;
        case EU_NAME:       nEditPos = LASTNAME_EDIT;   break;
        case EU_SHORTCUT:   nEditPos = SHORTNAME_EDIT;  break;
        case EU_COMPANY:    nEditPos = COMPANY_EDIT;    break;
        case EU_STREET:     nEditPos = STREET_EDIT;     break;
        case EU_TITLE:      nEditPos = TITLE_EDIT;      break;
        case EU_POSITION:   nEditPos = POSITION_EDIT;   break;
        case EU_PHONE_PRIVATE:nEditPos = TELPRIV_EDIT;  break;
        case EU_PHONE_COMPANY:nEditPos = TELCOMPANY_EDIT;   break;
        case EU_FAX:        nEditPos = FAX_EDIT;        break;
        case EU_EMAIL:      nEditPos = EMAIL_EDIT;      break;
        case EU_COUNTRY:    nEditPos = COUNTRY_EDIT;    break;
        case EU_ZIP:        nEditPos = PLZ_EDIT;        break;
        case EU_CITY:       nEditPos = CITY_EDIT;       break;
        case EU_STATE:      nEditPos = STATE_EDIT;      break;

        default:            nEditPos = UNKNOWN_EDIT;    break;

    }
    aSet.Put(SfxUInt16Item(SID_FIELD_GRABFOCUS, nEditPos));
    SwAbstractDialogFactory* pFact = swui::GetFactory();
    DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");

    AbstractSfxSingleTabDialog* pDlg = pFact->CreateSfxSingleTabDialog( this, aSet, RC_DLG_ADDR );
    DBG_ASSERT(pDlg, "Dialogdiet fail!");
    if(RET_OK == pDlg->Execute())
    {
        pSh->UpdateFlds( *pCurFld );
    }
    delete pDlg;
    return 0;
}


