/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef CONNECTIVITY_MLDAPATTRIBUTEMAP_HXX
#define CONNECTIVITY_MLDAPATTRIBUTEMAP_HXX

#include <MNSInclude.hxx>

#include <memory>


namespace connectivity { namespace mozab {


    struct AttributeMap_Data;
    class MQueryHelperResultEntry;


    //= class MLdapAttributeMap

    /** implements the nsIAbLDAPAttributeMap interface

        Somewhere between Mozilla 1.7.5 and SeaMonkey 1.1.12, the LDAP address book
        implementation was changed to take the attribute mapping (from LDAP attributes
        to address book properties) not directly from the preferences. Instead, this mapping
        is now delivered by a dedicated implementation (supporting the nsIAbLDAPAttributeMap
        interface).

    */
    class MLdapAttributeMap : public nsIAbLDAPAttributeMap
    {
    public:
        MLdapAttributeMap();

        NS_DECL_ISUPPORTS
        NS_DECL_NSIABLDAPATTRIBUTEMAP

        static void fillCardFromResult( nsIAbCard& _card, const MQueryHelperResultEntry& _result );
        static void fillResultFromCard( MQueryHelperResultEntry& _result, nsIAbCard& _card );

    protected:
        virtual ~MLdapAttributeMap();

    private:
        ::std::auto_ptr< AttributeMap_Data >    m_pData;
    };


} } // namespace connectivity::mozab


#endif // CONNECTIVITY_MLDAPATTRIBUTEMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
