/*************************************************************************
 *
 *  $RCSfile: gridwizard.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-05 14:52:33 $
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

#ifndef _EXTENSIONS_DBP_GRIDWIZARD_HXX_
#define _EXTENSIONS_DBP_GRIDWIZARD_HXX_

#ifndef _EXTENSIONS_DBP_CONTROLWIZARD_HXX
#include "controlwizard.hxx"
#endif
#ifndef _EXTENSIONS_DBP_COMMONPAGESDBP_HXX_
#include "commonpagesdbp.hxx"
#endif

//.........................................................................
namespace dbp
{
//.........................................................................

    //=====================================================================
    //= OGridSettings
    //=====================================================================
    struct OGridSettings : public OControlWizardSettings
    {
        ::com::sun::star::uno::Sequence< ::rtl::OUString >      aSelectedFields;
    };

    //=====================================================================
    //= OGridWizard
    //=====================================================================
    class OGridWizard : public OControlWizard
    {
    protected:
        OGridSettings   m_aSettings;
        sal_Bool        m_bHadDataSelection : 1;

    public:
        OGridWizard(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObjectModel,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

        OGridSettings& getSettings() { return m_aSettings; }

    protected:
        // OWizardMachine overridables
        virtual ::svt::OWizardPage*     createPage(sal_uInt16 _nState);
        virtual sal_uInt16              determineNextState(sal_uInt16 _nCurrentState);
        virtual void                    enterState(sal_uInt16 _nState);
        virtual sal_Bool                leaveState(sal_uInt16 _nState);

        virtual sal_Bool onFinish(sal_Int32 _nResult);

        virtual sal_Bool approveControl(sal_Int16 _nClassId);

    protected:
        void implApplySettings();
    };

    //=====================================================================
    //= OGridPage
    //=====================================================================
    class OGridPage : public OControlWizardPage
    {
    public:
        OGridPage( OGridWizard* _pParent, const ResId& _rId ) : OControlWizardPage(_pParent, _rId) { }

    protected:
        OGridSettings& getSettings() { return static_cast<OGridWizard*>(getDialog())->getSettings(); }
    };

    //=====================================================================
    //= OGridFieldsSelection
    //=====================================================================
    class OGridFieldsSelection : public OGridPage
    {
    protected:
        FixedLine       m_aFrame;
        FixedText       m_aExistFieldsLabel;
        ListBox         m_aExistFields;
        PushButton      m_aSelectOne;
        PushButton      m_aSelectAll;
        PushButton      m_aDeselectOne;
        PushButton      m_aDeselectAll;
        FixedText       m_aSelFieldsLabel;
        ListBox         m_aSelFields;

    public:
        OGridFieldsSelection( OGridWizard* _pParent );

    protected:
        // TabPage overridables
        virtual void ActivatePage();

        // OWizardPage overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage(COMMIT_REASON _eReason);
        sal_Bool            determineNextButtonState();

    protected:
        DECL_LINK(OnMoveOneEntry, PushButton*);
        DECL_LINK(OnMoveAllEntries, PushButton*);
        DECL_LINK(OnEntrySelected, ListBox*);
        DECL_LINK(OnEntryDoubleClicked, ListBox*);

        void implCheckButtons();
        void implApplySettings();

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > implGetColumns(sal_Bool _bShowError = sal_True);
    };

//.........................................................................
}   // namespace dbp
//.........................................................................

#endif // _EXTENSIONS_DBP_GRIDWIZARD_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 05.03.01 08:28:20  fs
 ************************************************************************/

