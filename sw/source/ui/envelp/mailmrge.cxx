/*************************************************************************
 *
 *  $RCSfile: mailmrge.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:26:11 $
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
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _MAILENUM_HXX //autogen
#include <goodies/mailenum.hxx>
#endif
#ifndef _SVX_MULTIFIL_HXX
#include <svx/multifil.hxx>
#endif

#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _DBUI_HXX
#include <dbui.hxx>
#endif
#ifndef _PRTOPT_HXX
#include <prtopt.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif

#ifndef _ENVELP_HRC
#include <envelp.hrc>
#endif
#ifndef _MAILMRGE_HRC
#include <mailmrge.hrc>
#endif
#ifndef _MAILMRGE_HXX
#include <mailmrge.hxx>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_XFOLDERPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XROWLOCATE_HPP_
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XRESULTSETACCESS_HPP_
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONCHANGELISTENER_HPP_
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTABCONTROLLERMODEL_HPP_
#include <com/sun/star/awt/XTabControllerModel.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif

using namespace rtl;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdb;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::uno;
using namespace com::sun::star::frame;
using namespace com::sun::star::form;
using namespace com::sun::star;
using namespace com::sun::star::view;
using namespace ::com::sun::star::ui::dialogs;

#define C2S(cChar) UniString::CreateFromAscii(cChar)
#define C2U(cChar) ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(cChar))

/* -----------------------------05.06.01 13:54--------------------------------

 ---------------------------------------------------------------------------*/
struct SwMailMergeDlg_Impl
{
    Reference<XFormController> xFController;
    Reference<XSelectionChangeListener> xChgLstnr;
    Reference<XSelectionSupplier> xSelSupp;
};
/* -----------------------------05.06.01 13:47--------------------------------
    helper classes
 ---------------------------------------------------------------------------*/
class SwXSelChgLstnr_Impl : public cppu::WeakImplHelper1
<
    com::sun::star::view::XSelectionChangeListener
>
{
    SwMailMergeDlg& rParent;
public:
    SwXSelChgLstnr_Impl(SwMailMergeDlg& rParentDlg);
    ~SwXSelChgLstnr_Impl();

    virtual void SAL_CALL selectionChanged( const EventObject& aEvent ) throw (RuntimeException);
    virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException);
};
/* -----------------------------05.06.01 13:51--------------------------------

 ---------------------------------------------------------------------------*/
SwXSelChgLstnr_Impl::SwXSelChgLstnr_Impl(SwMailMergeDlg& rParentDlg) :
    rParent(rParentDlg)
{}
/* -----------------------------05.06.01 14:06--------------------------------

 ---------------------------------------------------------------------------*/
SwXSelChgLstnr_Impl::~SwXSelChgLstnr_Impl()
{}
/* -----------------------------05.06.01 14:06--------------------------------

 ---------------------------------------------------------------------------*/
void SwXSelChgLstnr_Impl::selectionChanged( const EventObject& aEvent ) throw (RuntimeException)
{
    //call the parent to enable selection mode
    Sequence <Any> aSelection;
    if(rParent.pImpl->xSelSupp.is())
        rParent.pImpl->xSelSupp->getSelection() >>= aSelection;

    sal_Bool bEnable = aSelection.getLength() > 0;
    rParent.aMarkedRB.Enable(bEnable);
    if(bEnable)
        rParent.aMarkedRB.Check();
    else if(rParent.aMarkedRB.IsChecked())
    {
        rParent.aAllRB.Check();
        rParent.m_aSelection.realloc(0);
    }
}
/* -----------------------------05.06.01 14:06--------------------------------

 ---------------------------------------------------------------------------*/
