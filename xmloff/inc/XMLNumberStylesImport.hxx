/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLNumberStylesImport.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:52:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

//////////////////////////////////////////////////////////////////////////////
// presentations:animations

struct SdXMLFixedDataStyle;
class SdXMLImport;

class SdXMLNumberFormatImportContext : public SvXMLNumFormatContext
{
private:
    friend class SdXMLNumberFormatMemberImportContext;

    SdXMLImport& mrImport;

    sal_Bool    mbTimeStyle;
    sal_Bool    mbAutomatic;
    sal_uInt8   mnElements[16];
    sal_Int16   mnIndex;

    sal_Int32   mnKey;

    bool compareStyle( const SdXMLFixedDataStyle* pStyle, sal_Int16& nIndex ) const;

protected:
    void add( rtl::OUString& rNumberStyle, sal_Bool bLong, sal_Bool bTextual, sal_Bool  bDecimal02, rtl::OUString& rText );

public:
    TYPEINFO();

    SdXMLNumberFormatImportContext( SdXMLImport& rImport,
        sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        SvXMLNumImpData* pNewData, sal_uInt16 nNewType,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        SvXMLStylesContext& rStyles);
    virtual ~SdXMLNumberFormatImportContext();

    virtual void EndElement();

    virtual SvXMLImportContext * CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );

    sal_Int32 GetDrawKey() const { return mnKey; }
};

#endif  //  _XMLOFF_NUMBERSTYLESIMPORT_HXX

