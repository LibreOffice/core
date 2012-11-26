/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
    POOLITEM_FACTORY()
    SbxItem() : SfxPoolItem( 0 ), m_aDocument( ScriptDocument::NoDocument ), m_nType( 0 ) {}
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
