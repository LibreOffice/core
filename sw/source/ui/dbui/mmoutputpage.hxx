/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mmoutputpage.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:06:56 $
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
#ifndef _MAILMERGEOUTPUTPAGE_HXX
#define _MAILMERGEOUTPUTPAGE_HXX
#ifndef _SVTOOLS_WIZARDMACHINE_HXX_
#include <svtools/wizardmachine.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SVTABBX_HXX
#include <svtools/svtabbx.hxx>
#endif
#ifndef _HEADBAR_HXX
#include <svtools/headbar.hxx>
#endif
#ifndef _PRGSBAR_HXX
#include <svtools/prgsbar.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif
#ifndef _MAILMERGEHELPER_HXX
#include "mailmergehelper.hxx"
#endif

class SwMailMergeWizard;
class SfxPrinter;
class SwSendMailDialog;

namespace com{ namespace sun{ namespace star{
    namespace mail{
        class XMailMessage;
    }
}}}
/*-- 02.04.2004 09:21:06---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwMailMergeOutputPage : public svt::OWizardPage
{
    SwBoldFixedInfo m_aHeaderFI;
    FixedInfo       m_aOptionsFI;
    RadioButton     m_aSaveStartDocRB;
    RadioButton     m_aSaveMergedDocRB;
    RadioButton     m_aPrintRB;
    RadioButton     m_aSendMailRB;

    FixedLine       m_aSeparatorFL;

    PushButton      m_aSaveStartDocPB;

    RadioButton     m_aSaveAsOneRB;
    RadioButton     m_aSaveIndividualRB;
    RadioButton     m_aPrintAllRB; //has to be here for tab control reasons
    RadioButton     m_aSendAllRB;  //has to be here for tab control reasons
    //this group is used in save and print
    RadioButton     m_aFromRB;
    NumericField    m_aFromNF;
    FixedText       m_aToFT;
    NumericField    m_aToNF;
    PushButton      m_aSaveNowPB;

    FixedText       m_aPrinterFT;
    ListBox         m_aPrinterLB;
    PushButton      m_aPrinterSettingsPB;
    PushButton      m_aPrintNowPB;

    FixedText       m_aMailToFT;
    ListBox         m_aMailToLB;
    PushButton      m_aCopyToPB;
    FixedText       m_aSubjectFT;
    Edit            m_aSubjectED;
    FixedText       m_aSendAsFT;
    ListBox         m_aSendAsLB;
    FixedText       m_aAttachmentFT;
    Edit            m_aAttachmentED;
    PushButton      m_aSendAsPB;
    PushButton      m_aSendDocumentsPB;

    //some FixedLine labels
    String          m_sSaveStartST;
    String          m_sSaveMergedST;
    String          m_sPrintST;
    String          m_sSendMailST;

    //misc strings
    String          m_sDefaultAttachmentST;
    String          m_sNoSubjectQueryST;
    String          m_sNoSubjectST;
    String          m_sNoAttachmentNameST;
    String          m_sConfigureMail;

    String          m_sBody;

    long            m_nFromToRBPos;
    long            m_nFromToFTPos;
    long            m_nFromToNFPos;
    long            m_nRBOffset;

    SwMailMergeWizard*  m_pWizard;

    //some dialog data
    Printer*        m_pTempPrinter;
    SfxPrinter*     m_pDocumentPrinterCopy;
    String          m_sCC;
    String          m_sBCC;


    DECL_LINK(OutputTypeHdl_Impl, RadioButton*);
    DECL_LINK(CopyToHdl_Impl, PushButton*);
    DECL_LINK(SaveStartHdl_Impl, PushButton* );
    DECL_LINK(SaveOutputHdl_Impl, PushButton* );
    DECL_LINK(PrinterChangeHdl_Impl, ListBox* );
    DECL_LINK(PrintHdl_Impl, PushButton* );
    DECL_LINK(PrinterSetupHdl_Impl, PushButton* );
    DECL_LINK(SendTypeHdl_Impl, ListBox*);
    DECL_LINK(SendAsHdl_Impl, PushButton*);
    DECL_LINK(SendDocumentsHdl_Impl, PushButton*);
    DECL_LINK(DocumentSelectionHdl_Impl, RadioButton*);

protected:
        virtual bool    canAdvance() const;
        virtual void    ActivatePage();
public:
        SwMailMergeOutputPage( SwMailMergeWizard* _pParent);
        ~SwMailMergeOutputPage();

};
/*-- 21.05.2004 12:48:50---------------------------------------------------

  -----------------------------------------------------------------------*/
