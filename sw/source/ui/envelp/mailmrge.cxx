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

#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <svl/urihelper.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/mailenum.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <helpid.h>
#include <view.hxx>
#include <docsh.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <wrtsh.hxx>
#include <dbmgr.hxx>
#include <dbui.hxx>
#include <prtopt.hxx>
#include <swmodule.hxx>
#include <modcfg.hxx>
#include <mailmergehelper.hxx>
#include <envelp.hrc>
#include <mailmrge.hrc>
#include <mailmrge.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase1.hxx>

#include <unomid.h>

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::ui::dialogs;


struct SwMailMergeDlg_Impl
{
    uno::Reference<runtime::XFormController> xFController;
    uno::Reference<XSelectionChangeListener> xChgLstnr;
    uno::Reference<XSelectionSupplier> xSelSupp;
};

/* --------------------------------------------------------------------------
    helper classes
----------------------------------------------------------------------------*/
class SwXSelChgLstnr_Impl : public cppu::WeakImplHelper1
<
    view::XSelectionChangeListener
>
{
    SwMailMergeDlg& rParent;
public:
    SwXSelChgLstnr_Impl(SwMailMergeDlg& rParentDlg);
    ~SwXSelChgLstnr_Impl();

    virtual void SAL_CALL selectionChanged( const EventObject& aEvent ) throw (RuntimeException);
    virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException);
};

SwXSelChgLstnr_Impl::SwXSelChgLstnr_Impl(SwMailMergeDlg& rParentDlg) :
    rParent(rParentDlg)
{}

SwXSelChgLstnr_Impl::~SwXSelChgLstnr_Impl()
{}

void SwXSelChgLstnr_Impl::selectionChanged( const EventObject&  ) throw (RuntimeException)
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

void SwXSelChgLstnr_Impl::disposing( const EventObject&  ) throw (RuntimeException)
{
    OSL_FAIL("disposing");
}

