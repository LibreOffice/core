/*************************************************************************
 *
 *  $RCSfile: general.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-11-13 11:41:26 $
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

#ifndef _BIB_GENERAL_HXX
#define _BIB_GENERAL_HXX

#ifndef _COM_SUN_STAR_AWT_XFOCUSLISTENER_HPP_
#include <com/sun/star/awt/XFocusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
//#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
//#include <com/sun/star/lang/XMultiServiceFactory.hpp>
//#endif
#ifndef _COM_SUN_STAR_FORM_XBOUNDCOMPONENT_HPP_
#include <com/sun/star/form/XBoundComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSETLISTENER_HPP_
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#endif

#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _SVEDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif

#ifndef _SV_TABPAGE_HXX //autogen
#include <vcl/tabpage.hxx>
#endif

#ifndef _SV_COMBOBOX_HXX //
#include <vcl/combobox.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // helper for implementations
#endif


class BibDataManager;
#define TYPE_COUNT 22
#define FIELD_COUNT 31

typedef cppu::WeakAggImplHelper1
<
    ::com::sun::star::awt::XFocusListener
> BibGeneralPageBaseClass;

class BibGeneralPage:
                public BibGeneralPageBaseClass,
                public TabPage
{
    Window          aControlParentWin;
    FixedText       aIdentifierFT;
    FixedText       aAuthTypeFT;

    FixedText       aAuthorFT;
    FixedText       aMonthFT;
    FixedText       aYearFT;

    FixedText       aPublisherFT;
    FixedText       aISBNFT;

    FixedText       aAddressFT;
    FixedText       aAnnoteFT;
    FixedText       aBooktitleFT;
    FixedText       aChapterFT;
    FixedText       aEditionFT;
    FixedText       aEditorFT;
    FixedText       aHowpublishedFT;
    FixedText       aInstitutionFT;
    FixedText       aJournalFT;
    FixedText       aNoteFT;
    FixedText       aNumberFT;
    FixedText       aOrganizationsFT;
    FixedText       aPagesFT;
    FixedText       aSchoolFT;
    FixedText       aSeriesFT;
    FixedText       aTitleFT;
    FixedText       aReportTypeFT;
    FixedText       aVolumeFT;
    FixedText       aURLFT;
    FixedText       aCustom1FT;
    FixedText       aCustom2FT;
    FixedText       aCustom3FT;
    FixedText       aCustom4FT;
    FixedText       aCustom5FT;

     ScrollBar      aHoriScroll;
     ScrollBar      aVertScroll;

     FixedText*     aFixedTexts[FIELD_COUNT];
     ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>      aControls[FIELD_COUNT];

     Size           aStdSize;
     Point          aBasePos;

    String          aBibTypeArr[TYPE_COUNT];
    String          sErrorPrefix;
    String          sTableErrorString;

    String          sTypeColumnName;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >    xCtrlContnr;
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >         xFormCtrl;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xMgr;

    ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundComponent >         xCurrentBoundComponent;

    ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundComponent >         xLBModel;

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSetListener >         xPosListener;


    BibDataManager*         pDatMan;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >        AddXControl(const String& rName, Point aPos, Size aSize, sal_uInt16 nHelpId);

    void                    AddControlWithError(const rtl::OUString& rColumnName,
                                    const Point& rPos,  const Size& rSize, String& rErrorString,
                                    String aColumnUIName, sal_uInt16 nHelpId);

    void                    AdjustScrollbars();
     DECL_LINK(ScrollHdl, ScrollBar*);

protected:
     virtual void       Resize();
public:
        BibGeneralPage(Window* pParent, BibDataManager* pDatMan);
        virtual ~BibGeneralPage();

        const String& GetErrorString() const {return sTableErrorString;}

        ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundComponent >         GetTypeListBoxModel() {return xLBModel;}
        BibDataManager*         GetDataManager() {return pDatMan;}

        void                    CommitActiveControl();

         virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( com::sun::star::uno::RuntimeException );

         void SAL_CALL focusGained(const ::com::sun::star::awt::FocusEvent& e) throw( com::sun::star::uno::RuntimeException );
         void SAL_CALL focusLost(const ::com::sun::star::awt::FocusEvent& e) throw( com::sun::star::uno::RuntimeException );

         void RemoveListeners();
};


#endif


