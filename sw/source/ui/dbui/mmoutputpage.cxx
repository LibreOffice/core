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

#include <mmoutputpage.hxx>
#include <mailmergewizard.hxx>
#include <mmconfigitem.hxx>
#include <mailmergechildwindow.hxx>
#include <mailconfigpage.hxx>
#include <cmdid.h>
#include <swtypes.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <hintids.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/langitem.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>
#include <svl/urihelper.hxx>
#include <vcl/print.hxx>
#include <unotools/tempfile.hxx>
#include <osl/file.hxx>
#include <mmgreetingspage.hxx>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <dbmgr.hxx>
#include <swunohelper.hxx>
#include <osl/mutex.hxx>
#include <shellio.hxx>
#include <svtools/htmlcfg.hxx>
#include <sfx2/event.hxx>
#include <swevent.hxx>
#include <mmoutputpage.hrc>
#include <dbui.hxx>
#include <dbui.hrc>
#include <helpid.h>
#include <sfx2/app.hxx>
#include <statstr.hrc>
#include <unomid.h>
#include <comphelper/string.hxx>

using namespace svt;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

static String lcl_GetExtensionForDocType(sal_uLong nDocType)
{
    OUString sExtension;
    switch( nDocType )
    {
        case MM_DOCTYPE_OOO : sExtension = OUString( "odt" ); break;
        case MM_DOCTYPE_PDF : sExtension = OUString( "pdf" ); break;
        case MM_DOCTYPE_WORD: sExtension = OUString( "doc" ); break;
        case MM_DOCTYPE_HTML: sExtension = OUString( "html" ); break;
        case MM_DOCTYPE_TEXT: sExtension = OUString( "txt" ); break;
    }
    return sExtension;
}

static OUString lcl_GetColumnValueOf(const OUString& rColumn, Reference < container::XNameAccess>& rxColAccess )
{
    OUString sRet;
    try
    {
        if (rxColAccess->hasByName(rColumn))
        {
            Any aCol = rxColAccess->getByName(rColumn);
            Reference< sdb::XColumn > xColumn;
            aCol >>= xColumn;
            if(xColumn.is())
                sRet = xColumn->getString();
        }
    }
    catch (const uno::Exception&)
    {
    }
    return sRet;
}

class SwSaveWarningBox_Impl : public ModalDialog
{
    FixedImage      aWarningImageIM;
    FixedInfo       aWarningFI;

    FixedText       aFileNameFT;
    Edit            aFileNameED;

    FixedLine       aSeparatorFL;
    OKButton        aOKPB;
    CancelButton    aCancelPB;

    DECL_LINK( ModifyHdl, Edit*);
public:
    SwSaveWarningBox_Impl(Window* pParent, const String& rFileName);
    ~SwSaveWarningBox_Impl();

    String          GetFileName() const {return aFileNameED.GetText();}
};

class SwSendQueryBox_Impl : public ModalDialog
{
    FixedImage      aQueryImageIM;
    FixedInfo       aQueryFI;

    Edit            aTextED;

    FixedLine       aSeparatorFL;
    OKButton        aOKPB;
    CancelButton    aCancelPB;

    bool            bIsEmptyAllowed;
    DECL_LINK( ModifyHdl, Edit*);
public:
    SwSendQueryBox_Impl(Window* pParent, const String& rQueryText);
    ~SwSendQueryBox_Impl();

    void            SetValue(const String& rSet)
                        {
                            aTextED.SetText(rSet);
                            ModifyHdl( &aTextED );
                        }
    String          GetValue() const {return aTextED.GetText();}

    void            SetIsEmptyTextAllowed(bool bSet)
                        {
                            bIsEmptyAllowed = bSet;
                            ModifyHdl( &aTextED );
                        }
};

SwSaveWarningBox_Impl::SwSaveWarningBox_Impl(Window* pParent, const String& rFileName) :
    ModalDialog(pParent, SW_RES(   DLG_MM_SAVEWARNING )),
    aWarningImageIM(this,   SW_RES( IM_WARNING   )),
    aWarningFI(this,        SW_RES( FI_WARNING   )),
    aFileNameFT(this,       SW_RES( FT_FILENAME  )),
    aFileNameED(this,       SW_RES( ED_FILENAME  )),
    aSeparatorFL(this,      SW_RES( FL_SEPARATOR )),
    aOKPB(this,             SW_RES( PB_OK        )),
    aCancelPB(this,         SW_RES( PB_CANCEL    ))
{
    FreeResource();
    aWarningImageIM.SetImage(WarningBox::GetStandardImage());
    aFileNameED.SetText(rFileName);
    aFileNameED.SetModifyHdl(LINK(this, SwSaveWarningBox_Impl, ModifyHdl));
    ModifyHdl( &aFileNameED );
}

SwSaveWarningBox_Impl::~SwSaveWarningBox_Impl()
{
}

IMPL_LINK( SwSaveWarningBox_Impl, ModifyHdl, Edit*, pEdit)
{
    aOKPB.Enable(!pEdit->GetText().isEmpty());
    return 0;
}

SwSendQueryBox_Impl::SwSendQueryBox_Impl(Window* pParent, const String& rText) :
    ModalDialog(pParent, SW_RES(   DLG_MM_QUERY )),
    aQueryImageIM( this,    SW_RES( IM_QUERY     )),
    aQueryFI( this,         SW_RES( FI_QUERY     )),
    aTextED( this,          SW_RES( ED_TEXT      )),
    aSeparatorFL(this,      SW_RES( FL_SEPARATOR )),
    aOKPB(this,             SW_RES( PB_OK        )),
    aCancelPB(this,         SW_RES( PB_CANCEL    )),
    bIsEmptyAllowed(true)
{
    FreeResource();
    aQueryFI.SetText(rText);
    aQueryImageIM.SetImage(QueryBox::GetStandardImage());
    aTextED.SetModifyHdl(LINK(this, SwSendQueryBox_Impl, ModifyHdl));
    ModifyHdl( &aTextED );
}

SwSendQueryBox_Impl::~SwSendQueryBox_Impl()
{
}

IMPL_LINK( SwSendQueryBox_Impl, ModifyHdl, Edit*, pEdit)
{
    aOKPB.Enable(bIsEmptyAllowed  || !pEdit->GetText().isEmpty());
    return 0;
}

class SwCopyToDialog : public SfxModalDialog
{
    FixedInfo       m_aDescriptionFI;
    FixedText       m_aCCFT;
    Edit            m_aCCED;
    FixedText       m_aBCCFT;
    Edit            m_aBCCED;

    FixedInfo       m_aNoteFI;
    FixedLine       m_aSeparatorFL;

    OKButton        m_aOK;
    CancelButton    m_aCancel;
    HelpButton      m_aHelp;

public:
    SwCopyToDialog(Window* pParent);
    ~SwCopyToDialog();

    String          GetCC() {return m_aCCED.GetText();}
    void            SetCC(const String& rSet) {m_aCCED.SetText(rSet);}

    String          GetBCC() {return m_aBCCED.GetText();}
    void            SetBCC(const String& rSet) {m_aBCCED.SetText(rSet);}
};

SwCopyToDialog::SwCopyToDialog(Window* pParent) :
    SfxModalDialog(pParent, SW_RES(DLG_MM_COPYTO)),
