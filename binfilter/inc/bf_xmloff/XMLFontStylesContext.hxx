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
#ifndef _XMLOFF_XMLFONTSTYLESCONTEXT_HXX_
#define _XMLOFF_XMLFONTSTYLESCONTEXT_HXX_

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

#include <bf_xmloff/xmlstyle.hxx>
namespace binfilter {

struct XMLPropertyState;
class SvXMLTokenMap;
class XMLFontFamilyNamePropHdl;
class XMLFontFamilyPropHdl;
class XMLFontPitchPropHdl;
class XMLFontEncodingPropHdl;

class XMLFontStylesContext : public SvXMLStylesContext
{
    XMLFontFamilyNamePropHdl 	*pFamilyNameHdl;
    XMLFontFamilyPropHdl 		*pFamilyHdl;
    XMLFontPitchPropHdl 		*pPitchHdl;
    XMLFontEncodingPropHdl 		*pEncHdl;

    SvXMLTokenMap			*pFontStyleAttrTokenMap;

    rtl_TextEncoding		eDfltEncoding;

protected:

    virtual SvXMLStyleContext *CreateStyleChildContext( sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

public:

    TYPEINFO();

    XMLFontStylesContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            rtl_TextEncoding eDfltEnc );
    virtual ~XMLFontStylesContext();

    const SvXMLTokenMap& GetFontStyleAttrTokenMap() const
    {
        return *pFontStyleAttrTokenMap;
    }

    sal_Bool FillProperties( const ::rtl::OUString& rName,
                         ::std::vector< XMLPropertyState > &rProps,
                         sal_Int32 nFamilyNameIdx,
                         sal_Int32 nStyleNameIdx,
                         sal_Int32 nFamilyIdx,
                         sal_Int32 nPitchIdx,
                         sal_Int32 nCharsetIdx ) const;

    rtl_TextEncoding GetDfltCharset() const { return eDfltEncoding; }

    XMLFontFamilyNamePropHdl& GetFamilyNameHdl() const { return *pFamilyNameHdl; }
    XMLFontFamilyPropHdl& GetFamilyHdl() const { return *pFamilyHdl; }
    XMLFontPitchPropHdl& GetPitchHdl() const { return *pPitchHdl; }
    XMLFontEncodingPropHdl& GetEncodingHdl() const { return *pEncHdl; }

};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
