/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: binaryfilterbase.hxx,v $
 * $Revision: 1.3 $
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

#ifndef OOX_CORE_BINARYFILTERBASE_HXX
#define OOX_CORE_BINARYFILTERBASE_HXX

#include <rtl/ref.hxx>
#include "oox/core/filterbase.hxx"

namespace oox {
namespace core {

// ============================================================================

class BinaryFilterBase : public FilterBase
{
public:
    explicit            BinaryFilterBase(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxGlobalFactory );

    virtual             ~BinaryFilterBase();

private:
    virtual StorageRef  implCreateStorage(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStream ) const;
    virtual StorageRef  implCreateStorage(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& rxOutStream ) const;
};

typedef ::rtl::Reference< BinaryFilterBase > BinaryFilterRef;

// ============================================================================

} // namespace core
} // namespace oox

#endif

