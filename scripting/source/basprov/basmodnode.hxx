/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basmodnode.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:23:11 $
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

#ifndef SCRIPTING_BASMODNODE_HXX
#define SCRIPTING_BASMODNODE_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_BROWSE_XBROWSENODE_HPP_
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

class SbModule;


//.........................................................................
namespace basprov
{
//.........................................................................

    //  ----------------------------------------------------
    //  class BasicModuleNodeImpl
    //  ----------------------------------------------------

    typedef ::cppu::WeakImplHelper1<
        ::com::sun::star::script::browse::XBrowseNode > BasicModuleNodeImpl_BASE;


    class BasicModuleNodeImpl : public BasicModuleNodeImpl_BASE
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >    m_xContext;
    ::rtl::OUString m_sScriptingContext;
        SbModule* m_pModule;
        bool m_bIsAppScript;

    public:
        BasicModuleNodeImpl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
            const ::rtl::OUString& sScriptingContext,
            SbModule* pModule, bool isAppScript = true );
        virtual ~BasicModuleNodeImpl();

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

#endif // SCRIPTING_BASMODNODE_HXX
