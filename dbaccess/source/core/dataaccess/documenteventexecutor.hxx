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



#ifndef DBACCESS_DOCUMENTEVENTEXECUTOR_HXX
#define DBACCESS_DOCUMENTEVENTEXECUTOR_HXX

/** === begin UNO includes === **/
#include <com/sun/star/document/XDocumentEventListener.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>

#include <memory>

namespace comphelper
{
    class ComponentContext;
}

//........................................................................
namespace dbaccess
{
//........................................................................

    struct DocumentEventExecutor_Data;
    //====================================================================
    //= DocumentEventExecutor
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::document::XDocumentEventListener
                                    >   DocumentEventExecutor_Base;
    class DocumentEventExecutor : public DocumentEventExecutor_Base
    {
    public:
        DocumentEventExecutor(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventsSupplier >& _rxDocument );

    protected:
        virtual ~DocumentEventExecutor();

        // css.document.XDocumentEventListener
        virtual void SAL_CALL documentEventOccured( const ::com::sun::star::document::DocumentEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        // css.lang.XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    private:
        ::std::auto_ptr< DocumentEventExecutor_Data >   m_pData;
    };

//........................................................................
} // namespace dbaccess
//........................................................................

#endif // DBACCESS_DOCUMENTEVENTEXECUTOR_HXX
