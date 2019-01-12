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
#ifndef INCLUDED_XMLOFF_TXTSTYLI_HXX
#define INCLUDED_XMLOFF_TXTSTYLI_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <xmloff/prstylei.hxx>

class XMLEventsImportContext;

class XMLOFF_DLLPUBLIC XMLTextStyleContext : public XMLPropStyleContext
{
    OUString             m_sListStyleName;
    OUString             m_sCategoryVal;
    OUString             m_sDropCapTextStyleName;
    OUString             m_sMasterPageName;
    OUString             m_sDataStyleName; // for grid columns only

    sal_Int8    m_nOutlineLevel;

    bool        m_isAutoUpdate : 1;
    bool        m_bHasMasterPageName : 1;

    bool        m_bHasCombinedCharactersLetter : 1;

    // Introduce import of empty list style (#i69523#)
    bool        m_bListStyleSet : 1;

    rtl::Reference<XMLEventsImportContext> m_xEventContext;

protected:

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const OUString& rLocalName,
                               const OUString& rValue ) override;

public:


    XMLTextStyleContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
            SvXMLStylesContext& rStyles, sal_uInt16 nFamily,
            bool bDefaultStyle = false );
    ~XMLTextStyleContext() override;

    XMLTextStyleContext(const XMLTextStyleContext &) = delete;
    XMLTextStyleContext operator=(const XMLTextStyleContext &) = delete;

    virtual SvXMLImportContextRef CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    const OUString& GetListStyle() const { return m_sListStyleName; }
    // XML import: reconstruction of assignment of paragraph style to outline levels (#i69629#)
    bool IsListStyleSet() const
    {
        return m_bListStyleSet;
    }

    const OUString& GetMasterPageName() const { return m_sMasterPageName; }
    bool HasMasterPageName() const { return m_bHasMasterPageName; }
    const OUString& GetDropCapStyleName() const { return m_sDropCapTextStyleName; }
    const OUString& GetDataStyleName() const { return m_sDataStyleName; }

    virtual void CreateAndInsert( bool bOverwrite ) override;
    virtual void Finish( bool bOverwrite ) override;
    virtual void SetDefaults() override;

    // override FillPropertySet, so we can get at the combined characters
    virtual void FillPropertySet(
            const css::uno::Reference< css::beans::XPropertySet > & rPropSet ) override;

    bool HasCombinedCharactersLetter()
        { return m_bHasCombinedCharactersLetter; }

    const ::std::vector< XMLPropertyState > & GetProperties_() { return GetProperties(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
