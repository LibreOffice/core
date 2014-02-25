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
#ifndef SC_XMLTABI_HXX
#define SC_XMLTABI_HXX

#include "externalrefmgr.hxx"

#include <xmloff/xmlictxt.hxx>
#include <memory>

class ScXMLImport;

struct ScXMLExternalTabData
{
    OUString maFileUrl;
    ScExternalRefCache::TableTypeRef mpCacheTable;
    sal_Int32 mnRow;
    sal_Int32 mnCol;
    sal_uInt16 mnFileId;

    ScXMLExternalTabData();
};

class ScXMLTableContext : public SvXMLImportContext
{
    OUString   sPrintRanges;
    ::std::auto_ptr<ScXMLExternalTabData> pExternalRefInfo;
    sal_Int32       nStartOffset;
    bool            bStartFormPage;
    bool            bPrintEntireSheet;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLTableContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual ~ScXMLTableContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};



class ScXMLTableProtectionContext : public SvXMLImportContext
{
    ScXMLImport& GetScImport();

public:
    ScXMLTableProtectionContext( ScXMLImport& rImport, sal_uInt16 nPrefix,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                 ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual ~ScXMLTableProtectionContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
