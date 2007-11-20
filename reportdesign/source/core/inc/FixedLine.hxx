/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FixedLine.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 18:58:34 $
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
#ifndef RPT_FIXEDLINE_HXX
#define RPT_FIXEDLINE_HXX

#ifndef INCLUDED_CPPUHELPER_PROPERTYSETMIXIN_HXX
#include <cppuhelper/propertysetmixin.hxx>
#endif
#ifndef _COM_SUN_STAR_REPORT_XFIXEDLINE_HPP_
#include <com/sun/star/report/XFixedLine.hpp>
#endif
#ifndef RPT_REPORTCONTROLMODEL_HXX
#include "ReportControlModel.hxx"
#endif
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#include "ReportHelperDefines.hxx"


namespace reportdesign
{
    typedef ::cppu::PropertySetMixin<        com::sun::star::report::XFixedLine > FixedLinePropertySet;
    typedef ::cppu::WeakComponentImplHelper2<    com::sun::star::report::XFixedLine
                                                ,com::sun::star::lang::XServiceInfo > FixedLineBase;

    /** \class OFixedLine Defines the implementation of a \interface com:::sun::star::report::XFixedLine
     * \ingroup reportdesign_api
     *
     */
    class OFixedLine :  public comphelper::OBaseMutex,
                            public FixedLineBase,
                            public FixedLinePropertySet
    {
        friend class OShapeHelper;
        OReportControlModel     m_aProps;
        ::com::sun::star::drawing::LineStyle    m_LineStyle;
        ::com::sun::star::drawing::LineDash     m_LineDash;
        sal_Int32               m_nOrientation;
        ::sal_Int32             m_LineColor;
        ::sal_Int16             m_LineTransparence;
        ::sal_Int32             m_LineWidth;

    private:
        OFixedLine(const OFixedLine&);
        OFixedLine& operator=(const OFixedLine&);

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
        void checkIndex(sal_Int32 _nIndex);
    protected:
        virtual ~OFixedLine();
    public:
        explicit OFixedLine(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & _xContext);
        explicit OFixedLine(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & _xContext
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & _xFactory
                            ,::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& _xShape
                            ,sal_Int32 _nOrientation);

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

        // XReportComponent
        REPORTCOMPONENT_HEADER()

        // XShape
        SHAPE_HEADER()

        // XShapeDescriptor
        virtual ::rtl::OUString SAL_CALL getShapeType(  ) throw (::com::sun::star::uno::RuntimeException);

        // XReportControlModel
        REPORTCONTROLMODEL_HEADER()

        // XFixedLine
        virtual ::sal_Int32 SAL_CALL getOrientation() throw (::com::sun::star::beans::UnknownPropertyException,::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setOrientation( ::sal_Int32 _orientation ) throw (::com::sun::star::beans::UnknownPropertyException,::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::drawing::LineStyle SAL_CALL getLineStyle() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setLineStyle( ::com::sun::star::drawing::LineStyle _linestyle ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::drawing::LineDash SAL_CALL getLineDash() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setLineDash( const ::com::sun::star::drawing::LineDash& _linedash ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getLineColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setLineColor( ::sal_Int32 _linecolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int16 SAL_CALL getLineTransparence() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setLineTransparence( ::sal_Int16 _linetransparence ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getLineWidth() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setLineWidth( ::sal_Int32 _linewidth ) throw (::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::report::XReportControlFormat
        REPORTCONTROLFORMAT_HEADER()

        // XCloneable
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException);

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

        // XChild
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

        // XContainer
        virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

        // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException);

        // XIndexReplace
        virtual void SAL_CALL replaceByIndex( ::sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XIndexContainer
        virtual void SAL_CALL insertByIndex( ::sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XIndexAccess
        virtual ::sal_Int32 SAL_CALL getCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    };
}
#endif //RPT_FIXEDLINE_HXX

