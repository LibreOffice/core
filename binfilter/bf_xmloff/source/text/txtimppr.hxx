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
#ifndef _XMLOFF_TXTIMPPR_HXX
#define _XMLOFF_TXTIMPPR_HXX

#include "xmlimppr.hxx"
#include "xmlictxt.hxx"
namespace binfilter {

class XMLFontStylesContext;

class XMLTextImportPropertyMapper : public SvXMLImportPropertyMapper
{
    sal_Int32 nSizeTypeIndex;
    SvXMLImportContextRef xFontDecls;

    void FontFinished(
                XMLPropertyState *pFontFamilyNameState,
                XMLPropertyState *pFontStyleNameState,
                XMLPropertyState *pFontFamilyState,
                XMLPropertyState *pFontPitchState,
                XMLPropertyState *pFontCharsetState ) const;

    void FontDefaultsCheck(
                XMLPropertyState*pFontFamilyName,
                XMLPropertyState* pFontStyleName,
                XMLPropertyState* pFontFamily,
                XMLPropertyState* pFontPitch,
                XMLPropertyState* pFontCharSet,
                XMLPropertyState** ppNewFontStyleName,
                XMLPropertyState** ppNewFontFamily,
                XMLPropertyState** ppNewFontPitch,
                XMLPropertyState** ppNewFontCharSet ) const;

protected:
    virtual sal_Bool handleSpecialItem(
            XMLPropertyState& rProperty,
            ::std::vector< XMLPropertyState >& rProperties,
            const ::rtl::OUString& rValue,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap ) const;
public:
    XMLTextImportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper,
            SvXMLImport& rImport,
            XMLFontStylesContext *pFontDecls = 0 );
    virtual ~XMLTextImportPropertyMapper();

    void SetFontDecls( XMLFontStylesContext *pFontDecls );

    /** This method is called when all attributes have benn processed. It may be used to remove items that are incomplete */
    virtual void finished(
            ::std::vector< XMLPropertyState >& rProperties,
            sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const;
};

}//end of namespace binfilter
#endif	//  _XMLOFF_XMLIMPPR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
