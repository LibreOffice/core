/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xfm_addcondition.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:29:29 $
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

#ifndef SVX_SOURCE_INC_XFM_ADDCONDITION_HXX
#define SVX_SOURCE_INC_XFM_ADDCONDITION_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_XFORMS_XMODEL_HPP_
#include <com/sun/star/xforms/XModel.hpp>
#endif
/** === end UNO includes === **/

#ifndef _SVT_GENERICUNODIALOG_HXX_
#include <svtools/genericunodialog.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= OAddConditionDialog
    //====================================================================
    typedef ::svt::OGenericUnoDialog OAddConditionDialogBase;
    class OAddConditionDialog
            :public OAddConditionDialogBase
            ,public ::comphelper::OPropertyArrayUsageHelper< OAddConditionDialog >
    {
    private:
        // <properties>
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                m_xBinding;
        ::rtl::OUString         m_sFacetName;
        ::rtl::OUString         m_sConditionValue;
        ::com::sun::star::uno::Reference< ::com::sun::star::xforms::XModel >
                                m_xWorkModel;
        // </properties>

    public:
        static ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
            SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );

    protected:
        OAddConditionDialog( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB );

        // XTypeProvider
        virtual com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(com::sun::star::uno::RuntimeException);

        // XPropertySet
        virtual com::sun::star::uno::Reference<com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(com::sun::star::uno::RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    protected:
        // OGenericUnoDialog overridables
        virtual Dialog* createDialog(Window* _pParent);
        virtual void executedDialog(sal_Int16 _nExecutionResult);
    };

//........................................................................
} // namespace svxform
//........................................................................

#endif // SVX_SOURCE_INC_XFM_ADDCONDITION_HXX

