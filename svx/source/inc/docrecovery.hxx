/*************************************************************************
 *
 *  $RCSfile: docrecovery.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-12-03 09:03:55 $
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

#ifndef _SVX_DOCRECOVERY_HXX
#define _SVX_DOCRECOVERY_HXX

#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif
#ifndef _SV_TABDIALOG_HXX
#include <vcl/tabdlg.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SVEDIT_HXX
#include <svtools/svmedit.hxx>
#endif
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif
#include <simptabl.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HPP_
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

//===============================================
// const

#define RECOVERY_CMDPART_PROTOCOL                       ::rtl::OUString::createFromAscii("vnd.sun.star.autorecovery:")

#define RECOVERY_CMDPART_DO_EMERGENCY_SAVE              ::rtl::OUString::createFromAscii("/doEmergencySave")
#define RECOVERY_CMDPART_DO_RECOVERY                    ::rtl::OUString::createFromAscii("/doAutoRecovery" )
#define RECOVERY_CMDPART_DO_CRASHREPORT                 ::rtl::OUString::createFromAscii("/doCrashReport"  )

#define RECOVERY_CMD_DO_EMERGENCY_SAVE                  ::rtl::OUString::createFromAscii("vnd.sun.star.autorecovery:/doEmergencySave")
#define RECOVERY_CMD_DO_RECOVERY                        ::rtl::OUString::createFromAscii("vnd.sun.star.autorecovery:/doAutoRecovery" )
#define RECOVERY_CMD_DO_CRASHREPORT                     ::rtl::OUString::createFromAscii("vnd.sun.star.autorecovery:/doCrashReport"  )
#define RECOVERY_CMD_DO_FAILURE_SAVE                    ::rtl::OUString::createFromAscii("vnd.sun.star.autorecovery:/doFailureSave"  )

#define SERVICENAME_PROGRESSFACTORY                     ::rtl::OUString::createFromAscii("com.sun.star.task.StatusIndicatorFactory")
#define SERVICENAME_URLTRANSFORMER                      ::rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer"        )
#define SERVICENAME_RECOVERYCORE                        ::rtl::OUString::createFromAscii("com.sun.star.frame.AutoRecovery"         )
#define SERVICENAME_FOLDERPICKER                        ::rtl::OUString::createFromAscii("com.sun.star.ui.dialogs.FolderPicker"    )
#define SERVICENAME_DESKTOP                             ::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop"              )

#define PROP_PARENTWINDOW                               ::rtl::OUString::createFromAscii("Window"           )
#define PROP_STATUSINDICATOR                            ::rtl::OUString::createFromAscii("StatusIndicator"  )
#define PROP_DISPATCHASYNCHRON                          ::rtl::OUString::createFromAscii("DispatchAsynchron")
#define PROP_FAILUREPATH                                ::rtl::OUString::createFromAscii("FailurePath"      )

#define DLG_RET_UNKNOWN                                  -1
#define DLG_RET_OK                                        1
#define DLG_RET_CANCEL                                    0
#define DLG_RET_BACK                                    100
#define DLG_RET_OK_AUTOLUNCH                            101

//===============================================
// namespace

namespace svx{
    namespace DocRecovery{

#ifdef css
#   error "Who uses css? I need it as temp. namespace alias!"
#else
#   define css ::com::sun::star
#endif

//===============================================
enum EDocStates
{
    /* TEMP STATES */

    /// default state, if a document was new created or loaded
    E_UNKNOWN = 0,
    /// modified against the original file
    E_MODIFIED = 1,
    /// an active document can be postponed to be saved later.
    E_POSTPONED = 2,
    /// was already handled during one AutoSave/Recovery session.
    E_HANDLED = 4,
    /** an action was started (saving/loading) ... Can be interesting later if the process may be was interrupted by an exception. */
    E_TRY_SAVE = 8,
    E_TRY_LOAD_BACKUP = 16,
    E_TRY_LOAD_ORIGINAL = 32,

    /* FINAL STATES */

    /// the Auto/Emergency saved document isnt useable any longer
    E_DAMAGED = 64,
    /// the Auto/Emergency saved document isnt realy up-to-date (some changes can be missing)
    E_INCOMPLETE = 128,
    /// the Auto/Emergency saved document was processed successfully
    E_SUCCEDED = 512
};

//===============================================
enum ERecoveryState
{
    E_SUCCESSFULLY_RECOVERED,
    E_ORIGINAL_DOCUMENT_RECOVERED,
    E_RECOVERY_FAILED,
    E_RECOVERY_IN_PROGRESS,
    E_NOT_RECOVERED_YET
};

