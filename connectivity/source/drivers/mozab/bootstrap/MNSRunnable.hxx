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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MOZAB_BOOTSTRAP_MNSRUNNABLE_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MOZAB_BOOTSTRAP_MNSRUNNABLE_HXX
#include "mozillasrc/MNSDeclares.hxx"
#include <osl/thread.hxx>

#include "mozillasrc/MNSInclude.hxx"
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/mozilla/XCodeProxy.hpp>


using namespace com::sun::star::uno;
using namespace com::sun::star::mozilla;

namespace connectivity
{
    namespace mozab
    {
    class MNSRunnable : public nsIRunnable
        {
        public:
            nsIRunnable * ProxiedObject();
            MNSRunnable();
            virtual ~MNSRunnable();
            NS_DECL_ISUPPORTS
            NS_DECL_NSIRUNNABLE

        public:
            sal_Int32 StartProxy(const ::com::sun::star::uno::Reference< ::com::sun::star::mozilla::XCodeProxy >& aCode); //Call this to start proxy

        private:
            nsIRunnable* _ProxiedObject;
            Reference<XCodeProxy>  xCode;
#if OSL_DEBUG_LEVEL > 0
            oslThreadIdentifier m_oThreadID;
#endif
        };
    }
}
#endif //_CONNECTIVITY_MAB_MOZABHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
