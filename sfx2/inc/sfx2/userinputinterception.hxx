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



#ifndef USERINPUTINTERCEPTION_HXX
#define USERINPUTINTERCEPTION_HXX

#include "sfx2/dllapi.h"

/** === begin UNO includes === **/
#include <com/sun/star/awt/XUserInputInterception.hpp>
/** === end UNO includes === **/

#include <boost/noncopyable.hpp>
#include <memory>

class NotifyEvent;

namespace cppu { class OWeakObject; }

//........................................................................
namespace sfx2
{
//........................................................................

    //====================================================================
    //= UserInputInterception
    //====================================================================
    struct UserInputInterception_Data;
    /** helper class for implementing the XUserInputInterception interface
        for a controller implementation
    */
    class SFX2_DLLPUBLIC UserInputInterception : public ::boost::noncopyable
    {
    public:
        UserInputInterception( ::cppu::OWeakObject& _rControllerImpl, ::osl::Mutex& _rMutex );
        ~UserInputInterception();

        // delegator functions for your XUserInputInterception implementation
        void    addKeyHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException);
        void    removeKeyHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException);
        void    addMouseClickHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseClickHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException);
        void    removeMouseClickHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseClickHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException);

        // state
        bool    hasKeyHandlers() const;
        bool    hasMouseClickListeners() const;

        // forwarding a NotifyEvent to the KeyListeners respectively MouseClickListeners
        bool    handleNotifyEvent( const NotifyEvent& _rEvent );

    private:
        ::std::auto_ptr< UserInputInterception_Data >   m_pData;
    };

//........................................................................
} // namespace sfx2
//........................................................................

#endif // USERINPUTINTERCEPTION_HXX
