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
#ifndef BASCTL_SBXITEM_HXX
#define BASCTL_SBXITEM_HXX

#include "scriptdocument.hxx"
#include <svl/poolitem.hxx>

namespace basctl
{

enum ItemType
{
    TYPE_UNKNOWN,
    TYPE_SHELL,
    TYPE_LIBRARY,
    TYPE_MODULE,
    TYPE_DIALOG,
    TYPE_METHOD
};

class SbxItem : public SfxPoolItem
{
    const ScriptDocument    m_aDocument;
    const OUString          m_aLibName;
    const OUString          m_aName;
    const OUString          m_aMethodName;
    ItemType                m_eType;

public:
    TYPEINFO();
    SbxItem(sal_uInt16 nWhich, const ScriptDocument& rDocument, const OUString& aLibName, const OUString& aName, ItemType);
    SbxItem(sal_uInt16 nWhich, const ScriptDocument& rDocument, const OUString& aLibName, const OUString& aName, const OUString& aMethodName, ItemType eType);

    virtual SfxPoolItem* Clone(SfxItemPool *pPool = 0) const;
    virtual int operator==(const SfxPoolItem&) const;

    ScriptDocument const& GetDocument () const { return m_aDocument; }
    OUString const& GetLibName () const { return m_aLibName; }
    OUString const& GetName () const { return m_aName; }
    OUString const& GetMethodName () const { return m_aMethodName; }
    ItemType GetType () const { return m_eType; }
};

} // namespace basctl

// For baside.sdi, because I don't know how to use nested names in it.
using basctl::SbxItem;

#endif // BASCTL_SBXITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
