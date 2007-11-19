/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tbunocontroller.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-19 17:18:57 $
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

#ifndef __TBUNOCONTROLLER_HXX_
#define __TBUNOCONTROLLER_HXX_

#include <svtools/toolboxcontroller.hxx>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace svx
{

class SvxFontSizeBox_Impl;
class FontHeightToolBoxControl : public svt::ToolboxController,
                                 public ::com::sun::star::lang::XServiceInfo
{
    public:
        FontHeightToolBoxControl(
            const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager );
        ~FontHeightToolBoxControl();

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw ();
        virtual void SAL_CALL release() throw ();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

        static ::rtl::OUString getImplementationName_Static() throw()
        {
            return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.svx.FontHeightToolBoxController" ));
        }

        static ::com::sun::star::uno::Sequence< ::rtl::OUString >  getSupportedServiceNames_Static() throw();

        // XComponent
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL click() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL doubleClick() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createPopupWindow() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createItemWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& Parent ) throw (::com::sun::star::uno::RuntimeException);

        void dispatchCommand( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs );

    private:
        SvxFontSizeBox_Impl*                  m_pBox;
        ::com::sun::star::awt::FontDescriptor m_aCurrentFont;
};

::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL FontHeightToolBoxControl_createInstance( const com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rSMgr );

}

#endif // __TBUNOCONTROLLER_HXX_
