/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: basscript.hxx,v $
 * $Revision: 1.8 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SCRIPTING_BASSCRIPT_HXX
#define SCRIPTING_BASSCRIPT_HXX

#include <com/sun/star/script/provider/XScript.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <cppuhelper/implbase1.hxx>
#include <basic/sbmeth.hxx>


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


    class BasicScriptImpl : public BasicScriptImpl_BASE
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
    };

//.........................................................................
}   // namespace basprov
//.........................................................................

#endif // SCRIPTING_BASSCRIPT_HXX
