/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: abspage.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 12:51:44 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef EXTENSIONS_ABP_ABSPAGE_HXX
#include "abspage.hxx"
#endif
#ifndef EXTENSIONS_ABSPILOT_HXX
#include "abspilot.hxx"
#endif

//.........................................................................
namespace abp
{
//.........................................................................

    using namespace ::svt;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    //=====================================================================
    //= AddressBookSourcePage
    //=====================================================================
    AddressBookSourcePage::AddressBookSourcePage( OAddessBookSourcePilot* _pParent, const ResId& _rId )
        :AddressBookSourcePage_Base(_pParent, _rId)
    {
    }

    //---------------------------------------------------------------------
    void AddressBookSourcePage::DeactivatePage()
    {
        AddressBookSourcePage_Base::DeactivatePage();
        getDialog()->enableButtons(WZB_NEXT, sal_True);
    }

    //---------------------------------------------------------------------
    OAddessBookSourcePilot* AddressBookSourcePage::getDialog()
    {
        return static_cast<OAddessBookSourcePilot*>(GetParent());
    }

    //---------------------------------------------------------------------
    const OAddessBookSourcePilot* AddressBookSourcePage::getDialog() const
    {
        return static_cast<const OAddessBookSourcePilot*>(GetParent());
    }

    //---------------------------------------------------------------------
    AddressSettings& AddressBookSourcePage::getSettings()
    {
        return getDialog()->getSettings();
    }

    //---------------------------------------------------------------------
    const AddressSettings&  AddressBookSourcePage::getSettings() const
    {
        return getDialog()->getSettings();
    }

    //---------------------------------------------------------------------
    Reference< XMultiServiceFactory > AddressBookSourcePage::getORB()
    {
        return getDialog()->getORB();
    }

//.........................................................................
}   // namespace abp
//.........................................................................

