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
#ifndef _HASHMAPS_HXX
#define _HASHMAPS_HXX

#include <ZipEntry.hxx>
#include <rtl/ref.hxx>
#include <boost/unordered_map.hpp>

struct eqFunc
{
    sal_Bool operator()( const rtl::OUString &r1,
                         const rtl::OUString &r2) const
    {
        return r1 == r2;
    }
};

class ZipPackageFolder;
namespace com { namespace sun { namespace star { namespace packages {
class ContentInfo;
} } } }

typedef boost::unordered_map < rtl::OUString,
                        ZipPackageFolder *,
                        ::rtl::OUStringHash,
                        eqFunc > FolderHash;

typedef boost::unordered_map < rtl::OUString,
                        rtl::Reference < com::sun::star::packages::ContentInfo >,
                        ::rtl::OUStringHash,
                        eqFunc > ContentHash;

typedef boost::unordered_map < rtl::OUString,
                        ZipEntry,
                        rtl::OUStringHash,
                        eqFunc > EntryHash;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
