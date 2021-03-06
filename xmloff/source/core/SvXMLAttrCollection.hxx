/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "SvXMLAttr.hxx"

#include <xmloff/namespacemap.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vector>

class SvXMLAttrCollection
{
public:
    SvXMLNamespaceMap      aNamespaceMap;
    std::vector<SvXMLAttr> aAttrs;

    bool operator==(const SvXMLAttrCollection &rCmp) const;
    bool AddAttr( const OUString& rLName,
                      const OUString& rValue );
    bool AddAttr( const OUString& rPrefix,
                      const OUString& rNamespace,
                      const OUString& rLName,
                      const OUString& rValue );
    bool AddAttr( const OUString& rPrefix,
                      const OUString& rLName,
                      const OUString& rValue );

    bool SetAt( size_t i,
                    const OUString& rLName,
                    const OUString& rValue );
    bool SetAt( size_t i,
                    const OUString& rPrefix,
                    const OUString& rNamespace,
                    const OUString& rLName,
                    const OUString& rValue );
    bool SetAt( size_t i,
                    const OUString& rPrefix,
                    const OUString& rLName,
                    const OUString& rValue );

    void Remove( size_t i );

    size_t GetAttrCount() const;
    const OUString& GetAttrLName(size_t i) const;
    const OUString& GetAttrValue(size_t i) const;
    OUString GetAttrNamespace( size_t i ) const;
    OUString GetAttrPrefix( size_t i ) const;
    const OUString& GetNamespace( sal_uInt16 i ) const;
    const OUString& GetPrefix( sal_uInt16 i ) const;
    sal_uInt16 GetFirstNamespaceIndex() const;
    sal_uInt16 GetNextNamespaceIndex( sal_uInt16 nIdx ) const;

private:
    sal_uInt16 GetPrefixPos( size_t i ) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

