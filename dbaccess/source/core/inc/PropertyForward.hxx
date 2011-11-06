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


#ifndef DBA_PROPERTYSETFORWARD_HXX
#define DBA_PROPERTYSETFORWARD_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif

#include <vector>

//........................................................................
namespace dbaccess
{
//........................................................................

    // ===================================================================
    // = OPropertyForward
    // ===================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::beans::XPropertyChangeListener
                                    >   OPropertyForward_Base;
    class OPropertyForward  :public ::comphelper::OBaseMutex
                            ,public OPropertyForward_Base
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xSource;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xDest;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >   m_xDestInfo;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xDestContainer;
        ::rtl::OUString     m_sName;
        sal_Bool            m_bInInsert;

    protected:
        virtual ~OPropertyForward();

    public:
        OPropertyForward( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xSource,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xDestContainer,
                          const ::rtl::OUString& _sName,
                          const ::std::vector< ::rtl::OUString >& _aPropertyList
                         );

        // ::com::sun::star::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException);

        inline void setName( const ::rtl::OUString& _sName ) { m_sName = _sName; }
        void setDefinition( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xDest);
        inline ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getDefinition() const { return m_xDest; }
    };

//........................................................................
}   // namespace dbaccess
//........................................................................
#endif // DBA_PROPERTYSETFORWARD_HXX

