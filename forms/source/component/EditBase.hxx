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
#define PF_RESERVED_2           0x2000
#define PF_RESERVED_3           0x1000
#define PF_RESERVED_4           0x0800
#define PF_RESERVED_5           0x0400
#define PF_RESERVED_6           0x0200
#define PF_RESERVED_7           0x0100

#define PF_SPECIAL_FLAGS        0xFF00


namespace frm
{


//= OEditBaseModel

class OEditBaseModel :  public OBoundControlModel
{
    sal_Int16                   m_nLastReadVersion;

protected:
// [properties]         for all EditingFields
    ::com::sun::star::uno::Any  m_aDefault;
    OUString             m_aDefaultText;             // default value
    sal_Bool                    m_bEmptyIsNull : 1;         // empty string will be interepreted as NULL when committing
    sal_Bool                    m_bFilterProposal : 1;      // use a list of possible value in filtermode
// [properties]

    sal_Int16   getLastReadVersion() const { return m_nLastReadVersion; }

public:
    DECLARE_DEFAULT_BOUND_XTOR( OEditBaseModel );

    // XPersistObject
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XPropertySet
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue,
                                          sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                                        throw(::com::sun::star::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception, std::exception);
    using ::cppu::OPropertySetHelper::getFastPropertyValue;

    // XPropertyState
    virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const;

protected:
    // new properties common to all edit models should be handled with the following two methods
    void SAL_CALL readCommonEditProperties(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream);
    void SAL_CALL writeCommonEditProperties(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream);
    void defaultCommonEditProperties();

    virtual sal_uInt16 getPersistenceFlags() const;
        // derived classes may use this if they want this base class to write additional version flags
        // (one of the PF_.... constants). After ::read they may ask for that flags with getLastReadVersion
};


}


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_EDITBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
