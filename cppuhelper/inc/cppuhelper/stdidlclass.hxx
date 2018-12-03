/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _CPPUHELPER_STDIDLCLASS_HXX_
#define _CPPUHELPER_STDIDLCLASS_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>

#include "cppuhelper/cppuhelperdllapi.h"

namespace cppu {

/*
  @deprecated
*/
CPPUHELPER_DLLPUBLIC
::com::sun::star::reflection::XIdlClass * SAL_CALL createStandardClassWithSequence(
    const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > &rSMgr ,
    const ::rtl::OUString & sImplementationName ,
    const ::com::sun::star::uno::Reference < ::com::sun::star::reflection::XIdlClass > & rSuperClass,
    const ::com::sun::star::uno::Sequence < ::rtl::OUString > &seq )
    SAL_THROW( () );



/**
   Standardfunction to create an XIdlClass for a component.
   There is a function for each number of supported interfaces up to 10.

   Since the switch to the final component model, there are no use cases anymore, where
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
