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

#ifndef INCLUDED_XMLOFF_XMLNUMI_HXX
#define INCLUDED_XMLOFF_XMLNUMI_HXX

#include <com/sun/star/container/XIndexReplace.hpp>

#include <xmloff/xmlstyle.hxx>
#include <com/sun/star/style/NumberingType.hpp>

namespace com { namespace sun { namespace star { namespace frame { class XModel; } } } }
class SvI18NMap;
class SvxXMLListStyle_Impl;

class SvxXMLListStyleContext : public SvXMLStyleContext
{
    const OUString       sIsPhysical;
    const OUString       sNumberingRules;
    const OUString       sName;
    const OUString       sIsContinuousNumbering;

    ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexReplace > xNumRules;

    SvxXMLListStyle_Impl        *pLevelStyles;

    sal_Int32                   nLevels;
    sal_Bool                    bConsecutive : 1;
    sal_Bool                    bOutline : 1;

protected:

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const OUString& rLocalName,
                               const OUString& rValue ) SAL_OVERRIDE;

public:

    TYPEINFO_OVERRIDE();

    SvxXMLListStyleContext(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
            sal_Bool bOutl=sal_False );

    virtual ~SvxXMLListStyleContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList >& xAttrList ) SAL_OVERRIDE;

    void FillUnoNumRule(
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::container::XIndexReplace > & rNumRule,
            const SvI18NMap *pI18NMap ) const;

    const ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexReplace >& GetNumRules() const
        { return xNumRules; }
    sal_Bool IsOutline() const { return bOutline; }
    sal_Bool IsConsecutive() const { return bConsecutive; }
    sal_Int32 GetLevels() const { return nLevels; }

    static ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexReplace >
    CreateNumRule(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::frame::XModel > & rModel );

    static void SetDefaultStyle(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::container::XIndexReplace > & rNumRule,
        sal_Int16 nLevel,
        sal_Bool bOrdered );

    virtual void CreateAndInsertLate( sal_Bool bOverwrite ) SAL_OVERRIDE;

    void CreateAndInsertAuto() const;
};

#endif // INCLUDED_XMLOFF_XMLNUMI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