#ifdef _MSC_VER
#pragma warning (disable : 4355)
#endif
    m_aDescriptionFI( this, SW_RES(       FI_DESCRIPTION )),
    m_aCCFT( this, SW_RES(                FT_CC          )),
    m_aCCED( this, SW_RES(                ED_CC          )),
    m_aBCCFT( this, SW_RES(               FT_BCC         )),
    m_aBCCED( this, SW_RES(               ED_BCC         )),
    m_aNoteFI( this, SW_RES(              FI_NOTE        )),
    m_aSeparatorFL( this, SW_RES(         FL_SEPARATOR   )),
    m_aOK( this, SW_RES(                  PB_OK          )),
    m_aCancel( this, SW_RES(              PB_CANCEL      )),
    m_aHelp( this, SW_RES(                PB_HELP        ))
#ifdef _MSC_VER
#pragma warning (default : 4355)
#endif
{
    FreeResource();
}

SwCopyToDialog::~SwCopyToDialog()
{
}

SwMailMergeOutputPage::SwMailMergeOutputPage( SwMailMergeWizard* _pParent) :
    svt::OWizardPage( _pParent, SW_RES(DLG_MM_OUTPUT_PAGE)),
#ifdef _MSC_VER
#pragma warning (disable : 4355)
#endif
    m_aHeaderFI(this,           SW_RES(  FI_HEADER           ) ),
    m_aOptionsFI(this,          SW_RES(  FI_OPTIONS          ) ),
    m_aSaveStartDocRB(this,     SW_RES(  RB_SAVESTARTDOC     ) ),
    m_aSaveMergedDocRB(this,    SW_RES(  RB_SAVEMERGEDDOC    ) ),
    m_aPrintRB(this,            SW_RES(  RB_PRINT            ) ),
    m_aSendMailRB(this,         SW_RES(  RB_SENDMAIL         ) ),

    m_aSeparatorFL(this,        SW_RES(  FL_SEPARATOR        ) ),

    m_aSaveStartDocPB(this,     SW_RES(  PB_SAVESTARTDOC     ) ),

    m_aSaveAsOneRB(this,        SW_RES(  RB_SAVEASONE        ) ),
    m_aSaveIndividualRB(this,   SW_RES(  RB_SAVEINDIVIDUAL   ) ),
    m_aPrintAllRB(this,         SW_RES(  RB_PRINTALL         ) ),
    m_aSendAllRB( this, SW_RES(       RB_SENDALL             ) ),

    m_aFromRB(this,             SW_RES(  RB_FROM             ) ),
    m_aFromNF(this,             SW_RES(  NF_FROM             ) ),
    m_aToFT(this,               SW_RES(  FT_TO               ) ),
    m_aToNF(this,               SW_RES(  NF_TO               ) ),
    m_aSaveNowPB(this,          SW_RES(  PB_SAVENOW          ) ),

    m_aPrinterFT(this,          SW_RES(  FT_PRINT            ) ),
    m_aPrinterLB(this,          SW_RES(  LB_PRINT            ) ),
    m_aPrinterSettingsPB(this,  SW_RES(  PB_PRINTERSETTINGS  ) ),
    m_aPrintNowPB(this,         SW_RES(  PB_PRINTNOW         ) ),

    m_aMailToFT( this, SW_RES(        FT_MAILTO              ) ),
    m_aMailToLB( this, SW_RES(        LB_MAILTO              ) ),
    m_aCopyToPB( this, SW_RES(        PB_COPYTO              ) ),
    m_aSubjectFT( this, SW_RES(       FT_SUBJECT             ) ),
    m_aSubjectED( this, SW_RES(       ED_SUBJECT             ) ),
    m_aSendAsFT( this, SW_RES(        FT_SENDAS              ) ),
    m_aSendAsLB( this, SW_RES(        LB_SENDAS              ) ),
    m_aAttachmentFT( this, SW_RES(    FT_ATTACHMENT              ) ),
    m_aAttachmentED( this, SW_RES(    ED_ATTACHMENT              ) ),
    m_aSendAsPB( this, SW_RES(        PB_SENDAS              ) ),
    m_aSendDocumentsPB( this, SW_RES( PB_SENDDOCUMENTS       ) ),

    m_sSaveStartST(SW_RES(           ST_SAVESTART  ) ),
    m_sSaveMergedST(SW_RES(          ST_SAVEMERGED ) ),
    m_sPrintST(SW_RES(               ST_PRINT      ) ),
    m_sSendMailST(SW_RES(            ST_SENDMAIL   ) ),

    m_sDefaultAttachmentST(SW_RES(   ST_DEFAULTATTACHMENT )),
    m_sNoSubjectQueryST(SW_RES(      ST_SUBJECTQUERY      )),
    m_sNoSubjectST(SW_RES(           ST_NOSUBJECT )),
    m_sNoAttachmentNameST(SW_RES(    ST_NOATTACHMENTNAME )),
    m_sConfigureMail(SW_RES(         ST_CONFIGUREMAIL)),
#ifdef _MSC_VER
#pragma warning (default : 4355)
#endif
    m_bCancelSaving( false ),
    m_pWizard(_pParent),
    m_pTempPrinter( 0 )
{
    FreeResource();

    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    // #i51949# hide e-Mail option if e-Mail is not supported
    if(!rConfigItem.IsMailAvailable())
        m_aSendMailRB.Hide();

    Link aLink = LINK(this, SwMailMergeOutputPage, OutputTypeHdl_Impl);
    m_aSaveStartDocRB.SetClickHdl(aLink);
    m_aSaveMergedDocRB.SetClickHdl(aLink);
    m_aPrintRB.SetClickHdl(aLink);
    m_aSendMailRB.SetClickHdl(aLink);
    m_aSaveStartDocRB.Check();
    m_aPrintAllRB.Check();
    m_aSaveAsOneRB.Check();
    m_aSendAllRB.Check();

    m_aSaveStartDocPB.SetClickHdl(LINK(this, SwMailMergeOutputPage, SaveStartHdl_Impl));
    m_aSaveNowPB.SetClickHdl(LINK(this, SwMailMergeOutputPage, SaveOutputHdl_Impl));
    m_aPrinterLB.SetSelectHdl(LINK(this, SwMailMergeOutputPage, PrinterChangeHdl_Impl));
    m_aPrintNowPB.SetClickHdl(LINK(this, SwMailMergeOutputPage, PrintHdl_Impl));
    m_aPrinterSettingsPB.SetClickHdl(LINK(this, SwMailMergeOutputPage, PrinterSetupHdl_Impl));

    m_aSendAsPB.SetClickHdl(LINK(this, SwMailMergeOutputPage, SendAsHdl_Impl)),
    m_aSendDocumentsPB.SetClickHdl(LINK(this, SwMailMergeOutputPage, SendDocumentsHdl_Impl)),
    m_aSendAsLB.SetSelectHdl(LINK(this, SwMailMergeOutputPage, SendTypeHdl_Impl));

    m_nFromToRBPos = m_aFromRB.GetPosPixel().Y();
    m_nFromToFTPos = m_aToFT.GetPosPixel().Y();
    m_nFromToNFPos = m_aFromNF.GetPosPixel().Y();

    m_nRBOffset = m_nFromToRBPos - m_aSaveIndividualRB.GetPosPixel().Y();

    OutputTypeHdl_Impl(&m_aSaveStartDocRB);

    m_aCopyToPB.SetClickHdl(LINK(this, SwMailMergeOutputPage, CopyToHdl_Impl));

    m_aSaveAsOneRB.SetClickHdl(LINK(this, SwMailMergeOutputPage, DocumentSelectionHdl_Impl));
    m_aSaveIndividualRB.SetClickHdl(LINK(this, SwMailMergeOutputPage, DocumentSelectionHdl_Impl));
    m_aPrintAllRB.SetClickHdl(LINK(this, SwMailMergeOutputPage, DocumentSelectionHdl_Impl));
    m_aSendAllRB.SetClickHdl(LINK(this, SwMailMergeOutputPage, DocumentSelectionHdl_Impl));

    m_aFromRB.SetClickHdl(LINK(this, SwMailMergeOutputPage, DocumentSelectionHdl_Impl));
    //#i63267# printing might be disabled
    m_aPrintRB.Enable(!Application::GetSettings().GetMiscSettings().GetDisablePrinting());
}

