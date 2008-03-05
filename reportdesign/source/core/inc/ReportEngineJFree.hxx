#ifndef REPORTDESIGN_API_REPORTENGINEJFREE_HXX
#define REPORTDESIGN_API_REPORTENGINEJFREE_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportEngineJFree.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:56:06 $
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

#ifndef _COM_SUN_STAR_REPORT_XREPORTENGINE_HPP_
#include <com/sun/star/report/XReportEngine.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef INCLUDED_CPPUHELPER_PROPERTYSETMIXIN_HXX
#include <cppuhelper/propertysetmixin.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COMPHELPER_IMPLEMENTATIONREFERENCE_HXX
#include <comphelper/implementationreference.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

namespace reportdesign
{
    typedef ::cppu::WeakComponentImplHelper2< com::sun::star::report::XReportEngine
                                             ,com::sun::star::lang::XServiceInfo> ReportEngineBase;
    typedef ::cppu::PropertySetMixin<com::sun::star::report::XReportEngine> ReportEnginePropertySet;

    class OReportEngineJFree : public comphelper::OMutexAndBroadcastHelper,
                    public ReportEngineBase,
                    public ReportEnginePropertySet
    {
        typedef ::std::multimap< ::rtl::OUString, ::com::sun::star::uno::Any , ::comphelper::UStringMixLess>            TComponentMap;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >     m_xReport;
        ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator>         m_StatusIndicator;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >             m_xActiveConnection;
    private:
        OReportEngineJFree(const OReportEngineJFree&);
        OReportEngineJFree& operator=(const OReportEngineJFree&);
        template <typename T> void set(  const ::rtl::OUString& _sProperty
                                        ,const T& _Value
                                        ,T& _member)
        {
            BoundListeners l;
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                prepareSet(_sProperty, ::com::sun::star::uno::makeAny(_member), ::com::sun::star::uno::makeAny(_Value), &l);
                _member = _Value;
            }
            l.notify();
        }

        /** transform the report defintion format into a jfree report format.
         *
         * \return The URL of the newly created file.
         */
        ::rtl::OUString transform();

        /** returns the file url for a new model
        *
        * \return The new file url.
        */
        ::rtl::OUString getNewOutputName();

        /** generates the order statement defined by the groups of the report
        *
        * \return the ORDER BY part
        */
        ::rtl::OUString getOrderStatement() const;
    protected:
        // TODO: VirtualFunctionFinder: This is virtual function!
        //
        virtual ~OReportEngineJFree();
    public:
        typedef ::comphelper::ImplementationReference< OReportEngineJFree   ,::com::sun::star::report::XReportEngine,::com::sun::star::uno::XWeak > TReportEngine;

        OReportEngineJFree(const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& context);

        DECLARE_XINTERFACE( )
        // ::com::sun::star::lang::XServiceInfo
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
            create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
        // com::sun::star::beans::XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XReportEngine
            // Attributes
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition > SAL_CALL getReportDefinition() throw (::com::sun::star::uno::RuntimeException) ;
        virtual void SAL_CALL setReportDefinition( const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >& _reportdefinition ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getActiveConnection() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setActiveConnection( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _activeconnection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > SAL_CALL getStatusIndicator() throw (::com::sun::star::uno::RuntimeException) ;
        virtual void SAL_CALL setStatusIndicator( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >& _statusindicator ) throw (::com::sun::star::uno::RuntimeException) ;
            // Methods
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SAL_CALL createDocumentModel(  ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException) ;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SAL_CALL createDocumentAlive( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _frame ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException) ;
        virtual ::com::sun::star::util::URL SAL_CALL createDocument(  ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException) ;
        virtual void SAL_CALL interrupt(  ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException) ;

        // XComponent
        virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException)
        {
            cppu::WeakComponentImplHelperBase::addEventListener(aListener);
        }
        virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException)
        {
            cppu::WeakComponentImplHelperBase::removeEventListener(aListener);
        }
    };
}
#endif //REPORTDESIGN_API_REPORTENGINEJFREE_HXX
