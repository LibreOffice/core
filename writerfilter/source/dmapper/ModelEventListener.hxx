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
#ifndef INCLUDED_MODELEVENTLISTENER_HXX
#define INCLUDED_MODELEVENTLISTENER_HXX

#include <WriterFilterDllApi.hxx>
#include <com/sun/star/document/XEventListener.hpp>
#include <cppuhelper/implbase1.hxx>

namespace writerfilter {
namespace dmapper{


class ModelEventListener :
    public cppu::WeakImplHelper1< ::com::sun::star::document::XEventListener >
{
    bool m_bIndexes;
    bool m_bControls;
public:
    ModelEventListener(bool bIndexes, bool bControls);
    ~ModelEventListener();

    virtual void SAL_CALL notifyEvent( const ::com::sun::star::document::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception);

};
}//namespace writerfilter
}//namespace dmapper
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
