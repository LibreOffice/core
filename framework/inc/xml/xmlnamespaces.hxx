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

#pragma once
#if 1

#include <com/sun/star/xml/sax/SAXException.hpp>

#include <map>
#include <framework/fwedllapi.h>

namespace framework
{

class FWE_DLLPUBLIC XMLNamespaces
{
    public:
        XMLNamespaces();
        XMLNamespaces( const XMLNamespaces& );
        virtual ~XMLNamespaces();

        void addNamespace( const ::rtl::OUString& aName, const ::rtl::OUString& aValue )
            throw(  ::com::sun::star::xml::sax::SAXException );

        ::rtl::OUString applyNSToAttributeName( const ::rtl::OUString& ) const
            throw(  ::com::sun::star::xml::sax::SAXException );
        ::rtl::OUString applyNSToElementName( const ::rtl::OUString& ) const
            throw(  ::com::sun::star::xml::sax::SAXException );

    private:
        typedef ::std::map< ::rtl::OUString, ::rtl::OUString > NamespaceMap;

        ::rtl::OUString getNamespaceValue( const ::rtl::OUString& aNamespace ) const
            throw( ::com::sun::star::xml::sax::SAXException );

        ::rtl::OUString m_aDefaultNamespace;
        ::rtl::OUString m_aXMLAttributeNamespace;
        NamespaceMap    m_aNamespaceMap;
};

}

#endif // __FRAMEWORK_XML_XMLNAMESPACES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
