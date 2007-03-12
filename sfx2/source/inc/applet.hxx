/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: applet.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-12 11:00:34 $
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

#ifndef _SFX_APPLET_HXX
#define _SFX_APPLET_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSYNCHRONOUSFRAMELOADER_HPP_
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#endif
#ifndef  _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>
#endif

#include <rtl/ustring.hxx>
#include <svtools/ownlist.hxx>
#include <svtools/itemprop.hxx>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
    class XInterface;
} } } }

class SjApplet2;
namespace sfx2
{

class AppletObject : public ::cppu::WeakImplHelper6 <
        com::sun::star::util::XCloseable,
        com::sun::star::lang::XEventListener,
        com::sun::star::frame::XSynchronousFrameLoader,
        com::sun::star::ui::dialogs::XExecutableDialog,
        com::sun::star::lang::XInitialization,
        com::sun::star::beans::XPropertySet >
{
    com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >
        mxContext;
    com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > mxObj;
    SfxItemPropertySet  maPropSet;
    SvCommandList       maCmdList;
    ::rtl::OUString     maClass;
    ::rtl::OUString     maName;
    ::rtl::OUString     maCodeBase;
    ::rtl::OUString     maDocBase;
    SjApplet2*          mpApplet;
    sal_Bool            mbMayScript;

    AppletObject( AppletObject & ); // not defined
    void operator =( AppletObject & ); // not defined

                        AppletObject( const com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >& rContext );
                        ~AppletObject();

    virtual sal_Bool SAL_CALL load( const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& lDescriptor,
            const com::sun::star::uno::Reference < com::sun::star::frame::XFrame >& xFrame ) throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL cancel() throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL close( sal_Bool bDeliverOwnership ) throw( com::sun::star::util::CloseVetoException, com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addCloseListener( const com::sun::star::uno::Reference < com::sun::star::util::XCloseListener >& xListener ) throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeCloseListener( const com::sun::star::uno::Reference < com::sun::star::util::XCloseListener >& xListener ) throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& aEvent ) throw (com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL setTitle( const ::rtl::OUString& aTitle ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL execute(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addPropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removePropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::sal_Bool SAL_CALL supportsService(
        const ::rtl::OUString& sServiceName )
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

    static ::com::sun::star::uno::Sequence< ::rtl::OUString >
    impl_getStaticSupportedServiceNames();
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    SAL_CALL impl_createInstance(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >& xContext )
        throw( ::com::sun::star::uno::Exception );

public:
    static ::rtl::OUString impl_getStaticImplementationName();
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    impl_createFactory();
};

}
#endif
