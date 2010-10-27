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

#ifndef CHART2_MODIFYLISTENERCALLBACK_HXX
#define CHART2_MODIFYLISTENERCALLBACK_HXX

// header for class Link
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
    explicit ModifyListenerCallBack( const Link& rCallBack );

    virtual ~ModifyListenerCallBack();

    void startListening( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyBroadcaster >& xBroadcaster );
    SAL_DLLPRIVATE void stopListening();

private: //methods
    SAL_DLLPRIVATE ModifyListenerCallBack();
    SAL_DLLPRIVATE ModifyListenerCallBack( const ModifyListenerCallBack& );

private: //member
    ModifyListenerCallBack_impl* pModifyListener_impl;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XModifyListener >   m_xModifyListener;
};

} // namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
