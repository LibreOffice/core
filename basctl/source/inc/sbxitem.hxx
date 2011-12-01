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
    ScriptDocument          m_aDocument;
    ::rtl::OUString         m_aLibName;
    ::rtl::OUString         m_aName;
    ::rtl::OUString         m_aMethodName;
    BasicIDEType            m_nType;

public:
    TYPEINFO();
    SbxItem(sal_uInt16 nWhich, const ScriptDocument& rDocument, const ::rtl::OUString& aLibName, const ::rtl::OUString& aName, BasicIDEType nType);
    SbxItem(sal_uInt16 nWhich, const ScriptDocument& rDocument, const ::rtl::OUString& aLibName, const ::rtl::OUString& aName, const ::rtl::OUString& aMethodName, BasicIDEType nType);
    SbxItem(const SbxItem&);

    virtual SfxPoolItem* Clone(SfxItemPool *pPool = 0) const;
    virtual int operator==(const SfxPoolItem&) const;

    const ScriptDocument& GetDocument() const;
    void SetDocument(const ScriptDocument& rDocument);

    const ::rtl::OUString& GetLibName() const;
    void SetLibName(const ::rtl::OUString& aLibName);

    const ::rtl::OUString& GetName() const;
    void SetName(const ::rtl::OUString& aName);

    const ::rtl::OUString& GetMethodName() const;
    void SetMethodName(const ::rtl::OUString& aMethodName);

    BasicIDEType GetType() const;
    void SetType(BasicIDEType nType);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
