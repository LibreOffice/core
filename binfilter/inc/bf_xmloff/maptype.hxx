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

#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX
#define _XMLOFF_PROPMAPPINGTYPES_HXX

#include <tools/solar.h>

#include <com/sun/star/uno/Any.hxx>

#include <bf_xmloff/xmltoken.hxx>
namespace binfilter {

///////////////////////////////////////////////////////////////////////////
//
/** Represents a property with its API-name, its XML-name and the type of
    its value.
*/
struct XMLPropertyMapEntry
{
    const sal_Char*	msApiName;		/// Property-Name
    sal_Int32       nApiNameLength; /// length of property name
    sal_uInt16		mnNameSpace;	/** declares the Namespace in wich this
                                        property exists */
    enum ::binfilter::xmloff::token::XMLTokenEnum meXMLName;		/// XML-Name
    sal_Int32		mnType;			/** Bit 0..23 : XML-Type of its value
                                        Bit 24..31: The flags specifies
                                                    additional rules how
                                                    to im/export the porperty */
    sal_Int16		mnContextId;	/// User defined id for context filtering
};

///////////////////////////////////////////////////////////////////////////
//
/** Smart struct to transport an Any with an index to the appropriate
    property-name
*/
struct XMLPropertyState
{
    sal_Int32                  mnIndex;
    ::com::sun::star::uno::Any maValue;

    XMLPropertyState( sal_Int32 nIndex )
        : mnIndex( nIndex ) {}
    XMLPropertyState( sal_Int32 nIndex, const ::com::sun::star::uno::Any& rValue )
        : mnIndex( nIndex ), maValue( rValue ) {}
    XMLPropertyState( const XMLPropertyState& rPropState )
        : mnIndex( rPropState.mnIndex ), maValue( rPropState.maValue ) {}
};

}//end of namespace binfilter
#endif // _XMLOFF_PROPMAPPINGTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
