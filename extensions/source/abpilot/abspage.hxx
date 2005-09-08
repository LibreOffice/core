/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: abspage.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:03:49 $
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

#ifndef EXTENSIONS_ABP_ABSPAGE_HXX
#define EXTENSIONS_ABP_ABSPAGE_HXX

#ifndef _SVTOOLS_WIZARDMACHINE_HXX_
#include <svtools/wizardmachine.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef EXTENSIONS_ABPRESID_HRC
#include "abpresid.hrc"
#endif
#ifndef _EXTENSIONS_COMPONENT_MODULE_HXX_
#include "componentmodule.hxx"
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

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

