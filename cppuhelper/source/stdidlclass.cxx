/*************************************************************************
 *
 *  $RCSfile: stdidlclass.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dbo $ $Date: 2002-06-14 13:20:20 $
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

#include <osl/mutex.hxx>

#include <cppuhelper/weakref.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/stdidlclass.hxx>

#include <com/sun/star/reflection/XIdlClassProvider.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::reflection;
using namespace rtl;

namespace cppu {

/*---------------------------------------------------------
*   This helper class implements XIdlClass. Is used by
*   createStdIdlClass()
*---------------------------------------------------------*/
class OStdIdlClass :
            public OWeakObject,
            public XIdlClass,
            public XIdlClassProvider
{
public:
    OStdIdlClass(
                    const Reference < XMultiServiceFactory > &rSMgr ,
                    const OUString & sImplementationName ,
                    const Reference < XIdlClass > & rSuperClass,
                    const Sequence < OUString > &seq
                ) SAL_THROW( () );

    // XInterface
    Any                 SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType )
        throw(::com::sun::star::uno::RuntimeException);

    void                SAL_CALL acquire() throw()   { OWeakObject::acquire(); }
    void                SAL_CALL release() throw()   { OWeakObject::release(); }

    // XIdlClassProvider
    Sequence< Reference < XIdlClass > > SAL_CALL getIdlClasses(void)
        throw (RuntimeException);

    // XIdlClass
    virtual Sequence< Reference< XIdlClass > > SAL_CALL getClasses(  ) throw(RuntimeException)
                                        { return Sequence < Reference < XIdlClass > > (); }
    virtual Reference< XIdlClass > SAL_CALL getClass( const ::rtl::OUString& aName ) throw(RuntimeException)
                                        { return Reference < XIdlClass > (); }
    virtual sal_Bool SAL_CALL equals( const Reference< XIdlClass >& Type ) throw(RuntimeException)
                                        { return getName() == Type->getName(); }
    virtual sal_Bool SAL_CALL isAssignableFrom( const Reference< XIdlClass >& xType ) throw(RuntimeException)
                                        { return equals( xType ); }
    virtual TypeClass SAL_CALL getTypeClass(  ) throw(RuntimeException)
                                        { return TypeClass_UNKNOWN; }
    virtual OUString SAL_CALL getName(  ) throw(RuntimeException)
                                        { return m_sImplementationName; }
    virtual Uik SAL_CALL getUik(  ) throw(RuntimeException)
                                        { return Uik(); }
    virtual Sequence< Reference< XIdlClass > > SAL_CALL getSuperclasses(  ) throw(RuntimeException)
                                        { return m_seqSuperClasses; }
    virtual Sequence< Reference< XIdlClass > > SAL_CALL getInterfaces(  ) throw(RuntimeException);

    virtual Reference< XIdlClass > SAL_CALL getComponentType(  ) throw(RuntimeException)
                                        { return Reference < XIdlClass > (); }
    virtual Reference< XIdlField > SAL_CALL getField( const ::rtl::OUString& aName ) throw(RuntimeException)
                                        { return Reference < XIdlField > (); }
    virtual Sequence< Reference< XIdlField > > SAL_CALL getFields(  ) throw(RuntimeException)
                                        { return Sequence< Reference < XIdlField > > (); }
    virtual Reference< XIdlMethod > SAL_CALL getMethod( const ::rtl::OUString& aName ) throw(RuntimeException)
                                        { return Reference < XIdlMethod > (); }
    virtual Sequence< Reference< XIdlMethod > > SAL_CALL getMethods(  ) throw(RuntimeException)
                                        { return Sequence < Reference < XIdlMethod > > (); }
    virtual Reference< XIdlArray > SAL_CALL getArray(  ) throw(RuntimeException)
                                        { return Reference < XIdlArray > (); }
    virtual void SAL_CALL createObject( Any& obj ) throw(RuntimeException) {}

private:
    OUString                                m_sImplementationName;
    Sequence < OUString >                   m_seqSupportedInterface;
    Sequence < Reference < XIdlClass > >    m_seqSuperClasses;
    Reference < XMultiServiceFactory >      m_rSMgr;

    Reference< XIdlReflection > m_xCorefl;
    Reference< XIdlReflection > const & get_corefl() SAL_THROW( (RuntimeException) );
};

