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

#ifndef _XMLOFF_NUMBERSTYLESIMPORT_HXX
#define _XMLOFF_NUMBERSTYLESIMPORT_HXX

#ifndef _XMLOFF_XMLNUMFI_HXX
#include "xmlnumfi.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_ 
#include <com/sun/star/container/XNameAccess.hpp>
#endif
namespace binfilter {

//////////////////////////////////////////////////////////////////////////////
// presentations:animations

struct SdXMLFixedDataStyle;
class SdXMLImport;

class SdXMLNumberFormatImportContext : public SvXMLNumFormatContext
{
private:
    friend class SdXMLNumberFormatMemberImportContext;

    SdXMLImport& mrImport;

    sal_Bool	mbTimeStyle;
    sal_Bool	mbAutomatic;
    sal_uInt8	mnElements[8];
    sal_Int16	mnIndex;

    sal_Int32	mnKey;

    sal_Bool compareStyle( const SdXMLFixedDataStyle* pStyle ) const;

protected:
    void add( ::rtl::OUString& rNumberStyle, sal_Bool bLong, sal_Bool bTextual, sal_Bool	bDecimal02, ::rtl::OUString& rText );

public:
    TYPEINFO();

    SdXMLNumberFormatImportContext( SdXMLImport& rImport, 
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName, 
        SvXMLNumImpData* pNewData, sal_uInt16 nNewType,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
        SvXMLStylesContext& rStyles);
    virtual ~SdXMLNumberFormatImportContext();

    virtual void EndElement();

    virtual SvXMLImportContext * CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    sal_Int32 GetDrawKey() const { return mnKey; }
};

}//end of namespace binfilter
#endif	//  _XMLOFF_NUMBERSTYLESIMPORT_HXX

