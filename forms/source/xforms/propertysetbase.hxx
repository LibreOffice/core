/*************************************************************************
 *
 *  $RCSfile: propertysetbase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:55:54 $
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
#ifndef _PROPERTYSETBASE_HXX
#define _PROPERTYSETBASE_HXX


// include for parent class
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <comphelper/propertysethelper.hxx>

// include for inlined helper function below
#include <com/sun/star/lang/IllegalArgumentException.hpp>

// forward declarations for method arguments
namespace com { namespace sun { namespace star { namespace uno {
        class Any;
        class Type;
        class RuntimeException;
        template<class T> class Sequence;
} } } }



/** turn PropertySetHelper into a full UNO object.
 * PropertySetBase can be used with ImplInheritanceHelper classes.
 */
class PropertySetBase : public cppu::OWeakObject,
                        public com::sun::star::lang::XTypeProvider,
                        public comphelper::PropertySetHelper
{
public:
    PropertySetBase();
    PropertySetBase( comphelper::PropertySetInfo* pInfo ) throw();
    virtual ~PropertySetBase() throw();

    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    virtual com::sun::star::uno::Any SAL_CALL queryInterface(
        com::sun::star::uno::Type const & rType )
        throw( com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence<com::sun::star::uno::Type> SAL_CALL getTypes() throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId() throw( com::sun::star::uno::RuntimeException );

};

/** helper function template; set a property value through a member function */
template<typename CLASS, typename VALUE>
void setAny( CLASS* pObject,
             void (CLASS::*pMethod)(const VALUE&),
             const com::sun::star::uno::Any& rAny )
{
    VALUE aVal;
    if( rAny >>= aVal )
        (pObject->*pMethod)( aVal );
    else
        throw com::sun::star::lang::IllegalArgumentException();
}

template<typename CLASS>
void setAny( CLASS* pObject,
             void (CLASS::*pMethod)(bool),
             const com::sun::star::uno::Any& rAny )
{
    bool bVal;
    if( rAny >>= bVal )
        (pObject->*pMethod)( bVal );
    else
        throw com::sun::star::lang::IllegalArgumentException();
}

/** helper function template; get a property value through a member function */
template<typename CLASS, typename VALUE>
void getAny( const CLASS* pObject,
             VALUE (CLASS::*pMethod)() const,
             com::sun::star::uno::Any& rAny )
{
    rAny <<= (pObject->*pMethod)();
}


#ifdef _MSC_VER

/* With the MS Windows compiler, we make use of the SAL_CALL macro for
 * different calling conventions for normal and API methods. To
 * support these, the following templates are copies of the templates
 * above, except with SAL_CALL.
 *
 * To porters: This applies to any platform that uses SAL_CALL. If you
 * have such a platform, add it to the ifdef above.
 */

template<typename CLASS, typename VALUE>
void setAny( CLASS* pObject,
             void (SAL_CALL CLASS::*pMethod)(const VALUE&),
             const com::sun::star::uno::Any& rAny )
{
    VALUE aVal;
    if( rAny >>= aVal )
        (pObject->*pMethod)( aVal );
    else
        throw com::sun::star::lang::IllegalArgumentException();
}
#endif // _MSC_VER

template<typename CLASS, typename VALUE>
void getAny( CLASS* pObject,
             VALUE (SAL_CALL CLASS::*pMethod)(),
             com::sun::star::uno::Any& rAny )
{
    rAny <<= (pObject->*pMethod)();
}


#endif
