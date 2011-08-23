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

#ifndef _XMLOFF_NAMEDBOOLPROPERTYHANDLER_HXX
#define _XMLOFF_NAMEDBOOLPROPERTYHANDLER_HXX

#include <bf_xmloff/xmlprhdl.hxx>
#include <bf_xmloff/xmltoken.hxx>

#include <com/sun/star/uno/Type.h>

#include <bf_xmloff/xmlement.hxx>
namespace binfilter {

/**
    PropertyHandler for a named xml bool type:
*/
class XMLNamedBoolPropertyHdl : public XMLPropertyHandler
{
private:
    const ::rtl::OUString maTrueStr;
    const ::rtl::OUString maFalseStr;

public:
    XMLNamedBoolPropertyHdl( const ::rtl::OUString& rTrueStr, const ::rtl::OUString& rFalseStr ) : maTrueStr( rTrueStr ), maFalseStr( rFalseStr ) {}

    XMLNamedBoolPropertyHdl( 
            ::binfilter::xmloff::token::XMLTokenEnum eTrue,
            ::binfilter::xmloff::token::XMLTokenEnum eFalse
            ) :
        maTrueStr( ::binfilter::xmloff::token::GetXMLToken( eTrue ) ),
        maFalseStr( ::binfilter::xmloff::token::GetXMLToken( eFalse ) )
    {}

    virtual ~XMLNamedBoolPropertyHdl();

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

}//end of namespace binfilter
#endif		// _XMLOFF_NAMEDBOOLPROPERTYHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
