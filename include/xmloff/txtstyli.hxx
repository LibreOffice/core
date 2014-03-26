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

    sal_Bool    bAutoUpdate : 1;
    sal_Bool    bHasMasterPageName : 1;

    sal_Bool bHasCombinedCharactersLetter : 1;

    // Introduce import of empty list style (#i69523#)
    sal_Bool mbListStyleSet : 1;

    XMLEventsImportContext* pEventContext;

protected:

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const OUString& rLocalName,
                               const OUString& rValue ) SAL_OVERRIDE;

public:

    TYPEINFO_OVERRIDE();

    XMLTextStyleContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            SvXMLStylesContext& rStyles, sal_uInt16 nFamily,
            sal_Bool bDefaultStyle = sal_False );
    virtual ~XMLTextStyleContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) SAL_OVERRIDE;

    sal_Bool IsAutoUpdate() const { return bAutoUpdate; }

    const OUString& GetListStyle() const { return sListStyleName; }
    // XML import: reconstrution of assignment of paragraph style to outline levels (#i69629#)
    sal_Bool IsListStyleSet() const
    {
        return mbListStyleSet;
    }

    const OUString& GetMasterPageName() const { return sMasterPageName; }
    sal_Bool HasMasterPageName() const { return bHasMasterPageName; }
    const OUString& GetDropCapStyleName() const { return sDropCapTextStyleName; }
    const OUString& GetDataStyleName() const { return sDataStyleName; }

    virtual void CreateAndInsert( sal_Bool bOverwrite ) SAL_OVERRIDE;
    virtual void Finish( bool bOverwrite ) SAL_OVERRIDE;
    virtual void SetDefaults() SAL_OVERRIDE;

    // overload FillPropertySet, so we can get at the combined characters
    virtual void FillPropertySet(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & rPropSet ) SAL_OVERRIDE;

    inline sal_Bool HasCombinedCharactersLetter()
        { return bHasCombinedCharactersLetter; }

    const ::std::vector< XMLPropertyState > & _GetProperties() { return GetProperties(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
