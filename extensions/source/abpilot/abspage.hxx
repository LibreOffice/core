/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: abspage.hxx,v $
 * $Revision: 1.5 $
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

#ifndef EXTENSIONS_ABP_ABSPAGE_HXX
#define EXTENSIONS_ABP_ABSPAGE_HXX

#include <svtools/wizardmachine.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#ifndef EXTENSIONS_ABPRESID_HRC
#include "abpresid.hrc"
#endif
#include "componentmodule.hxx"
#include <vcl/fixed.hxx>

//.........................................................................
namespace abp
{
//.........................................................................

    class OAddessBookSourcePilot;
    struct AddressSettings;

    //=====================================================================
    //= AddressBookSourcePage
    //=====================================================================
    typedef ::svt::OWizardPage AddressBookSourcePage_Base;
    /// the base class for all tab pages in the address book source wizard
    class AddressBookSourcePage : public AddressBookSourcePage_Base
    {
    protected:
        AddressBookSourcePage( OAddessBookSourcePilot* _pParent, const ResId& _rId );

    protected:
        // helper
        OAddessBookSourcePilot* getDialog();
        const OAddessBookSourcePilot*   getDialog() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                getORB();
        AddressSettings&        getSettings();
        const AddressSettings&  getSettings() const;

        // TabDialog overridables
        virtual void        DeactivatePage();
    };

//.........................................................................
}   // namespace abp
//.........................................................................

#endif // EXTENSIONS_ABP_ABSPAGE_HXX

