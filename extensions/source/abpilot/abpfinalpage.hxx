/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: abpfinalpage.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:35:15 $
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