//===============================================
struct TURLInfo
{
    public:

    /// the full qualified document URL
    ::rtl::OUString URL;

    /// the pure file name, without path, disc etcpp.
    ::rtl::OUString DisplayName;

    /// state info as e.g. VALID, CORRUPTED, NON EXISTING ...
    sal_Int32 DocState;

    /// ui representation for DocState!
    ERecoveryState RecoveryState;

    /// standard icon
    Image StandardImage;

    /// high contrast icon
    Image HCImage;
};

//===============================================
typedef ::std::vector< TURLInfo > TURLList;

//===============================================
class IRecoveryUpdateListener
{
    public:

        // inform listener about changed items, which should be refreshed
        virtual void updateItems() = 0;

        // inform listener about starting of the asynchronous recovery operation
        virtual void start() = 0;

        // inform listener about ending of the asynchronous recovery operation
        virtual void end() = 0;

        // TODO
        virtual void stepNext(TURLInfo* pItem) = 0;
};

//===============================================
class RecoveryCore : public ::cppu::WeakImplHelper1< css::frame::XStatusListener >
{
    //-------------------------------------------
    // types, const
    public:

    //-------------------------------------------
    // member
    private:

        /// TODO
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /// TODO
        css::uno::Reference< css::frame::XDispatch > m_xRealCore;

        /// TODO
        css::uno::Reference< css::task::XStatusIndicator > m_xProgress;

        /// TODO
        TURLList m_lURLs;

        /// TODO
        IRecoveryUpdateListener* m_pListener;

    //-------------------------------------------
    // native interface
    public:

        //---------------------------------------
        /** @short  TODO */
        RecoveryCore(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);

        //---------------------------------------
        /** @short  TODO */
        virtual ~RecoveryCore();

        //---------------------------------------
        /** @short  TODO */
        virtual css::uno::Reference< css::lang::XMultiServiceFactory > getSMGR();

        //---------------------------------------
        /** @short  TODO */
        virtual TURLList* getURLListAccess();

        //---------------------------------------
        /** @short  TODO */
        virtual sal_Bool existsFailureURLs();

        //---------------------------------------
        /** @short  TODO */
        virtual void storeFailureURLsTo(const ::rtl::OUString& sPath);

        //---------------------------------------
        /** @short  TODO */
        virtual void setProgressHandler(const css::uno::Reference< css::task::XStatusIndicator >& xProgress);

        //---------------------------------------
        /** @short  TODO */
        virtual void setUpdateListener(IRecoveryUpdateListener* pListener);

        //---------------------------------------
        /** @short  TODO */
        virtual void doEmergencySave();
        virtual void doRecovery();

        //---------------------------------------
        /** @short  TODO */
        static ERecoveryState mapDocState2RecoverState(sal_Int32 eDocState);

    //-------------------------------------------
    // uno interface
    public:

        // css.frame.XStatusListener
        virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& aEvent)
            throw(css::uno::RuntimeException);

        // css.lang.XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);

    //-------------------------------------------
    // helper
    private:

        //---------------------------------------
        /** @short  TODO */
        void impl_startListening();

        //---------------------------------------
        /** @short  TODO */
        css::util::URL impl_getParsedURL(const ::rtl::OUString& sURL);
};

//===============================================
class PluginProgressWindow : public Window
{
    private:

        css::uno::Reference< css::lang::XComponent > m_xProgress;

    public:

        PluginProgressWindow(      Window*                                       pParent  ,
                             const css::uno::Reference< css::lang::XComponent >& xProgress);
        ~PluginProgressWindow();
};

class PluginProgress : public ::cppu::WeakImplHelper2< css::task::XStatusIndicator ,
                                                       css::lang::XComponent       >
{

    //-------------------------------------------
    // member
    private:

        /** @short  TODO */
        css::uno::Reference< css::task::XStatusIndicatorFactory > m_xProgressFactory;

        css::uno::Reference< css::task::XStatusIndicator > m_xProgress;

        PluginProgressWindow* m_pPlugProgressWindow;

    //-------------------------------------------
    // native interface
    public:

        //---------------------------------------
        /** @short  TODO */
        PluginProgress(      Window*                                                 pParent,
                       const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR  );

        //---------------------------------------
        /** @short  TODO */
        virtual ~PluginProgress();

        //---------------------------------------
        /** @short  TODO */
        Window* getPlugWindow();

    //-------------------------------------------
    // uno interface
    public:

        //---------------------------------------
        // XStatusIndicator
        virtual void SAL_CALL start(const ::rtl::OUString& sText ,
                                          sal_Int32        nRange)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL end()
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL setText(const ::rtl::OUString& sText)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL setValue(sal_Int32 nValue)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL reset()
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // XComponent
        virtual void SAL_CALL dispose()
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener >& xListener)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener)
            throw(css::uno::RuntimeException);
};

