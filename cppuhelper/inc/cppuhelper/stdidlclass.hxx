/*************************************************************************
 *
 *  $RCSfile: stdidlclass.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:26:09 $
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
#ifndef _CPPUHELPER_STDIDLCLASS_HXX_
#define _CPPUHELPER_STDIDLCLASS_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>


namespace cppu {

/*****
*
* Standardfunction to create a XIdlClass for a component.
* This method is usually used as a helper class. ( Better use the methods below  ).
*
* HACK : Function returns pointer to XIdlClass instead of
*        a reference. This is because of a MSC 4.x bug.
*
******/
::com::sun::star::reflection::XIdlClass * SAL_CALL createStandardClassWithSequence(
    const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > &rSMgr ,
    const ::rtl::OUString & sImplementationName ,
    const ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass > & rSuperClass,
    const ::com::sun::star::uno::Sequence < ::rtl::OUString > &seq );



/*--------------------------------------------------
* Standardfunction to create an XIdlClass for a component. There is a function for each number of supported
* interfaces up to 10.
*
* HACK : Function returns pointer to XIdlClass instead of
*        a reference. This is because of a MSC 4.x bug.
*
* Sample use for an XIdlClassProvider :
*
* // XIdlClassProvider
*Sequence< Reference < XIdlClass > > SAL_CALL MyComponent::getIdlClasses(void)
*{
*   // weak reference to cache the standard class
*   static WeakReference< XIdlClass >   weakRef;
*
*   // try to make weakref hard
*   Reference < XIdlClass > r = weakRef;
*
*   if( ! r.is() ) {
*       // xidlclass has not been initialized before or has been destroyed already.
*       r = ::cppu::createStandardClass(
*                                       m_rSMgr ,                               // Servicemanager
*                                       L"com.sun.star.comp.cppuhelper.OStdIdlClass" ,  // ImplementationName
*                                       Reference < XIdlClass > () ,            // Base classes
*                                       STATIC_CAST(XIdlClassProvider *, this),                 // Supported Interfaces
*                                       STATIC_CAST(XServiceInfo *, this ) ,
*                                       STATIC_CAST(XMyService * , this )
*                                       // ....
*                                       );
*
*       // store reference for later use
*       weakRef = r;
*   }
*
*   return Sequence < Reference < XIdlClass > > ( &r , 1 );
*}
*
*--------------------------------------------------*/
template < class Interface1 >
inline ::com::sun::star::reflection::XIdlClass * SAL_CALL
//inline ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass >
createStandardClass(    const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > &rSMgr,
                         const ::rtl::OUString &sImplementationName  ,
                         const ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass > &rSuperClass ,
                         const Interface1 *p1
                     )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(1);
    seqInterface.getArray()[0] =
        ::getCppuType( ( ::com::sun::star::uno::Reference< Interface1 > *) 0 ).getTypeName();

    return createStandardClassWithSequence(
                                            rSMgr,
                                            sImplementationName,
                                            rSuperClass,
                                            seqInterface
                                          );
}



template < class Interface1, class Interface2 >
inline ::com::sun::star::reflection::XIdlClass * SAL_CALL
//inline ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass >
createStandardClass(    const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > &rSMgr,
                         const ::rtl::OUString &sImplementationName  ,
                         const ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass > &rSuperClass ,
                         const Interface1 *p1 ,
                         const Interface2 *p2
                     )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(2);
    seqInterface.getArray()[0] =
        ::getCppuType( ( ::com::sun::star::uno::Reference< Interface1 > *) 0 ).getTypeName();

    seqInterface.getArray()[1] =
        ::getCppuType( ( ::com::sun::star::uno::Reference< Interface2 > *) 0 ).getTypeName();

    return createStandardClassWithSequence(
                                            rSMgr,
                                            sImplementationName,
                                            rSuperClass,
                                            seqInterface
                                          );
}





template < class Interface1, class Interface2 , class Interface3 >
inline ::com::sun::star::reflection::XIdlClass * SAL_CALL
//inline ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass >
createStandardClass(    const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > &rSMgr,
                         const ::rtl::OUString &sImplementationName  ,
                         const ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass > &rSuperClass ,
                         const Interface1 *p1 ,
                         const Interface2 *p2 ,
                         const Interface3 *p3
                     )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(3);
    seqInterface.getArray()[0] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface1 > *) 0 ).getTypeName();

    seqInterface.getArray()[1] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface2 > *) 0 ).getTypeName();

    seqInterface.getArray()[2] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface3 > *) 0 ).getTypeName();

    return createStandardClassWithSequence(
                                            rSMgr,
                                            sImplementationName,
                                            rSuperClass,
                                            seqInterface
                                          );
}





