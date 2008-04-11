/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: baslibnode.hxx,v $
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

#ifndef SCRIPTING_BASLIBNODE_HXX
#define SCRIPTING_BASLIBNODE_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase1.hxx>

class BasicManager;


//.........................................................................
namespace basprov
{
//.........................................................................

    //  ----------------------------------------------------
    //  class BasicLibraryNodeImpl
    //  ----------------------------------------------------

    typedef ::cppu::WeakImplHelper1<
        ::com::sun::star::script::browse::XBrowseNode > BasicLibraryNodeImpl_BASE;


    class BasicLibraryNodeImpl : public BasicLibraryNodeImpl_BASE
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >    m_xContext;
     ::rtl::OUString  m_sScriptingContext;
        BasicManager* m_pBasicManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer > m_xLibContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > m_xLibrary;
        ::rtl::OUString m_sLibName;
        bool m_bIsAppScript;

    public:
        BasicLibraryNodeImpl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
        const ::rtl::OUString& sScriptingContext,
            BasicManager* pBasicManager,
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer >& xLibContainer,
            const ::rtl::OUString& sLibName, bool isAppScript=true );
        virtual ~BasicLibraryNodeImpl();

        // XBrowseNode
        virtual ::rtl::OUString SAL_CALL getName(  )
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode > > SAL_CALL getChildNodes(  )
            throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasChildNodes(  )
            throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getType(  )
            throw (::com::sun::star::uno::RuntimeException);
    };

//.........................................................................
}   // namespace basprov
//.........................................................................

#endif // SCRIPTING_BASLIBNODE_HXX
