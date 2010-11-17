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
#ifndef _XMLOFF_PRSTYLEI_HXX_
#define _XMLOFF_PRSTYLEI_HXX_

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/style/XStyle.hpp>
#include <vector>
#include <xmloff/xmlstyle.hxx>

struct XMLPropertyState;
class SvXMLStylesContext;

namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
} } }

class XMLOFF_DLLPUBLIC XMLPropStyleContext : public SvXMLStyleContext
{
    const ::rtl::OUString msIsPhysical;
    const ::rtl::OUString msFollowStyle;
    ::std::vector< XMLPropertyState > maProperties;
    ::com::sun::star::uno::Reference < ::com::sun::star::style::XStyle > mxStyle;
    SvXMLImportContextRef               mxStyles;

    SAL_DLLPRIVATE XMLPropStyleContext(XMLPropStyleContext &); // not defined
    SAL_DLLPRIVATE void operator =(XMLPropStyleContext &); // not defined

protected:

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const ::rtl::OUString& rLocalName,
                               const ::rtl::OUString& rValue );
    SvXMLStylesContext *GetStyles() { return (SvXMLStylesContext *)&mxStyles; }
    ::std::vector< XMLPropertyState > & GetProperties() { return maProperties; }

    // This methos my be overloaded to create a new style. Its called by
    // CreateInsert to create a style if a style with the requested family and
    // name couldn't be found. The st
    virtual ::com::sun::star::uno::Reference <
        ::com::sun::star::style::XStyle > Create();

public:

    TYPEINFO();

    XMLPropStyleContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            SvXMLStylesContext& rStyles, sal_uInt16 nFamily = 0,
            sal_Bool bDefaultStyle=sal_False );
    virtual ~XMLPropStyleContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void FillPropertySet(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & rPropSet );

    const SvXMLStylesContext *GetStyles() const { return (const SvXMLStylesContext *)&mxStyles; }
    const ::std::vector< XMLPropertyState > & GetProperties() const { return maProperties; }

    const ::com::sun::star::uno::Reference <
                ::com::sun::star::style::XStyle >&
               GetStyle() const { return mxStyle; }
    void SetStyle(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle >& xStl) { mxStyle = xStl; }

    virtual void SetDefaults();

    virtual void CreateAndInsert( sal_Bool bOverwrite );
    virtual void Finish( sal_Bool bOverwrite );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
