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

#include <xmloff/xmlictxt.hxx>
#include "rtl/ustring.hxx"
#include <vector>

namespace com { namespace sun { namespace star { namespace xml { namespace sax {
        class XAttributeList;
}}}}}

class SchXMLTextListContext : public SvXMLImportContext
{
public:
    SchXMLTextListContext( SvXMLImport& rImport,
                            const ::rtl::OUString& rLocalName,
                            ::com::sun::star::uno::Sequence< ::rtl::OUString>& rTextList );
    virtual ~SchXMLTextListContext();
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );

private:
    ::com::sun::star::uno::Sequence< ::rtl::OUString>& m_rTextList;
    std::vector< ::rtl::OUString> m_aTextVector;
};

#endif  // _SCH_XMLTEXTLISTCONTEXT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
