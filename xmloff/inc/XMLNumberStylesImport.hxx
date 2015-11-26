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
#include <com/sun/star/container/XNameAccess.hpp>

// presentations:animations

struct SdXMLFixedDataStyle;
class SdXMLImport;

class SdXMLNumberFormatImportContext : public SvXMLNumFormatContext
{
private:
    friend class SdXMLNumberFormatMemberImportContext;

    bool    mbTimeStyle;
    bool    mbAutomatic;
    sal_uInt8   mnElements[16];
    sal_Int16   mnIndex;

    sal_Int32   mnKey;

    bool compareStyle( const SdXMLFixedDataStyle* pStyle, sal_Int16& nIndex ) const;

protected:
    void add( OUString& rNumberStyle, bool bLong, bool bTextual, bool  bDecimal02, OUString& rText );

public:

    SdXMLNumberFormatImportContext( SdXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        SvXMLNumImpData* pNewData, sal_uInt16 nNewType,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        SvXMLStylesContext& rStyles);
    virtual ~SdXMLNumberFormatImportContext();

    virtual void EndElement() override;

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;

    sal_Int32 GetDrawKey() const { return mnKey; }
};

#endif // INCLUDED_XMLOFF_INC_XMLNUMBERSTYLESIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
