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



#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPEVENTTRANSLATION_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_PROPEVENTTRANSLATION_HXX

/** === begin UNO includes === **/
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
/** === end UNO includes === **/
#include <cppuhelper/implbase1.hxx>

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= PropertyEventTranslation
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::beans::XPropertyChangeListener
                                    >   PropertyEventTranslation_Base;

    class PropertyEventTranslation : public PropertyEventTranslation_Base
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >
                m_xDelegator;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                m_xTranslatedEventSource;

    public:
        /** constructs the object
            @throws NullPointerException
                if <arg>_rxDelegator</arg> is <NULL/>
        */
        PropertyEventTranslation(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxDelegator,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxTranslatedEventSource
        );

        inline const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >&
            getDelegator() const { return m_xDelegator; }

    protected:
        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException);
        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    private:
        PropertyEventTranslation();                                             // never implemented
        PropertyEventTranslation( const PropertyEventTranslation& );            // never implemented
        PropertyEventTranslation& operator=( const PropertyEventTranslation& ); // never implemented
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_PROPEVENTTRANSLATION_HXX

