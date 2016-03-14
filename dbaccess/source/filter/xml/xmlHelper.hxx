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
#ifndef INCLUDED_DBACCESS_SOURCE_FILTER_XML_XMLHELPER_HXX
#define INCLUDED_DBACCESS_SOURCE_FILTER_XML_XMLHELPER_HXX

#include <xmloff/xmlprmap.hxx>
#include <xmloff/contextid.hxx>
#include <xmloff/controlpropertyhdl.hxx>

#include <memory>

#define CTF_DB_ROWHEIGHT                            (XML_DB_CTF_START + 1)
#define CTF_DB_ISVISIBLE                            (XML_DB_CTF_START + 2)
#define CTF_DB_MASTERPAGENAME                       (XML_DB_CTF_START + 3)
#define CTF_DB_NUMBERFORMAT                         (XML_DB_CTF_START + 4)
#define CTF_DB_COLUMN_TEXT_ALIGN                    (XML_DB_CTF_START + 5)

#define XML_DB_TYPE_EQUAL                           (XML_DB_TYPES_START + 1)

namespace dbaxml
{
    class OPropertyHandlerFactory : public ::xmloff::OControlPropertyHandlerFactory
    {
    protected:
        mutable ::std::unique_ptr<XMLConstantsPropertyHandler>    m_pDisplayHandler;
    public:
        OPropertyHandlerFactory();
        virtual ~OPropertyHandlerFactory();

        virtual const XMLPropertyHandler* GetPropertyHandler(sal_Int32 _nType) const override;
    };

    class OXMLHelper
    {
    public:
        static rtl::Reference < XMLPropertySetMapper > GetTableStylesPropertySetMapper( bool bForExport );
        static rtl::Reference < XMLPropertySetMapper > GetColumnStylesPropertySetMapper( bool bForExport );
        static rtl::Reference < XMLPropertySetMapper > GetCellStylesPropertySetMapper( bool bForExport );
        static rtl::Reference < XMLPropertySetMapper > GetRowStylesPropertySetMapper();
    };
} // dbaxml
#endif // INCLUDED_DBACCESS_SOURCE_FILTER_XML_XMLHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