//===============================================
class IExtendedTabPage : public TabPage
{
    //-------------------------------------------
    // member
    protected:

        short m_nResult;

    //-------------------------------------------
    // interface
    public:

        IExtendedTabPage( Window* pParent, WinBits nStyle = 0 )
            : TabPage( pParent, nStyle )
            , m_nResult(DLG_RET_UNKNOWN)
        {}

        IExtendedTabPage( Window* pParent, const ResId& rResId )
            : TabPage( pParent, rResId )
            , m_nResult(DLG_RET_UNKNOWN)
        {}

        virtual ~IExtendedTabPage()
        {}

        virtual short   execute() = 0;
        virtual void    setDefButton() = 0;
};

typedef ::std::vector< IExtendedTabPage* > TTabPageList;

//===============================================
class TabDialog4Recovery : public TabDialog
{
    //-------------------------------------------
    // member
    private:

        TTabPageList m_lTabPages;
        TTabPageList::iterator m_pActualPage;

    //-------------------------------------------
    // interface
    public:

        TabDialog4Recovery(Window* pParent);
        virtual ~TabDialog4Recovery();

        virtual void addTabPage(IExtendedTabPage* pPage);
        virtual short Execute();
};

//===============================================
class SaveDialog : public IExtendedTabPage
{
    //-------------------------------------------
    // member
    private:

        Window          m_aTitleWin;
        FixedText       m_aTitleFT;
        FixedText       m_aDescrFT;
        FixedText       m_aFileListFT;
        ListBox         m_aFileListLB;
        CheckBox        m_aLaunchAutoCB;
        FixedLine       m_aBottomFL;
        OKButton        m_aOkBtn;

        RecoveryCore*   m_pCore;

    //-------------------------------------------
    // interface
    public:

        //---------------------------------------
        /** @short  create all child controls of this dialog.

            @descr  The dialog isnt shown nor it starts any
                    action by itself!

            @param  pParent
                    can point to a parent window.
                    If its set to 0, the defmodal-dialog-parent
                    is used automaticly.

            @param  pCore
                    provides access to the recovery core service
                    and the current list of open documents,
                    which should be shown inside this dialog.
         */
        SaveDialog(Window*       pParent,
                   RecoveryCore* pCore  );

        //---------------------------------------
        /** @short  free all controls and used memory. */
        virtual ~SaveDialog();

        //---------------------------------------
        /** @short  TODO*/
        virtual short   execute();

        //---------------------------------------
        /** @short  TODO*/
        virtual void    setDefButton();

        DECL_LINK(OKButtonHdl, void*);
};

//===============================================
class SaveProgressDialog : public ModalDialog
                         , public IRecoveryUpdateListener
{
    //-------------------------------------------
    // member
    private:

        FixedText       m_aHintFT;
        FixedText       m_aProgrFT;
        String          m_aProgrBaseTxt;
        Window          m_aProgrParent;

        // @short   TODO
        RecoveryCore* m_pCore;

        // @short   TODO
        css::uno::Reference< css::task::XStatusIndicator > m_xProgress;

    //-------------------------------------------
    // interface
    public:

        //---------------------------------------
        /** @short  create all child controls of this dialog.

            @descr  The dialog isnt shown nor it starts any
                    action by itself!

            @param  pParent
                    can point to a parent window.
                    If its set to 0, the defmodal-dialog-parent
                    is used automaticly.

            @param  pCore
                    used to start emegrency save.
         */
        SaveProgressDialog(Window*       pParent,
                           RecoveryCore* pCore  );

        //---------------------------------------
        /** @short  free all controls and used memory. */
        virtual ~SaveProgressDialog();

        //---------------------------------------
        /** @short  start the emergency save operation. */
        virtual short Execute();

        // IRecoveryUpdateListener
        virtual void updateItems();
        virtual void stepNext(TURLInfo* pItem);
        virtual void start();
        virtual void end();
};

