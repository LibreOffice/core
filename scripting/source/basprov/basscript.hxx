/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basscript.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:19:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SCRIPTING_BASSCRIPT_HXX
#define SCRIPTING_BASSCRIPT_HXX

#ifndef _COM_SUN_STAR_SCRIPT_PROVIDER_XSCRIPT_HPP_
#include <com/sun/star/script/provider/XScript.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XSCRIPTINVOCATIONCONTEXT_HPP_
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _SB_SBMETH_HXX
#include <basic/sbmeth.hxx>
#endif


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