void SwXSelChgLstnr_Impl::disposing( const EventObject& Source ) throw (RuntimeException)
{
    DBG_ERROR("disposing")
}
/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/
SwMailMergeDlg::SwMailMergeDlg(Window* pParent, SwWrtShell& rShell,
         const String& rSourceName,
        const String& rTblName,
        sal_Int32 nCommandType,
        const Reference< XConnection>& _xConnection,
        Sequence< Any >* pSelection) :

    SvxStandardDialog(pParent, SW_RES(DLG_MAILMERGE)),
    pBeamerWin      (new Window(this, ResId(WIN_BEAMER))),
    aAllRB          (this, SW_RES(RB_ALL)),
    aMarkedRB       (this, SW_RES(RB_MARKED)),
    aFromRB         (this, SW_RES(RB_FROM)),
    aFromNF         (this, SW_RES(NF_FROM)),
    aBisFT          (this, SW_RES(FT_BIS)),
    aToNF           (this, SW_RES(NF_TO)),
    aRecordFL       (this, SW_RES(FL_RECORD)),

    aSeparatorFL    (this, SW_RES(FL_SEPARATOR)),
    aPrinterRB      (this, SW_RES(RB_PRINTER)),
    aMailingRB      (this, SW_RES(RB_MAILING)),
    aFileRB         (this, SW_RES(RB_FILE)),

    aSingleJobsCB   (this, SW_RES(CB_SINGLE_JOBS)),

    aPathFT         (this, SW_RES(FT_PATH)),
    aPathED         (this, SW_RES(ED_PATH)),
    aPathPB         (this, SW_RES(PB_PATH)),
    aFilenameFT     (this, SW_RES(FT_FILENAME)),
    aColumnRB       (this, SW_RES(RB_COLUMN)),
    aFilenameRB     (this, SW_RES(RB_FILENAME)),
    aColumnLB       (this, SW_RES(LB_COLUMN)),
    aFilenameED     (this, SW_RES(ED_FILENAME)),

    aAddressFT      (this, SW_RES(FT_ADDRESS)),
    aAddressFldLB   (this, SW_RES(LB_ADDRESSFLD)),
    aSubjectFT      (this, SW_RES(FT_SUBJECT)),
    aSubjectED      (this, SW_RES(ED_SUBJECT)),
    aFormatFT       (this, SW_RES(FT_FORMAT)),
    aAttachFT       (this, SW_RES(FT_ATTACH)),
    aAttachED       (this, SW_RES(ED_ATTACH)),
    aAttachPB       (this, SW_RES(PB_ATTACH)),
    aFormatSwCB     (this, SW_RES(CB_FORMAT_SW)),
    aFormatHtmlCB   (this, SW_RES(CB_FORMAT_HTML)),
    aFormatRtfCB    (this, SW_RES(CB_FORMAT_RTF)),
    aDestFL         (this, SW_RES(FL_DEST)),

    aOkBTN          (this, SW_RES(BTN_OK)),
    aCancelBTN      (this, SW_RES(BTN_CANCEL)),
    aHelpBTN        (this, SW_RES(BTN_HELP)),

    pImpl           (new SwMailMergeDlg_Impl),
    rSh             (rShell),
    rDBName         (rSourceName),
    rTableName      (rTblName),
    nMergeType      (DBMGR_MERGE_MAILING)

