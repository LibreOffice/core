/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mmoutputpage.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:06:41 $
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

#ifndef _MAILMERGEOUTPUTPAGE_HXX
#include <mmoutputpage.hxx>
#endif
#ifndef _MAILMERGEWIZARD_HXX
#include <mailmergewizard.hxx>
#endif
#ifndef _MMCONFIGITEM_HXX
#include <mmconfigitem.hxx>
#endif
#ifndef _SWMAILMERGECHILDWINDOW_HXX
#include <mailmergechildwindow.hxx>
#endif
#ifndef _MAILCONFIGPAGE_HXX
#include <mailconfigpage.hxx>
#endif
#include <cmdid.h>
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef IDOCUMENTDEVICEACCESS_HXX_INCLUDED
#include <IDocumentDeviceAccess.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <svx/scripttypeitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFX_DINFDLG_HXX
#include <sfx2/dinfdlg.hxx>
#endif
#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
//#ifndef _SFX_DOCFILT_HACK_HXX
//#include <sfx2/docfilt.hxx>
//#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _VCL_PRINT_HXX
#include <vcl/print.hxx>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#include <osl/file.hxx>
#ifndef _MAILMERGEGREETINGSPAGE_HXX
#include <mmgreetingspage.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOLUMN_HPP_
#include <com/sun/star/sdb/XColumn.hpp>
#endif
//#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
//#include <com/sun/star/beans/PropertyValue.hpp>
//#endif
//#ifndef _BASEDLGS_HXX
//#include <sfx2/basedlgs.hxx>
//#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _SWUNOHELPER_HXX
#include <swunohelper.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _SVX_HTMLCFG_HXX
#include <svx/htmlcfg.hxx>
#endif
#ifndef _SFXEVENT_HXX
#include <sfx2/event.hxx>
#endif
#ifndef _SWEVENT_HXX
#include <swevent.hxx>
#endif
#include <mmoutputpage.hrc>
#include <dbui.hrc>
#include <helpid.h>
#include <sfx2/app.hxx>

#include <unomid.h>

using namespace svt;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/*-- 01.07.2004 16:47:49---------------------------------------------------

  -----------------------------------------------------------------------*/
String lcl_GetExtensionForDocType(ULONG nDocType)
{
    String sExtension;
    switch( nDocType )
    {
        case MM_DOCTYPE_OOO : sExtension = String::CreateFromAscii( "odt" ); break;
        case MM_DOCTYPE_PDF : sExtension = String::CreateFromAscii( "pdf" ); break;
        case MM_DOCTYPE_WORD: sExtension = String::CreateFromAscii( "doc" ); break;
        case MM_DOCTYPE_HTML: sExtension = String::CreateFromAscii( "html" ); break;
        case MM_DOCTYPE_TEXT: sExtension = String::CreateFromAscii( "txt" ); break;
    }
    return sExtension;
}
/*-- 28.06.2004 11:49:21---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString lcl_GetColumnValueOf(const ::rtl::OUString& rColumn, Reference < container::XNameAccess>& rxColAccess )
{
    ::rtl::OUString sRet;
    if(rxColAccess->hasByName(rColumn))
    {
        Any aCol = rxColAccess->getByName(rColumn);
        Reference< sdb::XColumn > xColumn;
        aCol >>= xColumn;
        if(xColumn.is())
            sRet = xColumn->getString();
    }
    return sRet;
}
/*-- 21.06.2004 14:01:13---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 02.07.2004 08:54:42---------------------------------------------------

  -----------------------------------------------------------------------*/
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

/*-- 21.06.2004 14:11:58---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 21.06.2004 14:11:58---------------------------------------------------

  -----------------------------------------------------------------------*/
SwSaveWarningBox_Impl::~SwSaveWarningBox_Impl()
{
}
/*-- 21.06.2004 14:11:58---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK( SwSaveWarningBox_Impl, ModifyHdl, Edit*, pEdit)
{
    aOKPB.Enable(pEdit->GetText().Len() > 0);
    return 0;
}
/*-- 02.07.2004 09:02:53---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 02.07.2004 08:58:45---------------------------------------------------

  -----------------------------------------------------------------------*/
