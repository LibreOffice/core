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



#ifndef _COMPHELPER_SYNCHRONOUSDISPATCH_HXX
#define _COMPHELPER_SYNCHRONOUSDISPATCH_HXX

#include "comphelper/comphelperdllapi.h"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"

#include "com/sun/star/beans/PropertyValue.hpp"

namespace com { namespace sun { namespace star {
    namespace uno {
        class XInterface; }
    namespace lang {
        class XComponent; }
} } }

namespace rtl { class OUString; }

//........................................................................
namespace comphelper
{
//........................................................................

    //====================================================================
    //= SynchronousDispatch
    //====================================================================
    /** a helper class for working with the dispatch interface replacing
        loadComponentFromURL
    */
    class SynchronousDispatch
    {
    public:
        static COMPHELPER_DLLPUBLIC com::sun::star::uno::Reference< com::sun::star::lang::XComponent > dispatch(
            const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > &xStartPoint,
            const rtl::OUString &sURL,
            const rtl::OUString &sTarget,
            const sal_Int32 nFlags,
            const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > &lArguments );
    };

//........................................................................
} // namespace comphelper
//........................................................................

#endif // _COMPHELPER_SYNCHRONOUSDISPATCH_HXX

