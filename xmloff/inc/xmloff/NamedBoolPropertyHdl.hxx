/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NamedBoolPropertyHdl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:20:06 $
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

#ifndef _XMLOFF_NAMEDBOOLPROPERTYHANDLER_HXX
#define _XMLOFF_NAMEDBOOLPROPERTYHANDLER_HXX

#ifndef _XMLOFF_PROPERTYHANDLERBASE_HXX
#include <xmloff/xmlprhdl.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_TYPE_H_
#include <com/sun/star/uno/Type.h>
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include <xmloff/xmlement.hxx>
#endif

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
            ::xmloff::token::XMLTokenEnum eTrue,
            ::xmloff::token::XMLTokenEnum eFalse
            ) :
        maTrueStr( ::xmloff::token::GetXMLToken( eTrue ) ),
        maFalseStr( ::xmloff::token::GetXMLToken( eFalse ) )
    {}

    virtual ~XMLNamedBoolPropertyHdl();

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

#endif      // _XMLOFF_NAMEDBOOLPROPERTYHANDLER_HXX
