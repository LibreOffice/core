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

#ifndef _XMLOFF_XMLANCHORTYPEPROPHDL_HXX
#define _XMLOFF_XMLANCHORTYPEPROPHDL_HXX

#include <com/sun/star/text/TextContentAnchorType.hpp>

#include "xmlprhdl.hxx"
namespace binfilter {


class XMLAnchorTypePropHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLAnchorTypePropHdl ();

    virtual sal_Bool importXML(
            const ::rtl::OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML(
            ::rtl::OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
    static sal_Bool convert( const ::rtl::OUString& rStrImpValue,
                 const SvXMLUnitConverter& rUnitConverter,
                 ::com::sun::star::text::TextContentAnchorType& rType );
};

}//end of namespace binfilter
#endif	//  _XMLOFF_XMLANCHORTYPEPROPHDL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
