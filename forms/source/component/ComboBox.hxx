/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_COMBOBOX_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_COMBOBOX_HXX

#include "FormComponent.hxx"
#include "errorbroadcaster.hxx"
#include "entrylisthelper.hxx"
#include "cachedrowset.hxx"

#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>

#include <connectivity/formattedcolumnvalue.hxx>

#include <cppuhelper/interfacecontainer.hxx>

#include <vcl/timer.hxx>


namespace frm
{


// OComboBoxModel

class OComboBoxModel
            :public OBoundControlModel
            ,public OEntryListHelper
            ,public OErrorBroadcaster
{
    CachedRowSet                            m_aListRowSet;          // the row set to fill the list
    ::com::sun::star::uno::Any              m_aBoundColumn;         // obsolet
    OUString                         m_aListSource;
    OUString                         m_aDefaultText;         // DefaultText
    ::com::sun::star::uno::Any              m_aLastKnownValue;

    StringSequence                          m_aDesignModeStringItems;
        // upon loading, in some cases we reset fill our string item list ourself. We don't want
        // to lose the user's items then, so we remember them here.


    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter> m_xFormatter;

    ::com::sun::star::form::ListSourceType  m_eListSourceType;      // ListSource's type
    bool                                m_bEmptyIsNull;         // Empty string is interpreted as NULL

    ::std::auto_ptr< ::dbtools::FormattedColumnValue >
                                            m_pValueFormatter;



protected:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type>   _getTypes() SAL_OVERRIDE;

public:
    DECLARE_DEFAULT_LEAF_XTOR( OComboBoxModel );

    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    // OPropertySetHelper
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle) const SAL_OVERRIDE;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                throw (::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                ::com::sun::star::uno::Any& _rConvertedValue, ::com::sun::star::uno::Any& _rOldValue, sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue )
                throw (::com::sun::star::lang::IllegalArgumentException) SAL_OVERRIDE;

    // XLoadListener
    virtual void SAL_CALL reloaded( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    IMPLEMENTATION_NAME(OComboBoxModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // UNO
    DECLARE_UNO3_AGG_DEFAULTS(OComboBoxModel, OBoundControlModel)
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPersistObject
    virtual OUString SAL_CALL    getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL
        write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL
        read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
    ) const SAL_OVERRIDE;
    virtual void describeAggregateProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
    ) const SAL_OVERRIDE;

    // XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // prevent method hiding
    using OBoundControlModel::getFastPropertyValue;

protected:
    // OBoundControlModel overridables
    virtual ::com::sun::star::uno::Any
                            translateDbColumnToControlValue( ) SAL_OVERRIDE;
    virtual sal_Bool        commitControlValueToDbColumn( bool _bPostReset ) SAL_OVERRIDE;

    virtual void            onConnectedDbColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxForm ) SAL_OVERRIDE;
    virtual void            onDisconnectedDbColumn() SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Any
                            getDefaultForReset() const SAL_OVERRIDE;

    virtual void            resetNoBroadcast() SAL_OVERRIDE;

    // OEntryListHelper overriables
    virtual void    stringItemListChanged( ControlModelLock& _rInstanceLock ) SAL_OVERRIDE;
    virtual void    connectedExternalListSource( ) SAL_OVERRIDE;
    virtual void    disconnectedExternalListSource( ) SAL_OVERRIDE;
    virtual void    refreshInternalEntryList() SAL_OVERRIDE;

protected:
    void loadData( bool _bForce );

    DECLARE_XCLONEABLE();
};


// OComboBoxControl


class OComboBoxControl : public OBoundControl
{
public:
    OComboBoxControl(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxContext);

    // XServiceInfo
    IMPLEMENTATION_NAME(OComboBoxControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


}


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_COMBOBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
