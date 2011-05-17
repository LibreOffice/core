/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_MAB_MOZAB_RUNNABLE_HXX_
#define _CONNECTIVITY_MAB_MOZAB_RUNNABLE_HXX_
#include "MNSDeclares.hxx"
#include <osl/thread.hxx>

#include <MNSInclude.hxx>
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