SwMailMergeDlg::SwMailMergeDlg(Window* pParent, SwWrtShell& rShell,
         const String& rSourceName,
        const String& rTblName,
        sal_Int32 nCommandType,
        const uno::Reference< XConnection>& _xConnection,
        Sequence< Any >* pSelection) :

    SvxStandardDialog(pParent, SW_RES(DLG_MAILMERGE)),
    pBeamerWin      (new Window(this, SW_RES(WIN_BEAMER))),

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

    aSaveMergedDocumentFL(this, SW_RES(     FL_SAVE_MERGED_DOCUMENT)),
    aSaveSingleDocRB(this, SW_RES(          RB_SAVE_SINGLE_DOC )),
    aSaveIndividualRB(this, SW_RES(         RB_SAVE_INDIVIDUAL )),
    aGenerateFromDataBaseCB(this, SW_RES(   RB_GENERATE_FROM_DATABASE )),

    aColumnFT       (this, SW_RES(FT_COLUMN)),
    aColumnLB       (this, SW_RES(LB_COLUMN)),

    aPathFT         (this, SW_RES(FT_PATH)),
    aPathED         (this, SW_RES(ED_PATH)),
    aPathPB         (this, SW_RES(PB_PATH)),
    aFilterFT       (this, SW_RES(FT_FILTER)),
    aFilterLB       (this, SW_RES(LB_FILTER)),

    aAddressFldLB   (this, SW_RES(LB_ADDRESSFLD)),
    aSubjectFT      (this, SW_RES(FT_SUBJECT)),
    aSubjectED      (this, SW_RES(ED_SUBJECT)),
    aFormatFT       (this, SW_RES(FT_FORMAT)),
    aAttachFT       (this, SW_RES(FT_ATTACH)),
    aAttachED       (this, SW_RES(ED_ATTACH)),
    aAttachPB       (this, SW_RES(PB_ATTACH)),
    aFormatHtmlCB   (this, SW_RES(CB_FORMAT_HTML)),
    aFormatRtfCB    (this, SW_RES(CB_FORMAT_RTF)),
    aFormatSwCB     (this, SW_RES(CB_FORMAT_SW)),
    aDestFL         (this, SW_RES(FL_DEST)),

    aBottomSeparatorFL(this, SW_RES(FL_BOTTOM_SEPARATOR)),

    aOkBTN          (this, SW_RES(BTN_OK)),
    aCancelBTN      (this, SW_RES(BTN_CANCEL)),
    aHelpBTN        (this, SW_RES(BTN_HELP)),

    pImpl           (new SwMailMergeDlg_Impl),

    rSh             (rShell),
    rDBName         (rSourceName),
    rTableName      (rTblName),
    nMergeType      (DBMGR_MERGE_MAILING),
    m_aDialogSize( GetSizePixel() )
{
    FreeResource();
    aSingleJobsCB.Show(sal_False); // not supported in since cws printerpullpages anymore
    //task #97066# mailing of form letters is currently not supported
    aMailingRB.Show(sal_False);
    aSubjectFT.Show(sal_False);
    aSubjectED.Show(sal_False);
    aFormatFT.Show(sal_False);
    aFormatSwCB.Show(sal_False);
    aFormatHtmlCB.Show(sal_False);
    aFormatRtfCB.Show(sal_False);
    aAttachFT.Show(sal_False);
    aAttachED.Show(sal_False);
    aAttachPB.Show(sal_False);

    Point aMailPos = aMailingRB.GetPosPixel();
    Point aFilePos = aFileRB.GetPosPixel();
    aFilePos.X() -= (aFilePos.X() - aMailPos.X()) /2;
    aFileRB.SetPosPixel(aFilePos);
    uno::Reference< lang::XMultiServiceFactory > xMSF = comphelper::getProcessServiceFactory();
    if(pSelection)
    {
        m_aSelection = *pSelection;
        //move all controls
        long nDiff = aRecordFL.GetPosPixel().Y() - pBeamerWin->GetPosPixel().Y();
        pBeamerWin->Show(sal_False);
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
            &aFilterFT    ,
            &aFilterLB    ,
            &aColumnLB    ,
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
            m_xFrame = frame::Frame::create( comphelper::getProcessComponentContext() );
            m_xFrame->initialize( VCLUnoHelper::GetInterface ( pBeamerWin ) );
        }
        catch (const Exception&)
        {
            m_xFrame.clear();
        }
        if(m_xFrame.is())
        {
            URL aURL;
            aURL.Complete = ".component:DB/DataSourceBrowser";
            uno::Reference<XDispatch> xD = m_xFrame->queryDispatch(aURL,
                        "",
                        0x0C);
            if(xD.is())
            {
                Sequence<PropertyValue> aProperties(3);
                PropertyValue* pProperties = aProperties.getArray();
                pProperties[0].Name = "DataSourceName";
                pProperties[0].Value <<= OUString(rSourceName);
                pProperties[1].Name = "Command";
                pProperties[1].Value <<= OUString(rTableName);
                pProperties[2].Name = "CommandType";
                pProperties[2].Value <<= nCommandType;
                xD->dispatch(aURL, aProperties);
                pBeamerWin->Show();
            }
            uno::Reference<XController> xController = m_xFrame->getController();
            pImpl->xFController = uno::Reference<runtime::XFormController>(xController, UNO_QUERY);
            if(pImpl->xFController.is())
            {
                uno::Reference< awt::XControl > xCtrl = pImpl->xFController->getCurrentControl(  );
                pImpl->xSelSupp = uno::Reference<XSelectionSupplier>(xCtrl, UNO_QUERY);
                if(pImpl->xSelSupp.is())
                {
                    pImpl->xChgLstnr = new SwXSelChgLstnr_Impl(*this);
                    pImpl->xSelSupp->addSelectionChangeListener(  pImpl->xChgLstnr );
                }
            }
        }
    }

    pModOpt = SW_MOD()->GetModuleConfig();

    sal_Int16 nMailingMode(pModOpt->GetMailingFormats());
    aFormatSwCB.Check((nMailingMode & TXTFORMAT_OFFICE) != 0);
    aFormatHtmlCB.Check((nMailingMode & TXTFORMAT_HTML) != 0);
    aFormatRtfCB.Check((nMailingMode & TXTFORMAT_RTF) != 0);

    aAllRB.Check(sal_True);

    // Install handlers
    Link aLk = LINK(this, SwMailMergeDlg, ButtonHdl);
    aOkBTN.SetClickHdl(aLk);

    aPathPB.SetClickHdl(LINK(this, SwMailMergeDlg, InsertPathHdl));
    aAttachPB.SetClickHdl(LINK(this, SwMailMergeDlg, AttachFileHdl));

    aLk = LINK(this, SwMailMergeDlg, OutputTypeHdl);
    aPrinterRB.SetClickHdl(aLk);
    aMailingRB.SetClickHdl(aLk);
    aFileRB.SetClickHdl(aLk);

    //#i63267# printing might be disabled
    bool bIsPrintable = !Application::GetSettings().GetMiscSettings().GetDisablePrinting();
    aPrinterRB.Enable(bIsPrintable);
    OutputTypeHdl(bIsPrintable ? &aPrinterRB : &aFileRB);

    aLk = LINK(this, SwMailMergeDlg, FilenameHdl);
    aGenerateFromDataBaseCB.SetClickHdl( aLk );
    bool bColumn = pModOpt->IsNameFromColumn();
    if(bColumn)
        aGenerateFromDataBaseCB.Check();

    FilenameHdl( &aGenerateFromDataBaseCB );
    aLk = LINK(this, SwMailMergeDlg, SaveTypeHdl);
    aSaveSingleDocRB.Check( true );
    aSaveSingleDocRB.SetClickHdl( aLk );
    aSaveIndividualRB.SetClickHdl( aLk );
    aLk.Call( &aSaveSingleDocRB );

    aLk = LINK(this, SwMailMergeDlg, ModifyHdl);
    aFromNF.SetModifyHdl(aLk);
    aToNF.SetModifyHdl(aLk);
    aFromNF.SetMax(SAL_MAX_INT32);
    aToNF.SetMax(SAL_MAX_INT32);

    SwNewDBMgr* pNewDBMgr = rSh.GetNewDBMgr();
    if(_xConnection.is())
        pNewDBMgr->GetColumnNames(&aAddressFldLB, _xConnection, rTableName);
    else
        pNewDBMgr->GetColumnNames(&aAddressFldLB, rDBName, rTableName);
    for(sal_uInt16 nEntry = 0; nEntry < aAddressFldLB.GetEntryCount(); nEntry++)
        aColumnLB.InsertEntry(aAddressFldLB.GetEntry(nEntry));

    aAddressFldLB.SelectEntry(OUString("EMAIL"));

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

    if (!bColumn )
    {
        aColumnLB.SelectEntry(OUString("NAME"));
    }
    else
        aColumnLB.SelectEntry(pModOpt->GetNameFromColumn());

    if (aAddressFldLB.GetSelectEntryCount() == 0)
        aAddressFldLB.SelectEntryPos(0);
    if (aColumnLB.GetSelectEntryCount() == 0)
        aColumnLB.SelectEntryPos(0);

    const sal_Bool bEnable = m_aSelection.getLength() != 0;
    aMarkedRB.Enable(bEnable);
    if (bEnable)
        aMarkedRB.Check();
    else
    {
        aAllRB.Check();
        aMarkedRB.Enable(sal_False);
    }
    SetMinOutputSizePixel(m_aDialogSize);
    try
    {
        uno::Reference< container::XNameContainer> xFilterFactory(
                xMSF->createInstance("com.sun.star.document.FilterFactory"), UNO_QUERY_THROW);
        uno::Reference< container::XContainerQuery > xQuery(xFilterFactory, UNO_QUERY_THROW);
        OUString sCommand("matchByDocumentService=com.sun.star.text.TextDocument:iflags=");
        sCommand += OUString::number(SFX_FILTER_EXPORT);
        sCommand += ":eflags=";
        sCommand += OUString::number(SFX_FILTER_NOTINFILEDLG);
        sCommand += ":default_first";
        uno::Reference< container::XEnumeration > xList = xQuery->createSubSetEnumerationByQuery(sCommand);
        const OUString sName("Name");
        sal_uInt16 nODT = USHRT_MAX;
        while(xList->hasMoreElements())
        {
            comphelper::SequenceAsHashMap aFilter(xList->nextElement());
            OUString sFilter = aFilter.getUnpackedValueOrDefault(sName, OUString());

            uno::Any aProps = xFilterFactory->getByName(sFilter);
            uno::Sequence< beans::PropertyValue > aFilterProperties;
            aProps >>= aFilterProperties;
            OUString sUIName2;
            const beans::PropertyValue* pFilterProperties = aFilterProperties.getConstArray();
            for(int nProp = 0; nProp < aFilterProperties.getLength(); nProp++)
            {
                if(!pFilterProperties[nProp].Name.compareToAscii("UIName"))
                {
                    pFilterProperties[nProp].Value >>= sUIName2;
                    break;
                }
            }
            if( !sUIName2.isEmpty() )
            {
                sal_uInt16 nFilter = aFilterLB.InsertEntry( sUIName2 );
                if( 0 == sFilter.compareToAscii("writer8") )
                    nODT = nFilter;
                aFilterLB.SetEntryData( nFilter, new OUString( sFilter ) );
            }
        }
        aFilterLB.SelectEntryPos( nODT );
    }
    catch (const uno::Exception&)
    {
    }
}

