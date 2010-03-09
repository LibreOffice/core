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
#include "precompiled_configmgr.hxx"

#include "simpletypehelper.hxx"
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>

namespace configmgr
{
    namespace uno = com::sun::star::uno;
    namespace SimpleTypeHelper
    {

        uno::Type getBooleanType() { return ::getBooleanCppuType(); }

        uno::Type getShortType()        { return ::getCppuType(static_cast<sal_Int16 const*>(0)); }
        uno::Type getIntType()      { return ::getCppuType(static_cast<sal_Int32 const*>(0)); }
        uno::Type getLongType()     { return ::getCppuType(static_cast<sal_Int64 const*>(0)); }

        uno::Type getDoubleType()   { return ::getCppuType(static_cast<double const*>(0)); }

        uno::Type getStringType()   { return ::getCppuType(static_cast<rtl::OUString const*>(0)); }

        uno::Type getBinaryType()   { return ::getCppuType(static_cast<uno::Sequence<sal_Int8> const*>(0)); }
        uno::Type getAnyType()      { return ::getCppuType(static_cast<uno::Any const*>(0)); }
    }
}