SwSendQueryBox_Impl::~SwSendQueryBox_Impl()
{
}
/*-- 02.07.2004 08:58:25---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK( SwSendQueryBox_Impl, ModifyHdl, Edit*, pEdit)
{
    aOKPB.Enable(bIsEmptyAllowed  || (pEdit->GetText().Len() > 0));
    return 0;
}

/*-- 16.04.2004 16:34:48---------------------------------------------------

  -----------------------------------------------------------------------*/
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

/*-- 16.04.2004 16:43:18---------------------------------------------------

  -----------------------------------------------------------------------*/
SwCopyToDialog::SwCopyToDialog(Window* pParent) :
    SfxModalDialog(pParent, SW_RES(DLG_MM_COPYTO)),
#ifdef MSC
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
#ifdef MSC
#pragma warning (default : 4355)
#endif
{
    FreeResource();
}
/*-- 16.04.2004 16:43:10---------------------------------------------------

  -----------------------------------------------------------------------*/
SwCopyToDialog::~SwCopyToDialog()
{
}

/*-- 02.04.2004 13:15:54---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailMergeOutputPage::SwMailMergeOutputPage( SwMailMergeWizard* _pParent) :
    svt::OWizardPage( _pParent, SW_RES(DLG_MM_OUTPUT_PAGE)),
#ifdef MSC
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
#ifdef MSC
#pragma warning (default : 4355)
#endif
    m_pWizard(_pParent),
    m_pTempPrinter( 0 ),
    m_pDocumentPrinterCopy(0)
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
}

/*-- 02.04.2004 13:15:44---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailMergeOutputPage::~SwMailMergeOutputPage()
{
    delete m_pTempPrinter;
    delete m_pDocumentPrinterCopy;
}
/*-- 31.01.2005 08:38:14---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeOutputPage::ActivatePage()
{
    //fill printer ListBox
    const std::vector<rtl::OUString>& rPrinters = Printer::GetPrinterQueues();
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
    DBG_ASSERT(pTargetView, "no target view exists")
    if(pTargetView)
    {
        SfxPrinter* pPrinter = pTargetView->GetWrtShell().getIDocumentDeviceAccess()->getPrinter( true );
        m_aPrinterLB.SelectEntry( pPrinter->GetName() );
        m_aToNF.SetValue( rConfigItem.GetMergedDocumentCount() );
        m_aToNF.SetMax( rConfigItem.GetMergedDocumentCount() );
        m_pDocumentPrinterCopy = pTargetView->GetWrtShell().getIDocumentDeviceAccess()->getPrinter( true )->Clone();
    }
    m_aPrinterLB.SelectEntry( rConfigItem.GetSelectedPrinter() );

    SwView* pSourceView = rConfigItem.GetSourceView();
    DBG_ASSERT(pSourceView, "no source view exists")
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
/*-- 05.07.2004 13:54:11---------------------------------------------------

  -----------------------------------------------------------------------*/
