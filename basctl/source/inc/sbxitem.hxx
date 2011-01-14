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

const sal_uInt16 BASICIDE_TYPE_UNKNOWN  =   0;
const sal_uInt16 BASICIDE_TYPE_SHELL    =   1;
const sal_uInt16 BASICIDE_TYPE_LIBRARY  =   2;
const sal_uInt16 BASICIDE_TYPE_MODULE   =   3;
const sal_uInt16 BASICIDE_TYPE_DIALOG   =   4;
const sal_uInt16 BASICIDE_TYPE_METHOD   =   5;


class SbxItem : public SfxPoolItem
{
    ScriptDocument          m_aDocument;
    String                  m_aLibName;
    String                  m_aName;
    String                  m_aMethodName;
    sal_uInt16                  m_nType;

public:
    TYPEINFO();
    SbxItem( sal_uInt16 nWhich, const ScriptDocument& rDocument, const String& aLibName, const String& aName, sal_uInt16 nType );
    SbxItem( sal_uInt16 nWhich, const ScriptDocument& rDocument, const String& aLibName, const String& aName, const String& aMethodName, sal_uInt16 nType );
    SbxItem( const SbxItem& );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    const ScriptDocument&
                            GetDocument() const { return m_aDocument; }
    void                    SetDocument( const ScriptDocument& rDocument ) { m_aDocument = rDocument; }

    const String&           GetLibName() const { return m_aLibName; }
    void                    SetLibName( const String& aLibName ) { m_aLibName = aLibName; }

    const String&           GetName() const { return m_aName; }
    void                    SetName( const String& aName ) { m_aName = aName; }

    const String&           GetMethodName() const { return m_aMethodName; }
    void                    SetMethodName( const String& aMethodName ) { m_aMethodName = aMethodName; }

    sal_uInt16                  GetType() const { return m_nType; }
    void                    SetType( sal_uInt16 nType ) { m_nType = nType; }
};


#endif
