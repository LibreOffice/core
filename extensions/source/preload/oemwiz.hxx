/*************************************************************************
 *
 *  $RCSfile: oemwiz.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: os $ $Date: 2001-11-14 13:18:01 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

//.........................................................................
namespace preload
{
    #define OEM_WELCOME     0
    #define OEM_LICENSE     1
    #define OEM_USERDATA    2
    #define OEM_README      3

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

        static sal_Bool LoadFromLocalFile(const String& rFileName, String& rContent);
    };
    class OEMWelcomeTabPage : public TabPage
    {
        FixedText   aInfoFT;
        public:
            OEMWelcomeTabPage(Window* pParent);
            ~OEMWelcomeTabPage();
    };
    class OEMLicenseTabPage : public TabPage
    {
        FixedText       aInfo1FT;
        MultiLineEdit   aLicenseML;
        FixedText       aInfo2FT;
        public:
            OEMLicenseTabPage(Window* pParent);
            ~OEMLicenseTabPage();

            virtual void ActivatePage();
    };
    class OEMReadMeTabPage : public TabPage
    {
        FixedText       aInfoFT;
        MultiLineEdit   aReadmeML;

        public:
            OEMReadMeTabPage(Window* pParent);
            ~OEMReadMeTabPage();

            virtual void ActivatePage();
    };

//.........................................................................
}   // namespace preload
//.........................................................................

#endif // _EXTENSIONS_PRELOAD_OEMWIZ_HXX_

