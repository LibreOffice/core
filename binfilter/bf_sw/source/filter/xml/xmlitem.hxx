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

#ifndef _XMLITEM_HXX
#define _XMLITEM_HXX

#include <com/sun/star/xml/sax/XAttributeList.hpp>

#include <tools/solar.h>
#include <limits.h>

#include <bf_xmloff/xmlictxt.hxx>

namespace rtl { class OUString; }
namespace binfilter {

class SfxItemSet; 

class SvXMLUnitConverter; 
class SvXMLImportItemMapper;

struct SvXMLItemMapEntry;

class SvXMLItemSetContext : public SvXMLImportContext
{
protected:
    SfxItemSet					&rItemSet;
    const SvXMLImportItemMapper	&rIMapper;
    const SvXMLUnitConverter	&rUnitConv;

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
    // SvXMLImportItemMapper with the mid flag MID_FLAG_ELEMENT_ITEM_IMPORT
    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
                                    SfxItemSet&  rItemSet,
                                   const SvXMLItemMapEntry& rEntry,
                                   const SvXMLUnitConverter& rUnitConv );


};


} //namespace binfilter
#endif	//  _XMLITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