bool SwMailMergeOutputPage::canAdvance() const
{
    return false;
}
/*-- 02.04.2004 13:15:44---------------------------------------------------

  -----------------------------------------------------------------------*/
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
    SetUpdateMode(TRUE);
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
    else /*if(&m_aSendMailRB == pButton)*/
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
        if(!m_aAttachmentED.GetText().Len())
        {
            String sAttach( m_sDefaultAttachmentST );
            sAttach += '.';
            sAttach += lcl_GetExtensionForDocType(
                        (ULONG)m_aSendAsLB.GetEntryData(m_aSendAsLB.GetSelectEntryPos()));
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
            uno::Sequence< ::rtl::OUString > aFields;
            if(xColAccess.is())
                aFields = xColAccess->getElementNames();
            const ::rtl::OUString* pFields = aFields.getConstArray();
            for(sal_Int32 nField = 0; nField < aFields.getLength(); ++nField)
                m_aMailToLB.InsertEntry(pFields[nField]);

            m_aMailToLB.SelectEntryPos(0);
            // then select the right one - may not be available
            const ResStringArray& rHeaders = rConfigItem.GetDefaultAddressHeaders();
            String sEMailColumn = rHeaders.GetString( MM_PART_E_MAIL );
            Sequence< ::rtl::OUString> aAssignment =
                            rConfigItem.GetColumnAssignment( rConfigItem.GetCurrentDBData() );
            if(aAssignment.getLength() > MM_PART_E_MAIL && aAssignment[MM_PART_E_MAIL].getLength())
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

    SetUpdateMode(FALSE);
    return 0;
}
/*-- 22.08.2005 12:15:10---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwMailMergeOutputPage, DocumentSelectionHdl_Impl, RadioButton*, pButton)
{
    sal_Bool bEnableFromTo = pButton == &m_aFromRB;
    m_aFromNF.Enable(bEnableFromTo);
    m_aToFT.Enable(bEnableFromTo);
    m_aToNF.Enable(bEnableFromTo);
    return 0;
}

/*-- 16.04.2004 16:45:10---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 17.05.2004 13:51:02---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwMailMergeOutputPage, SaveStartHdl_Impl, PushButton*, pButton)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    SwView* pSourceView = rConfigItem.GetSourceView();
    DBG_ASSERT( pSourceView, "source view missing")
    if(pSourceView)
    {
        SfxViewFrame* pSourceViewFrm = pSourceView->GetViewFrame();
        uno::Reference< frame::XFrame > xFrame =
                pSourceViewFrm->GetFrame()->GetFrameInterface();
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
            if(!m_aAttachmentED.GetText().Len())
            {
                if ( pDocShell->HasName() )
                {
                    m_aAttachmentED.SetText(aURL.getName(
                            INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET ));
                }
            }

            rConfigItem.AddSavedDocument(
                    aURL.GetMainURL(INetURLObject::DECODE_TO_IURI));
            pButton->Enable(FALSE);
            m_pWizard->enableButtons(WZB_FINISH, sal_True);
            pButton->Enable(FALSE);

        }
    }
    return 0;
}
/*-- 17.05.2004 13:51:02---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwMailMergeOutputPage, SaveOutputHdl_Impl, PushButton*, pButton)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    SwView* pTargetView = rConfigItem.GetTargetView();
    DBG_ASSERT(pTargetView, "no target view exists")
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
        pValues[0].Name = C2U("FilterName");
        pValues[0].Value <<= ::rtl::OUString(sFilter);

        uno::Reference< frame::XStorable > xStore( pTargetView->GetDocShell()->GetModel(), uno::UNO_QUERY);
        xStore->storeToURL( sPath, aValues );
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
                String::CreateFromAscii( GetFILTER_XML() ),
                SwDocShell::Factory().GetFilterContainer() );

        uno::Sequence< beans::PropertyValue > aValues(1);
        beans::PropertyValue* pValues = aValues.getArray();
        pValues[0].Name = C2U("FilterName");
        pValues[0].Value <<= ::rtl::OUString(pSfxFlt->GetFilterName());

        uno::Reference< frame::XStorable > xStore( pTargetView->GetDocShell()->GetModel(), uno::UNO_QUERY);
        xStore->storeToURL( sTargetTempURL, aValues   );

        for(sal_uInt32 nDoc = nBegin; nDoc < nEnd; ++nDoc)
        {
            SwDocMergeInfo& rInfo = rConfigItem.GetDocumentMergeInfo(nDoc);
            INetURLObject aURL(sPath);
            String sFile = aURL.GetBase();
            String sExtension = aURL.getExtension();
            if(!sExtension.Len())
            {
                sExtension = pSfxFlt->GetWildcard()().GetToken(1, '.');
                sPath += '.';
                sPath += sExtension;
            }

            //now extract a document from the target document
            SfxObjectShellRef xTempDocShell( new SwDocShell( SFX_CREATE_MODE_STANDARD ) );
            xTempDocShell->DoInitNew( 0 );
            SfxViewFrame* pTempFrame = SfxViewFrame::CreateViewFrame( *xTempDocShell, 0, TRUE );
//            pTempFrame->GetFrame()->Appear();
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
                    (USHORT)rInfo.nStartPageInTarget, (USHORT)rInfo.nEndPageInTarget );
            pTargetView->GetWrtShell().EndAction();
            //then save it
            String sOutPath = aURL.GetMainURL(INetURLObject::DECODE_TO_IURI);
            String sCounter('_');
            sCounter += String::CreateFromInt32(nDoc);
            sOutPath.Insert(sCounter, sOutPath.Len() - sExtension.Len() - 1);
            //SfxStringItem aName(SID_FILE_NAME, sOutPath);
            //SfxStringItem aFilter(SID_FILTER_NAME, sFilter);

            while(true)
            {
                //time for other slots is needed
                for(sal_Int16 r = 0; r < 10; ++r)
                    Application::Reschedule();
                bool bFailed = false;
                try
                {
                    pValues[0].Value <<= ::rtl::OUString(sFilter);
                    uno::Reference< frame::XStorable > xTempStore( xTempDocShell->GetModel(), uno::UNO_QUERY);
                    xTempStore->storeToURL( sOutPath, aValues   );
                }
                catch( const uno::Exception& )
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
    m_pWizard->enableButtons(WZB_FINISH, sal_True);
    return 0;
}
/*-- 22.06.2004 11:51:30---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwMailMergeOutputPage, PrinterChangeHdl_Impl, ListBox*, pBox)
{
    if( m_pDocumentPrinterCopy && pBox->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        const QueueInfo* pInfo = Printer::GetQueueInfo( pBox->GetSelectEntry(), false );

        if( pInfo )
        {
            if ( !m_pTempPrinter )
            {
                if( (m_pDocumentPrinterCopy->GetName() == pInfo->GetPrinterName()) &&
                     (m_pDocumentPrinterCopy->GetDriverName() == pInfo->GetDriver()) )
                    m_pTempPrinter = new Printer( m_pDocumentPrinterCopy->GetJobSetup() );
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

//    return m_pTempPrinter;
    return 0;
}

/*-- 17.05.2004 13:51:02---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwMailMergeOutputPage, PrintHdl_Impl, PushButton*, EMPTYARG)
{
    SwView* pTargetView = m_pWizard->GetConfigItem().GetTargetView();
    DBG_ASSERT(pTargetView, "no target view exists")
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
    SfxBoolItem aSilent( SID_SILENT, sal_True);
    rConfigItem.SetPrintRange( (USHORT)nBegin, (USHORT)nEnd );
    SwWrtShell& rSh = pTargetView->GetWrtShell();
    pTargetView->SetMailMergeConfigItem(&rConfigItem, 0, sal_False);
    if(m_pTempPrinter)
    {
        m_pDocumentPrinterCopy->SetPrinterProps(m_pTempPrinter);
        pTargetView->SetPrinter(m_pDocumentPrinterCopy->Clone());
    }

    SfxObjectShell* pObjSh = pTargetView->GetViewFrame()->GetObjectShell();
    SFX_APP()->NotifyEvent(SfxEventHint(SW_EVENT_MAIL_MERGE, pObjSh));
    rSh.GetNewDBMgr()->SetMergeType( DBMGR_MERGE_DOCUMENTS );
    SfxDispatcher *pDis = pTargetView->GetViewFrame()->GetDispatcher();
    SfxBoolItem aMergeSilent(SID_SILENT, TRUE);
    pDis->Execute(SID_PRINTDOCDIRECT,
            SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD, &aMergeSilent, 0L);
    SFX_APP()->NotifyEvent(SfxEventHint(SW_EVENT_MAIL_MERGE_END, pObjSh));

    pTargetView->SetMailMergeConfigItem(0, 0, sal_False);
    m_pWizard->enableButtons(WZB_FINISH, sal_True);
    return 0;
}
/*-- 17.05.2004 13:51:02---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwMailMergeOutputPage, PrinterSetupHdl_Impl, PushButton*, pButton)
{
    if( !m_pTempPrinter )
        PrinterChangeHdl_Impl(&m_aPrinterLB);
    if(m_pTempPrinter)
        m_pTempPrinter->Setup(pButton);
    return 0;
}
/*-- 14.06.2004 09:34:01---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwMailMergeOutputPage, SendTypeHdl_Impl, ListBox*, pBox)
{
    ULONG nDocType = (ULONG)pBox->GetEntryData(pBox->GetSelectEntryPos());
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
            xub_StrLen nTokenCount = sAttach.GetTokenCount( '.' );
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
/*-- 17.05.2004 13:51:02---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 21.05.2004 12:03:25---------------------------------------------------
    Send documents as e-mail
  -----------------------------------------------------------------------*/
