/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataSeriesPointWrapper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-08-17 12:11:38 $
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
#ifndef CHART_DATASERIESPOINTWRAPPER_HXX
#define CHART_DATASERIESPOINTWRAPPER_HXX

#include "ServiceMacros.hxx"
#include "WrappedPropertySet.hxx"
#include "ReferenceSizePropertyProvider.hxx"

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <com/sun/star/chart2/XDataSeries.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#include <boost/shared_ptr.hpp>

namespace chart
{

namespace wrapper
{

class Chart2ModelContact;

class DataSeriesPointWrapper : public ::cppu::ImplInheritanceHelper4<
                                          WrappedPropertySet
                                        , com::sun::star::lang::XServiceInfo
                                        , com::sun::star::lang::XInitialization
                                           , com::sun::star::lang::XComponent
                                           , com::sun::star::lang::XEventListener
                                        >
                                        , public ReferenceSizePropertyProvider

{
public:
    enum eType
    {
        DATA_SERIES,
        DATA_POINT
    };

    //this constructor needs an initialize call afterwards
    DataSeriesPointWrapper( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );

    DataSeriesPointWrapper( eType eType
            , sal_Int32 nSeriesIndexInNewAPI
            , sal_Int32 nPointIndex //ignored for series
            , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact  );

    virtual ~DataSeriesPointWrapper();

    bool isSupportingAreaProperties();
    bool isLinesForbidden();

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

    // ___lang::XInitialization___
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
                throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    //ReferenceSizePropertyProvider
    virtual void setCurrentSizeAsReference();
    virtual ::com::sun::star::uno::Any getReferenceSize();
    virtual ::com::sun::star::awt::Size getCurrentSizeForReference();

protected:
    // ____ XComponent ____
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::lang::XEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference<
                                               ::com::sun::star::lang::XEventListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener ____
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

protected:
    // ____ WrappedPropertySet ____
    virtual const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& getPropertySequence();
    virtual const std::vector< WrappedProperty* > createWrappedProperties();
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                        ::com::sun::star::beans::XPropertySet > getInnerPropertySet();

    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //own methods
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > getDataSeries();
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getDataPointProperties();

private:

    void SAL_CALL getStatisticsPropertyValue
        ( ::com::sun::star::uno::Any& rValue,
          sal_Int32 nHandle ) const;

    void SAL_CALL setStatisticsPropertyValue_NoBroadcast(
        sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception);

    // ----------------------------------------

    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    ::cppu::OInterfaceContainerHelper           m_aEventListenerContainer;

    eType               m_eType;
    sal_Int32           m_nSeriesIndexInNewAPI;
    sal_Int32           m_nPointIndex;

    sal_Bool            m_bLinesAllowed;

    //this should only be used, if the DataSeriesPointWrapper is initialized via the XInitialize interface
    //because a big change in the chartmodel may leed to an dataseriespointer thats not connected to the model anymore
    //with the indizes instead we are can aleays get the new dataseries
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDataSeries >     m_xDataSeries;
};

} //  namespace wrapper
} //  namespace chart

// CHART_DATASERIESPOINTWRAPPER_HXX
#endif