SwMailMergeOutputPage::~SwMailMergeOutputPage()
{
    delete m_pTempPrinter;
}

void SwMailMergeOutputPage::ActivatePage()
{
    //fill printer ListBox
    const std::vector<OUString>& rPrinters = Printer::GetPrinterQueues();
    unsigned int nCount = rPrinters.size();
    if ( nCount )
    {
        for( unsigned int i = 0; i < nCount; i++ )
        {
            m_aPrinterLB.InsertEntry( rPrinters[i] );
        }

    }
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();

    SwView* pTargetView = rConfigItem.GetTargetView();
    OSL_ENSURE(pTargetView, "no target view exists");
    if(pTargetView)
    {
        SfxPrinter* pPrinter = pTargetView->GetWrtShell().getIDocumentDeviceAccess()->getPrinter( true );
        m_aPrinterLB.SelectEntry( pPrinter->GetName() );
        m_aToNF.SetValue( rConfigItem.GetMergedDocumentCount() );
        m_aToNF.SetMax( rConfigItem.GetMergedDocumentCount() );
    }
    m_aPrinterLB.SelectEntry( rConfigItem.GetSelectedPrinter() );

    SwView* pSourceView = rConfigItem.GetSourceView();
    OSL_ENSURE(pSourceView, "no source view exists");
    if(pSourceView)
    {
        SwDocShell* pDocShell = pSourceView->GetDocShell();
        if ( pDocShell->HasName() )
        {
            INetURLObject aTmp( pDocShell->GetMedium()->GetName() );
            m_aAttachmentED.SetText(aTmp.getName(
                    INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET ));
        }
    }
}

bool SwMailMergeOutputPage::canAdvance() const
{
    return false;
}

IMPL_LINK(SwMailMergeOutputPage, OutputTypeHdl_Impl, RadioButton*, pButton)
{
    Control* aControls[] =
    {
        &m_aSaveStartDocPB,
        &m_aSaveAsOneRB, &m_aSaveIndividualRB,
        &m_aFromRB, &m_aFromNF, &m_aToFT, &m_aToNF,
        &m_aSaveNowPB,
        &m_aPrinterFT, &m_aPrinterLB, &m_aPrinterSettingsPB, &m_aPrintAllRB,
        &m_aPrintNowPB,
        &m_aMailToFT, &m_aMailToLB, &m_aCopyToPB,
        &m_aSubjectFT, &m_aSubjectED,
        &m_aSendAsFT, &m_aSendAsLB, &m_aSendAsPB,
        &m_aAttachmentFT, &m_aAttachmentED,
        &m_aSendAllRB, &m_aSendDocumentsPB,
        0
    };
    SetUpdateMode(sal_True);
    Control** pControl = aControls;
    do
    {
        (*pControl)->Show(sal_False);

    } while(*(++pControl));

    if(&m_aSaveStartDocRB == pButton)
    {
        m_aSaveStartDocPB.Show();
        m_aSeparatorFL.SetText(m_sSaveStartST);

    }
    else if(&m_aSaveMergedDocRB == pButton)
    {
        Control* aSaveMergedControls[] =
        {
            &m_aSaveAsOneRB, &m_aSaveIndividualRB,
            &m_aFromRB, &m_aFromNF, &m_aToFT, &m_aToNF,
            &m_aSaveNowPB,
            0
        };
        Control** pSaveMergeControl = aSaveMergedControls;
        do
        {
            (*pSaveMergeControl)->Show(sal_True);

        } while(*(++pSaveMergeControl));
        if(!m_aFromRB.IsChecked() && !m_aSaveAsOneRB.IsChecked())
        {
            m_aSaveIndividualRB.Check();
        }
        m_aSeparatorFL.SetText(m_sSaveMergedST);
        //reposition the from/to line
        if(m_aFromRB.GetPosPixel().Y() != m_nFromToRBPos)
        {
            Point aPos(m_aFromRB.GetPosPixel()); aPos.Y() = m_nFromToRBPos; m_aFromRB.SetPosPixel(aPos);
            aPos =   m_aToFT.GetPosPixel();      aPos.Y() = m_nFromToFTPos; m_aToFT.SetPosPixel(aPos);
            aPos =   m_aFromNF.GetPosPixel();    aPos.Y() = m_nFromToNFPos; m_aFromNF.SetPosPixel(aPos);
            aPos =   m_aToNF.GetPosPixel();      aPos.Y() = m_nFromToNFPos; m_aToNF.SetPosPixel(aPos);
        }
    }
    else if(&m_aPrintRB == pButton)
    {
        Control* aPrintControls[] =
        {
            &m_aFromRB, &m_aFromNF, &m_aToFT, &m_aToNF,
            &m_aPrinterFT, &m_aPrinterLB, &m_aPrinterSettingsPB, &m_aPrintAllRB,
            &m_aPrintNowPB,
            0
        };
        Control** pPrinterControl = aPrintControls;
        do
        {
            (*pPrinterControl)->Show(sal_True);

        } while(*(++pPrinterControl));
        if(!m_aFromRB.IsChecked())
            m_aPrintAllRB.Check();

        m_aSeparatorFL.SetText(m_sPrintST);
        //reposition the from/to line
        long nRB_FT_Offset = m_nFromToRBPos - m_nFromToFTPos;
        long nNewRBXPos = m_aPrintAllRB.GetPosPixel().Y() + m_nRBOffset;

        Point aPos(m_aFromRB.GetPosPixel());aPos.Y() = nNewRBXPos;                 m_aFromRB.SetPosPixel(aPos);
        aPos = m_aToFT.GetPosPixel();       aPos.Y() = nNewRBXPos + nRB_FT_Offset; m_aToFT.SetPosPixel(aPos);
        aPos = m_aFromNF.GetPosPixel();     aPos.Y() = nNewRBXPos + nRB_FT_Offset; m_aFromNF.SetPosPixel(aPos);
        aPos = m_aToNF.GetPosPixel();       aPos.Y() = nNewRBXPos + nRB_FT_Offset; m_aToNF.SetPosPixel(aPos);
    }
    else
    {
        Control* aMailControls[] =
        {
            &m_aFromRB, &m_aFromNF, &m_aToFT, &m_aToNF,
            &m_aMailToFT, &m_aMailToLB, &m_aCopyToPB,
            &m_aSubjectFT, &m_aSubjectED,
            &m_aSendAsFT, &m_aSendAsLB, &m_aSendAsPB,
            &m_aAttachmentFT, &m_aAttachmentED,
            &m_aSendAllRB, &m_aSendDocumentsPB, 0
        };
        Control** pMailControl = aMailControls;
        do
        {
            (*pMailControl)->Show(sal_True);

        } while(*(++pMailControl));

        if(!m_aFromRB.IsChecked())
            m_aSendAllRB.Check();
        if(m_aAttachmentED.GetText().isEmpty())
        {
            String sAttach( m_sDefaultAttachmentST );
            sAttach += '.';
            sAttach += lcl_GetExtensionForDocType(
                        (sal_uLong)m_aSendAsLB.GetEntryData(m_aSendAsLB.GetSelectEntryPos()));
            m_aAttachmentED.SetText( sAttach );

        }
        m_aSeparatorFL.SetText(m_sSendMailST);
        //fill mail address ListBox
        if(!m_aMailToLB.GetEntryCount())
        {
            SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
            //select first column
            uno::Reference< sdbcx::XColumnsSupplier > xColsSupp( rConfigItem.GetResultSet(), uno::UNO_QUERY);
            //get the name of the actual columns
            uno::Reference < container::XNameAccess> xColAccess = xColsSupp.is() ? xColsSupp->getColumns() : 0;
            uno::Sequence< OUString > aFields;
            if(xColAccess.is())
                aFields = xColAccess->getElementNames();
            const OUString* pFields = aFields.getConstArray();
            for(sal_Int32 nField = 0; nField < aFields.getLength(); ++nField)
                m_aMailToLB.InsertEntry(pFields[nField]);

            m_aMailToLB.SelectEntryPos(0);
            // then select the right one - may not be available
            const ResStringArray& rHeaders = rConfigItem.GetDefaultAddressHeaders();
            String sEMailColumn = rHeaders.GetString( MM_PART_E_MAIL );
            Sequence< OUString> aAssignment =
                            rConfigItem.GetColumnAssignment( rConfigItem.GetCurrentDBData() );
            if(aAssignment.getLength() > MM_PART_E_MAIL && !aAssignment[MM_PART_E_MAIL].isEmpty())
                sEMailColumn = aAssignment[MM_PART_E_MAIL];
            m_aMailToLB.SelectEntry(sEMailColumn);
            // HTML format pre-selected
            m_aSendAsLB.SelectEntryPos(3);
            SendTypeHdl_Impl(&m_aSendAsLB);
        }
        if(m_aSendAllRB.GetPosPixel().Y() + m_nRBOffset != m_aFromRB.GetPosPixel().Y())
        {
            long nRB_FT_Offset = m_nFromToRBPos - m_nFromToFTPos;
            long nNewRBXPos = m_aSendAllRB.GetPosPixel().Y() + m_nRBOffset;

            Point aPos(m_aFromRB.GetPosPixel());aPos.Y() = nNewRBXPos;                 m_aFromRB.SetPosPixel(aPos);
            aPos = m_aToFT.GetPosPixel();       aPos.Y() = nNewRBXPos + nRB_FT_Offset; m_aToFT.SetPosPixel(aPos);
            aPos = m_aFromNF.GetPosPixel();     aPos.Y() = nNewRBXPos + nRB_FT_Offset; m_aFromNF.SetPosPixel(aPos);
            aPos = m_aToNF.GetPosPixel();       aPos.Y() = nNewRBXPos + nRB_FT_Offset; m_aToNF.SetPosPixel(aPos);
        }
    }
    m_aFromRB.GetClickHdl().Call(m_aFromRB.IsChecked() ? &m_aFromRB : 0);

    SetUpdateMode(sal_False);
    return 0;
}

