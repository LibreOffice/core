/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: defaultforminspection.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-10 10:47:28 $
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

#ifndef INSPECTORMODELBASE_HXX
#include "inspectormodelbase.hxx"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <memory>

//........................................................................
namespace pcr
{
//........................................................................

    class OPropertyInfoService;
    //====================================================================
    //= DefaultFormComponentInspectorModel
    //====================================================================
    class DefaultFormComponentInspectorModel : public ImplInspectorModel
    {
    private:
        bool                                        m_bUseFormComponentHandlers;
        bool                                        m_bConstructed;

        /// access to property meta data
        ::std::auto_ptr< OPropertyInfoService >     m_pInfoService;

    protected:
        ~DefaultFormComponentInspectorModel();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

        // XObjectInspectorModel
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getHandlerFactories() throw (::com::sun::star::uno::RuntimeException);
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