struct SwMailDescriptor
{
    ::rtl::OUString                             sEMail;
    ::rtl::OUString                             sAttachmentURL;
    ::rtl::OUString                             sAttachmentName;
    ::rtl::OUString                             sMimeType;
    ::rtl::OUString                             sSubject;
    ::rtl::OUString                             sBodyMimeType;
    ::rtl::OUString                             sBodyContent;


    ::rtl::OUString                             sCC;
    ::rtl::OUString                             sBCC;
};
struct SwSendMailDialog_Impl;
class SwMailMergeConfigItem;
class SW_DLLPUBLIC SwSendMailDialog : public ModelessDialog //SfxModalDialog
{
    FixedLine               m_aStatusFL;
    FixedText               m_aStatusFT;

    FixedLine               m_aTransferStatusFL;
    FixedText               m_aTransferStatusFT;
    FixedInfo               m_PausedFI;
    ProgressBar             m_aProgressBar;
    FixedText               m_aErrorStatusFT;

    PushButton              m_aDetailsPB;
    HeaderBar               m_aStatusHB;
    SvTabListBox            m_aStatusLB;

    FixedLine               m_aSeparatorFL;

    PushButton              m_aStopPB;
    PushButton              m_aClosePB;

    String                  m_sMore;
    String                  m_sLess;
    String                  m_sContinue;
    String                  m_sStop;
    String                  m_sSend;
    String                  m_sTransferStatus;
    String                  m_sErrorStatus;
    String                  m_sSendingTo;
    String                  m_sCompleted;
    String                  m_sFailed;
    String                  m_sTerminateQuery;

    bool                    m_bCancel;
    bool                    m_bDesctructionEnabled;

    ImageList               m_aImageList;
    ImageList               m_aImageListHC;

    SwSendMailDialog_Impl*  m_pImpl;
    SwMailMergeConfigItem*  m_pConfigItem;
    sal_Int32               m_nStatusHeight;
    sal_Int32               m_nSendCount;
    sal_Int32               m_nErrorCount;

    SW_DLLPRIVATE DECL_LINK( DetailsHdl_Impl, PushButton* );
    SW_DLLPRIVATE DECL_LINK( StopHdl_Impl, PushButton* );
    SW_DLLPRIVATE DECL_LINK( CloseHdl_Impl, PushButton* );
    SW_DLLPRIVATE DECL_STATIC_LINK( SwSendMailDialog, StartSendMails, SwSendMailDialog* );
    SW_DLLPRIVATE DECL_STATIC_LINK( SwSendMailDialog, StopSendMails, SwSendMailDialog* );
    SW_DLLPRIVATE DECL_STATIC_LINK( SwSendMailDialog, RemoveThis, Timer* );

    SW_DLLPRIVATE void        IterateMails();
    SW_DLLPRIVATE void        SendMails();
    SW_DLLPRIVATE void        UpdateTransferStatus();

    virtual void        StateChanged( StateChangedType nStateChange );

public:
    SwSendMailDialog( Window* pParent, SwMailMergeConfigItem& );
    ~SwSendMailDialog();

    void                AddDocument( SwMailDescriptor& rDesc );
    void                SetDocumentCount( sal_Int32 nAllDocuments );
    void                EnableDesctruction() {m_bDesctructionEnabled = true;}
    void                ShowDialog();

    void                DocumentSent( ::com::sun::star::uno::Reference< ::com::sun::star::mail::XMailMessage>,
                                        bool bResult,
                                        const ::rtl::OUString* pError );
    void                AllMailsSent();

};
#endif


