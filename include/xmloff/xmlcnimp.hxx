/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_XMLOFF_XMLCNIMP_HXX
#define INCLUDED_XMLOFF_XMLCNIMP_HXX

#include <memory>

#include <rtl/ustring.hxx>
#include <xmloff/dllapi.h>

class SvXMLAttrCollection;

class XMLOFF_DLLPUBLIC SvXMLAttrContainerData
{
private:
    std::unique_ptr<SvXMLAttrCollection> pimpl;

public:
    SvXMLAttrContainerData();
    SvXMLAttrContainerData(const SvXMLAttrContainerData &rCopy);
    ~SvXMLAttrContainerData();

    bool operator ==( const SvXMLAttrContainerData& rCmp ) const;

    bool AddAttr( const OUString& rLName, const OUString& rValue );
    bool AddAttr( const OUString& rPrefix, const OUString& rNamespace,
                    const OUString& rLName, const OUString& rValue );
    bool AddAttr( const OUString& rPrefix,
                  const OUString& rLName,
                  const OUString& rValue );

    size_t GetAttrCount() const;
    const OUString GetAttrNamespace( size_t i ) const;
    const OUString GetAttrPrefix( size_t i ) const;
    const OUString& GetAttrLName( size_t i ) const;
    const OUString& GetAttrValue( size_t i ) const;

    sal_uInt16 GetFirstNamespaceIndex() const;
    sal_uInt16 GetNextNamespaceIndex( sal_uInt16 nIdx ) const;
    const OUString& GetNamespace( sal_uInt16 i ) const;
    const OUString& GetPrefix( sal_uInt16 i ) const;

    bool SetAt( size_t i,
                const OUString& rLName, const OUString& rValue );
    bool SetAt( size_t i,
                const OUString& rPrefix, const OUString& rNamespace,
                const OUString& rLName, const OUString& rValue );
    bool SetAt( size_t i,
                const OUString& rPrefix,
                const OUString& rLName,
                const OUString& rValue );

    void Remove( size_t i );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