IMPL_LINK(SwMailMergeOutputPage, DocumentSelectionHdl_Impl, RadioButton*, pButton)
{
    sal_Bool bEnableFromTo = pButton == &m_aFromRB;
    m_aFromNF.Enable(bEnableFromTo);
    m_aToFT.Enable(bEnableFromTo);
    m_aToNF.Enable(bEnableFromTo);
    return 0;
}

IMPL_LINK(SwMailMergeOutputPage, CopyToHdl_Impl, PushButton*, pButton)
{
    SwCopyToDialog* pDlg = new SwCopyToDialog(pButton);
    pDlg->SetCC(m_sCC );
    pDlg->SetBCC(m_sBCC);
    if(RET_OK == pDlg->Execute())
    {
        m_sCC =     pDlg->GetCC() ;
        m_sBCC =    pDlg->GetBCC();
    }
    delete pDlg;
    return 0;
}

IMPL_LINK(SwMailMergeOutputPage, SaveStartHdl_Impl, PushButton*, pButton)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    SwView* pSourceView = rConfigItem.GetSourceView();
    OSL_ENSURE( pSourceView, "source view missing");
    if(pSourceView)
    {
        SfxViewFrame* pSourceViewFrm = pSourceView->GetViewFrame();
        uno::Reference< frame::XFrame > xFrame =
                pSourceViewFrm->GetFrame().GetFrameInterface();
        xFrame->getContainerWindow()->setVisible(sal_True);
        pSourceViewFrm->GetDispatcher()->Execute(SID_SAVEDOC, SFX_CALLMODE_SYNCHRON);
        xFrame->getContainerWindow()->setVisible(sal_False);
        SwDocShell* pDocShell = pSourceView->GetDocShell();
        //if the document has been saved it's URL has to be stored for
        // later use and it can be closed now
        if(pDocShell->HasName() && !pDocShell->IsModified())
        {
            INetURLObject aURL = pDocShell->GetMedium()->GetURLObject();
            //update the attachment name
            if(m_aAttachmentED.GetText().isEmpty())
            {
                if ( pDocShell->HasName() )
                {
                    m_aAttachmentED.SetText(aURL.getName(
                            INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET ));
                }
            }

            rConfigItem.AddSavedDocument(
                    aURL.GetMainURL(INetURLObject::DECODE_TO_IURI));
            pButton->Enable(sal_False);
            m_pWizard->enableButtons(WZB_FINISH, sal_True);
            pButton->Enable(sal_False);

        }
    }
    return 0;
}

IMPL_LINK_NOARG(SwMailMergeOutputPage, SaveCancelHdl_Impl)
{
    m_bCancelSaving = true;
    return 0;
}

