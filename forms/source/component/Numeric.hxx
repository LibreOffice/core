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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_NUMERIC_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_NUMERIC_HXX

#include "EditBase.hxx"


namespace frm
{



//= ONumericModel

class ONumericModel
                :public OEditBaseModel
{
private:
    ::com::sun::star::uno::Any          m_aSaveValue;
protected:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes() SAL_OVERRIDE;

public:
    DECLARE_DEFAULT_LEAF_XTOR( ONumericModel );

    // ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(ONumericModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(std::exception) SAL_OVERRIDE;

    // ::com::sun::star::io::XPersistObject
    virtual OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
    ) const SAL_OVERRIDE;

protected:
    // OBoundControlModel overridables
    virtual ::com::sun::star::uno::Any
                            translateDbColumnToControlValue( ) SAL_OVERRIDE;
    virtual sal_Bool        commitControlValueToDbColumn( bool _bPostReset ) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Any
                            getDefaultForReset() const SAL_OVERRIDE;
    virtual void            resetNoBroadcast() SAL_OVERRIDE;

protected:
    DECLARE_XCLONEABLE();
};


//= ONumericControl

class ONumericControl: public OBoundControl
{
protected:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes() SAL_OVERRIDE;

public:
    ONumericControl(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxFactory);

    // ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(ONumericControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(std::exception) SAL_OVERRIDE;
};


}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_NUMERIC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
