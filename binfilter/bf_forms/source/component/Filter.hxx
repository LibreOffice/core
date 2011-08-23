/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef FORMS_COMPONENT_FILTER_HXX
#define FORMS_COMPONENT_FILTER_HXX

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/form/XBoundComponent.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>

#include <toolkit/controls/unocontrol.hxx>


#include <cppuhelper/implbase5.hxx>

#include <comphelper/uno3.hxx>
#include <connectivity/sqlparse.hxx>

#include <bf_svx/ParseContext.hxx>
class Window;
namespace binfilter {


//.........................................................................
namespace frm
{
//.........................................................................

    //=====================================================================
    // OFilterControl
    //=====================================================================
    typedef ::cppu::ImplHelper5	<	::com::sun::star::awt::XTextComponent
                                ,	::com::sun::star::awt::XFocusListener
                                ,	::com::sun::star::awt::XItemListener
                                ,	::com::sun::star::form::XBoundComponent
                                ,	::com::sun::star::lang::XInitialization
                                >	OFilterControl_BASE;

    class OFilterControl	:public UnoControl
                            ,public OFilterControl_BASE
                            ,public ::binfilter::svxform::OParseContextClient//STRIP008 							,public ::svxform::OParseContextClient
    {
        TextListenerMultiplexer		m_aTextListeners;

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >		m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > 				m_xField;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > 			m_xFormatter;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > 			m_xMetaData;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >						m_xMessageParent;

        ::rtl::OUString					m_aText;
        ::connectivity::OSQLParser		m_aParser;
        sal_Int16						m_nControlClass;		// which kind of control do we use?
        sal_Bool						m_bFilterList : 1;
        sal_Bool						m_bMultiLine : 1;
        sal_Bool						m_bFilterListFilled : 1;

    private:
//		OFilterControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);

        void implInitFilterList();

    public:
        OFilterControl(	const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB );

        DECLARE_UNO3_AGG_DEFAULTS(OFilterControl,OWeakAggObject);
        ::com::sun::star::uno::Any	SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

        static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();
        sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 > & rId );

        virtual ::rtl::OUString	GetComponentServiceName();
        virtual void SAL_CALL	createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > & rxToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  & rParentPeer ) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::lang::XComponent
        virtual void SAL_CALL	dispose(void) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::awt::XTextComponent
        virtual void			SAL_CALL addTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener > & l ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void			SAL_CALL removeTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener > & l ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void			SAL_CALL setText( const ::rtl::OUString& aText ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void			SAL_CALL insertText( const ::com::sun::star::awt::Selection& rSel, const ::rtl::OUString& aText ) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::rtl::OUString	SAL_CALL getText() throw( ::com::sun::star::uno::RuntimeException );
        virtual ::rtl::OUString	SAL_CALL getSelectedText() throw( ::com::sun::star::uno::RuntimeException );
        virtual void			SAL_CALL setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::awt::Selection SAL_CALL getSelection() throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool		SAL_CALL isEditable() throw( ::com::sun::star::uno::RuntimeException );
        virtual void			SAL_CALL setEditable( sal_Bool bEditable ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void			SAL_CALL setMaxTextLen( sal_Int16 nLength ) throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Int16		SAL_CALL getMaxTextLen() throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::form::XBoundComponent
        virtual void			SAL_CALL addUpdateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener > & l) throw( ::com::sun::star::uno::RuntimeException ) {}
        virtual void			SAL_CALL removeUpdateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener > & l) throw( ::com::sun::star::uno::RuntimeException ) {}
        virtual sal_Bool		SAL_CALL commit() throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::lang::XEventListener
        virtual void			SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::awt::XFocusListener
        virtual void			SAL_CALL focusGained(const ::com::sun::star::awt::FocusEvent& e) throw( ::com::sun::star::uno::RuntimeException );
        virtual void			SAL_CALL focusLost(const ::com::sun::star::awt::FocusEvent& e) throw( ::com::sun::star::uno::RuntimeException ){}

    // ::com::sun::star::awt::XItemListener
        virtual void			SAL_CALL itemStateChanged(const ::com::sun::star::awt::ItemEvent& rEvent) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::util::XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // com::sun::star::lang::XServiceInfo - static version
        static	::rtl::OUString	SAL_CALL getImplementationName_Static();
        static	::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_Static();
        static	::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getCurrentServiceNames_Static();
        static	::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory );

    protected:
        virtual void PrepareWindowDescriptor( ::com::sun::star::awt::WindowDescriptor& rDesc );
        virtual void ImplSetPeerProperty( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rVal );

        sal_Bool ensureInitialized( );
    };
//.........................................................................
}	// namespace frm
//.........................................................................

}//end of namespace binfilter
#endif // FORMS_COMPONENT_FILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