template < class Interface1, class Interface2 , class Interface3 , class Interface4 >
inline ::com::sun::star::reflection::XIdlClass * SAL_CALL
//inline ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass >
createStandardClass(    const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > &rSMgr,
                         const ::rtl::OUString &sImplementationName  ,
                         const ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass > &rSuperClass ,
                         const Interface1 *p1 ,
                         const Interface2 *p2 ,
                         const Interface3 *p3 ,
                         const Interface4 *p4
                     )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(4);
    seqInterface.getArray()[0] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface1 > *) 0 ).getTypeName();

    seqInterface.getArray()[1] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface2 > *) 0 ).getTypeName();

    seqInterface.getArray()[2] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface3 > *) 0 ).getTypeName();

    seqInterface.getArray()[3] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface4 > *) 0 ).getTypeName();

    return createStandardClassWithSequence(
                                            rSMgr,
                                            sImplementationName,
                                            rSuperClass,
                                            seqInterface
                                          );
}







template < class Interface1, class Interface2 , class Interface3 , class Interface4 , class Interface5 >
inline ::com::sun::star::reflection::XIdlClass * SAL_CALL
//inline ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass >
createStandardClass(    const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > &rSMgr,
                         const ::rtl::OUString &sImplementationName  ,
                         const ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass > &rSuperClass ,
                         const Interface1 *p1 ,
                         const Interface2 *p2 ,
                         const Interface3 *p3 ,
                         const Interface4 *p4 ,
                         const Interface5 *p5
                     )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(5);
    seqInterface.getArray()[0] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface1 > *) 0 ).getTypeName();

    seqInterface.getArray()[1] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface2 > *) 0 ).getTypeName();

    seqInterface.getArray()[2] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface3 > *) 0 ).getTypeName();

    seqInterface.getArray()[3] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface4 > *) 0 ).getTypeName();

    seqInterface.getArray()[4] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface5 > *) 0 ).getTypeName();

    return createStandardClassWithSequence(
                                            rSMgr,
                                            sImplementationName,
                                            rSuperClass,
                                            seqInterface
                                          );
}




template <  class Interface1, class Interface2 , class Interface3 , class Interface4 , class Interface5 ,
            class Interface6 >
inline ::com::sun::star::reflection::XIdlClass * SAL_CALL
//inline ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass >
createStandardClass(    const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > &rSMgr,
                         const ::rtl::OUString &sImplementationName  ,
                         const ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass > &rSuperClass ,
                         const Interface1 *p1 ,
                         const Interface2 *p2 ,
                         const Interface3 *p3 ,
                         const Interface4 *p4 ,
                         const Interface5 *p5 ,
                         const Interface6 *p6
                     )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(6);
    seqInterface.getArray()[0] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface1 > *) 0 ).getTypeName();

    seqInterface.getArray()[1] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface2 > *) 0 ).getTypeName();

    seqInterface.getArray()[2] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface3 > *) 0 ).getTypeName();

    seqInterface.getArray()[3] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface4 > *) 0 ).getTypeName();

    seqInterface.getArray()[4] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface5 > *) 0 ).getTypeName();

    seqInterface.getArray()[5] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface6 > *) 0 ).getTypeName();

    return createStandardClassWithSequence(
                                            rSMgr,
                                            sImplementationName,
                                            rSuperClass,
                                            seqInterface
                                          );
}





template <  class Interface1, class Interface2 , class Interface3 , class Interface4 , class Interface5 ,
            class Interface6, class Interface7 >
inline ::com::sun::star::reflection::XIdlClass * SAL_CALL
//inline ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass >
createStandardClass(    const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > &rSMgr,
                         const ::rtl::OUString &sImplementationName  ,
                         const ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass > &rSuperClass ,
                         const Interface1 *p1 ,
                         const Interface2 *p2 ,
                         const Interface3 *p3 ,
                         const Interface4 *p4 ,
                         const Interface5 *p5 ,
                         const Interface6 *p6 ,
                         const Interface7 *p7
                     )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(7);
    seqInterface.getArray()[0] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface1 > *) 0 ).getTypeName();

    seqInterface.getArray()[1] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface2 > *) 0 ).getTypeName();

    seqInterface.getArray()[2] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface3 > *) 0 ).getTypeName();

    seqInterface.getArray()[3] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface4 > *) 0 ).getTypeName();

    seqInterface.getArray()[4] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface5 > *) 0 ).getTypeName();

    seqInterface.getArray()[5] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface6 > *) 0 ).getTypeName();

    seqInterface.getArray()[6] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface7 > *) 0 ).getTypeName();

    return createStandardClassWithSequence(
                                            rSMgr,
                                            sImplementationName,
                                            rSuperClass,
                                            seqInterface
                                          );
}






template <  class Interface1, class Interface2, class Interface3 , class Interface4 , class Interface5 ,
            class Interface6, class Interface7, class Interface8 >
