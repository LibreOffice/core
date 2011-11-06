/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

