/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: richtextcontrol.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 13:00:11 $
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

#ifndef FORMS_SOURCE_RICHTEXT_RICHTEXTCONTROL_HXX
#define FORMS_SOURCE_RICHTEXT_RICHTEXTCONTROL_HXX

#ifndef _TOOLKIT_HELPER_UNOCONTROLS_HXX_
#include <toolkit/controls/unocontrols.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
/** === end UNO includes === **/

#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_IMPLEMENTATIONREFERENCE_HXX
#include <comphelper/implementationreference.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _SV_WINTYPES_HXX
#include <vcl/wintypes.hxx>
#endif

#ifndef FORMS_SOURCE_RICHTEXT_RTATTRIBUTES_HXX
#include "rtattributes.hxx"
#endif
#ifndef FORMS_SOURCE_RICHTEXT_ATTRIBUTEDISPATCHER_HXX
#include "attributedispatcher.hxx"
#endif

#include <map>

//.........................................................................
namespace frm
{
//.........................................................................

    class ORichTextFeatureDispatcher;
    class RichTextControl;

    //==================================================================
    // ORichTextControl
    //==================================================================
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::frame::XDispatchProvider
                                >   ORichTextControl_Base;
    class ORichTextControl  :public UnoEditControl
                            ,public ORichTextControl_Base
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        m_xORB;

    public:
        ORichTextControl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

    protected:
        ~ORichTextControl();

    public:
        // XServiceInfo - static version
        static  ::rtl::OUString SAL_CALL getImplementationName_Static();
        static  ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_Static();
        static  ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory );

    protected:
        // UNO
        DECLARE_UNO3_AGG_DEFAULTS( ORichTextControl, UnoEditControl );
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw ( ::com::sun::star::uno::RuntimeException );

        // XControl
        virtual void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& _rToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& _rParent ) throw( ::com::sun::star::uno::RuntimeException );

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName()  throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()  throw(::com::sun::star::uno::RuntimeException);

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XDispatchProvider
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& _rURL, const ::rtl::OUString& _rTargetFrameName, sal_Int32 _rSearchFlags ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& Requests ) throw (::com::sun::star::uno::RuntimeException);

        // UnoControl
        virtual sal_Bool   requiresNewPeer( const ::rtl::OUString& _rPropertyName ) const;
    };

    //==================================================================
    // ORichTextPeer
    //==================================================================
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::frame::XDispatchProvider
                                >   ORichTextPeer_Base;
    class ORichTextPeer :public VCLXWindow
                        ,public ORichTextPeer_Base
                        ,public ITextSelectionListener
    {
    private:
        typedef ::comphelper::ImplementationReference< ORichTextFeatureDispatcher, ::com::sun::star::frame::XDispatch > SingleAttributeDispatcher;
        typedef ::std::map< SfxSlotId, SingleAttributeDispatcher >                                                      AttributeDispatchers;
        AttributeDispatchers                                                                                            m_aDispatchers;

    public:
        /** factory method
            @return
                a new ORichTextPeer instance, which has been aquired once!
        */
        static ORichTextPeer* Create(
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >&         _rxModel,
            Window* _pParentWindow,
            WinBits _nStyle
        );

        // XInterface
        DECLARE_XINTERFACE( )

    protected:
        ORichTextPeer();
        ~ORichTextPeer();

        // XVclWindowPeer
        virtual void SAL_CALL setProperty( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rValue ) throw (::com::sun::star::uno::RuntimeException);

        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

        // XComponent
        virtual void SAL_CALL dispose( ) throw(::com::sun::star::uno::RuntimeException);

        // XDispatchProvider
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& _rURL, const ::rtl::OUString& _rTargetFrameName, sal_Int32 _rSearchFlags ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& Requests ) throw (::com::sun::star::uno::RuntimeException);

        // ITextSelectionListener
        virtual void    onSelectionChanged( const ESelection& _rSelection );

    private:
        SingleAttributeDispatcher implCreateDispatcher( SfxSlotId _nSlotId, const ::com::sun::star::util::URL& _rURL );
    };

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // FORMS_SOURCE_RICHTEXT_RICHTEXTCONTROL_HXX

