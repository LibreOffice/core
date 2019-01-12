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

#ifndef INCLUDED_XMLOFF_HATCHSTYLE_HXX
#define INCLUDED_XMLOFF_HATCHSTYLE_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <rtl/ustring.hxx>

class SvXMLImport;
class SvXMLExport;
namespace com { namespace sun { namespace star {
    namespace uno { template<class A> class Reference; }
    namespace xml { namespace sax { class XAttributeList; } }
    namespace uno { class Any; }
} } }


class XMLOFF_DLLPUBLIC XMLHatchStyleImport
{
    SvXMLImport& rImport;

public:
    XMLHatchStyleImport( SvXMLImport& rImport );
    ~XMLHatchStyleImport();

    void importXML(
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Any& rValue,
        OUString& rStrName );
};

class XMLOFF_DLLPUBLIC XMLHatchStyleExport
{
    SvXMLExport& rExport;

public:
    XMLHatchStyleExport( SvXMLExport& rExport );
    ~XMLHatchStyleExport();

    void exportXML( const OUString& rStrName, const css::uno::Any& rValue );
};

#endif // _XMLOFF_HATCHELEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
