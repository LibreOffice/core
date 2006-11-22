/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: commonpicker.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-22 10:15:10 $
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

#ifndef SVTOOLS_COMMONPICKER_HXX
#define SVTOOLS_COMMONPICKER_HXX

#ifndef  _CPPUHELPER_COMPBASE5_HXX_
#include <cppuhelper/compbase5.hxx>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_XCONTROLINFORMATION_HPP_
#include <com/sun/star/ui/dialogs/XControlInformation.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XControlAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCANCELLABLE_HPP_
#include <com/sun/star/util/XCancellable.hpp>
#endif
#ifndef  _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _COMPHELPER_PROPERTYCONTAINER_HXX_
#include <comphelper/propertycontainer.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

class SvtFileDialog;
class Window;

namespace comphelper
{
    class OWeakEventListenerAdapter;
}

//.........................................................................
namespace svt
{
//.........................................................................

    typedef ::cppu::WeakComponentImplHelper5    <   ::com::sun::star::ui::dialogs::XControlAccess
                                                ,   ::com::sun::star::ui::dialogs::XControlInformation
                                                ,   ::com::sun::star::lang::XEventListener
                                                ,   ::com::sun::star::util::XCancellable
                                                ,   ::com::sun::star::lang::XInitialization
                                                >   OCommonPicker_Base;
    /** implements common functionality for the 2 UNO picker components
    */
    class OCommonPicker
                    :public ::comphelper::OBaseMutex
                    ,public OCommonPicker_Base
                    ,public ::comphelper::OPropertyContainer
                    ,public ::comphelper::OPropertyArrayUsageHelper< OCommonPicker >
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xORB;

        // <properties>
        ::rtl::OUString                                                     m_sHelpURL;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  m_xWindow;
        // </properties>

        SvtFileDialog*      m_pDlg;
        sal_uInt32          m_nCancelEvent;
        sal_Bool            m_bExecuting;

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  m_xDialogParent;

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >  m_xWindowListenerAdapter;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >  m_xParentListenerAdapter;

    protected:
        ::rtl::OUString     m_aTitle;
        ::rtl::OUString     m_aDisplayDirectory;

    protected:
        inline  SvtFileDialog*  getDialog() { return m_pDlg; }

        inline const    ::cppu::OBroadcastHelper&   GetBroadcastHelper() const  { return OCommonPicker_Base::rBHelper; }
        inline          ::cppu::OBroadcastHelper&   GetBroadcastHelper()        { return OCommonPicker_Base::rBHelper; }

    public:
        OCommonPicker( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory );

    protected:
        virtual ~OCommonPicker();

        // overridables

        // will be called with locked SolarMutex
        virtual SvtFileDialog*  implCreateDialog( Window* _pParent ) = 0;
        virtual sal_Int16       implExecutePicker( ) = 0;
            // do NOT override XExecutableDialog::execute! We need to do some stuff there ourself ...

    protected:
        //------------------------------------------------------------------------------------
        // disambiguate XInterface
        //------------------------------------------------------------------------------------
        DECLARE_XINTERFACE( )

        //------------------------------------------------------------------------------------
        // disambiguate XTypeProvider
        //------------------------------------------------------------------------------------
        DECLARE_XTYPEPROVIDER( )

        //------------------------------------------------------------------------------------
        // ComponentHelper/XComponent
        //------------------------------------------------------------------------------------
        virtual void SAL_CALL disposing();

        //------------------------------------------------------------------------------------
        // XEventListner
        //------------------------------------------------------------------------------------
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

        //------------------------------------------------------------------------------------
        // property set related methods
        //------------------------------------------------------------------------------------

        // XPropertySet pure methods
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
        // OPropertySetHelper pure methods
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        // OPropertyArrayUsageHelper pure methods
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

        // OPropertySetHelper overridden methods
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue ) throw (::com::sun::star::uno::Exception);

        //------------------------------------------------------------------------------------
        // XExecutableDialog functions
        //------------------------------------------------------------------------------------
        virtual void SAL_CALL           setTitle( const ::rtl::OUString& _rTitle ) throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Int16 SAL_CALL      execute() throw( ::com::sun::star::uno::RuntimeException );

        //------------------------------------------------------------------------------------
        // XControlAccess functions
        //------------------------------------------------------------------------------------
        virtual void SAL_CALL setControlProperty( const ::rtl::OUString& aControlName, const ::rtl::OUString& aControlProperty, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getControlProperty( const ::rtl::OUString& aControlName, const ::rtl::OUString& aControlProperty ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        //------------------------------------------------------------------------------------
        // XControlInformation functions
        //------------------------------------------------------------------------------------
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedControls(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isControlSupported( const ::rtl::OUString& aControlName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedControlProperties( const ::rtl::OUString& aControlName ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isControlPropertySupported( const ::rtl::OUString& aControlName, const ::rtl::OUString& aControlProperty ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        //------------------------------------------------------------------------------------
        // XCancellable functions
        //------------------------------------------------------------------------------------
        virtual void SAL_CALL cancel(  ) throw (::com::sun::star::uno::RuntimeException);

        //------------------------------------------------------------------------------------
        // XInitialization functions
        //------------------------------------------------------------------------------------

        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw ( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException );

        //------------------------------------------------------------------------------------
        // misc
        //------------------------------------------------------------------------------------
        void checkAlive() const SAL_THROW( (::com::sun::star::lang::DisposedException) );

        void prepareDialog();

    protected:
                sal_Bool    createPicker();

        /** handle a single argument from the XInitialization::initialize method

            @return <TRUE/> if the argument could be handled
        */
        virtual sal_Bool    implHandleInitializationArgument(
                                const ::rtl::OUString& _rName,
                                const ::com::sun::star::uno::Any& _rValue
                            )
                            SAL_THROW( ( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException ) );

    private:
        void        stopWindowListening();

        DECL_LINK( OnCancelPicker, void* );
    };
//.........................................................................
}   // namespace svt
//.........................................................................

#endif // SVTOOLS_COMMONPICKER_HXX

