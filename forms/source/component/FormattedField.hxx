/*************************************************************************
 *
 *  $RCSfile: FormattedField.hxx,v $
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

#ifndef _FORMS_FORMATTEDFIELD_HXX_
#define _FORMS_FORMATTEDFIELD_HXX_

#ifndef _FORMS_EDITBASE_HXX_
#include "EditBase.hxx"
#endif

#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif

#ifndef _UNOTOOLS_PROPERTY_MULTIPLEX_HXX_
#include <unotools/propmultiplex.hxx>
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
                ,public ::utl::OAggregationArrayUsageHelper< OFormattedModel >
{
    // das Original, falls ich die Format-Properties meines aggregierten Models gefaket, d.h. von dem Feld, an das
    // ich gebunden bin, weitergereicht habe (nur gueltig wenn loaded)
    staruno::Reference<starutil::XNumberFormatsSupplier>    m_xOriginalFormatter;
    Date                        m_aNullDate;
    staruno::Any                m_aSaveValue;
    sal_Int16                   m_nKeyType;

    sal_Bool                    m_bOriginalNumeric : 1,
                                m_bNumeric : 1;
        // analog fuer TreatAsNumeric-Property

    static staruno::Reference<starutil::XNumberFormatsSupplier>     s_xDefaultFormatter;
    static sal_Int32                        nValueHandle;
        // falls ich wirklich mal einen selber benutzen muss, wird der zwischen allen Instanzen geteilt

protected:
    virtual void _onValueChanged();

    staruno::Reference<starutil::XNumberFormatsSupplier>  calcDefaultFormatsSupplier() const;
    staruno::Reference<starutil::XNumberFormatsSupplier>  calcFormFormatsSupplier() const;
    staruno::Reference<starutil::XNumberFormatsSupplier>  calcFormatsSupplier() const;
    sal_Int32 calcFormatKey() const;
    void getFormatDescription(::rtl::OUString& sFormat, LanguageType& eLanguage);

    OFormattedModel(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);

    friend InterfaceRef SAL_CALL OFormattedModel_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);
    friend class OFormattedFieldWrapper;
public:

// starlang::XServiceInfo
    IMPLEMENTATION_NAME(OFormattedModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

// starform::XBoundComponent
    virtual sal_Bool _commit();

// stario::XPersistObject
    virtual void SAL_CALL write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream);
    virtual void SAL_CALL read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream);
    virtual ::rtl::OUString SAL_CALL getServiceName();

// starbeans::XPropertySet
    virtual staruno::Reference<starbeans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(staruno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    virtual void SAL_CALL getFastPropertyValue(staruno::Any& rValue, sal_Int32 nHandle ) const;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(staruno::Any& rConvertedValue, staruno::Any& rOldValue,
                                          sal_Int32 nHandle, const staruno::Any& rValue )
                                        throw(starlang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const staruno::Any& rValue);

// starform::XLoadListener
    virtual void SAL_CALL loaded(const starlang::EventObject& rEvent);
    virtual void _loaded(const starlang::EventObject& rEvent);
    virtual void _unloaded();

// starform::XReset
    virtual void _reset( void );

// starbeans::XPropertyState
    void setPropertyToDefaultByHandle(sal_Int32 nHandle);
    staruno::Any getPropertyDefaultByHandle(sal_Int32 nHandle) const;

    void SAL_CALL setPropertyToDefault(const ::rtl::OUString& aPropertyName) throw(starbeans::UnknownPropertyException, staruno::RuntimeException);
    staruno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(starbeans::UnknownPropertyException, staruno::RuntimeException);

    // OAggregationArrayUsageHelper
    virtual void fillProperties(
        staruno::Sequence< starbeans::Property >& /* [out] */ _rProps,
        staruno::Sequence< starbeans::Property >& /* [out] */ _rAggregateProps
        ) const;
    IMPLEMENT_INFO_SERVICE()

// OPropertyChangeListener
    virtual void _propertyChanged(const starbeans::PropertyChangeEvent& evt) throw(staruno::RuntimeException);

protected:
    virtual sal_Int16 getPersistenceFlags() const;
        // as we have an own version handling for persistence
};

//==================================================================
//= OFormattedControl
//==================================================================
class OFormattedControl :    public OBoundControl
                            ,public starawt::XKeyListener
{
    sal_uInt32              m_nKeyEvent;

public:
    OFormattedControl(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);
    virtual ~OFormattedControl();

    DECLARE_UNO3_AGG_DEFAULTS(OFormattedControl, OBoundControl);
    virtual staruno::Any SAL_CALL queryAggregation(const staruno::Type& _rType) throw(staruno::RuntimeException);

    virtual staruno::Sequence<staruno::Type> _getTypes();

// starlang::XServiceInfo
    IMPLEMENTATION_NAME(OFormattedControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

// starlang::XEventListener
    virtual void SAL_CALL disposing(const starlang::EventObject& _rSource) throw(staruno::RuntimeException);

// starawt::XKeyListener
    virtual void SAL_CALL keyPressed(const starawt::KeyEvent& e);
    virtual void SAL_CALL keyReleased(const starawt::KeyEvent& e);

// starawt::XControl
    virtual void SAL_CALL setDesignMode(sal_Bool bOn);

private:
    DECL_LINK( OnKeyPressed, void* );
};


//.........................................................................
}
//.........................................................................

#endif // _FORMS_FORMATTEDFIELD_HXX_