SwMailMergeDlg::~SwMailMergeDlg()
{
    if(m_xFrame.is())
    {
        m_xFrame->setComponent(NULL, NULL);
        m_xFrame->dispose();
    }
    else
        delete pBeamerWin;

    for( sal_uInt16 nFilter = 0; nFilter < aFilterLB.GetEntryCount(); ++nFilter )
    {
        OUString* pData = reinterpret_cast< OUString* >( aFilterLB.GetEntryData(nFilter) );
        delete pData;
    }
    delete pImpl;
}

void SwMailMergeDlg::Apply()
{
}

static void lcl_MoveControlY( Window* ppW, long nDiffSize )
{
    Point aPos( ppW->GetPosPixel());
    aPos.Y() += nDiffSize;
    ppW->SetPosPixel( aPos );
}

static void lcl_MoveControlX( Window* ppW, long nDiffSize )
{
    Point aPos( ppW->GetPosPixel());
    aPos.X() += nDiffSize;
    ppW->SetPosPixel( aPos );
}

static void lcl_ChangeWidth( Window* ppW, long nDiffSize )
{
    Size aSize( ppW->GetSizePixel());
    aSize.Width() += nDiffSize;
    ppW->SetSizePixel( aSize );
}

void    SwMailMergeDlg::Resize()
{
    //the only controls that profit from the resize is pBeamerWin
    // and aPathED, aFilenameED and aColumnLB

    Size aCurSize( GetSizePixel() );
    //find the difference
    Size aDiffSize( aCurSize.Width() - m_aDialogSize.Width(),
                            aCurSize.Height() - m_aDialogSize.Height() );
    m_aDialogSize = aCurSize;
    if( pBeamerWin->IsVisible() )
    {
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
            &aSaveMergedDocumentFL,
            &aSaveSingleDocRB,
            &aSaveIndividualRB,
            &aGenerateFromDataBaseCB,
            &aPathFT      ,
            &aPathED      ,
            &aPathPB      ,
            &aColumnFT,
            &aColumnLB    ,
            &aFilterFT    ,
            &aFilterLB    ,
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
            &aBottomSeparatorFL,
            &aOkBTN,
            &aCancelBTN,
            &aHelpBTN,
            0};
        for( Window** ppW = aCntrlArr; *ppW; ++ppW )
        {
            lcl_MoveControlY( *ppW, aDiffSize.Height() );
        }
        //some controls have to be extended horizontally
        lcl_MoveControlX( &aOkBTN, aDiffSize.Width() );
        lcl_MoveControlX( &aCancelBTN, aDiffSize.Width() );
        lcl_MoveControlX( &aHelpBTN, aDiffSize.Width() );
        lcl_MoveControlX( &aPathPB, aDiffSize.Width() );
        lcl_MoveControlX( &aFileRB, aDiffSize.Width()/2 );

        lcl_ChangeWidth( &aBottomSeparatorFL, aDiffSize.Width() );
        lcl_ChangeWidth( &aSaveMergedDocumentFL, aDiffSize.Width() );
        lcl_ChangeWidth( &aColumnLB, aDiffSize.Width() );
        lcl_ChangeWidth( &aPathED, aDiffSize.Width() );
        lcl_ChangeWidth( &aFilterLB, aDiffSize.Width() );
        lcl_ChangeWidth( &aDestFL, aDiffSize.Width() );

        Size aBeamerSize( pBeamerWin->GetSizePixel() ) ;
        aBeamerSize.Width() += aDiffSize.Width();
        aBeamerSize.Height() += aDiffSize.Height();
        pBeamerWin->SetSizePixel(aBeamerSize);
    }
}

