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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_EDITBASE_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_EDITBASE_HXX

#include "FormComponent.hxx"
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/form/XChangeBroadcaster.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>

// persistence flags for use with the version id
#define PF_HANDLE_COMMON_PROPS  0x8000
    // Derived classes which use their own persistence methods (read/write) and have an own
    // version handling therein may want to clear this flag in getPersistenceFlags.
    // If done so, this class will write an version without a call to writeCommonEditProperties.
#define PF_FAKE_FORMATTED_FIELD 0x4000
    // .... hmmm .... a fake, as the name suggests. see OFormattedFieldWrapper

#define PF_SPECIAL_FLAGS        0xFF00


namespace frm
{

class OEditBaseModel :  public OBoundControlModel
{
    sal_Int16                   m_nLastReadVersion;

protected:
// [properties]         for all EditingFields
    css::uno::Any           m_aDefault;
    OUString                m_aDefaultText;             // default value
    bool                    m_bEmptyIsNull : 1;         // empty string will be interepreted as NULL when committing
    bool                    m_bFilterProposal : 1;      // use a list of possible value in filtermode
// [properties]

    sal_Int16   getLastReadVersion() const { return m_nLastReadVersion; }

public:
    OEditBaseModel(
        const css::uno::Reference< css::uno::XComponentContext>& _rxFactory,
        const OUString& _rUnoControlModelTypeName,
        const OUString& _rDefault,
        const bool _bSupportExternalBinding,
        const bool _bSupportsValidation
    );
    DECLARE_DEFAULT_CLONE_CTOR( OEditBaseModel )
    virtual ~OEditBaseModel();

    // XPersistObject
    virtual void SAL_CALL write(const css::uno::Reference< css::io::XObjectOutputStream>& _rxOutStream) throw ( css::io::IOException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL read(const css::uno::Reference< css::io::XObjectInputStream>& _rxInStream) throw ( css::io::IOException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPropertySet
    virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle ) const SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(css::uno::Any& rConvertedValue, css::uno::Any& rOldValue,
                                          sal_Int32 nHandle, const css::uno::Any& rValue )
                                        throw(css::lang::IllegalArgumentException) SAL_OVERRIDE;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const css::uno::Any& rValue) throw ( css::uno::Exception, std::exception) SAL_OVERRIDE;
    using ::cppu::OPropertySetHelper::getFastPropertyValue;

    // XPropertyState
    virtual css::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const SAL_OVERRIDE;

protected:
    // new properties common to all edit models should be handled with the following two methods
    void SAL_CALL readCommonEditProperties(const css::uno::Reference< css::io::XObjectInputStream>& _rxInStream);
    void SAL_CALL writeCommonEditProperties(const css::uno::Reference< css::io::XObjectOutputStream>& _rxOutStream);
    void defaultCommonEditProperties();

    virtual sal_uInt16 getPersistenceFlags() const;
    // derived classes may use this if they want this base class to write additional version flags
    // (one of the PF_.... constants). After ::read they may ask for that flags with getLastReadVersion
};


}


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_EDITBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
