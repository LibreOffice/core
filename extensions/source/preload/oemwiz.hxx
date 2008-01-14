/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: oemwiz.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:55:19 $
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

#ifndef _EXTENSIONS_PRELOAD_OEMWIZ_HXX_
#define _EXTENSIONS_PRELOAD_OEMWIZ_HXX_

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _SVT_WIZDLG_HXX
#include <svtools/wizdlg.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SVEDIT_HXX
#include <svtools/svmedit.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif

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
        BOOL            mbEndReached;
        Link            maEndReachedHdl;
        Link            maScrolledHdl;

    public:
                        LicenceView( Window* pParent, const ResId& rResId );
                       ~LicenceView();

        void            ScrollDown( ScrollType eScroll );

        BOOL            IsEndReached() const;
        BOOL            EndReached() const { return mbEndReached; }
        void            SetEndReached( BOOL bEnd ) { mbEndReached = bEnd; }

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
        BOOL            bEndReached;

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

