/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ImageControl.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: ihi $ $Date: 2006-10-18 13:17:31 $
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

#ifndef _FRM_IMAGE_CONTROL_HXX_
#define _FRM_IMAGE_CONTROL_HXX_

#include "FormComponent.hxx"

#ifndef _PRODUCE_HXX
#include "imgprod.hxx"
#endif

#ifndef _COM_SUN_STAR_FORM_XIMAGEPRODUCERSUPPLIER_HPP_
#include <com/sun/star/form/XImageProducerSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMOUSELISTENER_HPP_
#include <com/sun/star/awt/XMouseListener.hpp>
#endif

#ifndef _COMPHELPER_PROPERTY_MULTIPLEX_HXX_
#include <comphelper/propmultiplex.hxx>
#endif
#ifndef _COMPHELPER_IMPLEMENTATIONREFERENCE_HXX
#include <comphelper/implementationreference.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

using namespace comphelper;

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
// OImageControlModel
//==================================================================
typedef ::cppu::ImplHelper2 <   ::com::sun::star::form::XImageProducerSupplier
                            ,   ::com::sun::star::awt::XImageProducer
                            >   OImageControlModel_Base;

class OImageControlModel
                :public OImageControlModel_Base
                ,public ::comphelper::OAggregationArrayUsageHelper<OImageControlModel>
                ,public OBoundControlModel
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer>    m_xImageProducer;
    ImageProducer*                                  m_pImageProducer;
    sal_Bool                                        m_bReadOnly;

protected:
    // UNO Anbindung
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

    inline ImageProducer* GetImageProducer() { return m_pImageProducer; }

public:
    DECLARE_DEFAULT_LEAF_XTOR( OImageControlModel );

    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception);

    virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue, sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
        throw(::com::sun::star::lang::IllegalArgumentException);

    // UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OImageControlModel, OBoundControlModel);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    IMPLEMENTATION_NAME(OImageControlModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

    // OComponentHelper
    virtual void SAL_CALL disposing();

    // OPropertyChangeListener
    virtual void _propertyChanged( const ::com::sun::star::beans::PropertyChangeEvent& ) throw(::com::sun::star::uno::RuntimeException);

    // Reference< XPropertySet>
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XImageProducerSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer> SAL_CALL getImageProducer() throw ( ::com::sun::star::uno::RuntimeException);

    // XImageProducer
    virtual void SAL_CALL addConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startProduction(  ) throw (::com::sun::star::uno::RuntimeException);

    // ::comphelper::OAggregationArrayUsageHelper
    virtual void fillProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
        ) const;
    IMPLEMENT_INFO_SERVICE()

    // prevent method hiding
    using OBoundControlModel::disposing;
    using OBoundControlModel::getFastPropertyValue;

protected:
    // OBoundControlModel overridables
    virtual ::com::sun::star::uno::Any
                            translateDbColumnToControlValue( );
    virtual sal_Bool        commitControlValueToDbColumn( bool _bPostReset );

    virtual void            doSetControlValue( const ::com::sun::star::uno::Any& _rValue );

    virtual sal_Bool        approveDbColumnType(sal_Int32 _nColumnType);

    virtual void            resetNoBroadcast();

protected:
    DECLARE_XCLONEABLE();

    void implConstruct();

    /** updates the database column we're bound to with the given stream

        <p>If the stream is <NULL/>, then XColumnUpdate::updateNull will be called for the
        column.</p>

        @precond
            m_xColumnUpdate is not <NULL/>
    */
    void        updateColumnWithStream( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& _rxStream );

    /** displays the image described by the given URL
        @precond
            our own mutex is locked
    */
    sal_Bool    handleNewImageURL( const ::rtl::OUString& _rURL, ValueChangeInstigator _eInstigator );
};

//==================================================================
//= OImageIndicator (helper class for OImageControlControl)
//==================================================================
typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::awt::XImageConsumer
                                >   OImageIndicator_Base;

class OImageIndicator : public OImageIndicator_Base
{
private:
    sal_Bool    m_bIsProducing  : 1;
    sal_Bool    m_bIsEmptyImage : 1;

public:
    OImageIndicator( );

            void        reset();
    inline  sal_Bool    isEmptyImage() const { return m_bIsEmptyImage; }

protected:
    ~OImageIndicator( );
    // XImageProducer
    virtual void SAL_CALL init( sal_Int32 Width, sal_Int32 Height ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setColorModel( sal_Int16 BitCount, const ::com::sun::star::uno::Sequence< sal_Int32 >& RGBAPal, sal_Int32 RedMask, sal_Int32 GreenMask, sal_Int32 BlueMask, sal_Int32 AlphaMask ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPixelsByBytes( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, const ::com::sun::star::uno::Sequence< sal_Int8 >& aProducerData, sal_Int32 nOffset, sal_Int32 nScanSize ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPixelsByLongs( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, const ::com::sun::star::uno::Sequence< sal_Int32 >& aProducerData, sal_Int32 nOffset, sal_Int32 nScanSize ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL complete( sal_Int32 Status, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer >& xProducer ) throw (::com::sun::star::uno::RuntimeException);

private:
    OImageIndicator( const OImageIndicator& );              // never implemented
    OImageIndicator& operator=( const OImageIndicator& );   // never implemented
};

//==================================================================
//= OImageControlControl
//==================================================================
class OImageControlControl  :public ::com::sun::star::awt::XMouseListener
                            ,public OBoundControl
{
private:
    typedef ::comphelper::ImplementationReference< OImageIndicator, ::com::sun::star::awt::XImageConsumer >
                                ImageIndicatorReference;
    ImageIndicatorReference     m_pImageIndicator;

protected:
    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

public:
    OImageControlControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

    // UNO
    DECLARE_UNO3_AGG_DEFAULTS(OImageControlControl, OBoundControl);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException)
        { OBoundControl::disposing(_rSource); }

    // XServiceInfo
    IMPLEMENTATION_NAME(OImageControlControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

    // XMouseListener
    virtual void SAL_CALL mousePressed(const ::com::sun::star::awt::MouseEvent& e) throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mouseReleased(const ::com::sun::star::awt::MouseEvent& e) throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mouseEntered(const ::com::sun::star::awt::MouseEvent& e) throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mouseExited(const ::com::sun::star::awt::MouseEvent& e) throw ( ::com::sun::star::uno::RuntimeException);

    // XControl
    virtual sal_Bool SAL_CALL setModel(const ::com::sun::star::uno::Reference<starawt::XControlModel>& _rxModel ) throw (::com::sun::star::uno::RuntimeException);

    // prevent method hiding
    using OBoundControl::disposing;

private:
    void    implClearGraphics( sal_Bool _bForce );
    void    implInsertGraphics();

    /** determines whether the control does currently have an empty grahic set
    */
    bool    impl_isEmptyGraphics_nothrow() const;
};

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // _FRM_IMAGE_CONTROL_HXX_