{
    FreeResource();
    //task #97066# mailing of form letters is currently not supported
    aMailingRB.Show(FALSE);
    aSubjectFT.Show(FALSE);
    aSubjectED.Show(FALSE);
    aFormatFT.Show(FALSE);
    aFormatSwCB.Show(FALSE);
    aFormatHtmlCB.Show(FALSE);
    aFormatRtfCB.Show(FALSE);
    aAttachFT.Show(FALSE);
    aAttachED.Show(FALSE);
    aAttachPB.Show(FALSE);

    Point aMailPos = aMailingRB.GetPosPixel();
    Point aFilePos = aFileRB.GetPosPixel();
    aFilePos.X() -= (aFilePos.X() - aMailPos.X()) /2;
    aFileRB.SetPosPixel(aFilePos);
    if(pSelection)
    {
        m_aSelection = *pSelection;
        //move all controls
        long nDiff = aRecordFL.GetPosPixel().Y() - pBeamerWin->GetPosPixel().Y();
        pBeamerWin->Show(FALSE);
        ::Size aSize = GetSizePixel();
        aSize.Height() -= nDiff;
        SetSizePixel(aSize);
        Window* aCntrlArr[] = {
            &aAllRB       ,
            &aMarkedRB    ,
            &aFromRB      ,
            &aFromNF      ,
            &aBisFT       ,
            &aToNF        ,
            &aRecordFL    ,
            &aPrinterRB   ,
            &aMailingRB   ,
            &aFileRB      ,
            &aSingleJobsCB,
            &aPathFT      ,
            &aPathED      ,
            &aPathPB      ,
            &aFilenameFT  ,
            &aColumnRB    ,
            &aFilenameRB  ,
            &aColumnLB    ,
            &aFilenameED  ,
            &aAddressFT   ,
            &aAddressFldLB,
            &aSubjectFT   ,
            &aSubjectED   ,
            &aFormatFT    ,
            &aAttachFT    ,
            &aAttachED    ,
            &aAttachPB    ,
            &aFormatHtmlCB,
            &aFormatRtfCB ,
            &aFormatSwCB  ,
            &aDestFL      ,
            &aSeparatorFL ,
            0};

        for( Window** ppW = aCntrlArr; *ppW; ++ppW )
        {
            ::Point aPnt( (*ppW)->GetPosPixel() );
            aPnt.Y() -= nDiff;
            (*ppW)->SetPosPixel( aPnt );
        }
    }
    else
    {
        try
        {
            // create a frame wrapper for myself
            Reference< XMultiServiceFactory >
                                        xMgr = comphelper::getProcessServiceFactory();
            xFrame = Reference< XFrame >(xMgr->createInstance(C2U("com.sun.star.frame.Frame")), UNO_QUERY);
            if(xFrame.is())
            {
                xFrame->initialize( VCLUnoHelper::GetInterface ( pBeamerWin ) );
            }
        }
        catch (Exception&)
        {
            xFrame.clear();
        }
        if(xFrame.is())
        {
            Reference<XDispatchProvider> xDP(xFrame, UNO_QUERY);
            URL aURL;
            aURL.Complete = C2U(".component:DB/DataSourceBrowser");
            Reference<XDispatch> xD = xDP->queryDispatch(aURL,
                        C2U(""),
                        0x0C);
            if(xD.is())
            {
                Sequence<PropertyValue> aProperties(3);
                PropertyValue* pProperties = aProperties.getArray();
                pProperties[0].Name = C2U("DataSourceName");
                pProperties[0].Value <<= OUString(rSourceName);
                pProperties[1].Name = C2U("Command");
                pProperties[1].Value <<= OUString(rTableName);
                pProperties[2].Name = C2U("CommandType");
                pProperties[2].Value <<= nCommandType;
                xD->dispatch(aURL, aProperties);
                pBeamerWin->Show();
            }
            Reference<XController> xController = xFrame->getController();
            pImpl->xFController = Reference<XFormController>(xController, UNO_QUERY);
            if(pImpl->xFController.is())
            {
                Reference< awt::XControl > xCtrl = pImpl->xFController->getCurrentControl(  );
                pImpl->xSelSupp = Reference<XSelectionSupplier>(xCtrl, UNO_QUERY);
                if(pImpl->xSelSupp.is())
                {
                    pImpl->xChgLstnr = new SwXSelChgLstnr_Impl(*this);
                    pImpl->xSelSupp->addSelectionChangeListener(  pImpl->xChgLstnr );
                }
            }
        }
    }

    pModOpt = SW_MOD()->GetModuleConfig();

    aSingleJobsCB.Check(pModOpt->IsSinglePrintJob());

    sal_Int16 nMailingMode(pModOpt->GetMailingFormats());
    aFormatSwCB.Check((nMailingMode & TXTFORMAT_OFFICE) != 0);
    aFormatHtmlCB.Check((nMailingMode & TXTFORMAT_HTML) != 0);
    aFormatRtfCB.Check((nMailingMode & TXTFORMAT_RTF) != 0);

    aAllRB.Check(TRUE);

    // Handler installieren
    Link aLk = LINK(this, SwMailMergeDlg, ButtonHdl);
    aOkBTN.SetClickHdl(aLk);

    aPathPB.SetClickHdl(LINK(this, SwMailMergeDlg, InsertPathHdl));
    aAttachPB.SetClickHdl(LINK(this, SwMailMergeDlg, AttachFileHdl));

    aLk = LINK(this, SwMailMergeDlg, RadioButtonHdl);
    aPrinterRB.SetClickHdl(aLk);
    aMailingRB.SetClickHdl(aLk);
    aFileRB.SetClickHdl(aLk);
    RadioButtonHdl(&aPrinterRB);

    aLk = LINK(this, SwMailMergeDlg, FilenameHdl);
    aColumnRB.SetClickHdl(aLk);
    aFilenameRB.SetClickHdl(aLk);
    BOOL bColumn = pModOpt->IsNameFromColumn();
    if(bColumn)
        aColumnRB.Check();
    else
        aFilenameRB.Check();

    FilenameHdl(bColumn ? &aColumnRB : &aFilenameRB);

    aLk = LINK(this, SwMailMergeDlg, ModifyHdl);
    aFromNF.SetModifyHdl(aLk);
    aToNF.SetModifyHdl(aLk);

    SwNewDBMgr* pNewDBMgr = rSh.GetNewDBMgr();
    if(_xConnection.is())
        pNewDBMgr->GetColumnNames(&aAddressFldLB, _xConnection, rTableName);
    else
        pNewDBMgr->GetColumnNames(&aAddressFldLB, rDBName, rTableName);
    for(USHORT nEntry = 0; nEntry < aAddressFldLB.GetEntryCount(); nEntry++)
        aColumnLB.InsertEntry(aAddressFldLB.GetEntry(nEntry));

    aAddressFldLB.SelectEntry(C2S("EMAIL"));

    String sPath(pModOpt->GetMailingPath());
    if(!sPath.Len())
    {
        SvtPathOptions aPathOpt;
        sPath = aPathOpt.GetWorkPath();
    }
    INetURLObject aURL(sPath);
    if(aURL.GetProtocol() == INET_PROT_FILE)
        aPathED.SetText(aURL.PathToFileName());
    else
        aPathED.SetText(aURL.GetFull());

    String sMailName = pModOpt->GetMailName();

    if (!bColumn || !sMailName.Len())
    {
        aColumnLB.SelectEntry(C2S("NAME"));
        aFilenameED.SetText(sMailName);
    }
    else
        aColumnLB.SelectEntry(pModOpt->GetNameFromColumn());

    if (aAddressFldLB.GetSelectEntryCount() == 0)
        aAddressFldLB.SelectEntryPos(0);
    if (aColumnLB.GetSelectEntryCount() == 0)
        aColumnLB.SelectEntryPos(0);

    const BOOL bEnable = m_aSelection.getLength() != 0;
    aMarkedRB.Enable(bEnable);
    if (bEnable)
        aMarkedRB.Check();
    else
    {
        aAllRB.Check();
        aMarkedRB.Enable(FALSE);
    }
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

SwMailMergeDlg::~SwMailMergeDlg()
{
    if(xFrame.is())
    {
        xFrame->setComponent(NULL, NULL);
        xFrame->dispose();
    }
    else
        delete pBeamerWin;
    delete pImpl;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

void SwMailMergeDlg::Apply()
{
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwMailMergeDlg, ButtonHdl, Button *, pBtn )
{
    if (pBtn == &aOkBTN)
    {
        ExecQryShell(FALSE);
        EndDialog(RET_OK);
    }
    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwMailMergeDlg, RadioButtonHdl, RadioButton *, pBtn )
{
    sal_Bool bPrint = pBtn == &aPrinterRB;
    aSingleJobsCB.Enable(bPrint);

    aPathFT.Enable(!bPrint);
    aPathED.Enable(!bPrint);
    aPathPB.Enable(!bPrint);
    aFilenameFT.Enable(!bPrint);
    aColumnRB.Enable(!bPrint);
    aFilenameRB.Enable(!bPrint);
    aColumnLB.Enable(!bPrint);
    aFilenameED.Enable(!bPrint);
    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwMailMergeDlg, FilenameHdl, RadioButton *, pBtn )
{
    BOOL bEnable = pBtn == &aColumnRB;

    aColumnLB.Enable(bEnable);
    aFilenameED.Enable(!bEnable);

    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwMailMergeDlg, ModifyHdl, NumericField *, pFld )
{
    aFromRB.Check();
    return (0);
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

void SwMailMergeDlg::ExecQryShell(BOOL bVisible)
{
    if(pImpl->xSelSupp.is())
    {
        pImpl->xSelSupp->removeSelectionChangeListener(  pImpl->xChgLstnr );
    }
    SwNewDBMgr* pMgr = rSh.GetNewDBMgr();

    if (aPrinterRB.IsChecked())
        nMergeType = DBMGR_MERGE_MAILMERGE;
    else if (aMailingRB.IsChecked())
    {
        nMergeType = DBMGR_MERGE_MAILING;
        pMgr->SetEMailColumn(aAddressFldLB.GetSelectEntry());
        pMgr->SetSubject(aSubjectED.GetText());
        pMgr->SetAttachment(aAttachED.GetText());
    }
    else
    {
        nMergeType = DBMGR_MERGE_MAILFILES;
        String sPath(URIHelper::SmartRelToAbs(aPathED.GetText()));
        pModOpt->SetMailingPath(sPath);
        String sDelim(INET_PATH_TOKEN);

        if (sPath.Len() >= sDelim.Len() &&
            sPath.Copy(sPath.Len()-sDelim.Len()).CompareTo(sDelim) != COMPARE_EQUAL)
            sPath += sDelim;

        pModOpt->SetIsNameFromColumn(aColumnRB.IsChecked());

        if (aColumnRB.IsChecked())
        {
            pMgr->SetEMailColumn(aColumnLB.GetSelectEntry());
            pModOpt->SetNameFromColumn(aColumnLB.GetSelectEntry());
        }
        else
        {
            String sName(aFilenameED.GetText());
            if (!sName.Len())
            {
                sName = rSh.GetView().GetDocShell()->GetTitle();
                INetURLObject aTemp(sName);
                sName = aTemp.GetBase();
            }
            sPath += sName;
            pMgr->SetEMailColumn(aEmptyStr);
            pModOpt->SetMailName(sName);
        }

        pMgr->SetSubject(sPath);
    }

    if (aFromRB.IsChecked())    // Liste Einfuegen
    {
        ULONG nStart = aFromNF.GetValue();
        ULONG nEnd = aToNF.GetValue();

        if (nEnd < nStart)
        {
            ULONG nZw = nEnd;
            nEnd = nStart;
            nStart = nZw;
        }

        m_aSelection.realloc(nEnd - nStart + 1);
        Any* pSelection = m_aSelection.getArray();
        for (ULONG i = nStart; i <= nEnd; ++i, ++pSelection)
            *pSelection <<= i;
    }
    else if (aAllRB.IsChecked() )
        m_aSelection.realloc(0);    // Leere Selektion = Alles einfuegen
    else
    {
        if(pImpl->xSelSupp.is())
        {
            //update selection
            Reference< XRowLocate > xRowLocate(GetResultSet(),UNO_QUERY);
            Reference< XResultSet > xRes(xRowLocate,UNO_QUERY);
            pImpl->xSelSupp->getSelection() >>= m_aSelection;
            if ( xRowLocate.is() )
            {
                Any* pBegin = m_aSelection.getArray();
                Any* pEnd   = pBegin + m_aSelection.getLength();
                for (;pBegin != pEnd ; ++pBegin)
                {
                    if ( xRowLocate->moveToBookmark(*pBegin) )
                        *pBegin <<= xRes->getRow();
                }
            }
        }
    }
    SwPrintData aPrtData = *SW_MOD()->GetPrtOptions(FALSE);
    SwPrintData* pShellPrintData = rSh.GetPrintData();
    if(pShellPrintData)
        aPrtData = *pShellPrintData;
    aPrtData.SetPrintSingleJobs(aSingleJobsCB.IsChecked());
    rSh.SetPrintData(aPrtData);

    pModOpt->SetSinglePrintJob(aSingleJobsCB.IsChecked());

    BYTE nMailingMode = 0;

    if (aFormatSwCB.IsChecked())
        nMailingMode |= TXTFORMAT_OFFICE;
    if (aFormatHtmlCB.IsChecked())
        nMailingMode |= TXTFORMAT_HTML;
    if (aFormatRtfCB.IsChecked())
        nMailingMode |= TXTFORMAT_RTF;
    pModOpt->SetMailingFormats(nMailingMode);
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwMailMergeDlg, InsertPathHdl, PushButton *, pBtn )
{
    String sPath( aPathED.GetText() );
    if( !sPath.Len() )
    {
        SvtPathOptions aPathOpt;
        sPath = aPathOpt.GetWorkPath();
    }

    Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
    Reference < XFolderPicker > xFP;
    if( xMgr.is() )
    {
        xFP = Reference< XFolderPicker >(
                xMgr->createInstance(
                    C2U( "com.sun.star.ui.dialogs.FolderPicker" ) ),
                UNO_QUERY );
    }
//    pFileDlg->SetHelpId(HID_FILEDLG_MAILMRGE1);
    xFP->setDisplayDirectory(sPath);
    if( xFP->execute() == RET_OK )
    {
        INetURLObject aURL(xFP->getDirectory());
        if(aURL.GetProtocol() == INET_PROT_FILE)
            aPathED.SetText(aURL.PathToFileName());
        else
            aPathED.SetText(aURL.GetFull());
    }
    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwMailMergeDlg, AttachFileHdl, PushButton *, pBtn )
{
    SvxMultiFileDialog* pFileDlg = new SvxMultiFileDialog(this);
    pFileDlg->SetFiles(aAttachED.GetText());
    pFileDlg->SetHelpId(HID_FILEDLG_MAILMRGE2);

    if (pFileDlg->Execute())
        aAttachED.SetText(pFileDlg->GetFiles());

    delete pFileDlg;

    return 0;
}
/* -----------------------------05.06.01 14:56--------------------------------

 ---------------------------------------------------------------------------*/
Reference<XResultSet> SwMailMergeDlg::GetResultSet() const
{
    Reference< XResultSet >  xResSetClone;
    if ( pImpl->xFController.is() )
    {
        // we create a clone to do the task
        Reference< XResultSetAccess > xResultSetAccess( pImpl->xFController->getModel(),UNO_QUERY);
        if ( xResultSetAccess.is() )
            xResSetClone = xResultSetAccess->createResultSet();
    }
    return xResSetClone;
}
/*-- 27.11.2002 12:27:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailMergeCreateFromDlg::SwMailMergeCreateFromDlg(Window* pParent) :
    ModalDialog(pParent, SW_RES(DLG_MERGE_CREATE)),
    aCreateFromFL(  this, ResId( FL_CREATEFROM  )),
    aThisDocRB(     this, ResId( RB_THISDOC     )),
    aUseTemplateRB( this, ResId( RB_TEMPLATE    )),
    aOK(            this, ResId( BT_OK          )),
    aCancel(        this, ResId( BT_CANCEL      )),
    aHelp(          this, ResId( BT_HELP        ))
{
    FreeResource();
}
/*-- 27.11.2002 12:27:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailMergeCreateFromDlg::~SwMailMergeCreateFromDlg()
{
}
/* -----------------04.02.2003 13:45-----------------
 *
 * --------------------------------------------------*/
SwMailMergeFieldConnectionsDlg::SwMailMergeFieldConnectionsDlg(Window* pParent) :
    ModalDialog(pParent, SW_RES(DLG_MERGE_FIELD_CONNECTIONS)),
    aConnectionsFL( this, ResId( FL_CONNECTIONS  )),
    aUseExistingRB( this, ResId( RB_USEEXISTING )),
    aCreateNewRB(   this, ResId( RB_CREATENEW    )),
    aInfoFI(        this, ResId( FT_INFO        )),
    aOK(            this, ResId( BT_OK          )),
    aCancel(        this, ResId( BT_CANCEL      )),
    aHelp(          this, ResId( BT_HELP        ))
{
    FreeResource();
}
/* -----------------04.02.2003 13:45-----------------
 *
 * --------------------------------------------------*/
SwMailMergeFieldConnectionsDlg::~SwMailMergeFieldConnectionsDlg()
{
}


