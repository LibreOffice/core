/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: abpfinalpage.hxx,v $
 * $Revision: 1.7 $
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

#ifndef EXTENSIONS_ABP_ABPFINALPAGE_HXX
#define EXTENSIONS_ABP_ABPFINALPAGE_HXX

#include "abspage.hxx"
#include "abptypes.hxx"

#include <svtools/urlcontrol.hxx>
#include <svx/databaselocationinput.hxx>
#include <vcl/edit.hxx>

//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= FinalPage
    //=====================================================================
    class FinalPage : public AddressBookSourcePage
    {
    protected:
        FixedText       m_aExplanation;
        FixedText       m_aLocationLabel;
        ::svt::OFileURLControl  m_aLocation;
        PushButton      m_aBrowse;
        CheckBox        m_aRegisterName;
        FixedText       m_aNameLabel;
        Edit            m_aName;
        FixedText       m_aDuplicateNameError;

        ::svx::DatabaseLocationInputController
                        m_aLocationController;

        StringBag       m_aInvalidDataSourceNames;

    public:
        FinalPage( OAddessBookSourcePilot* _pParent );

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
        DECL_LINK( OnNameModified, Edit* );
        DECL_LINK( OnRegister, CheckBox* );

        sal_Bool    isValidName() const;
        void        implCheckName();
        void        setFields();
    };

//.........................................................................
}   // namespace abp
//.........................................................................

#endif // EXTENSIONS_ABP_ABPFINALPAGE_HXX

