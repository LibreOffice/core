/*************************************************************************
 *
 *  $RCSfile: Filter.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:01:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef FORMS_COMPONENT_FILTER_HXX
#define FORMS_COMPONENT_FILTER_HXX

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XBOUNDCOMPONENT_HPP_
#include <com/sun/star/form/XBoundComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTCOMPONENT_HPP_
#include <com/sun/star/awt/XTextComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif

#ifndef _TOOLKIT_CONTROLS_UNOCONTROL_HXX_
#include <toolkit/controls/unocontrol.hxx>
#endif

#ifndef _TOOLKIT_AWT_LISTENERMULTIPLEXER_HXX_
#include <toolkit/helper/listenermultiplexer.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif

#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include <connectivity/sqlparse.hxx>
#endif

#ifndef SVX_QUERYDESIGNCONTEXT_HXX
#include <svx/ParseContext.hxx>
#endif

class Window;

//.........................................................................
namespace frm
{
//.........................................................................

    //=====================================================================
    // OFilterControl
    //=====================================================================
    typedef ::cppu::ImplHelper5 <   ::com::sun::star::awt::XTextComponent
                                ,   ::com::sun::star::awt::XFocusListener
                                ,   ::com::sun::star::awt::XItemListener
                                ,   ::com::sun::star::form::XBoundComponent
                                ,   ::com::sun::star::lang::XInitialization
                                >   OFilterControl_BASE;

    class OFilterControl    :public UnoControl
                            ,public OFilterControl_BASE
                            ,public ::svxform::OParseContextClient
    {
        TextListenerMultiplexer     m_aTextListeners;

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >        m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >               m_xField;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >            m_xFormatter;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >           m_xMetaData;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >                      m_xMessageParent;

        ::rtl::OUString                 m_aText;
        ::connectivity::OSQLParser      m_aParser;
        sal_Int16                       m_nControlClass;        // which kind of control do we use?
        sal_Bool                        m_bFilterList : 1;
        sal_Bool                        m_bMultiLine : 1;
        sal_Bool                        m_bFilterListFilled : 1;

    private:
//      OFilterControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);

        void implInitFilterList();

    public:
        OFilterControl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB );

        DECLARE_UNO3_AGG_DEFAULTS(OFilterControl,OWeakAggObject);
        ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

        static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();
        sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 > & rId );

        virtual ::rtl::OUString GetComponentServiceName();
        virtual void SAL_CALL   createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > & rxToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  & rParentPeer ) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::lang::XComponent
        virtual void SAL_CALL   dispose(void) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::awt::XTextComponent
        virtual void            SAL_CALL addTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener > & l ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void            SAL_CALL removeTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener > & l ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void            SAL_CALL setText( const ::rtl::OUString& aText ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void            SAL_CALL insertText( const ::com::sun::star::awt::Selection& rSel, const ::rtl::OUString& aText ) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::rtl::OUString SAL_CALL getText() throw( ::com::sun::star::uno::RuntimeException );
        virtual ::rtl::OUString SAL_CALL getSelectedText() throw( ::com::sun::star::uno::RuntimeException );
        virtual void            SAL_CALL setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::awt::Selection SAL_CALL getSelection() throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool        SAL_CALL isEditable() throw( ::com::sun::star::uno::RuntimeException );
        virtual void            SAL_CALL setEditable( sal_Bool bEditable ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void            SAL_CALL setMaxTextLen( sal_Int16 nLength ) throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Int16       SAL_CALL getMaxTextLen() throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::form::XBoundComponent
        virtual void            SAL_CALL addUpdateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener > & l) throw( ::com::sun::star::uno::RuntimeException ) {}
        virtual void            SAL_CALL removeUpdateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener > & l) throw( ::com::sun::star::uno::RuntimeException ) {}
        virtual sal_Bool        SAL_CALL commit() throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::lang::XEventListener
        virtual void            SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::awt::XFocusListener
        virtual void            SAL_CALL focusGained(const ::com::sun::star::awt::FocusEvent& e) throw( ::com::sun::star::uno::RuntimeException );
        virtual void            SAL_CALL focusLost(const ::com::sun::star::awt::FocusEvent& e) throw( ::com::sun::star::uno::RuntimeException ){}

    // ::com::sun::star::awt::XItemListener
        virtual void            SAL_CALL itemStateChanged(const ::com::sun::star::awt::ItemEvent& rEvent) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::util::XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // com::sun::star::lang::XServiceInfo - static version
        static  ::rtl::OUString SAL_CALL getImplementationName_Static();
        static  ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_Static();
        static  ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getCurrentServiceNames_Static();
        static  ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory );

    protected:
        virtual void PrepareWindowDescriptor( ::com::sun::star::awt::WindowDescriptor& rDesc );
        virtual void ImplSetPeerProperty( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rVal );

        sal_Bool ensureInitialized( );

        void displayException( const ::com::sun::star::sdb::SQLContext& _rExcept );
    };
//.........................................................................
}   // namespace frm
//.........................................................................

#endif // FORMS_COMPONENT_FILTER_HXX
