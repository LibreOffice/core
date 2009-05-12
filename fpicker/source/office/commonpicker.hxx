/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: commonpicker.hxx,v $
 * $Revision: 1.5 $
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

#ifndef SVTOOLS_COMMONPICKER_HXX
#define SVTOOLS_COMMONPICKER_HXX

#include <cppuhelper/compbase5.hxx>
#include <com/sun/star/ui/dialogs/XControlInformation.hpp>
#include <com/sun/star/ui/dialogs/XControlAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/uno3.hxx>
#include <tools/link.hxx>

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

