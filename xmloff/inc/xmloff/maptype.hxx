/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: maptype.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:27:46 $
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

#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX
#define _XMLOFF_PROPMAPPINGTYPES_HXX

#include <tools/solar.h>

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

///////////////////////////////////////////////////////////////////////////
//
/** Represents a property with its API-name, its XML-name and the type of
    its value.
*/
struct XMLPropertyMapEntry
{
    const sal_Char* msApiName;      /// Property-Name
    sal_Int32       nApiNameLength; /// length of property name
    sal_uInt16      mnNameSpace;    /** declares the Namespace in wich this
                                        property exists */
    enum ::xmloff::token::XMLTokenEnum meXMLName;       /// XML-Name
    sal_Int32       mnType;         /** Bit 0..23 : XML-Type of its value
                                        Bit 24..31: The flags specifies
                                                    additional rules how
                                                    to im/export the porperty */
    sal_Int16       mnContextId;    /// User defined id for context filtering
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

#endif // _XMLOFF_PROPMAPPINGTYPES_HXX
