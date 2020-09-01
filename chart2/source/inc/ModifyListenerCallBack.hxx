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

#ifndef INCLUDED_CHART2_SOURCE_INC_MODIFYLISTENERCALLBACK_HXX
#define INCLUDED_CHART2_SOURCE_INC_MODIFYLISTENERCALLBACK_HXX

#include <tools/link.hxx>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include "charttoolsdllapi.hxx"

namespace chart {

/** Use this class as a member if you want to listen on a XModifyBroadcaster
without becoming a XModifyListener yourself
 */

class ModifyListenerCallBack_impl;

class OOO_DLLPUBLIC_CHARTTOOLS ModifyListenerCallBack
{
public:
    explicit ModifyListenerCallBack( const Link<void*,void>& rCallBack );

    virtual ~ModifyListenerCallBack();

    void startListening( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyBroadcaster >& xBroadcaster );
    SAL_DLLPRIVATE void stopListening();

private: //methods
    ModifyListenerCallBack( const ModifyListenerCallBack& ) = delete;

private: //member
    ModifyListenerCallBack_impl* pModifyListener_impl;
    css::uno::Reference< css::util::XModifyListener >   m_xModifyListener;
};

} // namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
