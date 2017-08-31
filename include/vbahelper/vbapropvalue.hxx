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
#ifndef SC_VBA_PROPVALULE_HXX
#define SC_VBA_PROPVALULE_HXX

#include <exception>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <cppuhelper/implbase.hxx>
#include <ooo/vba/XPropValue.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vbahelper/vbadllapi.h>
#include <vbahelper/vbahelper.hxx>

class VBAHELPER_DLLPUBLIC PropListener
{
public:
    virtual void setValueEvent( const css::uno::Any& value ) = 0;
    virtual css::uno::Any getValueEvent() = 0;

protected:
    ~PropListener() {}
};


class VBAHELPER_DLLPUBLIC ScVbaPropValue : public ::cppu::WeakImplHelper< ov::XPropValue >
{
    PropListener* m_pListener;
public:
    ScVbaPropValue( PropListener* pListener );

    // Attributes
    virtual css::uno::Any SAL_CALL getValue() override;
    virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;

    OUString SAL_CALL getDefaultPropertyName() override { return OUString("Value"); }

};
#endif //SC_VBA_PROPVALULE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
