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
#ifndef _CPPUHELPER_STDIDLCLASS_HXX_
#define _CPPUHELPER_STDIDLCLASS_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>


namespace cppu {

/*
  @deprecated
*/
::com::sun::star::reflection::XIdlClass * SAL_CALL createStandardClassWithSequence(
    const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > &rSMgr ,
    const ::rtl::OUString & sImplementationName ,
    const ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass > & rSuperClass,
    const ::com::sun::star::uno::Sequence < ::rtl::OUString > &seq )
    SAL_THROW( () );



/**
   Standard function to create an XIdlClass for a component.
   There is a function for each number of supported interfaces up to 10.

   Since the switch to the final component model, there are no use cases anymore where
   these functions should be used. Instead use the implementation helpers directly
   (see cppuhelper/implbase1.hxx).

   @see OTypeCollection

   @deprecated
 */
template < class Interface1 >
inline ::com::sun::star::reflection::XIdlClass * SAL_CALL
//inline ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass >
createStandardClass(    const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > &rSMgr,
                         const ::rtl::OUString &sImplementationName  ,
                         const ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass > &rSuperClass ,
                         const Interface1 *
                     )
    SAL_THROW( () )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(1);
    seqInterface.getArray()[0] = Interface1::static_type().getTypeName();
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
                         const Interface1 *,
                         const Interface2 *
                     )
    SAL_THROW( () )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(2);
    seqInterface.getArray()[0] = Interface1::static_type().getTypeName();
    seqInterface.getArray()[1] = Interface2::static_type().getTypeName();
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
                         const Interface1 *,
                         const Interface2 *,
                         const Interface3 *
                     )
    SAL_THROW( () )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(3);
    seqInterface.getArray()[0] = Interface1::static_type().getTypeName();
    seqInterface.getArray()[1] = Interface2::static_type().getTypeName();
    seqInterface.getArray()[2] = Interface3::static_type().getTypeName();
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
                         const Interface1 *,
                         const Interface2 *,
                         const Interface3 *,
                         const Interface4 *
                     )
    SAL_THROW( () )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(4);
    seqInterface.getArray()[0] = Interface1::static_type().getTypeName();
    seqInterface.getArray()[1] = Interface2::static_type().getTypeName();
    seqInterface.getArray()[2] = Interface3::static_type().getTypeName();
    seqInterface.getArray()[3] = Interface4::static_type().getTypeName();
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
                         const Interface1 *,
                         const Interface2 *,
                         const Interface3 *,
                         const Interface4 *,
                         const Interface5 *
                     )
    SAL_THROW( () )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(5);
    seqInterface.getArray()[0] = Interface1::static_type().getTypeName();
    seqInterface.getArray()[1] = Interface2::static_type().getTypeName();
    seqInterface.getArray()[2] = Interface3::static_type().getTypeName();
    seqInterface.getArray()[3] = Interface4::static_type().getTypeName();
    seqInterface.getArray()[4] = Interface5::static_type().getTypeName();
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
                         const Interface1 *,
                         const Interface2 *,
                         const Interface3 *,
                         const Interface4 *,
                         const Interface5 *,
                         const Interface6 *
                     )
    SAL_THROW( () )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(6);
    seqInterface.getArray()[0] = Interface1::static_type().getTypeName();
    seqInterface.getArray()[1] = Interface2::static_type().getTypeName();
    seqInterface.getArray()[2] = Interface3::static_type().getTypeName();
    seqInterface.getArray()[3] = Interface4::static_type().getTypeName();
    seqInterface.getArray()[4] = Interface5::static_type().getTypeName();
    seqInterface.getArray()[5] = Interface6::static_type().getTypeName();
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
                         const Interface1 *,
                         const Interface2 *,
                         const Interface3 *,
                         const Interface4 *,
                         const Interface5 *,
                         const Interface6 *,
                         const Interface7 *
                     )
    SAL_THROW( () )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(7);
    seqInterface.getArray()[0] = Interface1::static_type().getTypeName();
    seqInterface.getArray()[1] = Interface2::static_type().getTypeName();
    seqInterface.getArray()[2] = Interface3::static_type().getTypeName();
    seqInterface.getArray()[3] = Interface4::static_type().getTypeName();
    seqInterface.getArray()[4] = Interface5::static_type().getTypeName();
    seqInterface.getArray()[5] = Interface6::static_type().getTypeName();
    seqInterface.getArray()[6] = Interface7::static_type().getTypeName();
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
                         const Interface1 *,
                         const Interface2 *,
                         const Interface3 *,
                         const Interface4 *,
                         const Interface5 *,
                         const Interface6 *,
                         const Interface7 *,
                         const Interface8 *
                     )
    SAL_THROW( () )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(8);
    seqInterface.getArray()[0] = Interface1::static_type().getTypeName();
    seqInterface.getArray()[1] = Interface2::static_type().getTypeName();
    seqInterface.getArray()[2] = Interface3::static_type().getTypeName();
    seqInterface.getArray()[3] = Interface4::static_type().getTypeName();
    seqInterface.getArray()[4] = Interface5::static_type().getTypeName();
    seqInterface.getArray()[5] = Interface6::static_type().getTypeName();
    seqInterface.getArray()[6] = Interface7::static_type().getTypeName();
    seqInterface.getArray()[7] = Interface8::static_type().getTypeName();
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
                         const Interface1 *,
                         const Interface2 *,
                         const Interface3 *,
                         const Interface4 *,
                         const Interface5 *,
                         const Interface6 *,
                         const Interface7 *,
                         const Interface8 *,
                         const Interface9 *
                     )
    SAL_THROW( () )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(9);
    seqInterface.getArray()[0] = Interface1::static_type().getTypeName();
    seqInterface.getArray()[1] = Interface2::static_type().getTypeName();
    seqInterface.getArray()[2] = Interface3::static_type().getTypeName();
    seqInterface.getArray()[3] = Interface4::static_type().getTypeName();
    seqInterface.getArray()[4] = Interface5::static_type().getTypeName();
    seqInterface.getArray()[5] = Interface6::static_type().getTypeName();
    seqInterface.getArray()[6] = Interface7::static_type().getTypeName();
    seqInterface.getArray()[7] = Interface8::static_type().getTypeName();
    seqInterface.getArray()[8] = Interface9::static_type().getTypeName();
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
                         const Interface1 *,
                         const Interface2 *,
                         const Interface3 *,
                         const Interface4 *,
                         const Interface5 *,
                         const Interface6 *,
                         const Interface7 *,
                         const Interface8 *,
                         const Interface9 *,
                         const Interface10 *
                     )
    SAL_THROW( () )
{
    ::com::sun::star::uno::Sequence < ::rtl::OUString > seqInterface(10);
    seqInterface.getArray()[0] = Interface1::static_type().getTypeName();
    seqInterface.getArray()[1] = Interface2::static_type().getTypeName();
    seqInterface.getArray()[2] = Interface3::static_type().getTypeName();
    seqInterface.getArray()[3] = Interface4::static_type().getTypeName();
    seqInterface.getArray()[4] = Interface5::static_type().getTypeName();
    seqInterface.getArray()[5] = Interface6::static_type().getTypeName();
    seqInterface.getArray()[6] = Interface7::static_type().getTypeName();
    seqInterface.getArray()[7] = Interface8::static_type().getTypeName();
    seqInterface.getArray()[8] = Interface9::static_type().getTypeName();
    seqInterface.getArray()[9] = Interface10::static_type().getTypeName();
    return createStandardClassWithSequence(
                                            rSMgr,
                                            sImplementationName,
                                            rSuperClass,
                                            seqInterface
                                          );
}

} // end namespace cppu

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
