/*************************************************************************
 *
 *  $RCSfile: implbase6.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-15 15:47:29 $
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
#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#define _CPPUHELPER_IMPLBASE6_HXX_

#ifndef _CPPUHELPER_IMPLBASE_HXX_
#include <cppuhelper/implbase.hxx>
#endif

/*
#define __IFC6 Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6
#define __CLASS_IFC6 class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5, class Ifc6
#define __PUBLIC_IFC6 public Ifc1, public Ifc2, public Ifc3, public Ifc4, public Ifc5, public Ifc6
__DEF_IMPLHELPER_PRE( 6 )
    __IFC_WRITEOFFSET( 1 ) __IFC_WRITEOFFSET( 2 ) __IFC_WRITEOFFSET( 3 ) __IFC_WRITEOFFSET( 4 )
    __IFC_WRITEOFFSET( 5 ) __IFC_WRITEOFFSET( 6 )
__DEF_IMPLHELPER_POST( 6 )
*/

namespace cppu
{
    struct ClassData6 : public ClassDataBase
    {
        Type_Offset arType2Offset[ 6 ];
        ClassData6( sal_Int32 nClassCode ) SAL_THROW( () )
            : ClassDataBase( nClassCode )
            {}
    };
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5, class Ifc6 >
    class ImplHelperBase6
        : public ::com::sun::star::lang::XTypeProvider
        , public Ifc1, public Ifc2, public Ifc3, public Ifc4, public Ifc5, public Ifc6
    {
    protected:
        ClassData & SAL_CALL getClassData( ClassDataBase & s_aCD ) SAL_THROW( () )
            {
                ClassData & rCD = * static_cast< ClassData * >( &s_aCD );
                if (! rCD.bOffsetsInit)
                {
                    ::osl::MutexGuard aGuard( getImplHelperInitMutex() );
                    if (! rCD.bOffsetsInit)
                    {
                        char * pBase = (char *)this;
                        rCD.writeTypeOffset( ::getCppuType( (const ::com::sun::star::uno::Reference< Ifc1 > *)0 ),
                                             (char *)(Ifc1 *)this - pBase );
                        rCD.writeTypeOffset( ::getCppuType( (const ::com::sun::star::uno::Reference< Ifc2 > *)0 ),
                                             (char *)(Ifc2 *)this - pBase );
                        rCD.writeTypeOffset( ::getCppuType( (const ::com::sun::star::uno::Reference< Ifc3 > *)0 ),
                                             (char *)(Ifc3 *)this - pBase );
                        rCD.writeTypeOffset( ::getCppuType( (const ::com::sun::star::uno::Reference< Ifc4 > *)0 ),
                                             (char *)(Ifc4 *)this - pBase );
                        rCD.writeTypeOffset( ::getCppuType( (const ::com::sun::star::uno::Reference< Ifc5 > *)0 ),
                                             (char *)(Ifc5 *)this - pBase );
                        rCD.writeTypeOffset( ::getCppuType( (const ::com::sun::star::uno::Reference< Ifc6 > *)0 ),
                                             (char *)(Ifc6 *)this - pBase );
                        rCD.bOffsetsInit = sal_True;
                    }
                }
                return rCD;
            }
    };
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5, class Ifc6 >
    class ImplHelper6
        : public ImplHelperBase6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 >
    {
        static ClassData6 s_aCD;
    public:
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException)
            { return getClassData( s_aCD ).query( rType, (ImplHelperBase6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 > *)this ); }
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException)
            { return getClassData( s_aCD ).getTypes(); }
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException)
            { return getClassData( s_aCD ).getImplementationId(); }
    };
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5, class Ifc6 >
    class WeakImplHelper6
        : public ::cppu::OWeakObject
        , public ImplHelperBase6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 >
    {
        static ClassData6 s_aCD;
    public:
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException)
            {
                ::com::sun::star::uno::Any aRet( getClassData( s_aCD ).query( rType, (ImplHelperBase6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 > *)this ) );
                return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
            }
        virtual void SAL_CALL acquire() throw ()
            { OWeakObject::acquire(); }
        virtual void SAL_CALL release() throw ()
            { OWeakObject::release(); }
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException)
            { return getClassData( s_aCD ).getTypes(); }
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException)
            { return getClassData( s_aCD ).getImplementationId(); }
    };
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5, class Ifc6 >
    class WeakAggImplHelper6
        : public ::cppu::OWeakAggObject
        , public ImplHelperBase6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 >
    {
        static ClassData6 s_aCD;
    public:
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException)
            { return OWeakAggObject::queryInterface( rType ); }
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException)
            {
                ::com::sun::star::uno::Any aRet( getClassData( s_aCD ).query( rType, (ImplHelperBase6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 > *)this ) );
                return (aRet.hasValue() ? aRet : OWeakAggObject::queryAggregation( rType ));
            }
        virtual void SAL_CALL acquire() throw ()
            { OWeakAggObject::acquire(); }
        virtual void SAL_CALL release() throw ()
            { OWeakAggObject::release(); }
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException)
            { return getClassData( s_aCD ).getTypes(); }
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException)
            { return getClassData( s_aCD ).getImplementationId(); }
    };

#ifndef MACOSX
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5, class Ifc6 >
    ClassData6 ImplHelper6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 >::s_aCD = ClassData6( 0 );
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5, class Ifc6 >
    ClassData6 WeakImplHelper6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 >::s_aCD = ClassData6( 1 );
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5, class Ifc6 >
    ClassData6 WeakAggImplHelper6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 >::s_aCD = ClassData6( 2 );
#endif

}

#endif
