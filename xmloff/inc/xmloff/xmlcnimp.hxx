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

#ifndef _XMLOFF_XMLCNIMP_HXX
#define _XMLOFF_XMLCNIMP_HXX

#include "xmloff/dllapi.h"     //XMLOFF_DLLPUBLIC
#include <sal/types.h>         //sal_uInt16 and sal_Bool
#include <boost/scoped_ptr.hpp>

namespace rtl { class OUString; }
class SvXMLAttrCollection; //Forward declaration only.

class XMLOFF_DLLPUBLIC SvXMLAttrContainerData
{
private:
    boost::scoped_ptr<SvXMLAttrCollection> pimpl;

public:
    SvXMLAttrContainerData();
    SvXMLAttrContainerData(const SvXMLAttrContainerData &rCopy);
    ~SvXMLAttrContainerData();

    int  operator ==( const SvXMLAttrContainerData& rCmp ) const;

    sal_Bool AddAttr( const ::rtl::OUString& rLName, const rtl::OUString& rValue );
    sal_Bool AddAttr( const ::rtl::OUString& rPrefix, const rtl::OUString& rNamespace,
                    const ::rtl::OUString& rLName, const rtl::OUString& rValue );
    sal_Bool AddAttr( const ::rtl::OUString& rPrefix,
                  const ::rtl::OUString& rLName,
                  const ::rtl::OUString& rValue );

    size_t GetAttrCount() const;
    const rtl::OUString GetAttrNamespace( size_t i ) const;
    const rtl::OUString GetAttrPrefix( size_t i ) const;
    const rtl::OUString& GetAttrLName( size_t i ) const;
    const rtl::OUString& GetAttrValue( size_t i ) const;

    sal_uInt16 GetFirstNamespaceIndex() const;
    sal_uInt16 GetNextNamespaceIndex( sal_uInt16 nIdx ) const;
    const ::rtl::OUString& GetNamespace( sal_uInt16 i ) const;
    const ::rtl::OUString& GetPrefix( sal_uInt16 i ) const;

    sal_Bool SetAt( size_t i,
                const ::rtl::OUString& rLName, const rtl::OUString& rValue );
    sal_Bool SetAt( size_t i,
                const ::rtl::OUString& rPrefix, const rtl::OUString& rNamespace,
                const ::rtl::OUString& rLName, const rtl::OUString& rValue );
    sal_Bool SetAt( size_t i,
                const ::rtl::OUString& rPrefix,
                const ::rtl::OUString& rLName,
                const ::rtl::OUString& rValue );

    void Remove( size_t i );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