IMPL_LINK(SwMailMergeOutputPage, SaveOutputHdl_Impl, PushButton*, pButton)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    SwView* pTargetView = rConfigItem.GetTargetView();
    OSL_ENSURE(pTargetView, "no target view exists");
    if(!pTargetView)
        return 0;

    if(m_aSaveAsOneRB.IsChecked())
    {
        String sFilter;
        String sPath = SwMailMergeHelper::CallSaveAsDialog(sFilter);
        if(!sPath.Len())
            return 0;
        uno::Sequence< beans::PropertyValue > aValues(1);
        beans::PropertyValue* pValues = aValues.getArray();
        pValues[0].Name = "FilterName";
        pValues[0].Value <<= OUString(sFilter);

        uno::Reference< frame::XStorable > xStore( pTargetView->GetDocShell()->GetModel(), uno::UNO_QUERY);
        sal_uInt32 nErrorCode = ERRCODE_NONE;
        try
        {
            xStore->storeToURL( sPath, aValues );
        }
        catch (const task::ErrorCodeIOException& rErrorEx)
        {
            nErrorCode = (sal_uInt32)rErrorEx.ErrCode;
        }
        catch (const Exception&)
        {
            nErrorCode = ERRCODE_IO_GENERAL;
        }
        if( nErrorCode != ERRCODE_NONE )
        {
            SfxErrorContext aEc(ERRCTX_SFX_SAVEASDOC, pTargetView->GetDocShell()->GetTitle());
            ErrorHandler::HandleError( nErrorCode );
        }
    }
    else
    {
        sal_uInt32 nBegin = 0;
        sal_uInt32 nEnd = 0;
        if(m_aSaveIndividualRB.IsChecked())
        {
            nBegin = 0;
            nEnd = rConfigItem.GetMergedDocumentCount();
        }
        else
        {
            nBegin  = static_cast< sal_Int32 >(m_aFromNF.GetValue() - 1);
            nEnd    = static_cast< sal_Int32 >(m_aToNF.GetValue());
            if(nEnd > rConfigItem.GetMergedDocumentCount())
                nEnd = rConfigItem.GetMergedDocumentCount();
        }
        String sFilter;
        String sPath = SwMailMergeHelper::CallSaveAsDialog(sFilter);
        if(!sPath.Len())
            return 0;
        String sTargetTempURL = URIHelper::SmartRel2Abs(
            INetURLObject(), utl::TempFile::CreateTempName(),
            URIHelper::GetMaybeFileHdl());
        const SfxFilter *pSfxFlt = SwIoSystem::GetFilterOfFormat(
                OUString( FILTER_XML ),
                SwDocShell::Factory().GetFilterContainer() );

        uno::Sequence< beans::PropertyValue > aValues(1);
        beans::PropertyValue* pValues = aValues.getArray();
        pValues[0].Name = "FilterName";
        pValues[0].Value <<= OUString(pSfxFlt->GetFilterName());

        uno::Reference< frame::XStorable > xStore( pTargetView->GetDocShell()->GetModel(), uno::UNO_QUERY);
        sal_uInt32 nErrorCode = ERRCODE_NONE;
        try
        {
            xStore->storeToURL( sTargetTempURL, aValues );
        }
        catch (const task::ErrorCodeIOException& rErrorEx)
        {
            nErrorCode = (sal_uInt32)rErrorEx.ErrCode;
        }
        catch (const Exception&)
        {
            nErrorCode = ERRCODE_IO_GENERAL;
        }
        if( nErrorCode != ERRCODE_NONE )
        {
            SfxErrorContext aEc(ERRCTX_SFX_SAVEASDOC, pTargetView->GetDocShell()->GetTitle());
            ErrorHandler::HandleError( nErrorCode );
        }

        SwView* pSourceView = rConfigItem.GetSourceView();
        PrintMonitor aSaveMonitor(this, PrintMonitor::MONITOR_TYPE_SAVE);
        aSaveMonitor.m_pDocName->SetText(pSourceView->GetDocShell()->GetTitle(22));
        aSaveMonitor.m_pCancel->SetClickHdl(LINK(this, SwMailMergeOutputPage, SaveCancelHdl_Impl));
        aSaveMonitor.m_pPrinter->SetText( INetURLObject( sPath ).getFSysPath( INetURLObject::FSYS_DETECT ) );
        m_bCancelSaving = false;
        aSaveMonitor.Show();
        m_pWizard->enableButtons(WZB_CANCEL, sal_False);

        for(sal_uInt32 nDoc = nBegin; nDoc < nEnd && !m_bCancelSaving; ++nDoc)
        {
            SwDocMergeInfo& rInfo = rConfigItem.GetDocumentMergeInfo(nDoc);
            INetURLObject aURL(sPath);
            String sExtension = aURL.getExtension();
            if(!sExtension.Len())
            {
                sExtension = comphelper::string::getToken(pSfxFlt->GetWildcard().getGlob(), 1, '.');
                sPath += '.';
                sPath += sExtension;
            }
            String sStat(SW_RES(STR_STATSTR_LETTER));
            sStat += ' ';
            sStat += OUString::number( nDoc );
            aSaveMonitor.m_pPrintInfo->SetText(sStat);

            //now extract a document from the target document
            // the shell will be closed at the end, but it is more safe to use SfxObjectShellLock here
            SfxObjectShellLock xTempDocShell( new SwDocShell( SFX_CREATE_MODE_STANDARD ) );
            xTempDocShell->DoInitNew( 0 );
            SfxViewFrame* pTempFrame = SfxViewFrame::LoadHiddenDocument( *xTempDocShell, 0 );
            SwView* pTempView = static_cast<SwView*>( pTempFrame->GetViewShell() );
            pTargetView->GetWrtShell().StartAction();
            SwgReaderOption aOpt;
            aOpt.SetTxtFmts( sal_True );
            aOpt.SetFrmFmts( sal_True );
            aOpt.SetPageDescs( sal_True );
            aOpt.SetNumRules( sal_True );
            aOpt.SetMerge( sal_False );
            pTempView->GetDocShell()->LoadStylesFromFile(
                    sTargetTempURL, aOpt, sal_True );

            pTargetView->GetWrtShell().PastePages(pTempView->GetWrtShell(),
                    (sal_uInt16)rInfo.nStartPageInTarget, (sal_uInt16)rInfo.nEndPageInTarget );
            pTargetView->GetWrtShell().EndAction();
            //then save it
            String sOutPath = aURL.GetMainURL(INetURLObject::DECODE_TO_IURI);
            String sCounter = OUString('_');
            sCounter += OUString::number(nDoc);
            sOutPath.Insert(sCounter, sOutPath.Len() - sExtension.Len() - 1);

            while(true)
            {
                //time for other slots is needed
                for(sal_Int16 r = 0; r < 10; ++r)
                    Application::Reschedule();
                bool bFailed = false;
                try
                {
                    pValues[0].Value <<= OUString(sFilter);
                    uno::Reference< frame::XStorable > xTempStore( xTempDocShell->GetModel(), uno::UNO_QUERY);
                    xTempStore->storeToURL( sOutPath, aValues   );
                }
                catch (const uno::Exception&)
                {
                    bFailed = true;
                }

                if(bFailed)
                {
                    SwSaveWarningBox_Impl aWarning( pButton, sOutPath );
                    if(RET_OK == aWarning.Execute())
                        sOutPath = aWarning.GetFileName();
                    else
                    {
                        xTempDocShell->DoClose();
                        return 0;
                    }
                }
                else
                {
                    xTempDocShell->DoClose();
                    break;
                }
            }
        }
        ::osl::File::remove( sTargetTempURL );
    }
    m_pWizard->enableButtons(WZB_CANCEL, sal_True);
    m_pWizard->enableButtons(WZB_FINISH, sal_True);
    return 0;
}

