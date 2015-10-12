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
#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_MODELEVENTLISTENER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_MODELEVENTLISTENER_HXX

#include <com/sun/star/document/XEventListener.hpp>
#include <cppuhelper/implbase.hxx>

namespace writerfilter {
namespace dmapper{


class ModelEventListener : public cppu::WeakImplHelper<css::document::XEventListener>
{
    bool m_bIndexes;
    bool m_bControls;
public:
    ModelEventListener(bool bIndexes, bool bControls);
    virtual ~ModelEventListener();

    virtual void SAL_CALL notifyEvent(const css::document::EventObject& Event) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw (css::uno::RuntimeException, std::exception) override;

};
}//namespace writerfilter
}//namespace dmapper
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
