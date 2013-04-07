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

#ifndef _XMLOFF_IMAGESTYLE_HXX
#define _XMLOFF_IMAGESTYLE_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

class SvXMLNamespaceMap;
class SvXMLAttributeList;
class SvXMLUnitConverter;
class SvXMLExport;
class SvXMLImport;

class XMLOFF_DLLPUBLIC XMLImageStyle
{
public:
    XMLImageStyle();
    ~XMLImageStyle();

    sal_Bool exportXML( const OUString& rStrName, const ::com::sun::star::uno::Any& rValue, SvXMLExport& rExport );
    sal_Bool importXML( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList, ::com::sun::star::uno::Any& rValue, OUString& rStrName, SvXMLImport& rImport );

private:

    SAL_DLLPRIVATE sal_Bool ImpExportXML( const OUString& rStrName, const ::com::sun::star::uno::Any& rValue,
                           SvXMLExport& rExport );
    SAL_DLLPRIVATE sal_Bool ImpImportXML( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
                           ::com::sun::star::uno::Any& rValue, OUString& rStrName,
                           SvXMLImport& rImport );
};

#endif // _XMLOFF_IMAGESTYLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
