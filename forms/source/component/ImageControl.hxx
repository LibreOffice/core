/*************************************************************************
 *
 *  $RCSfile: ImageControl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:05 $
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

#ifndef _FRM_IMAGE_CONTROL_HXX_
#define _FRM_IMAGE_CONTROL_HXX_

#include "FormComponent.hxx"

#ifndef _PRODUCE_HXX //autogen
#include <svtools/imgprod.hxx>
#endif

#ifndef _COM_SUN_STAR_FORM_XIMAGEPRODUCERSUPPLIER_HPP_
#include <com/sun/star/form/XImageProducerSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMOUSELISTENER_HPP_
#include <com/sun/star/awt/XMouseListener.hpp>
#endif

#ifndef _UNOTOOLS_PROPERTY_MULTIPLEX_HXX_
#include <unotools/propmultiplex.hxx>
#endif
using namespace utl;

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
// OImageControlModel
//==================================================================
class OImageControlModel
                :public starform::XImageProducerSupplier
                ,public ::utl::OAggregationArrayUsageHelper<OImageControlModel>
                ,public OBoundControlModel
                ,public OPropertyChangeListener
{
    staruno::Reference<starawt::XImageProducer>     m_xImageProducer;
    ImageProducer*                                  m_pImageProducer;
    sal_Bool                m_bReadOnly;

    // Helper functions
    void UpdateFromField();

protected:
    // UNO Anbindung
    virtual staruno::Sequence<staruno::Type> _getTypes();
    virtual void _onValueChanged();

    virtual sal_Bool        _approve(sal_Int32 _nColumnType);

    inline ImageProducer* GetImageProducer() { return m_pImageProducer; }

public:
    OImageControlModel(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);
    virtual ~OImageControlModel();

    virtual void SAL_CALL getFastPropertyValue(staruno::Any& rValue, sal_Int32 nHandle ) const;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const staruno::Any& rValue);

    virtual sal_Bool SAL_CALL convertFastPropertyValue(staruno::Any& rConvertedValue, staruno::Any& rOldValue, sal_Int32 nHandle, const staruno::Any& rValue )
        throw(starlang::IllegalArgumentException);

    // UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OImageControlModel, OBoundControlModel);
    virtual staruno::Any SAL_CALL queryAggregation(const staruno::Type& _rType) throw(staruno::RuntimeException);

// starform::XBoundComponent
    virtual staruno::Any _getControlValue() const;
    virtual void _reset();

// starlang::XServiceInfo
    IMPLEMENTATION_NAME(OImageControlModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

// OComponentHelper
    virtual void SAL_CALL disposing();

// OPropertyChangeListener
    virtual void _propertyChanged( const starbeans::PropertyChangeEvent& ) throw(staruno::RuntimeException);

// staruno::Reference<starbeans::XPropertySet>
    virtual staruno::Reference<starbeans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(staruno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

// stario::XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName();
    virtual void SAL_CALL write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream);
    virtual void SAL_CALL read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream);

// starform::XImageProducerSupplier
    virtual staruno::Reference<starawt::XImageProducer> SAL_CALL getImageProducer() { return m_xImageProducer; }

// ::utl::OAggregationArrayUsageHelper
    virtual void fillProperties(
        staruno::Sequence< starbeans::Property >& /* [out] */ _rProps,
        staruno::Sequence< starbeans::Property >& /* [out] */ _rAggregateProps
        ) const;
    IMPLEMENT_INFO_SERVICE()
};

//==================================================================
// OImageControlControl
//==================================================================
class OImageControlControl : public starawt::XMouseListener,
                               public OBoundControl
{
protected:
    // UNO Anbindung
    virtual staruno::Sequence<staruno::Type> _getTypes();

public:
    OImageControlControl(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);

// UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OImageControlControl, OBoundControl);
    virtual staruno::Any SAL_CALL queryAggregation(const staruno::Type& _rType) throw(staruno::RuntimeException);

// starlang::XEventListener
    virtual void SAL_CALL disposing(const starlang::EventObject& _rSource) throw(staruno::RuntimeException)
        { OBoundControl::disposing(_rSource); }

// starlang::XServiceInfo
    IMPLEMENTATION_NAME(OImageControlControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

// starawt::XMouseListener
    virtual void SAL_CALL mousePressed(const starawt::MouseEvent& e);
    virtual void SAL_CALL mouseReleased(const starawt::MouseEvent& e) { }
    virtual void SAL_CALL mouseEntered(const starawt::MouseEvent& e)    { }
    virtual void SAL_CALL mouseExited(const starawt::MouseEvent& e) { }

// starawt::XControl
    virtual void SAL_CALL createPeer(const staruno::Reference<starawt::XToolkit>& _rToolkit, const staruno::Reference<starawt::XWindowPeer>& _rParent) throw(staruno::RuntimeException);
};

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // _FRM_IMAGE_CONTROL_HXX_

