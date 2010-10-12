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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include "initwindowpeer.hxx"

using namespace ::com::sun::star;


uno::Any SAL_CALL InitWindowPeer::getWindowHandle( const uno::Sequence< sal_Int8 >& ProcessId, sal_Int16 SystemType )
    throw ( uno::RuntimeException )
{
    uno::Any aRes;
    sal_Int32 nHwnd = (sal_Int32)m_hwnd;
    aRes <<= nHwnd;
    return aRes;
}


uno::Reference< awt::XToolkit > SAL_CALL InitWindowPeer::getToolkit()
    throw ( uno::RuntimeException )
{
    return uno::Reference< awt::XToolkit >();
}

void SAL_CALL InitWindowPeer::setPointer( const uno::Reference< awt::XPointer >& Pointer )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::setBackground( sal_Int32 Color )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::invalidate( sal_Int16 Flags )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::invalidateRect( const awt::Rectangle& Rect, sal_Int16 Flags )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::dispose()
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::removeEventListener( const uno::Reference< lang::XEventListener >& aListener )
    throw ( uno::RuntimeException )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
