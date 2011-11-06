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



#ifndef EXTENSIONS_PROPERTYCONTROLEXTENDER_HXX
#define EXTENSIONS_PROPERTYCONTROLEXTENDER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/inspection/XPropertyControl.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>

#include <memory>

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= PropertyControlExtender
    //====================================================================
    struct PropertyControlExtender_Data;
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::awt::XKeyListener
                                    >   PropertyControlExtender_Base;
    class PropertyControlExtender : public PropertyControlExtender_Base
    {
    public:
        PropertyControlExtender(
            const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& _rxObservedControl
        );

        // XKeyListener
        virtual void SAL_CALL keyPressed( const ::com::sun::star::awt::KeyEvent& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL keyReleased( const ::com::sun::star::awt::KeyEvent& e ) throw (::com::sun::star::uno::RuntimeException);
        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~PropertyControlExtender();

    private:
        ::std::auto_ptr< PropertyControlExtender_Data > m_pData;
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_PROPERTYCONTROLEXTENDER_HXX
