/*************************************************************************
 *
 *  $RCSfile: RadioButton.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-19 11:52:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _FORMS_RADIOBUTTON_HXX_
#define _FORMS_RADIOBUTTON_HXX_

#ifndef _FORMS_FORMCOMPONENT_HXX_
#include "FormComponent.hxx"
#endif

#ifndef _COMPHELPER_PROPERTY_MULTIPLEX_HXX_
#include <comphelper/propmultiplex.hxx>
#endif

//.........................................................................
namespace frm
{

enum { RB_NOCHECK, RB_CHECK, RB_DONTKNOW };

//==================================================================
// ORadioButtonModel
//==================================================================
class ORadioButtonModel     :public OBoundControlModel
                            ,public OPropertyChangeListener
                            ,public ::comphelper::OAggregationArrayUsageHelper< ORadioButtonModel >
{
    ::rtl::OUString     m_sReferenceValue;  // Referenzwert zum Checken des Buttons
    sal_Int16           m_nDefaultChecked;  // Soll beim Reset gecheckt werden ?

    sal_Bool            m_bInReset : 1;

protected:
    sal_Int16   getState(const staruno::Any& rValue);

    virtual void            _onValueChanged();
    virtual staruno::Any    _getControlValue() const;

public:
    ORadioButtonModel(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);

    // XServiceInfo
    IMPLEMENTATION_NAME(ORadioButtonModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // OPropertySetHelper
    virtual void SAL_CALL getFastPropertyValue(staruno::Any& rValue, sal_Int32 nHandle) const;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const staruno::Any& rValue )
                throw (staruno::Exception);
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                staruno::Any& _rConvertedValue, staruno::Any& _rOldValue, sal_Int32 _nHandle, const staruno::Any& _rValue )
                throw (starlang::IllegalArgumentException);

    // XPropertySet
    virtual staruno::Reference<starbeans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(staruno::RuntimeException);
    virtual cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // XPersistObject
    virtual ::rtl::OUString SAL_CALL    getServiceName() throw(staruno::RuntimeException);
    virtual void SAL_CALL
        write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream) throw(stario::IOException, staruno::RuntimeException);
    virtual void SAL_CALL
        read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream) throw(stario::IOException, staruno::RuntimeException);

    // OPropertyChangeListener
    virtual void _propertyChanged(const starbeans::PropertyChangeEvent& evt) throw(staruno::RuntimeException);

    // XReset
    virtual void SAL_CALL reset() throw(staruno::RuntimeException);

    // OAggregationArrayUsageHelper
    virtual void fillProperties(
        staruno::Sequence< starbeans::Property >& /* [out] */ _rProps,
        staruno::Sequence< starbeans::Property >& /* [out] */ _rAggregateProps
        ) const;
    IMPLEMENT_INFO_SERVICE()

protected:
    virtual void        _reset();
    virtual sal_Bool    _commit();

protected:
    void SetSiblingPropsTo(const ::rtl::OUString& rPropName, const staruno::Any& rValue);
};

//==================================================================
// ORadioButtonControl
//==================================================================
class ORadioButtonControl: public OBoundControl
{
public:
    ORadioButtonControl(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);

    // XServiceInfo
    IMPLEMENTATION_NAME(ORadioButtonControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
};

//.........................................................................
}
//.........................................................................

#endif // _FORMS_RADIOBUTTON_HXX_

