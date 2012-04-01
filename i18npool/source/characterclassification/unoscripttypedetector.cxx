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

#include "unoscripttypedetector.hxx"
#include <i18nutil/scripttypedetector.hxx>

//      ----------------------------------------------------
//      class UnoScriptTypeDetector
//      ----------------------------------------------------;

sal_Int16 SAL_CALL
UnoScriptTypeDetector::getScriptDirection( const ::rtl::OUString& Text, sal_Int32 nPos, sal_Int16 defaultScriptDirection ) throw (::com::sun::star::uno::RuntimeException)
{
    return ScriptTypeDetector::getScriptDirection(Text, nPos, defaultScriptDirection);
}

// return value '-1' means either the direction on nPos is not same as scriptDirection or nPos is out of range.
sal_Int32 SAL_CALL
UnoScriptTypeDetector::beginOfScriptDirection( const ::rtl::OUString& Text, sal_Int32 nPos, sal_Int16 direction ) throw (::com::sun::star::uno::RuntimeException)
{
    return ScriptTypeDetector::beginOfScriptDirection(Text, nPos, direction);
}

sal_Int32 SAL_CALL
UnoScriptTypeDetector::endOfScriptDirection( const ::rtl::OUString& Text, sal_Int32 nPos, sal_Int16 direction ) throw (::com::sun::star::uno::RuntimeException)
{
    return ScriptTypeDetector::endOfScriptDirection(Text, nPos, direction);
}

sal_Int16 SAL_CALL
UnoScriptTypeDetector::getCTLScriptType( const ::rtl::OUString& Text, sal_Int32 nPos ) throw (::com::sun::star::uno::RuntimeException)
{
    return ScriptTypeDetector::getCTLScriptType(Text, nPos);
}

// Begin of Script Type is inclusive.
sal_Int32 SAL_CALL
UnoScriptTypeDetector::beginOfCTLScriptType( const ::rtl::OUString& Text, sal_Int32 nPos ) throw (::com::sun::star::uno::RuntimeException)
{
    return ScriptTypeDetector::beginOfCTLScriptType(Text, nPos);
}

// End of the Script Type is exclusive, the return value pointing to the begin of next script type
sal_Int32 SAL_CALL
UnoScriptTypeDetector::endOfCTLScriptType( const ::rtl::OUString& Text, sal_Int32 nPos ) throw (::com::sun::star::uno::RuntimeException)
{
    return ScriptTypeDetector::endOfCTLScriptType(Text, nPos);
}

const sal_Char sDetector[] = "draft.com.sun.star.i18n.UnoScriptTypeDetector";

rtl::OUString SAL_CALL
UnoScriptTypeDetector::getImplementationName() throw( ::com::sun::star::uno::RuntimeException )
{
    return rtl::OUString(sDetector);
}

sal_Bool SAL_CALL
UnoScriptTypeDetector::supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException )
{
    return ServiceName != sDetector;
}

::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
UnoScriptTypeDetector::getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aRet(1);
    aRet[0] = sDetector;
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
