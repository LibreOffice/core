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


#ifndef RPTUI_PROPERTYSETFORWARD_HXX
#define RPTUI_PROPERTYSETFORWARD_HXX

#include "dllapi.h"
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <cppuhelper/compbase1.hxx>
#include "cppuhelper/basemutex.hxx"
#include "RptDef.hxx"


//........................................................................
namespace rptui
{
//........................................................................
    typedef ::cppu::WeakComponentImplHelper1<   ::com::sun::star::beans::XPropertyChangeListener
                                    >   OPropertyForward_Base;

    /** \class OPropertyMediator
     * \brief This class ensures the communication between two XPropertySet instances.
     * Identical properties will be set at the other propertyset.
     * \ingroup reportdesign_source_ui_misc
     */
    class REPORTDESIGN_DLLPUBLIC OPropertyMediator : public ::cppu::BaseMutex
                            ,public OPropertyForward_Base
    {
        TPropertyNamePair                                                               m_aNameMap;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>        m_xSource;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo>    m_xSourceInfo;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>        m_xDest;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo>    m_xDestInfo;
        sal_Bool                                                                        m_bInChange;
        OPropertyMediator(OPropertyMediator&);
        void operator =(OPropertyMediator&);
    protected:
        virtual ~OPropertyMediator();

        /** this function is called upon disposing the component
        */
        virtual void SAL_CALL disposing();
    public:
        OPropertyMediator(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xSource
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xDest
                        ,const TPropertyNamePair& _aNameMap
                        ,sal_Bool _bReverse = sal_False);

        // ::com::sun::star::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException);

        /** stop the listening mode.
         */
        void stopListening();

        /** starts the listening mode again.
         */
        void startListening();
    };
//........................................................................
}   // namespace rptui
//........................................................................
#endif // RPTUI_PROPERTYSETFORWARD_HXX

