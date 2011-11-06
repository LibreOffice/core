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



#ifndef EXTENSIONS_BIB_FORMCONTROLCONTAINER_HXX
#define EXTENSIONS_BIB_FORMCONTROLCONTAINER_HXX

#include <comphelper/broadcasthelper.hxx>
#include "loadlisteneradapter.hxx"
#include <com/sun/star/awt/XControlContainer.hpp>

class BibDataManager;

//.........................................................................
namespace bib
{
//.........................................................................

    //=====================================================================
    //= FormControlContainer
    //=====================================================================
    class FormControlContainer
            :public ::comphelper::OBaseMutex
            ,public ::bib::OLoadListener
    {
    private:
        OLoadListenerAdapter*   m_pFormAdapter;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >
                                m_xForm;
    private:
        void    implSetDesignMode( sal_Bool _bDesign );

    protected:
        FormControlContainer( );
        ~FormControlContainer( );

        sal_Bool    isFormConnected() const { return NULL != m_pFormAdapter; }
        void        connectForm( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >& _rxForm );
        void        disconnectForm();

        void        ensureDesignMode();

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >
                    getControlContainer() = 0;

    protected:
    // XLoadListener equivalents
        virtual void _loaded( const ::com::sun::star::lang::EventObject& _rEvent );
        virtual void _unloading( const ::com::sun::star::lang::EventObject& _rEvent );
        virtual void _unloaded( const ::com::sun::star::lang::EventObject& _rEvent );
        virtual void _reloading( const ::com::sun::star::lang::EventObject& _rEvent );
        virtual void _reloaded( const ::com::sun::star::lang::EventObject& _rEvent );

    };

//.........................................................................
}   // namespace bib
//.........................................................................

#endif // EXTENSIONS_BIB_FORMCONTROLCONTAINER_HXX

