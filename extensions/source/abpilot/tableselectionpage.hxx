/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tableselectionpage.hxx,v $
 * $Revision: 1.6 $
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

#ifndef EXTENSIONS_ABP_TABLESELECTIONPAGE_HXX
#define EXTENSIONS_ABP_TABLESELECTIONPAGE_HXX

#include "abspage.hxx"
#include <vcl/lstbox.hxx>

//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= TableSelectionPage
    //=====================================================================
    class TableSelectionPage : public AddressBookSourcePage
    {
    protected:
        FixedText       m_aLabel;
        ListBox         m_aTableList;

    public:
        TableSelectionPage( OAddessBookSourcePilot* _pParent );

    protected:
        // OWizardPage overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage( CommitPageReason _eReason );

        // TabDialog overridables
        virtual void        ActivatePage();
        virtual void        DeactivatePage();

        // OImportPage overridables
        virtual bool        canAdvance() const;

    private:
        DECL_LINK( OnTableSelected, void* );
        DECL_LINK( OnTableDoubleClicked, void* );
    };

//.........................................................................
}   // namespace abp
//.........................................................................

#endif // EXTENSIONS_ABP_TABLESELECTIONPAGE_HXX

