/*************************************************************************
 *
 *  $RCSfile: servicehelper.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-14 15:52:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _UTL_SERVICEHELPER_HXX_
#define _UTL_SERVICEHELPER_HXX_

/** the UNO3_GETIMPLEMENTATION_* macros  implement a static helper function
    that gives access to your implementation for a given interface reference,
    if possible.

    Example:
        MyClass* pClass = MyClass::getImplementation( xRef );

    Usage:
        Put a UNO3_GETIMPLEMENTATION_DECL( classname ) inside your class
        definitian and UNO3_GETIMPLEMENTATION_IMPL( classname ) inside
        your cxx file. Your class must inherit ::com::sun::star::uno::XUnoTunnel
        and export it with queryInterface. Implementation of XUnoTunnel is
        done by this macro.
*/
#define UNO3_GETIMPLEMENTATION_DECL( classname ) \
    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw(); \
    static classname* getImplementation( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xInt ) throw(); \
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

#define UNO3_GETIMPLEMENTATION_BASE_IMPL( classname ) \
const ::com::sun::star::uno::Sequence< sal_Int8 > & classname::getUnoTunnelId() throw() \
{ \
    static ::com::sun::star::uno::Sequence< sal_Int8 > * pSeq = 0; \
    if( !pSeq ) \
    { \
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ); \
        if( !pSeq ) \
        { \
            static ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( 16 ); \
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True ); \
            pSeq = &aSeq; \
        } \
    } \
    return *pSeq; \
} \
\
classname* classname::getImplementation( uno::Reference< uno::XInterface > xInt ) throw() \
{ \
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUT( xInt, ::com::sun::star::uno::UNO_QUERY ); \
    if( xUT.is() ) \
        return (classname*)xUT->getSomething( classname::getUnoTunnelId() ); \
    else \
        return NULL; \
}

#define UNO3_GETIMPLEMENTATION_IMPL( classname )\
UNO3_GETIMPLEMENTATION_BASE_IMPL(classname)\
sal_Int64 SAL_CALL classname::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw(::com::sun::star::uno::RuntimeException) \
{ \
    if( rId.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), \
                                                         rId.getConstArray(), 16 ) ) \
    { \
        return (sal_Int64)this; \
    } \
    return 0; \
}

#define UNO3_GETIMPLEMENTATION2_IMPL( classname, baseclass )\
UNO3_GETIMPLEMENTATION_BASE_IMPL(classname)\
sal_Int64 SAL_CALL classname::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw(::com::sun::star::uno::RuntimeException) \
{ \
    if( rId.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), \
                                                         rId.getConstArray(), 16 ) ) \
    { \
        return (sal_Int64)this; \
    } \
    else \
    { \
        return baseclass::getSomething( rId ); \
    } \
}


#endif _UTL_SERVICEHELPER_HXX_

