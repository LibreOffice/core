/*************************************************************************
 *
 *  $RCSfile: miscuno.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:49 $
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

#ifndef SC_MISCUNO_HXX
#define SC_MISCUNO_HXX

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif


#define SC_SIMPLE_SERVICE_INFO( ClassName, ClassNameAscii, ServiceAscii )            \
::rtl::OUString SAL_CALL ClassName##::getImplementationName()                        \
    throw(::com::sun::star::uno::RuntimeException)                                   \
{                                                                                    \
    return ::rtl::OUString::createFromAscii(ClassNameAscii##);                       \
}                                                                                    \
sal_Bool SAL_CALL ClassName##::supportsService( const ::rtl::OUString& ServiceName ) \
    throw(::com::sun::star::uno::RuntimeException)                                   \
{                                                                                    \
    return !ServiceName.compareToAscii(ServiceAscii);                                \
}                                                                                    \
::com::sun::star::uno::Sequence< ::rtl::OUString >                                   \
    SAL_CALL ClassName##::getSupportedServiceNames(void)                             \
    throw(::com::sun::star::uno::RuntimeException)                                   \
{                                                                                    \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aRet(1);                      \
    ::rtl::OUString* pArray = aRet.getArray();                                       \
    pArray[0] = ::rtl::OUString::createFromAscii(ServiceAscii##);                    \
    return aRet;                                                                     \
}

#define SC_IMPL_DUMMY_PROPERTY_LISTENER( ClassName )                                \
    void SAL_CALL ClassName##::addPropertyChangeListener( const rtl::OUString&,     \
                            const uno::Reference<beans::XPropertyChangeListener>&)  \
                            throw(beans::UnknownPropertyException,                  \
                            lang::WrappedTargetException, uno::RuntimeException)    \
    { DBG_ERROR("not implemented"); }                                               \
    void SAL_CALL ClassName##::removePropertyChangeListener( const rtl::OUString&,  \
                            const uno::Reference<beans::XPropertyChangeListener>&)  \
                            throw(beans::UnknownPropertyException,                  \
                            lang::WrappedTargetException, uno::RuntimeException)    \
    { DBG_ERROR("not implemented"); }                                               \
    void SAL_CALL ClassName##::addVetoableChangeListener( const rtl::OUString&,     \
                            const uno::Reference<beans::XVetoableChangeListener>&)  \
                            throw(beans::UnknownPropertyException,                  \
                            lang::WrappedTargetException, uno::RuntimeException)    \
    { DBG_ERROR("not implemented"); }                                               \
    void SAL_CALL ClassName##::removeVetoableChangeListener( const rtl::OUString&,  \
                            const uno::Reference<beans::XVetoableChangeListener>&)  \
                            throw(beans::UnknownPropertyException,                  \
                            lang::WrappedTargetException, uno::RuntimeException)    \
    { DBG_ERROR("not implemented"); }


#define SC_QUERYINTERFACE(x)    \
    if (rType == getCppuType((const uno::Reference<x>*)0))  \
    { uno::Any aR; aR <<= uno::Reference<x>(this); return aR; }

// SC_QUERY_MULTIPLE( XElementAccess, XIndexAccess ):
//  use if interface is used several times in one class

#define SC_QUERY_MULTIPLE(x,y)  \
    if (rType == getCppuType((const uno::Reference<x>*)0))  \
    { uno::Any aR; aR <<= uno::Reference<x>(static_cast<y*>(this)); return aR; }


class ScIndexEnumeration : public cppu::WeakImplHelper2<
                                com::sun::star::container::XEnumeration,
                                com::sun::star::lang::XServiceInfo >
{
private:
    com::sun::star::uno::Reference<com::sun::star::container::XIndexAccess> xIndex;
    INT32                   nPos;

public:
                            ScIndexEnumeration(const com::sun::star::uno::Reference<
                                            com::sun::star::container::XIndexAccess>& rInd);
    virtual                 ~ScIndexEnumeration();

                            // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement()
                                throw(::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScEmptyEnumerationAccess : public cppu::WeakImplHelper2<
                                com::sun::star::container::XEnumerationAccess,
                                com::sun::star::lang::XServiceInfo >
{
public:
                            ScEmptyEnumerationAccess();
    virtual                 ~ScEmptyEnumerationAccess();

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScEmptyEnumeration : public cppu::WeakImplHelper2<
                                com::sun::star::container::XEnumeration,
                                com::sun::star::lang::XServiceInfo >
{
public:
                            ScEmptyEnumeration();
    virtual                 ~ScEmptyEnumeration();

                            // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement()
                                throw(::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
                                throw(::com::sun::star::uno::RuntimeException);
};

//  new (uno 3) variant
class ScNameToIndexAccess : public cppu::WeakImplHelper2<
                                com::sun::star::container::XIndexAccess,
                                com::sun::star::lang::XServiceInfo >
{
private:
    com::sun::star::uno::Reference<com::sun::star::container::XNameAccess> xNameAccess;
    com::sun::star::uno::Sequence<rtl::OUString> aNames;

public:
                            ScNameToIndexAccess(
                                const com::sun::star::uno::Reference<
                                    com::sun::star::container::XNameAccess>& rNameObj );
    virtual                 ~ScNameToIndexAccess();

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
                                throw(::com::sun::star::uno::RuntimeException);
};

class ScPrintSettingsObj : public cppu::WeakImplHelper2<
                                com::sun::star::beans::XPropertySet,
                                com::sun::star::lang::XServiceInfo >
{
public:
                            ScPrintSettingsObj();
    virtual                 ~ScPrintSettingsObj();

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScUnoHelpFunctions
{
public:
    static com::sun::star::uno::Reference<com::sun::star::uno::XInterface>
                            AnyToInterface( const com::sun::star::uno::Any& rAny );
    static BOOL             GetBoolProperty( const com::sun::star::uno::Reference<
                                                com::sun::star::beans::XPropertySet>& xProp,
                                            const ::rtl::OUString& rName, BOOL bDefault = FALSE );
    static long             GetLongProperty( const com::sun::star::uno::Reference<
                                                com::sun::star::beans::XPropertySet>& xProp,
                                            const ::rtl::OUString& rName, long nDefault = 0 );
    static long             GetEnumProperty( const com::sun::star::uno::Reference<
                                                com::sun::star::beans::XPropertySet>& xProp,
                                            const ::rtl::OUString& rName, long nDefault );

    static BOOL             GetBoolFromAny( const com::sun::star::uno::Any& aAny );
    static INT16            GetInt16FromAny( const com::sun::star::uno::Any& aAny );
    static INT32            GetEnumFromAny( const com::sun::star::uno::Any& aAny );
    static void             SetBoolInAny( com::sun::star::uno::Any& rAny, BOOL bValue );
};



#endif

