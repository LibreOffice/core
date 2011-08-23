/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLNUMI_HXX
#define _XMLOFF_XMLNUMI_HXX

#include <com/sun/star/container/XIndexReplace.hpp>

#include <bf_xmloff/xmlstyle.hxx>

#include <com/sun/star/style/NumberingType.hpp>

namespace com { namespace sun { namespace star { namespace frame { class XModel; } } } }
namespace binfilter {
class SvI18NMap;
class SvxXMLListStyle_Impl;

class SvxXMLListStyleContext : public SvXMLStyleContext
{
    const ::rtl::OUString		sIsPhysical;
    const ::rtl::OUString		sNumberingRules;
    const ::rtl::OUString		sName;
    const ::rtl::OUString		sIsContinuousNumbering;
    const ::rtl::OUString		sIsNumbering;

    ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexReplace > xNumRules;

    SvxXMLListStyle_Impl		*pLevelStyles;

    sal_Int16					nLevels;
    sal_Bool					bConsecutive : 1;
    sal_Bool					bOutline : 1;

protected:

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const ::rtl::OUString& rLocalName,
                               const ::rtl::OUString& rValue );

public:

    TYPEINFO();

    SvxXMLListStyleContext(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
            sal_Bool bOutl=sal_False );

    virtual ~SvxXMLListStyleContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    void FillUnoNumRule(
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::container::XIndexReplace > & rNumRule,
            const SvI18NMap *pI18NMap ) const;

    const ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexReplace >& GetNumRules() const
        { return xNumRules; }
    sal_Bool IsOutline() const { return bOutline; }
    sal_Bool IsConsecutive() const { return bConsecutive; }
    sal_Int16 GetLevels() const { return nLevels; }

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

    virtual void CreateAndInsertLate( sal_Bool bOverwrite );

    void CreateAndInsertAuto() const;
};

}//end of namespace binfilter
#endif	//  _XMLOFF_XMLNUMI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
