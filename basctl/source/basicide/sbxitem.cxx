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

#include "sbxitem.hxx"

SbxItem::SbxItem(sal_uInt16 nWhich_, const ScriptDocument& rDocument, const ::rtl::OUString& aLibName, const ::rtl::OUString& aName, BasicIDEType nType)
    :SfxPoolItem( nWhich_ )
    ,m_aDocument(rDocument)
    ,m_aLibName(aLibName)
    ,m_aName(aName)
    ,m_nType(nType)
{
}

SbxItem::SbxItem(sal_uInt16 nWhich_, const ScriptDocument& rDocument, const ::rtl::OUString& aLibName, const ::rtl::OUString& aName, const ::rtl::OUString& aMethodName, BasicIDEType nType)
    :SfxPoolItem( nWhich_ )
    ,m_aDocument(rDocument)
    ,m_aLibName(aLibName)
    ,m_aName(aName)
    ,m_aMethodName(aMethodName)
    ,m_nType(nType)
{
}

SbxItem::SbxItem(const SbxItem& rCopy)
    :SfxPoolItem(rCopy)
    ,m_aDocument(rCopy.m_aDocument)
    ,m_aLibName(rCopy.m_aLibName)
    ,m_aName(rCopy.m_aName)
    ,m_aMethodName(rCopy.m_aMethodName)
    ,m_nType(rCopy.m_nType)
{
}

SfxPoolItem *SbxItem::Clone(SfxItemPool*) const
{
    return new SbxItem(*this);
}

int SbxItem::operator==(const SfxPoolItem& rCmp) const
{
    DBG_ASSERT( rCmp.ISA( SbxItem ), "==: Kein SbxItem!" );
    return (SfxPoolItem::operator==(rCmp) &&
	    m_aDocument == ((const SbxItem&)rCmp).m_aDocument &&
	    m_aLibName == ((const SbxItem&)rCmp).m_aLibName &&
	    m_aName == ((const SbxItem&)rCmp).m_aName &&
	    m_aMethodName == ((const SbxItem&)rCmp).m_aMethodName &&
	    m_nType == ((const SbxItem&)rCmp).m_nType);
}

const ScriptDocument& SbxItem::GetDocument() const
{
    return m_aDocument;
}

const ::rtl::OUString& SbxItem::GetLibName() const
{
    return m_aLibName;
}

const ::rtl::OUString& SbxItem::GetName() const
{
    return m_aName;
}

const ::rtl::OUString& SbxItem::GetMethodName() const
{
    return m_aMethodName;
}

BasicIDEType SbxItem::GetType() const
{
    return m_nType;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
