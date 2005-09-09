/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlitem.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:23:17 $
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

#ifndef _XMLITEM_HXX
#define _XMLITEM_HXX

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#ifndef _SOLAR_H //autogen wg. USHORT
#include <tools/solar.h>
#endif
#include <limits.h>

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif

namespace rtl { class OUString; }

class SfxItemSet;
class SvXMLImportItemMapper;
class SvXMLUnitConverter;
struct SvXMLItemMapEntry;

class SvXMLItemSetContext : public SvXMLImportContext
{
protected:
    SfxItemSet                  &rItemSet;
    const SvXMLImportItemMapper &rIMapper;
    const SvXMLUnitConverter    &rUnitConv;

public:

    SvXMLItemSetContext( SvXMLImport& rImport, USHORT nPrfx,
                         const ::rtl::OUString& rLName,
                         const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
                         SfxItemSet&  rItemSet,
                         const SvXMLImportItemMapper& rIMappper,
                         const SvXMLUnitConverter& rUnitConv );

    virtual ~SvXMLItemSetContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    // This method is called from this instance implementation of
    // CreateChildContext if the element matches an entry in the
    // SvXMLImportItemMapper with the mid flag MID_SW_FLAG_ELEMENT_ITEM_IMPORT
    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
                                    SfxItemSet&  rItemSet,
                                   const SvXMLItemMapEntry& rEntry,
                                   const SvXMLUnitConverter& rUnitConv );


};


#endif  //  _XMLITEM_HXX

