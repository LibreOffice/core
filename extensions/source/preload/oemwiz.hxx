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



#ifndef _EXTENSIONS_PRELOAD_OEMWIZ_HXX_
#define _EXTENSIONS_PRELOAD_OEMWIZ_HXX_

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <svtools/wizdlg.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/tabpage.hxx>
#include <vcl/fixed.hxx>
#include <svtools/svmedit.hxx>
#include <svl/lstner.hxx>
#include <vcl/scrbar.hxx>

//.........................................................................
namespace preload
{
    #define OEM_WELCOME     0
    #define OEM_LICENSE     1
    #define OEM_USERDATA    2

//.........................................................................
    //=====================================================================
    //= OEMPreloadDialog
    //=====================================================================
    struct OEMPreloadDialog_Impl;
    class OEMPreloadDialog : public WizardDialog
    {
        PushButton      aPrevPB;
        PushButton      aNextPB;
        CancelButton    aCancelPB;

        String          aNextST;
        String          aAcceptST;
        String          aFinishST;
        String          aDlgTitle;
        String          aLicense;
        String          aUserData;
        OEMPreloadDialog_Impl* pImpl;

        DECL_LINK(NextPrevPageHdl, PushButton*);
    protected:

    public:
        OEMPreloadDialog(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObjectModel,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );
        ~OEMPreloadDialog();

        const String&   GetAcceptString()const {return aAcceptST;}
        const String    GetCancelString() const {return aCancelPB.GetText();}
        void            SetCancelString( const String& rText );

        static sal_Bool LoadFromLocalFile(const String& rFileName, String& rContent);
    };
    class OEMWelcomeTabPage : public TabPage
    {
        FixedText   aInfoFT;
        public:
            OEMWelcomeTabPage(Window* pParent);
            ~OEMWelcomeTabPage();
    };
    class LicenceView : public MultiLineEdit, public SfxListener
    {
        sal_Bool            mbEndReached;
        Link            maEndReachedHdl;
        Link            maScrolledHdl;

    public:
                        LicenceView( Window* pParent, const ResId& rResId );
                       ~LicenceView();

        void            ScrollDown( ScrollType eScroll );

        sal_Bool            IsEndReached() const;
        sal_Bool            EndReached() const { return mbEndReached; }
        void            SetEndReached( sal_Bool bEnd ) { mbEndReached = bEnd; }

        void            SetEndReachedHdl( const Link& rHdl )  { maEndReachedHdl = rHdl; }
        const Link&     GetAutocompleteHdl() const { return maEndReachedHdl; }

        void            SetScrolledHdl( const Link& rHdl )  { maScrolledHdl = rHdl; }
        const Link&     GetScrolledHdl() const { return maScrolledHdl; }

        virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    private:
        using MultiLineEdit::Notify;
    };
    class OEMLicenseTabPage : public TabPage
    {
        LicenceView     aLicenseML;
        FixedText       aInfo1FT;
        FixedText       aInfo2FT;
        FixedText       aInfo3FT;
        FixedText       aInfo2_1FT;
        FixedText       aInfo3_1FT;
        CheckBox        aCBAccept;
        PushButton      aPBPageDown;
        FixedImage      aArrow;
        String          aStrAccept;
        String          aStrNotAccept;
        String          aOldCancelText;
        sal_Bool            bEndReached;

        OEMPreloadDialog* pPreloadDialog;

        void                EnableControls();

        DECL_LINK(          AcceptHdl, CheckBox * );
        DECL_LINK(          PageDownHdl, PushButton * );
        DECL_LINK(          EndReachedHdl, LicenceView * );
        DECL_LINK(          ScrolledHdl, LicenceView * );

        public:
            OEMLicenseTabPage(OEMPreloadDialog* pParent);
            ~OEMLicenseTabPage();

            virtual void ActivatePage();
    };

//.........................................................................
}   // namespace preload
//.........................................................................

#endif // _EXTENSIONS_PRELOAD_OEMWIZ_HXX_