IMPL_LINK(SwMailMergeOutputPage, PrinterChangeHdl_Impl, ListBox*, pBox)
{
    SwView *const pTargetView = m_pWizard->GetConfigItem().GetTargetView();
    SfxPrinter *const pDocumentPrinter = pTargetView->GetWrtShell()
        .getIDocumentDeviceAccess()->getPrinter(true);
    if (pDocumentPrinter && pBox->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND)
    {
        const QueueInfo* pInfo = Printer::GetQueueInfo( pBox->GetSelectEntry(), false );

        if( pInfo )
        {
            if ( !m_pTempPrinter )
            {
                if ((pDocumentPrinter->GetName() == pInfo->GetPrinterName()) &&
                    (pDocumentPrinter->GetDriverName() == pInfo->GetDriver()))
                {
                    m_pTempPrinter = new Printer(pDocumentPrinter->GetJobSetup());
                }
                else
                    m_pTempPrinter = new Printer( *pInfo );
            }
            else
            {
                if( (m_pTempPrinter->GetName() != pInfo->GetPrinterName()) ||
                     (m_pTempPrinter->GetDriverName() != pInfo->GetDriver()) )
                {
                    delete m_pTempPrinter;
                    m_pTempPrinter = new Printer( *pInfo );
                }
            }
        }
        else if( ! m_pTempPrinter )
            m_pTempPrinter = new Printer();

        m_aPrinterSettingsPB.Enable( m_pTempPrinter->HasSupport( SUPPORT_SETUPDIALOG ) );
    }
    else
        m_aPrinterSettingsPB.Disable();
    m_pWizard->GetConfigItem().SetSelectedPrinter( pBox->GetSelectEntry() );

    return 0;
}

IMPL_LINK_NOARG(SwMailMergeOutputPage, PrintHdl_Impl)
{
    SwView* pTargetView = m_pWizard->GetConfigItem().GetTargetView();
    OSL_ENSURE(pTargetView, "no target view exists");
    if(!pTargetView)
        return 0;

    sal_uInt32 nBegin = 0;
    sal_uInt32 nEnd = 0;
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    if(m_aPrintAllRB.IsChecked())
    {
        nBegin = 0;
        nEnd = rConfigItem.GetMergedDocumentCount();
    }
    else
    {
        nBegin  = static_cast< sal_Int32 >(m_aFromNF.GetValue() - 1);
        nEnd    = static_cast< sal_Int32 >(m_aToNF.GetValue());
        if(nEnd > rConfigItem.GetMergedDocumentCount())
            nEnd = rConfigItem.GetMergedDocumentCount();
    }
    rConfigItem.SetPrintRange( (sal_uInt16)nBegin, (sal_uInt16)nEnd );
    SwDocMergeInfo& rStartInfo = rConfigItem.GetDocumentMergeInfo(nBegin);
    SwDocMergeInfo& rEndInfo = rConfigItem.GetDocumentMergeInfo(nEnd - 1);

    OUString sPages(OUString::number( rStartInfo.nStartPageInTarget ));
    sPages += OUString(" - ");
    sPages += OUString::number(  rEndInfo.nEndPageInTarget );

    SwWrtShell& rSh = pTargetView->GetWrtShell();
    pTargetView->SetMailMergeConfigItem(&rConfigItem, 0, sal_False);
    if(m_pTempPrinter)
    {
        SfxPrinter *const pDocumentPrinter = pTargetView->GetWrtShell()
            .getIDocumentDeviceAccess()->getPrinter(true);
        pDocumentPrinter->SetPrinterProps(m_pTempPrinter);
        // this should be able to handle setting its own printer
        pTargetView->SetPrinter(pDocumentPrinter);
    }

    SfxObjectShell* pObjSh = pTargetView->GetViewFrame()->GetObjectShell();
    SFX_APP()->NotifyEvent(SfxEventHint(SW_EVENT_MAIL_MERGE, SwDocShell::GetEventName(STR_SW_EVENT_MAIL_MERGE), pObjSh));
    rSh.GetNewDBMgr()->SetMergeType( DBMGR_MERGE_DOCUMENTS );
    SfxBoolItem aMergeSilent(SID_SILENT, sal_False);
    m_pWizard->enableButtons(WZB_CANCEL, sal_False);

    uno::Sequence < beans::PropertyValue > aProps( 2 );
    aProps[0]. Name = OUString("MonitorVisible");
    aProps[0].Value <<= sal_True;
    aProps[1]. Name = OUString("Pages");
    aProps[1]. Value <<= sPages;

    pTargetView->ExecPrint( aProps, false, true );
    SFX_APP()->NotifyEvent(SfxEventHint(SW_EVENT_MAIL_MERGE_END, SwDocShell::GetEventName(STR_SW_EVENT_MAIL_MERGE_END), pObjSh));

    pTargetView->SetMailMergeConfigItem(0, 0, sal_False);
    m_pWizard->enableButtons(WZB_CANCEL, sal_True);
    m_pWizard->enableButtons(WZB_FINISH, sal_True);
    return 0;
}

IMPL_LINK(SwMailMergeOutputPage, PrinterSetupHdl_Impl, PushButton*, pButton)
{
    if( !m_pTempPrinter )
        PrinterChangeHdl_Impl(&m_aPrinterLB);
    if(m_pTempPrinter)
        m_pTempPrinter->Setup(pButton);
    return 0;
}

IMPL_LINK(SwMailMergeOutputPage, SendTypeHdl_Impl, ListBox*, pBox)
{
    sal_uLong nDocType = (sal_uLong)pBox->GetEntryData(pBox->GetSelectEntryPos());
    sal_Bool bEnable = MM_DOCTYPE_HTML != nDocType && MM_DOCTYPE_TEXT != nDocType;
    m_aSendAsPB.Enable( bEnable );
    m_aAttachmentFT.Enable( bEnable );
    m_aAttachmentED.Enable( bEnable );
    if(bEnable)
    {
        //add the correct extension
        String sAttach(m_aAttachmentED.GetText());
        //do nothing if the user has removed the name - the warning will come early enough
        if(sAttach.Len())
        {
            xub_StrLen nTokenCount = comphelper::string::getTokenCount(sAttach, '.');
            if( 2 > nTokenCount)
            {
                sAttach += '.';
                ++nTokenCount;
            }
            sAttach.SetToken( nTokenCount - 1, '.', lcl_GetExtensionForDocType( nDocType ));
            m_aAttachmentED.SetText(sAttach);
        }
    }
    return 0;
}

IMPL_LINK(SwMailMergeOutputPage, SendAsHdl_Impl, PushButton*, pButton)
{
    SwMailBodyDialog* pDlg = new SwMailBodyDialog(pButton, m_pWizard);
    pDlg->SetBody(m_sBody);
    if(RET_OK == pDlg->Execute())
    {
        m_sBody = pDlg->GetBody();
    }
    return 0;
}

/*-------------------------------------------------------------------------
    Send documents as e-mail
  -----------------------------------------------------------------------*/