IMPL_LINK(SwMailMergeOutputPage, SendDocumentsHdl_Impl, PushButton*, pButton)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();

    //get the composed document
    SwView* pTargetView = rConfigItem.GetTargetView();
    DBG_ASSERT(pTargetView, "no target view exists")
    if(!pTargetView)
        return 0;

    if(!rConfigItem.GetMailServer().getLength() ||
            !SwMailMergeHelper::CheckMailAddress(rConfigItem.GetMailAddress()) )
    {
        QueryBox aQuery(pButton, WB_YES_NO_CANCEL, m_sConfigureMail);
        USHORT nRet = aQuery.Execute();
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
    //create the send dialog
    SwSendMailDialog* pDlg = new SwSendMailDialog( pButton, rConfigItem );
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
    bool bIsPDF = false;
    rtl_TextEncoding eEncoding = ::gsl_getSystemTextEncoding();
    SfxFilterContainer* pFilterContainer = SwDocShell::Factory().GetFilterContainer();
    const SfxFilter *pSfxFlt = 0;
    ULONG nDocType = (ULONG)m_aSendAsLB.GetEntryData(m_aSendAsLB.GetSelectEntryPos());
    String sExtension = lcl_GetExtensionForDocType(nDocType);
    switch( nDocType )
    {
        case MM_DOCTYPE_OOO : break;
        case MM_DOCTYPE_PDF : bIsPDF = true; break;
        case MM_DOCTYPE_WORD:
        {
            //the method SwIOSystemGetFilterOfFormat( ) returns the template filter
            //because it uses the same user data :-(
            SfxFilterMatcher aMatcher( pFilterContainer->GetName() );
            SfxFilterMatcherIter aIter( &aMatcher );
            const SfxFilter* pFilter = aIter.First();
            String sFilterUserData( String::CreateFromAscii(GetFILTER_WW8()));
            while ( pFilter )
            {
                if( pFilter->GetUserData() == sFilterUserData   && pFilter->CanExport() )
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
            SvxHtmlOptions* pHtmlOptions = SvxHtmlOptions::Get();
            eEncoding = pHtmlOptions->GetTextEncoding();
        }
        break;
        case MM_DOCTYPE_TEXT:
            bAsBody = true;
        break;
    }
    if(!pSfxFlt)
        pSfxFlt = pFilterContainer->GetFilter4Extension(sExtension, SFX_FILTER_EXPORT);

    if(!pSfxFlt)
        return 0;
    String sMimeType = pSfxFlt->GetMimeType();

    if(!m_aSubjectED.GetText().Len())
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
    if(!bAsBody && !m_aAttachmentED.GetText().Len())
    {
        SwSendQueryBox_Impl aQuery(pButton, m_sNoAttachmentNameST);
        aQuery.SetIsEmptyTextAllowed(false);
        if(RET_OK == aQuery.Execute())
        {
            String sAttach(aQuery.GetValue());
            xub_StrLen nTokenCount = sAttach.GetTokenCount( '.' );
            if( 2 > nTokenCount)
            {
                sAttach += '.';
                ++nTokenCount;
            }
            sAttach.SetToken( nTokenCount - 1, '.', lcl_GetExtensionForDocType(
                     (ULONG)m_aSendAsLB.GetEntryData(m_aSendAsLB.GetSelectEntryPos())));
            m_aAttachmentED.SetText(sAttach);
        }
        else
            return 0;
    }
    SfxStringItem aFilterName( SID_FILTER_NAME, pSfxFlt->GetFilterName() );
    String sEMailColumn = m_aMailToLB.GetSelectEntry();
    DBG_ASSERT( sEMailColumn.Len(), "No email column selected");
    Reference< sdbcx::XColumnsSupplier > xColsSupp( rConfigItem.GetResultSet(), UNO_QUERY);
    Reference < container::XNameAccess> xColAccess = xColsSupp.is() ? xColsSupp->getColumns() : 0;
    if(!sEMailColumn.Len() || !xColAccess.is() || !xColAccess->hasByName(sEMailColumn))
        return 0;

    String sFilterOptions;
    if(MM_DOCTYPE_TEXT == nDocType)
    {
        SwAsciiOptions aOpt;
        USHORT nAppScriptType = GetI18NScriptTypeOfLanguage( (USHORT)GetAppLanguage() );
        USHORT nWhich = GetWhichOfScript( RES_CHRATR_LANGUAGE, nAppScriptType);
        aOpt.SetLanguage( ((SvxLanguageItem&)pTargetView->GetWrtShell().
                            GetDefault( nWhich )).GetLanguage());
        aOpt.SetParaFlags( LINEEND_CR );
        aOpt.WriteUserData( sFilterOptions );
    }
    String sTargetTempURL = URIHelper::SmartRel2Abs(
        INetURLObject(), utl::TempFile::CreateTempName(),
        URIHelper::GetMaybeFileHdl());
    const SfxFilter *pTargetSfxFlt = SwIoSystem::GetFilterOfFormat(
            String::CreateFromAscii( GetFILTER_XML() ),
            SwDocShell::Factory().GetFilterContainer() );

    uno::Sequence< beans::PropertyValue > aValues(1);
    beans::PropertyValue* pValues = aValues.getArray();
    pValues[0].Name = C2U("FilterName");
    pValues[0].Value <<= ::rtl::OUString(pTargetSfxFlt->GetFilterName());

    uno::Reference< frame::XStorable > xStore( pTargetView->GetDocShell()->GetModel(), uno::UNO_QUERY);
    xStore->storeToURL( sTargetTempURL, aValues   );

    pDlg->SetDocumentCount( nEnd );
    pDlg->ShowDialog();
    //help to force painting the dialog
    //TODO/CLEANUP
    //Sollbruchstelle
    for ( sal_Int16 i = 0; i < 25; i++)
        Application::Reschedule();
    for(sal_uInt32 nDoc = nBegin; nDoc < nEnd; ++nDoc)
    {
        m_pWizard->EnterWait();
        SwDocMergeInfo& rInfo = rConfigItem.GetDocumentMergeInfo(nDoc);

        //now extract a document from the target document
        SfxObjectShellRef xTempDocShell( new SwDocShell( SFX_CREATE_MODE_STANDARD ) );
        xTempDocShell->DoInitNew( 0 );
        SfxViewFrame* pTempFrame = SfxViewFrame::CreateViewFrame( *xTempDocShell, 0, TRUE );
//        pTempFrame->GetFrame()->Appear();
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
                (USHORT)rInfo.nStartPageInTarget, (USHORT)rInfo.nEndPageInTarget );
        pTargetView->GetWrtShell().EndAction();

        //then save it
        SfxStringItem aName(SID_FILE_NAME,
                URIHelper::SmartRel2Abs(
                    INetURLObject(), utl::TempFile::CreateTempName(0),
                    URIHelper::GetMaybeFileHdl()) );

/*        if(bIsPDF)
        {
            SfxDispatcher* pSfxDispatcher = pTempView->GetViewFrame()->GetDispatcher();
            pSfxDispatcher->Execute(
                            SID_DIRECTEXPORTDOCASPDF,
                                  SFX_CALLMODE_SYNCHRON, &aName, &aFilterName, 0L );
        }
        else*/
        {
            uno::Sequence< beans::PropertyValue > aFilterValues(MM_DOCTYPE_TEXT == nDocType ? 2 : 1);
            beans::PropertyValue* pFilterValues = aFilterValues.getArray();
            pFilterValues[0].Name = C2U("FilterName");
            pFilterValues[0].Value <<= ::rtl::OUString(pSfxFlt->GetFilterName());
            if(MM_DOCTYPE_TEXT == nDocType)
            {
                pFilterValues[1].Name = C2U("FilterOptions");
                pFilterValues[1].Value <<= ::rtl::OUString(sFilterOptions);
            }

            uno::Reference< frame::XStorable > xTempStore( pTempView->GetDocShell()->GetModel(), uno::UNO_QUERY);
            xTempStore->storeToURL( aName.GetValue(), aFilterValues );
        }
        xTempDocShell->DoClose();

#ifdef DBG_UTIL
        sal_Int32 nTarget =
#endif
                rConfigItem.MoveResultSet(rInfo.nDBRow);
        DBG_ASSERT( nTarget == rInfo.nDBRow, "row of current document could not be selected")
        DBG_ASSERT( sEMailColumn.Len(), "No email column selected");
        ::rtl::OUString sEMail = lcl_GetColumnValueOf(sEMailColumn, xColAccess);
        SwMailDescriptor aDesc;
        aDesc.sEMail = sEMail;
        rtl::OUString sBody;
        if(bAsBody)
        {
            {
                //read in the temporary file and use it as mail body
                SfxMedium aMedium( aName.GetValue(),    STREAM_READ, TRUE);
                SvStream* pInStream = aMedium.GetInStream();
                if(pInStream)
                    pInStream->SetStreamCharSet( eEncoding );
                else
                {
                    DBG_ERROR("no output file created?")
                    continue;
                }
                ByteString sLine;
                sal_Bool bDone = pInStream->ReadLine( sLine );
                while ( bDone )
                {
                    sBody += rtl::OUString(String(sLine, eEncoding));
                    sBody += rtl::OUString('\n');
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
            xub_StrLen nTokenCount = sAttachment.GetTokenCount( '.' );
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
                ::rtl::OUString sNameColumn = rConfigItem.GetAssignedColumn(MM_PART_LASTNAME);
                ::rtl::OUString sName = lcl_GetColumnValueOf(sNameColumn, xColAccess);
                String sGreeting;
                if(sName.getLength() && rConfigItem.IsIndividualGreeting(sal_True))
                {
                    ::rtl::OUString sGenderColumn = rConfigItem.GetAssignedColumn(MM_PART_GENDER);
                    const ::rtl::OUString& sFemaleValue = rConfigItem.GetFemaleGenderValue();
                    ::rtl::OUString sGenderValue = lcl_GetColumnValueOf(sGenderColumn, xColAccess);
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
                ::rtl::OUString sTemp( sGreeting );
                sTemp += sBody;
                sBody = sTemp;
            }
        }
        aDesc.sBodyContent = sBody;
        if(MM_DOCTYPE_HTML == nDocType)
        {
            aDesc.sBodyMimeType = ::rtl::OUString::createFromAscii("text/html; charset=");
            aDesc.sBodyMimeType += ::rtl::OUString::createFromAscii(
                                rtl_getBestMimeCharsetFromTextEncoding( eEncoding ));
        }
        else
            aDesc.sBodyMimeType =
                ::rtl::OUString::createFromAscii("text/plain; charset=UTF-8; format=flowed");

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
