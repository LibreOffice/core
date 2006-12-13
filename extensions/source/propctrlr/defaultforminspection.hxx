/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: defaultforminspection.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 11:58:05 $
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
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_DEFAULTFORMINSPECTION_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_DEFAULTFORMINSPECTION_HXX

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PCROMPONENTCONTEXT_HXX
#include "pcrcomponentcontext.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_INSPECTION_XOBJECTINSPECTORMODEL_HPP_
#include <com/sun/star/inspection/XObjectInspectorModel.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#include <memory>

//........................................................................
namespace pcr
{
//........................................................................

    class OPropertyInfoService;
    //====================================================================
    //= DefaultFormComponentInspectorModel
    //====================================================================
    typedef ::cppu::WeakAggImplHelper3  <   ::com::sun::star::inspection::XObjectInspectorModel
                                        ,   ::com::sun::star::lang::XServiceInfo
                                        ,   ::com::sun::star::lang::XInitialization
                                        >   DefaultFormComponentInspectorModel_Base;
    class DefaultFormComponentInspectorModel : public DefaultFormComponentInspectorModel_Base
    {
    private:
        ::osl::Mutex                                m_aMutex;
        ComponentContext                            m_aContext;
        bool                                        m_bUseFormComponentHandlers;
        bool                                        m_bConstructed;
        bool                                        m_bHasHelpSection;
        sal_Int32                                   m_nMinHelpTextLines;
        sal_Int32                                   m_nMaxHelpTextLines;

        /// access to property meta data
        ::std::auto_ptr< OPropertyInfoService >     m_pInfoService;

    protected:
        ~DefaultFormComponentInspectorModel();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

        // XObjectInspectorModel
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getHandlerFactories() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getHasHelpSection() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getMinHelpTextLines() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getMaxHelpTextLines() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::inspection::PropertyCategoryDescriptor > SAL_CALL describeCategories(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getPropertyOrderIndex( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    public:
        // XServiceInfo - static versions
        static ::rtl::OUString getImplementationName_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

    public:
        DefaultFormComponentInspectorModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext, bool _bUseFormFormComponentHandlers = true );

    protected:
        // Service constructors
        void    createDefault();
        void    createWithHelpSection( sal_Int32 _nMinHelpTextLines, sal_Int32 _nMaxHelpTextLines );
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_DEFAULTFORMINSPECTION_HXX

