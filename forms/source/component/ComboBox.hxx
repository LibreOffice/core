/*************************************************************************
 *
 *  $RCSfile: ComboBox.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:04 $
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

#ifndef _FORMS_COMBOBOX_HXX_
#define _FORMS_COMBOBOX_HXX_

#ifndef _FORMS_FORMCOMPONENT_HXX_
#include "FormComponent.hxx"
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/form/XChangeBroadcaster.hpp>

//.........................................................................
namespace frm
{

//==================================================================
// OComboBoxModel
//==================================================================
class OComboBoxModel
            :public OBoundControlModel
            ,public starsdb::XSQLErrorBroadcaster
            ,public ::utl::OAggregationArrayUsageHelper< OComboBoxModel >
{
    staruno::Reference<starutil::XNumberFormatter>  m_xFormatter;
    starform::ListSourceType    m_eListSourceType;      // type der list source
    staruno::Any                m_aBoundColumn;         // obsolet
    ::rtl::OUString             m_aListSource;          //
    ::rtl::OUString             m_aDefaultText;         // DefaultText
    ::rtl::OUString             m_aSaveValue;
    sal_Int32                   m_nFormatKey;
    Date                        m_aNullDate;
    sal_Int32                   m_nFieldType;
    sal_Int16                   m_nKeyType;
    sal_Bool                    m_bEmptyIsNull;         // LeerString wird als NULL interpretiert

    ::cppu::OInterfaceContainerHelper   m_aErrorListeners;

    static sal_Int32        nTextHandle;

protected:
    virtual void _onValueChanged();
    virtual staruno::Sequence<staruno::Type>    _getTypes();

public:
    OComboBoxModel(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);
    ~OComboBoxModel();

    virtual void SAL_CALL disposing();

    // OPropertySetHelper
    virtual void SAL_CALL getFastPropertyValue(staruno::Any& rValue, sal_Int32 nHandle) const;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const staruno::Any& rValue )
                throw (staruno::Exception);
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                staruno::Any& _rConvertedValue, staruno::Any& _rOldValue, sal_Int32 _nHandle, const staruno::Any& _rValue )
                throw (starlang::IllegalArgumentException);

    // XLoadListener
    virtual void         _loaded(const starlang::EventObject& rEvent);
    virtual void         _unloaded();

    virtual void SAL_CALL reloaded( const starlang::EventObject& aEvent ) throw(staruno::RuntimeException);

    // XServiceInfo
    IMPLEMENTATION_NAME(OComboBoxModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(staruno::RuntimeException);

    // UNO
    DECLARE_UNO3_AGG_DEFAULTS(OComboBoxModel, OBoundControlModel);
    virtual staruno::Any SAL_CALL queryAggregation( const staruno::Type& _rType ) throw (staruno::RuntimeException);

    // XBoundComponent
    virtual sal_Bool _commit();

    // XPropertySet
    virtual staruno::Reference<starbeans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(staruno::RuntimeException);
    virtual cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // XPersistObject
    virtual ::rtl::OUString SAL_CALL    getServiceName() throw(staruno::RuntimeException);
    virtual void SAL_CALL
        write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream) throw(stario::IOException, staruno::RuntimeException);
    virtual void SAL_CALL
        read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream) throw(stario::IOException, staruno::RuntimeException);

    // XReset
    virtual void _reset();

    // XSQLErrorBroadcaster
    virtual void SAL_CALL addSQLErrorListener(const staruno::Reference<starsdb::XSQLErrorListener>& _rxListener) throw(staruno::RuntimeException);
    virtual void SAL_CALL removeSQLErrorListener(const staruno::Reference<starsdb::XSQLErrorListener>& _rxListener) throw(staruno::RuntimeException);

    // OAggregationArrayUsageHelper
    virtual void fillProperties(
        staruno::Sequence< starbeans::Property >& /* [out] */ _rProps,
        staruno::Sequence< starbeans::Property >& /* [out] */ _rAggregateProps
        ) const;
    IMPLEMENT_INFO_SERVICE()

protected:
    void loadData();

    void onError(starsdbc::SQLException& _rException, const ::rtl::OUString& _rContextDescription);
};

//==================================================================
// OComboBoxControl
//==================================================================

class OComboBoxControl : public OBoundControl
{
public:
    OComboBoxControl(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);

    // XServiceInfo
    IMPLEMENTATION_NAME(OComboBoxControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(staruno::RuntimeException);
};

//.........................................................................
}
//.........................................................................

#endif // _FORMS_COMBOBOX_HXX_

