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

#ifndef INCLUDED_XMLOFF_XMLCONSTANTSPROPERTYHANDLER_HXX
#define INCLUDED_XMLOFF_XMLCONSTANTSPROPERTYHANDLER_HXX

#include <config_options.h>
#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <xmloff/xmlprhdl.hxx>
#include <xmloff/xmltoken.hxx>

template<typename EnumT>
struct SvXMLEnumMapEntry;


/** Abstract base-class for different XML-types. Derivations of this class
    knows how to compare, im/export a special XML-type
*/
class UNLESS_MERGELIBS_MORE(XMLOFF_DLLPUBLIC) XMLConstantsPropertyHandler: public XMLPropertyHandler
{
    const SvXMLEnumMapEntry<sal_uInt16> *m_pMap;
    const enum ::xmloff::token::XMLTokenEnum m_eDefault;

public:
    template<typename EnumT>
    XMLConstantsPropertyHandler( const SvXMLEnumMapEntry<EnumT> *pM,
                                 enum ::xmloff::token::XMLTokenEnum eDflt)
     : m_pMap(reinterpret_cast<const SvXMLEnumMapEntry<sal_uInt16>*>(pM)), m_eDefault(eDflt) {}

    // Just needed for virtual destruction
    virtual ~XMLConstantsPropertyHandler() override;

    /// Imports the given value in case of the given XML-data-type
    virtual bool importXML(
            const OUString& rStrImpValue,
            css::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const override;

    /// Exports the given value in case of the given XML-data-type
    virtual bool exportXML(
            OUString& rStrExpValue,
            const css::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const override;
};

#endif // INCLUDED_XMLOFF_XMLCONSTANTSPROPERTYHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
