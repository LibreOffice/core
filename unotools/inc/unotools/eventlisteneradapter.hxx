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


#include "unotools/unotoolsdllapi.h"

#ifndef _UNOTOOLS_EVENTLISTENERADAPTER_HXX_
#define _UNOTOOLS_EVENTLISTENERADAPTER_HXX_
#include <com/sun/star/lang/XComponent.hpp>

//.........................................................................
namespace utl
{
//.........................................................................

    struct OEventListenerAdapterImpl;
    //=====================================================================
    //= OEventListenerAdapter
    //=====================================================================
    /** base class for non-UNO dispose listeners
    */
    class UNOTOOLS_DLLPUBLIC OEventListenerAdapter
    {
        friend class OEventListenerImpl;

    private:
        UNOTOOLS_DLLPRIVATE OEventListenerAdapter( const OEventListenerAdapter& _rSource ); // never implemented
        UNOTOOLS_DLLPRIVATE const OEventListenerAdapter& operator=( const OEventListenerAdapter& _rSource );    // never implemented

    protected:
        OEventListenerAdapterImpl*  m_pImpl;

    protected:
                OEventListenerAdapter();
        virtual ~OEventListenerAdapter();

        void startComponentListening( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _rxComp );
        void stopComponentListening( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _rxComp );
        void stopAllComponentListening(  );

        virtual void _disposing( const ::com::sun::star::lang::EventObject& _rSource ) = 0;
    };

//.........................................................................
}   // namespace utl
//.........................................................................

#endif // _UNOTOOLS_EVENTLISTENERADAPTER_HXX_