inline ::com::sun::star::reflection::XIdlClass * SAL_CALL
//inline ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass >
createStandardClass(    const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > &rSMgr,
                         const ::rtl::OUString &sImplementationName  ,
                         const ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass > &rSuperClass ,
                         const Interface1 *p1 ,
                         const Interface2 *p2 ,
                         const Interface3 *p3 ,
                         const Interface4 *p4 ,
                         const Interface5 *p5 ,
                         const Interface6 *p6 ,
                         const Interface7 *p7 ,
                         const Interface8 *p8
                     )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(8);
    seqInterface.getArray()[0] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface1 > *) 0 ).getTypeName();

    seqInterface.getArray()[1] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface2 > *) 0 ).getTypeName();

    seqInterface.getArray()[2] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface3 > *) 0 ).getTypeName();

    seqInterface.getArray()[3] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface4 > *) 0 ).getTypeName();

    seqInterface.getArray()[4] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface5 > *) 0 ).getTypeName();

    seqInterface.getArray()[5] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface6 > *) 0 ).getTypeName();

    seqInterface.getArray()[6] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface7 > *) 0 ).getTypeName();

    seqInterface.getArray()[7] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface8 > *) 0 ).getTypeName();

    return createStandardClassWithSequence(
                                            rSMgr,
                                            sImplementationName,
                                            rSuperClass,
                                            seqInterface
                                          );
}




template <  class Interface1, class Interface2, class Interface3 , class Interface4 , class Interface5 ,
            class Interface6, class Interface7, class Interface8 , class Interface9 >
inline ::com::sun::star::reflection::XIdlClass * SAL_CALL
//inline ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass >
createStandardClass(    const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > &rSMgr,
                         const ::rtl::OUString &sImplementationName  ,
                         const ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass > &rSuperClass ,
                         const Interface1 *p1 ,
                         const Interface2 *p2 ,
                         const Interface3 *p3 ,
                         const Interface4 *p4 ,
                         const Interface5 *p5 ,
                         const Interface6 *p6 ,
                         const Interface7 *p7 ,
                         const Interface8 *p8 ,
                         const Interface9 *p9
                     )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(9);
    seqInterface.getArray()[0] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface1 > *) 0 ).getTypeName();

    seqInterface.getArray()[1] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface2 > *) 0 ).getTypeName();

    seqInterface.getArray()[2] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface3 > *) 0 ).getTypeName();

    seqInterface.getArray()[3] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface4 > *) 0 ).getTypeName();

    seqInterface.getArray()[4] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface5 > *) 0 ).getTypeName();

    seqInterface.getArray()[5] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface6 > *) 0 ).getTypeName();

    seqInterface.getArray()[6] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface7 > *) 0 ).getTypeName();

    seqInterface.getArray()[7] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface8 > *) 0 ).getTypeName();

    seqInterface.getArray()[8] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface9 > *) 0 ).getTypeName();

    return createStandardClassWithSequence(
                                            rSMgr,
                                            sImplementationName,
                                            rSuperClass,
                                            seqInterface
                                          );
}


template <  class Interface1, class Interface2, class Interface3 , class Interface4 , class Interface5 ,
            class Interface6, class Interface7, class Interface8 , class Interface9 , class Interface10 >
inline ::com::sun::star::reflection::XIdlClass * SAL_CALL
//inline ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass >
createStandardClass(    const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > &rSMgr,
                         const ::rtl::OUString &sImplementationName  ,
                         const ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass > &rSuperClass ,
                         const Interface1 *p1 ,
                         const Interface2 *p2 ,
                         const Interface3 *p3 ,
                         const Interface4 *p4 ,
                         const Interface5 *p5 ,
                         const Interface6 *p6 ,
                         const Interface7 *p7 ,
                         const Interface8 *p8 ,
                         const Interface9 *p9 ,
                         const Interface10 *p10
                     )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(10);
    seqInterface.getArray()[0] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface1 > *) 0 ).getTypeName();

    seqInterface.getArray()[1] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface2 > *) 0 ).getTypeName();

    seqInterface.getArray()[2] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface3 > *) 0 ).getTypeName();

    seqInterface.getArray()[3] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface4 > *) 0 ).getTypeName();

    seqInterface.getArray()[4] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface5 > *) 0 ).getTypeName();

    seqInterface.getArray()[5] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface6 > *) 0 ).getTypeName();

    seqInterface.getArray()[6] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface7 > *) 0 ).getTypeName();

    seqInterface.getArray()[7] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface8 > *) 0 ).getTypeName();

    seqInterface.getArray()[8] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface9 > *) 0 ).getTypeName();

    seqInterface.getArray()[9] =
        ::getCppuType( (::com::sun::star::uno::Reference< Interface10 > *) 0 ).getTypeName();

    return createStandardClassWithSequence(
                                            rSMgr,
                                            sImplementationName,
                                            rSuperClass,
                                            seqInterface
                                          );
}

} // end namespace cppu

#endif
