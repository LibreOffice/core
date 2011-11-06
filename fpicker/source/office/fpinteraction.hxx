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



#ifndef SVTOOLS_FILEPICKER_INTERACTION_HXX
#define SVTOOLS_FILEPICKER_INTERACTION_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= OFilePickerInteractionHandler
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::task::XInteractionHandler
                                    >   OFilePickerInteractionHandler_Base;

    /** a InteractionHandler implementation which extends another handler with some customizability
    */
    class OFilePickerInteractionHandler : public OFilePickerInteractionHandler_Base
    {
    public:
        /** flags, which indicates special handled interactions
            These values will be used combained as flags - so they must
            in range [2^n]!
         */
        enum EInterceptedInteractions
        {
            E_NOINTERCEPTION = 0,
            E_DOESNOTEXIST   = 1
            // next values [2,4,8,16 ...]!
        };

    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > m_xMaster        ; // our master handler
        ::com::sun::star::uno::Any                                                      m_aException     ; // the last handled request
        sal_Bool                                                                        m_bUsed          ; // indicates using of this interaction handler instance
        EInterceptedInteractions                                                        m_eInterceptions ; // enable/disable interception of some special interactions

    public:
        OFilePickerInteractionHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxMaster );

        // some generic functions
        void     enableInterceptions( EInterceptedInteractions eInterceptions );
        sal_Bool wasUsed            () const;
        void     resetUseState      ();
        void     forgetRequest      ();

        // functions to analyze last cached request
        sal_Bool wasAccessDenied() const;

    protected:
        // XInteractionHandler
        virtual void SAL_CALL handle( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& _rxRequest ) throw (::com::sun::star::uno::RuntimeException);

    private:
        ~OFilePickerInteractionHandler();
    };

//........................................................................
}   // namespace svt
//........................................................................

#endif // SVTOOLS_FILEPICKER_INTERACTION_HXX

