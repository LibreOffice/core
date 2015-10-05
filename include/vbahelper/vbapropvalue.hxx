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
#include <ooo/vba/XPropValue.hpp>
#include <cppuhelper/implbase.hxx>

#include <vbahelper/vbahelper.hxx>

typedef ::cppu::WeakImplHelper< ov::XPropValue > PropValueImpl_BASE;

class VBAHELPER_DLLPUBLIC PropListener
{
public:
    virtual void setValueEvent( const css::uno::Any& value ) = 0;
    virtual css::uno::Any getValueEvent() = 0;

protected:
    ~PropListener() {}
};


class VBAHELPER_DLLPUBLIC ScVbaPropValue : public PropValueImpl_BASE
{
    PropListener* m_pListener;
public:
    ScVbaPropValue( PropListener* pListener );

    // Attributes
    virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    OUString SAL_CALL getDefaultPropertyName() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE { return OUString("Value"); }

};
#endif //SC_VBA_PROPVALULE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