IMPL_LINK(SwMailMergeOutputPage, SendDocumentsHdl_Impl, PushButton*, pButton)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();

    //get the composed document
    SwView* pTargetView = rConfigItem.GetTargetView();
    OSL_ENSURE(pTargetView, "no target view exists");
    if(!pTargetView)
        return 0;

    if(rConfigItem.GetMailServer().isEmpty() ||
            !SwMailMergeHelper::CheckMailAddress(rConfigItem.GetMailAddress()) )
    {
        QueryBox aQuery(pButton, WB_YES_NO_CANCEL, m_sConfigureMail);
        sal_uInt16 nRet = aQuery.Execute();
        if(RET_YES == nRet )
        {
            SfxAllItemSet aSet(pTargetView->GetPool());
            SwMailConfigDlg* pDlg = new SwMailConfigDlg(pButton, aSet);
            nRet = pDlg->Execute();
            delete pDlg;
        }

        if(nRet != RET_OK && nRet != RET_YES)
            return 0;
    }
    //add the documents
    sal_uInt32 nBegin = 0;
    sal_uInt32 nEnd = 0;
    if(m_aSendAllRB.IsChecked())
    {
        nBegin = 0;
        nEnd = rConfigItem.GetMergedDocumentCount();
    }
    else
    {
        nBegin  = static_cast< sal_Int32 >(m_aFromNF.GetValue() - 1);
        nEnd    = static_cast< sal_Int32 >(m_aToNF.GetValue());
        if(nEnd > rConfigItem.GetMergedDocumentCount())
            nEnd = rConfigItem.GetMergedDocumentCount();
    }
    bool bAsBody = false;
    rtl_TextEncoding eEncoding = ::osl_getThreadTextEncoding();
    SfxFilterContainer* pFilterContainer = SwDocShell::Factory().GetFilterContainer();
    const SfxFilter *pSfxFlt = 0;
    sal_uLong nDocType = (sal_uLong)m_aSendAsLB.GetEntryData(m_aSendAsLB.GetSelectEntryPos());
    String sExtension = lcl_GetExtensionForDocType(nDocType);
    switch( nDocType )
    {
        case MM_DOCTYPE_OOO:
        {
            //Make sure we don't pick e.g. the flat xml filter
            //for this format
            pSfxFlt = SwIoSystem::GetFilterOfFormat(
                OUString( FILTER_XML ),
                SwDocShell::Factory().GetFilterContainer() );
        }
        break;
        case MM_DOCTYPE_PDF:
        {
            pSfxFlt = pFilterContainer->GetFilter4FilterName(
                OUString("writer_pdf_Export"),
                SFX_FILTER_EXPORT);
        }
        break;
        case MM_DOCTYPE_WORD:
        {
            //the method SwIOSystemGetFilterOfFormat( ) returns the template filter
            //because it uses the same user data :-(
            SfxFilterMatcher aMatcher( pFilterContainer->GetName() );
            SfxFilterMatcherIter aIter( aMatcher );
            const SfxFilter* pFilter = aIter.First();
            while ( pFilter )
            {
                if( pFilter->GetUserData() == FILTER_WW8 && pFilter->CanExport() )
                {
                    pSfxFlt = pFilter;
                    break;
                }
                pFilter = aIter.Next();
            }

        }
        break;
        case MM_DOCTYPE_HTML:
        {
            bAsBody = true;
            SvxHtmlOptions& rHtmlOptions = SvxHtmlOptions::Get();
            eEncoding = rHtmlOptions.GetTextEncoding();
        }
        break;
        case MM_DOCTYPE_TEXT:
        {
            bAsBody = true;
            pSfxFlt = pFilterContainer->GetFilter4FilterName(
                OUString("Text (encoded)"), SFX_FILTER_EXPORT);
        }
        break;
    }
    if(!pSfxFlt)
        pSfxFlt = pFilterContainer->GetFilter4Extension(sExtension, SFX_FILTER_EXPORT);

    if(!pSfxFlt)
        return 0;
    OUString sMimeType = pSfxFlt->GetMimeType();

    if(m_aSubjectED.GetText().isEmpty())
    {
        SwSendQueryBox_Impl aQuery(pButton, m_sNoSubjectQueryST);
        aQuery.SetIsEmptyTextAllowed(true);
        aQuery.SetValue(m_sNoSubjectST);
        if(RET_OK == aQuery.Execute())
        {
            if(aQuery.GetValue() != m_sNoSubjectST)
                m_aSubjectED.SetText(aQuery.GetValue());
        }
        else
            return 0;
    }
    if(!bAsBody && m_aAttachmentED.GetText().isEmpty())
    {
        SwSendQueryBox_Impl aQuery(pButton, m_sNoAttachmentNameST);
        aQuery.SetIsEmptyTextAllowed(false);
        if(RET_OK == aQuery.Execute())
        {
            String sAttach(aQuery.GetValue());
            xub_StrLen nTokenCount = comphelper::string::getTokenCount(sAttach, '.');
            if( 2 > nTokenCount)
            {
                sAttach += '.';
                ++nTokenCount;
            }
            sAttach.SetToken( nTokenCount - 1, '.', lcl_GetExtensionForDocType(
                     (sal_uLong)m_aSendAsLB.GetEntryData(m_aSendAsLB.GetSelectEntryPos())));
            m_aAttachmentED.SetText(sAttach);
        }
        else
            return 0;
    }
    SfxStringItem aFilterName( SID_FILTER_NAME, pSfxFlt->GetFilterName() );
    String sEMailColumn = m_aMailToLB.GetSelectEntry();
    OSL_ENSURE( sEMailColumn.Len(), "No email column selected");
    Reference< sdbcx::XColumnsSupplier > xColsSupp( rConfigItem.GetResultSet(), UNO_QUERY);
    Reference < container::XNameAccess> xColAccess = xColsSupp.is() ? xColsSupp->getColumns() : 0;
    if(!sEMailColumn.Len() || !xColAccess.is() || !xColAccess->hasByName(sEMailColumn))
        return 0;

    OUString sFilterOptions;
    if(MM_DOCTYPE_TEXT == nDocType)
    {
        SwAsciiOptions aOpt;
        sal_uInt16 nAppScriptType = GetI18NScriptTypeOfLanguage( (sal_uInt16)GetAppLanguage() );
        sal_uInt16 nWhich = GetWhichOfScript( RES_CHRATR_LANGUAGE, nAppScriptType);
        aOpt.SetLanguage( ((SvxLanguageItem&)pTargetView->GetWrtShell().
                            GetDefault( nWhich )).GetLanguage());
        aOpt.SetParaFlags( LINEEND_CR );
        aOpt.WriteUserData( sFilterOptions );
    }
    String sTargetTempURL = URIHelper::SmartRel2Abs(
        INetURLObject(), utl::TempFile::CreateTempName(),
        URIHelper::GetMaybeFileHdl());
    const SfxFilter *pTargetSfxFlt = SwIoSystem::GetFilterOfFormat(
            OUString( FILTER_XML ),
            SwDocShell::Factory().GetFilterContainer() );

    uno::Sequence< beans::PropertyValue > aValues(1);
    beans::PropertyValue* pValues = aValues.getArray();
    pValues[0].Name = "FilterName";
    pValues[0].Value <<= OUString(pTargetSfxFlt->GetFilterName());

    uno::Reference< frame::XStorable > xStore( pTargetView->GetDocShell()->GetModel(), uno::UNO_QUERY);
    xStore->storeToURL( sTargetTempURL, aValues   );

    //create the send dialog
    SwSendMailDialog* pDlg = new SwSendMailDialog( pButton, rConfigItem );
    pDlg->SetDocumentCount( nEnd );
    pDlg->ShowDialog();
    //help to force painting the dialog
    //TODO/CLEANUP
    //predetermined breaking point
    for ( sal_Int16 i = 0; i < 25; i++)
        Application::Reschedule();
    for(sal_uInt32 nDoc = nBegin; nDoc < nEnd; ++nDoc)
    {
        m_pWizard->EnterWait();
        SwDocMergeInfo& rInfo = rConfigItem.GetDocumentMergeInfo(nDoc);

        //now extract a document from the target document
        // the shell will be closed at the end, but it is more safe to use SfxObjectShellLock here
        SfxObjectShellLock xTempDocShell( new SwDocShell( SFX_CREATE_MODE_STANDARD ) );
        xTempDocShell->DoInitNew( 0 );
        SfxViewFrame* pTempFrame = SfxViewFrame::LoadHiddenDocument( *xTempDocShell, 0 );
        SwView* pTempView = static_cast<SwView*>( pTempFrame->GetViewShell() );
        pTargetView->GetWrtShell().StartAction();
        SwgReaderOption aOpt;
        aOpt.SetTxtFmts( sal_True );
        aOpt.SetFrmFmts( sal_True );
        aOpt.SetPageDescs( sal_True );
        aOpt.SetNumRules( sal_True );
        aOpt.SetMerge( sal_False );
        pTempView->GetDocShell()->LoadStylesFromFile(
                sTargetTempURL, aOpt, sal_True );
        pTargetView->GetWrtShell().PastePages(pTempView->GetWrtShell(),
                (sal_uInt16)rInfo.nStartPageInTarget, (sal_uInt16)rInfo.nEndPageInTarget );
        pTargetView->GetWrtShell().EndAction();

        //then save it
        SfxStringItem aName(SID_FILE_NAME,
                URIHelper::SmartRel2Abs(
                    INetURLObject(), utl::TempFile::CreateTempName(0),
                    URIHelper::GetMaybeFileHdl()) );

        {
            uno::Sequence< beans::PropertyValue > aFilterValues(MM_DOCTYPE_TEXT == nDocType ? 2 : 1);
            beans::PropertyValue* pFilterValues = aFilterValues.getArray();
            pFilterValues[0].Name = "FilterName";
            pFilterValues[0].Value <<= OUString(pSfxFlt->GetFilterName());
            if(MM_DOCTYPE_TEXT == nDocType)
            {
                pFilterValues[1].Name = "FilterOptions";
                pFilterValues[1].Value <<= sFilterOptions;
            }

            uno::Reference< frame::XStorable > xTempStore( pTempView->GetDocShell()->GetModel(), uno::UNO_QUERY);
            xTempStore->storeToURL( aName.GetValue(), aFilterValues );
        }
        xTempDocShell->DoClose();

        sal_Int32 nTarget = rConfigItem.MoveResultSet(rInfo.nDBRow);
        OSL_ENSURE( nTarget == rInfo.nDBRow, "row of current document could not be selected");
        (void)nTarget;
        OSL_ENSURE( sEMailColumn.Len(), "No email column selected");
        OUString sEMail = lcl_GetColumnValueOf(sEMailColumn, xColAccess);
        SwMailDescriptor aDesc;
        aDesc.sEMail = sEMail;
        OUString sBody;
        if(bAsBody)
        {
            {
                //read in the temporary file and use it as mail body
                SfxMedium aMedium( aName.GetValue(),    STREAM_READ);
                SvStream* pInStream = aMedium.GetInStream();
                if(pInStream)
                    pInStream->SetStreamCharSet( eEncoding );
                else
                {
                    OSL_FAIL("no output file created?");
                    continue;
                }
                OString sLine;
                sal_Bool bDone = pInStream->ReadLine( sLine );
                while ( bDone )
                {
                    sBody += OStringToOUString(sLine, eEncoding);
                    sBody += OUString('\n');
                    bDone = pInStream->ReadLine( sLine );
                }
            }
            //remove the temporary file
            SWUnoHelper::UCB_DeleteFile( aName.GetValue() );
        }
        else
        {
            sBody = m_sBody;
            aDesc.sAttachmentURL = aName.GetValue();
            String sAttachment(m_aAttachmentED.GetText());
            xub_StrLen nTokenCount = comphelper::string::getTokenCount(sAttachment, '.');
            if( 2 > nTokenCount)
            {
                sAttachment += '.';
                sAttachment.SetToken( nTokenCount, '.', sExtension);
            }
            else if(sAttachment.GetToken( nTokenCount - 1, '.') != sExtension)
                sAttachment += sExtension;
            aDesc.sAttachmentName = sAttachment;
            aDesc.sMimeType = sMimeType;

            if(rConfigItem.IsGreetingLine(sal_True))
            {
                OUString sNameColumn = rConfigItem.GetAssignedColumn(MM_PART_LASTNAME);
                OUString sName = lcl_GetColumnValueOf(sNameColumn, xColAccess);
                String sGreeting;
                if(!sName.isEmpty() && rConfigItem.IsIndividualGreeting(sal_True))
                {
                    OUString sGenderColumn = rConfigItem.GetAssignedColumn(MM_PART_GENDER);
                    const OUString& sFemaleValue = rConfigItem.GetFemaleGenderValue();
                    OUString sGenderValue = lcl_GetColumnValueOf(sGenderColumn, xColAccess);
                    SwMailMergeConfigItem::Gender eGenderType = sGenderValue == sFemaleValue ?
                        SwMailMergeConfigItem::FEMALE :
                        SwMailMergeConfigItem::MALE;

                    sGreeting = SwAddressPreview::FillData(
                        rConfigItem.GetGreetings(eGenderType)
                        [rConfigItem.GetCurrentGreeting(eGenderType)],
                            rConfigItem);
                }
                else
                {
                    sGreeting =
                        rConfigItem.GetGreetings(SwMailMergeConfigItem::NEUTRAL)
                        [rConfigItem.GetCurrentGreeting(SwMailMergeConfigItem::NEUTRAL)];

                }
                sGreeting += '\n';
                OUString sTemp( sGreeting );
                sTemp += sBody;
                sBody = sTemp;
            }
        }
        aDesc.sBodyContent = sBody;
        if(MM_DOCTYPE_HTML == nDocType)
        {
            aDesc.sBodyMimeType = OUString("text/html; charset=");
            aDesc.sBodyMimeType += OUString::createFromAscii(
                                rtl_getBestMimeCharsetFromTextEncoding( eEncoding ));
        }
        else
            aDesc.sBodyMimeType =
                OUString("text/plain; charset=UTF-8; format=flowed");

        aDesc.sSubject = m_aSubjectED.GetText();
        aDesc.sCC = m_sCC;
        aDesc.sBCC = m_sBCC;
        pDlg->AddDocument( aDesc );
        //help to force painting the dialog
        for ( sal_Int16 i = 0; i < 25; i++)
            Application::Reschedule();
        //stop creating of data when dialog has been closed
        if(!pDlg->IsVisible())
        {
            m_pWizard->LeaveWait();
            break;
        }
        m_pWizard->LeaveWait();
    }
    pDlg->EnableDesctruction();
    ::osl::File::remove( sTargetTempURL );

    m_pWizard->enableButtons(WZB_FINISH, sal_True);
    //the dialog deletes itself
    //delete pDlg;
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
