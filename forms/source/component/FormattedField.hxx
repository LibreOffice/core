/*************************************************************************
 *
 *  $RCSfile: FormattedField.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: fs $ $Date: 2001-02-13 17:41:39 $
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

#ifndef _FORMS_FORMATTEDFIELD_HXX_
#define _FORMS_FORMATTEDFIELD_HXX_

#ifndef _FORMS_EDITBASE_HXX_
#include "EditBase.hxx"
#endif

#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif

#ifndef _COMPHELPER_PROPERTY_MULTIPLEX_HXX_
#include <comphelper/propmultiplex.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif


//.........................................................................
namespace frm
{

//==================================================================
//= OFormattedModel
//==================================================================
class OFormattedModel
                :public OEditBaseModel
                ,public OPropertyChangeListener
                ,public ::comphelper::OAggregationArrayUsageHelper< OFormattedModel >
{
    // das Original, falls ich die Format-Properties meines aggregierten Models gefaket, d.h. von dem Feld, an das
    // ich gebunden bin, weitergereicht habe (nur gueltig wenn loaded)
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>   m_xOriginalFormatter;
    ::com::sun::star::util::Date        m_aNullDate;
    ::com::sun::star::uno::Any          m_aSaveValue;

    sal_Int16                           m_nKeyType;
    sal_Bool                            m_bOriginalNumeric : 1,
                                        m_bNumeric : 1;
        // analog fuer TreatAsNumeric-Property

    static ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>    s_xDefaultFormatter;
    static sal_Int32                        nValueHandle;
        // falls ich wirklich mal einen selber benutzen muss, wird der zwischen allen Instanzen geteilt

protected:
    virtual void _onValueChanged();

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>  calcDefaultFormatsSupplier() const;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>  calcFormFormatsSupplier() const;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>  calcFormatsSupplier() const;
    sal_Int32 calcFormatKey() const;
    void getFormatDescription(::rtl::OUString& sFormat, LanguageType& eLanguage);

    OFormattedModel(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

    friend InterfaceRef SAL_CALL OFormattedModel_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    friend class OFormattedFieldWrapper;
public:

// ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OFormattedModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

// starform::XBoundComponent
    virtual sal_Bool _commit();

// stario::XPersistObject
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference<stario::XObjectOutputStream>& _rxOutStream);
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference<stario::XObjectInputStream>& _rxInStream);
    virtual ::rtl::OUString SAL_CALL getServiceName();

// ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue,
                                          sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                                        throw(::com::sun::star::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue);

// starform::XLoadListener
    virtual void SAL_CALL loaded(const ::com::sun::star::lang::EventObject& rEvent);
    virtual void _loaded(const ::com::sun::star::lang::EventObject& rEvent);
    virtual void _unloaded();

// starform::XReset
    virtual void _reset( void );

// ::com::sun::star::beans::XPropertyState
    void setPropertyToDefaultByHandle(sal_Int32 nHandle);
    ::com::sun::star::uno::Any getPropertyDefaultByHandle(sal_Int32 nHandle) const;

    void SAL_CALL setPropertyToDefault(const ::rtl::OUString& aPropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

    // OAggregationArrayUsageHelper
    virtual void fillProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
        ) const;
    IMPLEMENT_INFO_SERVICE()

// OPropertyChangeListener
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);

protected:
    virtual sal_Int16 getPersistenceFlags() const;
        // as we have an own version handling for persistence
};

//==================================================================
//= OFormattedControl
//==================================================================
typedef ::cppu::ImplHelper1< ::com::sun::star::awt::XKeyListener> OFormattedControl_BASE;
class OFormattedControl :    public OBoundControl
                            ,public OFormattedControl_BASE
{
    sal_uInt32              m_nKeyEvent;

public:
    OFormattedControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    virtual ~OFormattedControl();

    DECLARE_UNO3_AGG_DEFAULTS(OFormattedControl, OBoundControl);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

// ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OFormattedControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::awt::XKeyListener
    virtual void SAL_CALL keyPressed(const ::com::sun::star::awt::KeyEvent& e);
    virtual void SAL_CALL keyReleased(const ::com::sun::star::awt::KeyEvent& e);

// ::com::sun::star::awt::XControl
    virtual void SAL_CALL setDesignMode(sal_Bool bOn);

private:
    DECL_LINK( OnKeyPressed, void* );
};


//.........................................................................
}
//.........................................................................

#endif // _FORMS_FORMATTEDFIELD_HXX_