IMPL_LINK( SwMailMergeDlg, ButtonHdl, Button *, pBtn )
{
    if (pBtn == &aOkBTN)
    {
        if( ExecQryShell() )
            EndDialog(RET_OK);
    }
    return 0;
}

IMPL_LINK( SwMailMergeDlg, OutputTypeHdl, RadioButton *, pBtn )
{
    sal_Bool bPrint = pBtn == &aPrinterRB;
    aSingleJobsCB.Enable(bPrint);

    aSaveMergedDocumentFL.Enable( !bPrint );
    aSaveSingleDocRB.Enable( !bPrint );
    aSaveIndividualRB.Enable( !bPrint );

    if( !bPrint )
    {
        SaveTypeHdl( aSaveSingleDocRB.IsChecked() ? &aSaveSingleDocRB : &aSaveIndividualRB );
    }
    else
    {
        aPathFT.Enable(false);
        aPathED.Enable(false);
        aPathPB.Enable(false);
        aColumnFT.Enable(false);
        aColumnLB.Enable(false);
        aFilterFT.Enable(false);
        aFilterLB.Enable(false);
        aGenerateFromDataBaseCB.Enable(false);
    }

    return 0;
}

IMPL_LINK( SwMailMergeDlg, SaveTypeHdl, RadioButton*,  pBtn )
{
    bool bIndividual = pBtn == &aSaveIndividualRB;

    aGenerateFromDataBaseCB.Enable( bIndividual );
    if( bIndividual )
    {
        FilenameHdl( &aGenerateFromDataBaseCB );
    }
    else
    {
        aColumnFT.Enable(false);
        aColumnLB.Enable(false);
        aPathFT.Enable( false );
        aPathED.Enable( false );
        aPathPB.Enable( false );
        aFilterFT.Enable( false );
        aFilterLB.Enable( false );
    }
    return 0;
}

