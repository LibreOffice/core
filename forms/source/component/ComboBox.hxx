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

class OComboBoxModel
            :public OBoundControlModel
            ,public OEntryListHelper
            ,public OErrorBroadcaster
{
    CachedRowSet               m_aListRowSet;          // the row set to fill the list
    css::uno::Any              m_aBoundColumn;         // obsolete
    OUString                   m_aListSource;
    OUString                   m_aDefaultText;         // DefaultText
    css::uno::Any              m_aLastKnownValue;

    css::uno::Sequence<OUString>                          m_aDesignModeStringItems;
    // upon loading, in some cases we reset fill our string item list ourself. We don't want
    // to lose the user's items then, so we remember them here.
    css::uno::Reference< css::util::XNumberFormatter> m_xFormatter;

    css::form::ListSourceType  m_eListSourceType;      // ListSource's type
    bool                       m_bEmptyIsNull;         // Empty string is interpreted as NULL

    ::std::unique_ptr< ::dbtools::FormattedColumnValue > m_pValueFormatter;



protected:
    virtual css::uno::Sequence< css::uno::Type>   _getTypes() SAL_OVERRIDE;

public:
    DECLARE_DEFAULT_LEAF_XTOR( OComboBoxModel );

    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    // OPropertySetHelper
    virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle) const SAL_OVERRIDE;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue )
                throw (css::uno::Exception, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                css::uno::Any& _rConvertedValue, css::uno::Any& _rOldValue, sal_Int32 _nHandle, const css::uno::Any& _rValue )
                throw (css::lang::IllegalArgumentException) SAL_OVERRIDE;

    // XLoadListener
    virtual void SAL_CALL reloaded( const css::lang::EventObject& aEvent ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return OUString("com.sun.star.form.OComboBoxModel"); }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // UNO
    DECLARE_UNO3_AGG_DEFAULTS(OComboBoxModel, OBoundControlModel)
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& _rType ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPersistObject
    virtual OUString SAL_CALL    getServiceName() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL
        write(const css::uno::Reference< css::io::XObjectOutputStream>& _rxOutStream) throw(css::io::IOException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL
        read(const css::uno::Reference< css::io::XObjectInputStream>& _rxInStream) throw(css::io::IOException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps
    ) const SAL_OVERRIDE;
    virtual void describeAggregateProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rAggregateProps
    ) const SAL_OVERRIDE;

    // XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // prevent method hiding
    using OBoundControlModel::getFastPropertyValue;

protected:
    // OBoundControlModel overridables
    virtual css::uno::Any   translateDbColumnToControlValue( ) SAL_OVERRIDE;
    virtual bool            commitControlValueToDbColumn( bool _bPostReset ) SAL_OVERRIDE;

    virtual void            onConnectedDbColumn( const css::uno::Reference< css::uno::XInterface >& _rxForm ) SAL_OVERRIDE;
    virtual void            onDisconnectedDbColumn() SAL_OVERRIDE;

    virtual css::uno::Any   getDefaultForReset() const SAL_OVERRIDE;

    virtual void            resetNoBroadcast() SAL_OVERRIDE;

    // OEntryListHelper overridables
    virtual void    stringItemListChanged( ControlModelLock& _rInstanceLock ) SAL_OVERRIDE;
    virtual void    connectedExternalListSource( ) SAL_OVERRIDE;
    virtual void    disconnectedExternalListSource( ) SAL_OVERRIDE;
    virtual void    refreshInternalEntryList() SAL_OVERRIDE;

protected:
    void loadData( bool _bForce );

    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

class OComboBoxControl : public OBoundControl
{
public:
    explicit OComboBoxControl(const css::uno::Reference< css::uno::XComponentContext>& _rxContext);

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return OUString("com.sun.star.form.OComboBoxControl"); }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


}


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_COMBOBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
