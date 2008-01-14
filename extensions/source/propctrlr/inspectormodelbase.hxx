/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inspectormodelbase.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:59:40 $
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

#ifndef INSPECTORMODELBASE_HXX
#define INSPECTORMODELBASE_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_INSPECTION_XOBJECTINSPECTORMODEL_HPP_
#include <com/sun/star/inspection/XObjectInspectorModel.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
/** === end UNO includes === **/

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/propshlp.hxx>

#include <comphelper/broadcasthelper.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/uno3.hxx>

#include <memory>

//........................................................................
namespace pcr
{
//........................................................................

    class InspectorModelProperties;
    //====================================================================
    //= ImplInspectorModel
    //====================================================================
    typedef ::cppu::WeakImplHelper3 <   ::com::sun::star::inspection::XObjectInspectorModel
                                    ,   ::com::sun::star::lang::XInitialization
                                    ,   ::com::sun::star::lang::XServiceInfo
                                    >   ImplInspectorModel_Base;
    typedef ::cppu::OPropertySetHelper  ImplInspectorModel_PBase;

    class ImplInspectorModel
            :public ::comphelper::OMutexAndBroadcastHelper
            ,public ImplInspectorModel_Base
            ,public ImplInspectorModel_PBase
    {
    protected:
        ::comphelper::ComponentContext                                  m_aContext;
        ::std::auto_ptr< InspectorModelProperties >                     m_pProperties;

    protected:
        ~ImplInspectorModel();

    public:
        ImplInspectorModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext );

        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        // ::com::sun::star::beans::XPropertySet and friends
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
        virtual sal_Bool SAL_CALL convertFastPropertyValue( ::com::sun::star::uno::Any & rConvertedValue, ::com::sun::star::uno::Any & rOldValue, sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception);
        virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;

        // ::com::sun::star::inspection::XObjectInspectorModel
        virtual ::sal_Bool SAL_CALL getHasHelpSection() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getMinHelpTextLines() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getMaxHelpTextLines() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getIsReadOnly() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setIsReadOnly( ::sal_Bool _IsReadOnly ) throw (::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XServiceInfo
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        void enableHelpSectionProperties( sal_Int32 _nMinHelpTextLines, sal_Int32 _nMaxHelpTextLines );

    private:
        using ImplInspectorModel_PBase::getFastPropertyValue;
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // INSPECTORMODELBASE_HXX

