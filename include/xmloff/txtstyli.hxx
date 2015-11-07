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

class SvXMLTokenMap;
class XMLEventsImportContext;

class XMLOFF_DLLPUBLIC XMLTextStyleContext : public XMLPropStyleContext
{
    OUString             sListStyleName;
    OUString             sCategoryVal;
    OUString             sDropCapTextStyleName;
    OUString             sMasterPageName;
    OUString             sDataStyleName; // for grid columns only
    const OUString       sIsAutoUpdate;
    const OUString       sCategory;
    const OUString       sNumberingStyleName;
    const OUString       sOutlineLevel;

public:
    const OUString       sDropCapCharStyleName;
private:
    const OUString       sPageDescName;

    sal_Int8    nOutlineLevel;

    bool        bAutoUpdate : 1;
    bool        bHasMasterPageName : 1;

    bool        bHasCombinedCharactersLetter : 1;

    // Introduce import of empty list style (#i69523#)
    bool        mbListStyleSet : 1;

    XMLEventsImportContext* pEventContext;

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
    virtual ~XMLTextStyleContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    const OUString& GetListStyle() const { return sListStyleName; }
    // XML import: reconstrution of assignment of paragraph style to outline levels (#i69629#)
    bool IsListStyleSet() const
    {
        return mbListStyleSet;
    }

    const OUString& GetMasterPageName() const { return sMasterPageName; }
    bool HasMasterPageName() const { return bHasMasterPageName; }
    const OUString& GetDropCapStyleName() const { return sDropCapTextStyleName; }
    const OUString& GetDataStyleName() const { return sDataStyleName; }

    virtual void CreateAndInsert( bool bOverwrite ) override;
    virtual void Finish( bool bOverwrite ) override;
    virtual void SetDefaults() override;

    // override FillPropertySet, so we can get at the combined characters
    virtual void FillPropertySet(
            const css::uno::Reference< css::beans::XPropertySet > & rPropSet ) override;

    inline bool HasCombinedCharactersLetter()
        { return bHasCombinedCharactersLetter; }

    const ::std::vector< XMLPropertyState > & _GetProperties() { return GetProperties(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
