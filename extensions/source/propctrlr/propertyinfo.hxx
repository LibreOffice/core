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

#ifndef _EXTENSIONS_PROPCTRLR_PROPERTYINFO_HXX_
#define _EXTENSIONS_PROPCTRLR_PROPERTYINFO_HXX_

#include <sal/types.h>
#include <tools/string.hxx>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <vector>

//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    //= IPropertyInfoService
    //========================================================================
    class SAL_NO_VTABLE IPropertyInfoService
    {
    public:
        virtual sal_Int32                           getPropertyId(const String& _rName) const = 0;
        virtual String                              getPropertyTranslation(sal_Int32 _nId) const = 0;
        virtual rtl::OString                        getPropertyHelpId(sal_Int32 _nId) const = 0;
        virtual sal_Int16                           getPropertyPos(sal_Int32 _nId) const = 0;
        virtual sal_uInt32                          getPropertyUIFlags(sal_Int32 _nId) const = 0;
        virtual ::std::vector< ::rtl::OUString >    getPropertyEnumRepresentations(sal_Int32 _nId) const = 0;

        // this is only temporary, until the UNOization of the property browser is completed
        virtual String                  getPropertyName( sal_Int32 _nPropId ) = 0;

        virtual ~IPropertyInfoService() { }
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_PROPERTYINFO_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
