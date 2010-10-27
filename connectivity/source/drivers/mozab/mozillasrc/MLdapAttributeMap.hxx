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

#ifndef CONNECTIVITY_MLDAPATTRIBUTEMAP_HXX
#define CONNECTIVITY_MLDAPATTRIBUTEMAP_HXX

#include <MNSInclude.hxx>

#include <memory>

//........................................................................
namespace connectivity { namespace mozab {
//........................................................................

    struct AttributeMap_Data;
    class MQueryHelperResultEntry;

    //====================================================================
    //= class MLdapAttributeMap
    //====================================================================
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

//........................................................................
} } // namespace connectivity::mozab
//........................................................................

#endif // CONNECTIVITY_MLDAPATTRIBUTEMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