IMPL_LINK( SwMailMergeDlg, FilenameHdl, CheckBox*, pBox )
{
    sal_Bool bEnable = pBox->IsChecked();
    aColumnFT.Enable( bEnable );
    aColumnLB.Enable(bEnable);
    aPathFT.Enable( bEnable );
    aPathED.Enable(bEnable);
    aPathPB.Enable( bEnable );
    aFilterFT.Enable( bEnable );
    aFilterLB.Enable( bEnable );
 return 0;
}

IMPL_LINK_NOARG(SwMailMergeDlg, ModifyHdl)
{
    aFromRB.Check();
    return (0);
}

bool SwMailMergeDlg::ExecQryShell()
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
        nMergeType = static_cast< sal_uInt16 >( aSaveSingleDocRB.IsChecked() ?
                    DBMGR_MERGE_SINGLE_FILE : DBMGR_MERGE_MAILFILES );
        SfxMedium* pMedium = rSh.GetView().GetDocShell()->GetMedium();
        INetURLObject aAbs;
        if( pMedium )
            aAbs = pMedium->GetURLObject();
        String sPath(
            URIHelper::SmartRel2Abs(
                aAbs, aPathED.GetText(), URIHelper::GetMaybeFileHdl()));
        pModOpt->SetMailingPath(sPath);
        String sDelim = OUString(INET_PATH_TOKEN);

        if (sPath.Len() >= sDelim.Len() &&
            sPath.Copy(sPath.Len()-sDelim.Len()).CompareTo(sDelim) != COMPARE_EQUAL)
            sPath += sDelim;

        pModOpt->SetIsNameFromColumn(aGenerateFromDataBaseCB.IsChecked());

        if (aGenerateFromDataBaseCB.IsEnabled() && aGenerateFromDataBaseCB.IsChecked())
        {
            pMgr->SetEMailColumn(aColumnLB.GetSelectEntry());
            pModOpt->SetNameFromColumn(aColumnLB.GetSelectEntry());
            if( aFilterLB.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND)
                m_sSaveFilter = *static_cast<const OUString*>(aFilterLB.GetEntryData( aFilterLB.GetSelectEntryPos() ));
        }
        else
        {
            //#i97667# reset column name - otherwise it's remembered from the last run
            pMgr->SetEMailColumn(OUString());
            //start save as dialog
            String sFilter;
            sPath = SwMailMergeHelper::CallSaveAsDialog(sFilter);
            if(!sPath.Len())
                return false;
            m_sSaveFilter = sFilter;
        }

        pMgr->SetSubject(sPath);
    }

    if (aFromRB.IsChecked())    // Insert list
    {
        // Safe: the maximal value of the fields is limited
        sal_Int32 nStart = sal::static_int_cast<sal_Int32>(aFromNF.GetValue());
        sal_Int32 nEnd = sal::static_int_cast<sal_Int32>(aToNF.GetValue());

        if (nEnd < nStart)
            std::swap(nEnd, nStart);

        m_aSelection.realloc(nEnd - nStart + 1);
        Any* pSelection = m_aSelection.getArray();
        for (sal_Int32 i = nStart; i <= nEnd; ++i, ++pSelection)
            *pSelection <<= i;
    }
    else if (aAllRB.IsChecked() )
        m_aSelection.realloc(0);    // Empty selection = insert all
    else
    {
        if(pImpl->xSelSupp.is())
        {
            //update selection
            uno::Reference< XRowLocate > xRowLocate(GetResultSet(),UNO_QUERY);
            uno::Reference< XResultSet > xRes(xRowLocate,UNO_QUERY);
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
    IDocumentDeviceAccess* pIDDA = rSh.getIDocumentDeviceAccess();
    SwPrintData aPrtData( pIDDA->getPrintData() );
    aPrtData.SetPrintSingleJobs(aSingleJobsCB.IsChecked());
    pIDDA->setPrintData(aPrtData);

    pModOpt->SetSinglePrintJob(aSingleJobsCB.IsChecked());

    sal_uInt8 nMailingMode = 0;

    if (aFormatSwCB.IsChecked())
        nMailingMode |= TXTFORMAT_OFFICE;
    if (aFormatHtmlCB.IsChecked())
        nMailingMode |= TXTFORMAT_HTML;
    if (aFormatRtfCB.IsChecked())
        nMailingMode |= TXTFORMAT_RTF;
    pModOpt->SetMailingFormats(nMailingMode);
    return true;
}

IMPL_LINK_NOARG(SwMailMergeDlg, InsertPathHdl)
{
    String sPath( aPathED.GetText() );
    if( !sPath.Len() )
    {
        SvtPathOptions aPathOpt;
        sPath = aPathOpt.GetWorkPath();
    }

    uno::Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference < XFolderPicker2 > xFP = FolderPicker::create(xContext);
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

IMPL_LINK_NOARG(SwMailMergeDlg, AttachFileHdl)
{
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    if(pFact)
    {
        AbstractSvxMultiFileDialog* pFileDlg = pFact->CreateSvxMultiFileDialog( this );
        OSL_ENSURE(pFileDlg, "Dialogdiet fail!");
        pFileDlg->SetFiles(aAttachED.GetText());
        pFileDlg->SetHelpId(HID_FILEDLG_MAILMRGE2);

        if (pFileDlg->Execute())
            aAttachED.SetText(pFileDlg->GetFiles());

        delete pFileDlg;
    }
    return 0;
}

uno::Reference<XResultSet> SwMailMergeDlg::GetResultSet() const
{
    uno::Reference< XResultSet >  xResSetClone;
    if ( pImpl->xFController.is() )
    {
        // we create a clone to do the task
        uno::Reference< XResultSetAccess > xResultSetAccess( pImpl->xFController->getModel(),UNO_QUERY);
        if ( xResultSetAccess.is() )
            xResSetClone = xResultSetAccess->createResultSet();
    }
    return xResSetClone;
}

SwMailMergeCreateFromDlg::SwMailMergeCreateFromDlg(Window* pParent)
    : ModalDialog(pParent, "MailMergeDialog",
        "modules/swriter/ui/mailmergedialog.ui")
{
    get(m_pThisDocRB, "document");
}

SwMailMergeFieldConnectionsDlg::SwMailMergeFieldConnectionsDlg(Window* pParent)
    : ModalDialog(pParent, "MergeConnectDialog",
        "modules/swriter/ui/mergeconnectdialog.ui")
{
    get(m_pUseExistingRB, "existing");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