Reference< XIdlReflection > const & OStdIdlClass::get_corefl()
    SAL_THROW( (RuntimeException) )
{
    if (! m_xCorefl.is())
    {
        if( m_rSMgr.is() )
        {
            Reference< beans::XPropertySet > xProps( m_rSMgr, UNO_QUERY );
            OSL_ASSERT( xProps.is() );
            if (xProps.is())
            {
                Reference< XComponentContext > xContext;
                xProps->getPropertyValue(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ) >>= xContext;
                OSL_ASSERT( xContext.is() );
                if (xContext.is())
                {
                    Reference < XIdlReflection > x;
                    xContext->getValueByName(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.reflection.theCoreReflection") ) ) >>= x;
                    OSL_ENSURE( x.is(), "### CoreReflection singleton not accessable!?" );

                    if (x.is())
                    {
                        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
                        if (! m_xCorefl.is())
                        {
                            m_xCorefl = x;
                        }
                    }
                }
            }
        }
        if (! m_xCorefl.is())
        {
            throw DeploymentException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.reflection.theCoreReflection singleton not accessable") ),
                Reference< XInterface >() );
        }
    }
    return m_xCorefl;
}

OStdIdlClass::OStdIdlClass(
                    const Reference < XMultiServiceFactory > &rSMgr ,
                    const OUString & sImplementationName ,
                    const Reference < XIdlClass > & rSuperClass,
                    const Sequence < OUString > &seq
                          ) SAL_THROW( () ) :
                m_rSMgr( rSMgr ) ,
                m_sImplementationName( sImplementationName ) ,
                m_seqSupportedInterface( seq )
{
    if( rSuperClass.is() )
        m_seqSuperClasses = Sequence< Reference < XIdlClass > >( &rSuperClass, 1 );

}

Any SAL_CALL OStdIdlClass::queryInterface( const Type & rType )
     throw(::com::sun::star::uno::RuntimeException)
{
    Any aRet( ::cppu::queryInterface(
        rType, static_cast< XIdlClass * >( this ), static_cast< XIdlClassProvider * >( this ) ) );

    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}


Sequence< Reference< XIdlClass > > SAL_CALL OStdIdlClass::getInterfaces(  ) throw(RuntimeException)
{
    int nMax = m_seqSupportedInterface.getLength();

    Reference< XIdlReflection > const & rCoreRefl = get_corefl();
    if( rCoreRefl.is() )
    {
        Sequence< Reference< XIdlClass > > seqClasses( nMax );

        for( int n = 0 ; n < nMax ; n++ )
        {
            seqClasses.getArray()[n] = rCoreRefl->forName( m_seqSupportedInterface.getArray()[n] );
        }

        return seqClasses;
    }
    return Sequence< Reference< XIdlClass > > () ;
}


// XIdlClassProvider
Sequence< Reference < XIdlClass > > SAL_CALL OStdIdlClass::getIdlClasses(void)
    throw (RuntimeException)
{
    // weak reference to cache the standard class
    static WeakReference< XIdlClass >   weakRef;

    // try to make weakref hard
    Reference < XIdlClass > r = weakRef;

    if( ! r.is() ) {
        // xidlclass has not been initialized before or has been destroyed already.
        r = ::cppu::createStandardClass(
                                        m_rSMgr ,
                                        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.cppuhelper.OStdIdlClass") ) ,
                                        Reference < XIdlClass > () ,
                                          SAL_STATIC_CAST( XIdlClassProvider * , this ) ,
                                          SAL_STATIC_CAST( XIdlClass * , this )
                                         );

        // store reference for later use
        weakRef = r;
    }

    return Sequence < Reference < XIdlClass > > ( &r , 1 );
}




// external constructor
XIdlClass *  SAL_CALL createStandardClassWithSequence(
                    const Reference < XMultiServiceFactory > &rSMgr ,
                    const OUString & sImplementationName ,
                    const Reference < XIdlClass > & rSuperClass,
                    const Sequence < OUString > &seqInterfaceNames )
    SAL_THROW( () )
{
    return SAL_STATIC_CAST(
                        XIdlClass * ,
                        new OStdIdlClass(
                                            rSMgr ,
                                            sImplementationName,
                                            rSuperClass,
                                            seqInterfaceNames
                                          )
                       );
}

} //end namespace cppu
