/*************************************************************************
 *
 *  $RCSfile: CommonTools.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:48:07 $
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
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#define _CONNECTIVITY_COMMONTOOLS_HXX_

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _VECTOR_
#include <vector>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif
#ifndef INCLUDED_JVMACCESS_VIRTUALMACHINE_HXX
#include <jvmaccess/virtualmachine.hxx>
#endif // INCLUDED_JVMACCESS_VIRTUALMACHINE_HXX
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

namespace com { namespace sun { namespace star { namespace util {
    struct Date;
    struct DateTime;
    struct Time;
}
}}}

namespace connectivity
{
    //------------------------------------------------------------------------------
    sal_Bool match(const sal_Unicode* pWild, const sal_Unicode* pStr, const sal_Unicode cEscape);
    //------------------------------------------------------------------------------
    rtl::OUString toString(const ::com::sun::star::uno::Any& rValue);
    rtl::OUString toDateString(const ::com::sun::star::util::Date& rDate);
    rtl::OUString toTimeString(const ::com::sun::star::util::Time& rTime);
    rtl::OUString toDateTimeString(const ::com::sun::star::util::DateTime& rDateTime);

    // typedefs
    typedef std::vector< ::com::sun::star::uno::WeakReferenceHelper > OWeakRefArray;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>    OSQLTable;

    DECLARE_STL_MAP(::rtl::OUString,OSQLTable,comphelper::UStringMixLess,  OSQLTables);

    // -------------------------------------------------------------------------
    // class ORefVector allows reference counting on a std::vector
    // -------------------------------------------------------------------------
    template< class VectorVal > class ORefVector : public ::std::vector< VectorVal >
    {
        oslInterlockedCount         m_refCount;
        //  ORefVector(const ORefVector&);
        //  ORefVector& operator=(const ORefVector&);

        typedef ::std::vector< VectorVal > BaseClass;
    protected:
        virtual ~ORefVector(){}
    public:
        ORefVector() : m_refCount(0) {}
        ORefVector(size_t _st) : ::std::vector< VectorVal > (_st) , m_refCount(0) {}
        ORefVector(const ORefVector& _rRH) : ::std::vector< VectorVal > (_rRH),m_refCount(0)
        {
        }
        ORefVector& operator=(const ORefVector& _rRH)
        {
            if ( &_rRH != this )
            {
                BaseClass::operator=(_rRH);
            }
            return *this;
        }

        inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
            { return ::rtl_allocateMemory( nSize ); }
        inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
            { ::rtl_freeMemory( pMem ); }
        inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW( () )
            { return pMem; }
        inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW( () )
            {}

        void acquire()
        {
            osl_incrementInterlockedCount( &m_refCount );
        }
        void release()
        {
            if (! osl_decrementInterlockedCount( &m_refCount ))
                delete this;
        }

    };
    // -------------------------------------------------------------------------
    // class ORowVector incudes refcounting and initialze himself
    // with at least one element. This first element is reserved for
    // the bookmark
    // -------------------------------------------------------------------------
    template< class VectorVal > class ORowVector : public  ORefVector< VectorVal >
    {
    public:
        ORowVector() : ORefVector< VectorVal >(1){}
        ORowVector(size_t _st) : ORefVector< VectorVal >(_st+1)
            {}
    };

    typedef ORefVector< ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> > OSQLColumns;

    // =======================================================================================
    // search from __first to __last the column with the name _rVal
    // when no such column exist __last is returned
    OSQLColumns::const_iterator find(   OSQLColumns::const_iterator __first,
                                        OSQLColumns::const_iterator __last,
                                        const ::rtl::OUString& _rVal,
                                        const ::comphelper::UStringMixEqual& _rCase);
    // =======================================================================================
    // search from __first to __last the column with the realname _rVal
    // when no such column exist __last is returned
    OSQLColumns::const_iterator findRealName(   OSQLColumns::const_iterator __first,
                                                OSQLColumns::const_iterator __last,
                                                const ::rtl::OUString& _rVal,
                                                const ::comphelper::UStringMixEqual& _rCase);

    // =======================================================================================
    // the first two find methods are much faster than the one below
    // =======================================================================================
    // search from __first to __last the column with the property _rProp equals the value _rVal
    // when no such column exist __last is returned
    OSQLColumns::const_iterator find(   OSQLColumns::const_iterator __first,
                                        OSQLColumns::const_iterator __last,
                                        const ::rtl::OUString& _rProp,
                                        const ::rtl::OUString& _rVal,
                                        const ::comphelper::UStringMixEqual& _rCase);

    void checkDisposed(sal_Bool _bThrow) throw ( ::com::sun::star::lang::DisposedException );


    /** creates a java virtual machine
        @param  _rxFactory
            The ORB.
        @return
            The JavaVM.
    */
    ::rtl::Reference< jvmaccess::VirtualMachine > getJavaVM(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);

    /** return <TRUE/> if the java class exists, otherwise <FALSE/>.
        @param  _pJVM
            The JavaVM.
        @param  _sClassName
            The class name to look for.
    */
    sal_Bool existsJavaClassByName( const ::rtl::Reference< jvmaccess::VirtualMachine >& _pJVM,const ::rtl::OUString& _sClassName );
}

//==================================================================================

#define DECLARE_SERVICE_INFO()  \
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException); \
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException); \
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException) \

#define IMPLEMENT_SERVICE_INFO(classname, implasciiname, serviceasciiname)  \
    ::rtl::OUString SAL_CALL classname::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)   \
    {   \
        return ::rtl::OUString::createFromAscii(implasciiname); \
    }   \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL classname::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)  \
    {   \
        ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(1);   \
        aSupported[0] = ::rtl::OUString::createFromAscii(serviceasciiname); \
        return aSupported;  \
    }   \
    sal_Bool SAL_CALL classname::supportsService( const ::rtl::OUString& _rServiceName ) throw(::com::sun::star::uno::RuntimeException) \
    {   \
        Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());             \
        const ::rtl::OUString* pSupported = aSupported.getConstArray();                 \
        const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();              \
        for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)   \
            ;                                                                           \
                                                                                        \
        return pSupported != pEnd;                                                      \
    }   \

//==================================================================================

#endif // _CONNECTIVITY_COMMONTOOLS_HXX_