//===============================================
class RecovDocListEntry : public SvLBoxString
{
    //-------------------------------------------
    // interface
    private:

    //-------------------------------------------
    // interface
    public:

        //---------------------------------------
        /** @short TODO */
        RecovDocListEntry(      SvLBoxEntry* pEntry,
                                USHORT       nFlags,
                          const String&      sText );

        //---------------------------------------
        /** @short TODO */
        virtual void Paint(const Point&       aPos   ,
                                 SvLBox&      aDevice,
                                 USHORT       nFlags ,
                                 SvLBoxEntry* pEntry );
};

//===============================================
class RecovDocList : public SvxSimpleTable
{
    //-------------------------------------------
    // member
    public:

        Image  m_aGreenCheckImg;
        Image  m_aYellowCheckImg;
        Image  m_aRedCrossImg;
        Image  m_aGreenCheckImgHC;
        Image  m_aYellowCheckImgHC;
        Image  m_aRedCrossImgHC;

        String m_aSuccessRecovStr;
        String m_aOrigDocRecovStr;
        String m_aRecovFailedStr;
        String m_aRecovInProgrStr;
        String m_aNotRecovYetStr;

    //-------------------------------------------
    // interface
    public:

        //---------------------------------------
        /** @short TODO */
        RecovDocList(      Window* pParent,
                     const ResId&  rResId );

        //---------------------------------------
        /** @short TODO */
        virtual ~RecovDocList();

        //---------------------------------------
        /** @short TODO */
        virtual void InitEntry(      SvLBoxEntry* pEntry ,
                               const XubString&   sText  ,
                               const Image&       aImage1,
                               const Image&       aImage2);
};

//===============================================
class RecoveryDialog : public IExtendedTabPage
                     , public IRecoveryUpdateListener
{
    //-------------------------------------------
    // member
    private:
        Window          m_aTitleWin;
        FixedText       m_aTitleFT;
        FixedText       m_aDescrFT;
        FixedText       m_aProgressFT;
        Window          m_aProgrParent;
        FixedText       m_aFileListFT;
        RecovDocList    m_aFileListLB;
        FixedLine       m_aBottomFL;
        PushButton      m_aPrevBtn;
        PushButton      m_aNextBtn;
        CancelButton    m_aCancelBtn;

        PushButton*     m_pDefButton;
        RecoveryCore*   m_pCore;
        css::uno::Reference< css::task::XStatusIndicator > m_xProgress;

    //-------------------------------------------
    // member
    public:

        //---------------------------------------
        /** @short TODO */
        RecoveryDialog(Window*       pParent,
                       RecoveryCore* pCore  );

        //---------------------------------------
        /** @short TODO */
        virtual ~RecoveryDialog();

        //---------------------------------------
        // IRecoveryUpdateListener
        virtual void updateItems();
        virtual void stepNext(TURLInfo* pItem);
        virtual void start();
        virtual void end();

        //---------------------------------------
        /** @short TODO */
        virtual short execute();

        //---------------------------------------
        /** @short  TODO*/
        virtual void    setDefButton();

    //-------------------------------------------
    // helper
    private:

        //---------------------------------------
        /** @short TODO */
        DECL_LINK(NextButtonHdl, void*);
        DECL_LINK(CancelButtonHdl, void*);

        //---------------------------------------
        /** @short TODO */
        void impl_refreshDocList();
};

//===============================================
class BrokenRecoveryDialog : public ModalDialog
{
    //-------------------------------------------
    // member
    private:
        FixedText       m_aDescrFT;
        FixedText       m_aFileListFT;
        ListBox         m_aFileListLB;
        PushButton      m_aSaveBtn;
        FixedLine       m_aBottomFL;
        OKButton        m_aOkBtn;
        CancelButton    m_aCancelBtn;

        ::rtl::OUString m_sSavePath;
        RecoveryCore*   m_pCore;

    //-------------------------------------------
    // interface
    public:

        //---------------------------------------
        /** @short TODO */
        BrokenRecoveryDialog(Window*       pParent,
                             RecoveryCore* pCore  );

        //---------------------------------------
        /** @short TODO */
        virtual ~BrokenRecoveryDialog();

        //---------------------------------------
        /** @short TODO */
        virtual void refresh();

    //-------------------------------------------
    // helper
    private:

        //---------------------------------------
        /** @short TODO */
        DECL_LINK(SaveButtonHdl, void*);

