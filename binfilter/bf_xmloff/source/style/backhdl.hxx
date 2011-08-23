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

#ifndef _XMLOFF_PROPERTYHANDLER_BRUSHTYPES_HXX
#define _XMLOFF_PROPERTYHANDLER_BRUSHTYPES_HXX

#ifndef _XMLOFF_PROPERTYHANDLERBASE_HXX
#include <xmlprhdl.hxx>
#endif

#ifndef _COM_SUN_STAR_STYLE_GRAPHICLOCATION_HPP_
#include <com/sun/star/style/GraphicLocation.hpp>
#endif 

#ifndef _COM_SUN_STAR_STYLE_GRAPHICLOCATION_HPP_
#include <com/sun/star/style/GraphicLocation.hpp>
#endif
namespace binfilter {

/**
    PropertyHandler for the XML-data-type:
*/
class XMLBackGraphicPositionPropHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLBackGraphicPositionPropHdl();

    /// TabStops will be imported/exported as XML-Elements. So the Import/Export-work must be done at another place.
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;

private:
    void MergeXMLHoriPos( ::com::sun::star::style::GraphicLocation& ePos, ::com::sun::star::style::GraphicLocation eHori ) const;
    void MergeXMLVertPos( ::com::sun::star::style::GraphicLocation& ePos, ::com::sun::star::style::GraphicLocation eVert ) const;
};

}//end of namespace binfilter
#endif		// _XMLOFF_PROPERTYHANDLER_BRUSHTYPES_HXX
