/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: defaulthelpprovider.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 11:58:26 $
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

#ifndef EXTENSIONS_DEFAULTHELPPROVIDER_HXX
#define EXTENSIONS_DEFAULTHELPPROVIDER_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_INSPECTION_XPROPERTYCONTROLOBSERVER_HPP_
#include <com/sun/star/inspection/XPropertyControlObserver.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_XOBJECTINSPECTORUI_HPP_
#include <com/sun/star/inspection/XObjectInspectorUI.hpp>
#endif
/** === end UNO includes === **/

#include <cppuhelper/implbase2.hxx>
#include <comphelper/componentcontext.hxx>

class Window;

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= DefaultHelpProvider
    //====================================================================
    typedef ::cppu::WeakImplHelper2 <   ::com::sun::star::inspection::XPropertyControlObserver
                                    ,   ::com::sun::star::lang::XInitialization
                                    >   DefaultHelpProvider_Base;
    class DefaultHelpProvider : public DefaultHelpProvider_Base
    {
    private:
        ::comphelper::ComponentContext  m_aContext;
        bool                            m_bConstructed;
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >
                                        m_xInspectorUI;

    public:
        DefaultHelpProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext );

        // XServiceInfo - static versions
        static ::rtl::OUString getImplementationName_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

    protected:
        ~DefaultHelpProvider();

        // XPropertyControlObserver
        virtual void SAL_CALL focusGained( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& Control ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL valueChanged( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& Control ) throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    protected:
        // Service constructors
        void    create( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxUI );

    private:
        Window* impl_getVclControlWindow_nothrow( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& _rxControl );
        ::rtl::OUString impl_getHelpText_nothrow( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& _rxControl );
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_DEFAULTHELPPROVIDER_HXX
