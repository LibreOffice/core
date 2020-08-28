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

#ifndef INCLUDED_XMLOFF_INC_XMLNUMBERSTYLESIMPORT_HXX
#define INCLUDED_XMLOFF_INC_XMLNUMBERSTYLESIMPORT_HXX

#include <xmloff/xmlnumfi.hxx>

// presentations:animations

struct SdXMLFixedDataStyle;
class SdXMLImport;
enum class DataStyleNumber : sal_uInt8;

class SdXMLNumberFormatImportContext final : public SvXMLNumFormatContext
{
    friend class SdXMLNumberFormatMemberImportContext;

    bool    mbTimeStyle;
    bool    mbAutomatic;
    DataStyleNumber mnElements[16];
    sal_Int16   mnIndex;

    sal_Int32   mnKey;

    bool compareStyle( const SdXMLFixedDataStyle* pStyle, sal_Int16& nIndex ) const;

    void add( OUString const & rNumberStyle, bool bLong, bool bTextual, bool bDecimal02, OUString const & rText );

public:

    SdXMLNumberFormatImportContext( SdXMLImport& rImport,
        sal_Int32 nElement,
        SvXMLNumImpData* pNewData,  SvXMLStylesTokens nNewType,
        const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
        SvXMLStylesContext& rStyles);
    virtual ~SdXMLNumberFormatImportContext() override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    sal_Int32 GetDrawKey() const { return mnKey; }
};

#endif // INCLUDED_XMLOFF_INC_XMLNUMBERSTYLESIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
