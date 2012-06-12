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
#ifndef _SBXITEM_HXX
#define _SBXITEM_HXX

#include "scriptdocument.hxx"
#include <svl/poolitem.hxx>

enum BasicIDEType
{
    BASICIDE_TYPE_UNKNOWN,
    BASICIDE_TYPE_SHELL,
    BASICIDE_TYPE_LIBRARY,
    BASICIDE_TYPE_MODULE,
    BASICIDE_TYPE_DIALOG,
    BASICIDE_TYPE_METHOD
};

class SbxItem : public SfxPoolItem
{
    const ScriptDocument    m_aDocument;
    const ::rtl::OUString   m_aLibName;
    const ::rtl::OUString   m_aName;
    const ::rtl::OUString   m_aMethodName;
    BasicIDEType            m_nType;

public:
    TYPEINFO();
    SbxItem(sal_uInt16 nWhich, const ScriptDocument& rDocument, const ::rtl::OUString& aLibName, const ::rtl::OUString& aName, BasicIDEType nType);
    SbxItem(sal_uInt16 nWhich, const ScriptDocument& rDocument, const ::rtl::OUString& aLibName, const ::rtl::OUString& aName, const ::rtl::OUString& aMethodName, BasicIDEType nType);
    SbxItem(const SbxItem&);

    virtual SfxPoolItem* Clone(SfxItemPool *pPool = 0) const;
    virtual int operator==(const SfxPoolItem&) const;

    const ScriptDocument& GetDocument() const;
    const ::rtl::OUString& GetLibName() const;
    const ::rtl::OUString& GetName() const;
    const ::rtl::OUString& GetMethodName() const;
    BasicIDEType GetType() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
