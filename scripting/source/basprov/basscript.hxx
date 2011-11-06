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



#ifndef SCRIPTING_BASSCRIPT_HXX
#define SCRIPTING_BASSCRIPT_HXX

#include <com/sun/star/script/provider/XScript.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <cppuhelper/implbase1.hxx>
#include <basic/sbmeth.hxx>
#include <svl/lstner.hxx>


class BasicManager;

//.........................................................................
namespace basprov
{
//.........................................................................

    //  ----------------------------------------------------
    //  class BasicScriptImpl
    //  ----------------------------------------------------

    typedef ::cppu::WeakImplHelper1<
        ::com::sun::star::script::provider::XScript > BasicScriptImpl_BASE;


    class BasicScriptImpl : public BasicScriptImpl_BASE, public SfxListener
    {
    private:
        SbMethodRef         m_xMethod;
        ::rtl::OUString     m_funcName;
        BasicManager*       m_documentBasicManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::document::XScriptInvocationContext >
                            m_xDocumentScriptContext;

    public:
        BasicScriptImpl(
            const ::rtl::OUString& funcName,
            SbMethodRef xMethod
        );
        BasicScriptImpl(
            const ::rtl::OUString& funcName,
            SbMethodRef xMethod,
            BasicManager& documentBasicManager,
            const ::com::sun::star::uno::Reference< ::com::sun::star::document::XScriptInvocationContext >& documentScriptContext
        );
        virtual ~BasicScriptImpl();

        // XScript
        virtual ::com::sun::star::uno::Any SAL_CALL invoke(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aParams,
            ::com::sun::star::uno::Sequence< sal_Int16 >& aOutParamIndex,
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aOutParam )
            throw (
                    ::com::sun::star::script::provider::ScriptFrameworkErrorException,
                    ::com::sun::star::reflection::InvocationTargetException,
                    ::com::sun::star::uno::RuntimeException );

        // SfxListener
        virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    };

//.........................................................................
}   // namespace basprov
//.........................................................................

#endif // SCRIPTING_BASSCRIPT_HXX
