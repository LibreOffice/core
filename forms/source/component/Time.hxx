/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Time.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-09 13:32:11 $
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

#ifndef _FORMS_TIME_HXX_
#define _FORMS_TIME_HXX_

#ifndef _FORMS_EDITBASE_HXX_
#include "EditBase.hxx"
#endif
#ifndef _FORMS_LIMITED_FORMATS_HXX_
#include "limitedformats.hxx"
#endif

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
//= OTimeModel
//==================================================================
class OTimeModel
                :public OEditBaseModel
                ,public OLimitedFormats
{
private:
    ::com::sun::star::uno::Any      m_aSaveValue;
    sal_Bool                        m_bDateTimeField;

protected:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

public:
    DECLARE_DEFAULT_LEAF_XTOR( OTimeModel );

    // stario::XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XPropertySet
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue,
                                          sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                                        throw(::com::sun::star::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception);

    // ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OTimeModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

    // OControlModel's property handling
    virtual void describeFixedProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
    ) const;

    // prevent method hiding
    using OBoundControlModel::getFastPropertyValue;

protected:
    // OBoundControlModel overridables
    virtual ::com::sun::star::uno::Any
                            translateDbColumnToControlValue( );
    virtual sal_Bool        commitControlValueToDbColumn( bool _bPostReset );

    virtual ::com::sun::star::uno::Any
                            translateControlValueToExternalValue( ) const;
    virtual ::com::sun::star::uno::Any
                            translateExternalValueToControlValue( ) const;

    virtual ::com::sun::star::uno::Any
                            translateControlValueToValidatableValue( ) const;

    virtual ::com::sun::star::uno::Any
                            getDefaultForReset() const;

    virtual void            onConnectedDbColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxForm );

    virtual sal_Bool        approveValueBinding( const ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XValueBinding >& _rxBinding );

protected:
    DECLARE_XCLONEABLE();

private:
    /** translates the control value (the VCL-internal integer representation of a date) into
        a UNO-Date.
    */
    void                    impl_translateControlValueToUNOTime(
                                ::com::sun::star::uno::Any& _rUNOValue ) const;
};

//==================================================================
//= OTimeControl
//==================================================================
class OTimeControl: public OBoundControl
{
protected:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

public:
    OTimeControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    DECLARE_UNO3_AGG_DEFAULTS(OTimeControl, OBoundControl);

    // ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OTimeControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();
};

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // _FORMS_TIME_HXX_

