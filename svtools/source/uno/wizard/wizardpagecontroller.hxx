/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SVT_UNOWIZ_WIZARDPAGECONTROLLER_HXX
#define SVT_UNOWIZ_WIZARDPAGECONTROLLER_HXX

#include "svtools/wizardmachine.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/ui/dialogs/XWizardController.hpp>
/** === end UNO includes === **/

//......................................................................................................................
namespace svt { namespace uno
{
//......................................................................................................................

    class WizardShell;

    //==================================================================================================================
    //= WizardPageController
    //==================================================================================================================
    class WizardPageController : public IWizardPageController
    {
    public:
        WizardPageController(
            WizardShell& i_rParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XWizardController >& i_rController,
            const sal_Int16 i_nPageId
        );
        ~WizardPageController();

        // IWizardPageController overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage( WizardTypes::CommitPageReason _eReason );
        virtual bool        canAdvance() const;

        const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XWizardPage >&
                            getWizardPage() const { return m_xWizardPage; }
        TabPage*            getTabPage() const;

    private:
        const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XWizardController >  m_xController;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XWizardPage >              m_xWizardPage;
        const sal_Int16                                                                             m_nPageId;
    };

//......................................................................................................................
} } // namespace svt::uno
//......................................................................................................................

#endif // SVT_UNOWIZ_WIZARDPAGECONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