        //---------------------------------------
        /** @short TODO */
        DECL_LINK(OkButtonHdl, void*);
};



        class ErrorRepWelcomeDialog : public IExtendedTabPage
        {
        private:
            Window              maTitleWin;
            FixedText           maTitleFT;
            FixedText           maDescrFT;

            FixedLine           maBottomFL;
            PushButton          maPrevBtn;
            OKButton            maNextBtn;
            CancelButton        maCancelBtn;

            DECL_LINK(          PrevBtnHdl, void* );
            DECL_LINK(          NextBtnHdl, void* );
            DECL_LINK(          CancelBtnHdl, void* );
        public:
                                ErrorRepWelcomeDialog( Window* _pParent, sal_Bool _bAllowBack = sal_True );
            virtual             ~ErrorRepWelcomeDialog();
        /** @short  TODO*/
        virtual short execute();

        //---------------------------------------
        /** @short  TODO*/
        virtual void    setDefButton();
        };

        struct ErrorRepParams
        {
            ErrorRepParams()
#ifdef WNT
                : miHTTPConnectionType( 0 )
#else
                : miHTTPConnectionType( 1 )
#endif
                , mbAllowContact( false )
            {}

            String              maHTTPProxyServer;
            String              maHTTPProxyPort;
            int                 miHTTPConnectionType;
            bool                mbAllowContact;
            String              maReturnAddress;
            String              maSubject;
            String              maBody;
        };

        class ErrorRepSendDialog : public IExtendedTabPage
        {
        private:
            Window              maTitleWin;
            FixedText           maTitleFT;
            FixedText           maDescrFT;

            FixedText           maDocTypeFT;
            Edit                maDocTypeED;
            FixedText           maUsingFT;
            MultiLineEdit       maUsingML;
            PushButton          maShowRepBtn;
            PushButton          maOptBtn;
            CheckBox            maContactCB;
            FixedText           maEMailAddrFT;
            Edit                maEMailAddrED;

            FixedLine           maBottomFL;
            PushButton          maPrevBtn;
            OKButton            maNextBtn;
            CancelButton        maCancelBtn;

            ErrorRepParams      maParams;

            DECL_LINK(          ShowRepBtnHdl, void* );
            DECL_LINK(          OptBtnHdl, void* );
            DECL_LINK(          ContactCBHdl, void* );
            DECL_LINK(          PrevBtnHdl, void* );
            DECL_LINK(          SendBtnHdl, void* );
            DECL_LINK(          CancelBtnHdl, void* );

            void                initControls();

        public:
                                ErrorRepSendDialog( Window* _pParent );
            virtual             ~ErrorRepSendDialog();

            String              GetDocType( void ) const;
            String              GetUsing( void ) const;
            bool                IsContactAllowed( void ) const;
            String              GetEMailAddress( void ) const;

            bool                ReadParams();
            bool                SaveParams();
            bool                SendReport();

            /** @short  TODO*/
            virtual short execute();

            //---------------------------------------
            /** @short  TODO*/
            virtual void    setDefButton();
        };

        class ErrorRepOptionsDialog : public ModalDialog
        {
        private:
            FixedLine           maProxyFL;
            RadioButton         maSystemBtn;
            RadioButton         maDirectBtn;
            RadioButton         maManualBtn;
            FixedText           maProxyServerFT;
            Edit                maProxyServerEd;
            Edit                maProxyPortEd;
            FixedText           maProxyPortFT;
            FixedText           maDescriptionFT;
            FixedLine           maButtonsFL;
            OKButton            maOKBtn;
            CancelButton        maCancelBtn;

            ErrorRepParams&     mrParams;

            DECL_LINK(          CancelBtnHdl, void* );
            DECL_LINK(          OKBtnHdl, void * );
            DECL_LINK(          ManualBtnHdl, void * );

        public:
                                ErrorRepOptionsDialog( Window* _pParent, ErrorRepParams& rParams );
            virtual             ~ErrorRepOptionsDialog();
        };

        class ErrorRepEdit : public MultiLineEdit
        {
        public:
                        ErrorRepEdit( Window* pParent, const ResId& rResId );
            virtual     ~ErrorRepEdit();
        };

        class ErrorRepPreviewDialog : public ModalDialog
        {
        private:
            ErrorRepEdit        maContentML;
            OKButton            maOKBtn;

        public:
                                ErrorRepPreviewDialog( Window* _pParent );
            virtual             ~ErrorRepPreviewDialog();
        };
    }   // namespace DocRecovery
}   // namespace svx

#undef css

#endif
