/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mmlayoutpage.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:06:27 $
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
#ifndef _MAILMERGELAYOUTPAGE_HXX
#define _MAILMERGELAYOUTPAGE_HXX

#ifndef _SVTOOLS_WIZARDMACHINE_HXX_
#include <svtools/wizardmachine.hxx>
#endif
#ifndef _MAILMERGEHELPER_HXX
#include <mailmergehelper.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

class SwMailMergeWizard;
class SwFrmFmt;
class SwOneExampleFrame;
class SwWrtShell;
class SwView;

namespace com{ namespace sun{ namespace star{ namespace beans{ class XPropertySet;}}}}
/*-- 02.04.2004 09:21:06---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwMailMergeLayoutPage : public svt::OWizardPage
{
    SwBoldFixedInfo     m_aHeaderFI;

    FixedLine           m_aPositionFL;

    CheckBox            m_aAlignToBodyCB;
    FixedText           m_aLeftFT;
    MetricField         m_aLeftMF;
    FixedText           m_aTopFT;
    MetricField         m_aTopMF;

    FixedLine           m_aGreetingLineFL;
    FixedText           m_aUpFT;
    PushButton          m_aUpPB;
    FixedText           m_aDownFT;
    PushButton          m_aDownPB;

    Window              m_aExampleContainerWIN;
    Window              m_aExampleWIN;

    FixedText           m_aZoomFT;
    ListBox             m_aZoomLB;

    SwOneExampleFrame*  m_pExampleFrame;
    SwWrtShell*         m_pExampleWrtShell;

    String              m_sExampleURL;
    SwFrmFmt*           m_pAddressBlockFormat;

    bool                m_bIsGreetingInserted;

    SwMailMergeWizard*  m_pWizard;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  m_xViewProperties;

    DECL_LINK(PreviewLoadedHdl_Impl, void*);
    DECL_LINK(ZoomHdl_Impl, ListBox*);
    DECL_LINK(ChangeAddressHdl_Impl, MetricField*);
    DECL_LINK(GreetingsHdl_Impl, PushButton*);
    DECL_LINK(AlignToTextHdl_Impl, CheckBox*);

    static SwFrmFmt*        InsertAddressFrame(
                            SwWrtShell& rShell,
                            SwMailMergeConfigItem& rConfigItem,
                            const Point& rDestination,
                            bool bAlignToBody,
                            bool bExample);
    static void             InsertGreeting(SwWrtShell& rShell, SwMailMergeConfigItem& rConfigItem, bool bExample);

    virtual void        ActivatePage();
    virtual sal_Bool    commitPage(CommitPageReason _eReason);
public:
        SwMailMergeLayoutPage( SwMailMergeWizard* _pParent);
        ~SwMailMergeLayoutPage();

    static SwFrmFmt*        InsertAddressAndGreeting(SwView* pView,
                                            SwMailMergeConfigItem& rConfigItem,
                                            const Point& rAddressPos,
                                            bool bAlignToBody);
};

#endif


