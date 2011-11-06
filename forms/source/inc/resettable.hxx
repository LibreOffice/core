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



#ifndef FORMS_RESETTABLE_HXX
#define FORMS_RESETTABLE_HXX

/** === begin UNO includes === **/
#include <com/sun/star/form/XResetListener.hpp>
/** === end UNO includes === **/

#include <cppuhelper/interfacecontainer.hxx>

namespace cppu
{
    class OWeakObject;
}

//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= ResetHelper
    //====================================================================
    class ResetHelper
    {
    public:
        ResetHelper( ::cppu::OWeakObject& _parent, ::osl::Mutex& _mutex )
            :m_rParent( _parent )
            ,m_aResetListeners( _mutex )
        {
        }

        // XReset equivalents
        void addResetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener >& _listener );
        void removeResetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener >& _listener );

        // calling listeners
        bool approveReset();
        void notifyResetted();
        void disposing();

    private:
        ::cppu::OWeakObject&                m_rParent;
        ::cppu::OInterfaceContainerHelper   m_aResetListeners;
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_RESETTABLE_HXX
