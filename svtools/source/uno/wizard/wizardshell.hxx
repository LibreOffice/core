/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SVT_UNO_WIZARD_SHELL
#define SVT_UNO_WIZARD_SHELL

/** === begin UNO includes === **/
#include <com/sun/star/ui/dialogs/XWizardController.hpp>
#include <com/sun/star/ui/dialogs/XWizard.hpp>
/** === end UNO includes === **/

#include <svtools/roadmapwizard.hxx>

//......................................................................................................................
namespace svt { namespace uno
{
//......................................................................................................................

    //==================================================================================================================
    //= WizardShell
    //==================================================================================================================
    typedef ::svt::RoadmapWizard    WizardShell_Base;
    class WizardShell : public WizardShell_Base
    {
    public:
        WizardShell(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XWizard >& i_rWizard,
            const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XWizardController >& i_rController,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< sal_Int16 > >& i_rPaths
        );
        virtual ~WizardShell();

        // Dialog overridables
        virtual short   Execute();

        // OWizardMachine overridables
        virtual TabPage*    createPage( WizardState i_nState );
        virtual void        enterState( WizardState i_nState );
        virtual sal_Bool    prepareLeaveCurrentState( CommitPageReason _eReason );
        virtual sal_Bool    leaveState( WizardState i_nState );
        virtual String      getStateDisplayName( WizardState i_nState ) const;
        virtual bool        canAdvance() const;
        virtual sal_Bool    onFinish( sal_Int32 _nResult );

        // attribute access
        const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XWizard >&
            getWizard() const { return m_xWizard; }

        static sal_Int16 convertCommitReasonToTravelType( const CommitPageReason i_eReason );

        // operations
        sal_Bool    advanceTo( const sal_Int16 i_nPageId )
        {
            return skipUntil( impl_pageIdToState( i_nPageId ) );
        }
        sal_Bool    goBackTo( const sal_Int16 i_nPageId )
        {
            return skipBackwardUntil( impl_pageIdToState( i_nPageId ) );
        }
        sal_Bool    travelNext()        { return WizardShell_Base::travelNext(); }
        sal_Bool    travelPrevious()    { return WizardShell_Base::travelPrevious(); }

        void        preExecute()
        {
            // activate the first page
            ActivatePage();
        }

        ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XWizardPage >
                    getCurrentWizardPage() const;

    private:
        sal_Int16   impl_stateToPageId( const WizardTypes::WizardState i_nState ) const
        {
            return static_cast< sal_Int16 >( i_nState + m_nFirstPageID );
        }

        WizardState impl_pageIdToState( const sal_Int16 i_nPageId ) const
        {
            return static_cast< WizardState >( i_nPageId - m_nFirstPageID );
        }

        // prevent outside access to some base class members
        using WizardShell_Base::skip;
        using WizardShell_Base::skipUntil;
        using WizardShell_Base::skipBackwardUntil;
        using WizardShell_Base::getCurrentState;

    private:
        const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XWizard >            m_xWizard;
        const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XWizardController >  m_xController;
        const sal_Int16                                                                             m_nFirstPageID;
    };

//......................................................................................................................
} } // namespace svt::uno
//......................................................................................................................

#endif // SVT_UNO_WIZARD_SHELL
