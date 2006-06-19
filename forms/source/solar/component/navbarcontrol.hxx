/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: navbarcontrol.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 13:02:45 $
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

#ifndef FORMS_NAVBARCONTROL_HXX
#define FORMS_NAVBARCONTROL_HXX

#ifndef _TOOLKIT_CONTROLS_UNOCONTROL_HXX_
#include <toolkit/controls/unocontrol.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif

#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _SV_WINTYPES_HXX
#include <vcl/wintypes.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif
#ifndef FORMS_FORM_NAVIGATION_HXX
#include "formnavigation.hxx"
#endif

//.........................................................................
namespace frm
{
//.........................................................................

    //==================================================================
    // ONavigationBarControl
    //==================================================================
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::frame::XDispatchProviderInterception
                                >   ONavigationBarControl_Base;

    class ONavigationBarControl
                            :public UnoControl
                            ,public ONavigationBarControl_Base
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        m_xORB;

    public:
        ONavigationBarControl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

    protected:
        ~ONavigationBarControl();

    public:
        // XServiceInfo - static version
        static  ::rtl::OUString SAL_CALL getImplementationName_Static();
        static  ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_Static();
        static  ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory );

    protected:
        // UNO
        DECLARE_UNO3_AGG_DEFAULTS( ONavigationBarControl, UnoControl );
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw ( ::com::sun::star::uno::RuntimeException );

        // XControl
        virtual void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& _rToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& _rParent ) throw( ::com::sun::star::uno::RuntimeException );

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName()  throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()  throw(::com::sun::star::uno::RuntimeException);

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XVclWindowPeer
        virtual void SAL_CALL setDesignMode( sal_Bool _bOn ) throw( ::com::sun::star::uno::RuntimeException );

        // XDispatchProviderInterception
        virtual void SAL_CALL registerDispatchProviderInterceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& Interceptor ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL releaseDispatchProviderInterceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& Interceptor ) throw (::com::sun::star::uno::RuntimeException);
    };

    //==================================================================
    // ONavigationBarPeer
    //==================================================================
    class ONavigationBarPeer
                        :public VCLXWindow
                        ,public OFormNavigationHelper
    {
    public:
        /** factory method
            @return
                a new ONavigationBarPeer instance, which has been aquired once!
        */
        static ONavigationBarPeer* Create(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            Window* _pParentWindow,
            WinBits _nStyle
        );

    protected:
        ONavigationBarPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB );
        ~ONavigationBarPeer();

    public:
        // XInterface
        DECLARE_XINTERFACE( )

        // XVclWindowPeer
        virtual void SAL_CALL setDesignMode( sal_Bool _bOn ) throw( ::com::sun::star::uno::RuntimeException );

        // XWindow2
        using VCLXWindow::isEnabled;

    protected:
        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XComponent
        void SAL_CALL dispose(  ) throw( ::com::sun::star::uno::RuntimeException );

        // XVclWindowPeer
        void SAL_CALL setProperty( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rValue ) throw( ::com::sun::star::uno::RuntimeException );
        ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& _rPropertyName ) throw(::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

        // OFormNavigationHelper overriables
        virtual void    interceptorsChanged( );
        virtual void    featureStateChanged( sal_Int32 _nFeatureId, sal_Bool _bEnabled );
        virtual void    allFeatureStatesChanged( );
        virtual void    getSupportedFeatures( ::std::vector< sal_Int32 >& /* [out] */ _rFeatureIds );

        // IFeatureDispatcher overriables
        virtual bool    isEnabled( sal_Int32 _nFeatureId ) const;
    };

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // FORMS_NAVBARCONTROL_HXX
