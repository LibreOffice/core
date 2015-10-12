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

#ifndef INCLUDED_XMLOFF_SOURCE_STYLE_PAGEMASTERIMPORTPROPMAPPER_HXX
#define INCLUDED_XMLOFF_SOURCE_STYLE_PAGEMASTERIMPORTPROPMAPPER_HXX

#include <xmloff/xmlimppr.hxx>

class SvXMLImport;

class PageMasterImportPropertyMapper : public SvXMLImportPropertyMapper
{
    SvXMLImport& rImport;

protected:

public:

    PageMasterImportPropertyMapper(
            const rtl::Reference< XMLPropertySetMapper >& rMapper,
            SvXMLImport& rImp);
    virtual ~PageMasterImportPropertyMapper();

    /** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_IMPORT flag set */
    virtual bool handleSpecialItem(
            XMLPropertyState& rProperty,
            ::std::vector< XMLPropertyState >& rProperties,
            const OUString& rValue,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap ) const override;

    /** This method is called when all attributes have been processed. It may be used to remove items that are incomplete */
    virtual void finished(
            ::std::vector< XMLPropertyState >& rProperties,
            sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
