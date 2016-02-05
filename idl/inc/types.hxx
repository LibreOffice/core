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

#ifndef INCLUDED_IDL_INC_TYPES_HXX
#define INCLUDED_IDL_INC_TYPES_HXX

#include <rtl/strbuf.hxx>
#include <tools/ref.hxx>
#include <basobj.hxx>

struct SvSlotElement;
typedef std::vector< SvSlotElement* > SvSlotElementList;

class SvMetaSlot;

class SvMetaType;

class SvMetaAttribute : public SvMetaReference
{
    tools::SvRef<SvMetaType> aType;
    SvIdentifier             aSlotId;
    SvBOOL                   aExport;
    SvBOOL                   aReadOnlyDoc;
    bool                     bNewAttr;

protected:
    virtual void ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm ) override;
public:
                        SvMetaAttribute();
                        SvMetaAttribute( SvMetaType * );

    void                SetNewAttribute( bool bNew )
                        { bNewAttr = bNew; }
    void                SetSlotId( const SvIdentifier & rId )
                        { aSlotId = rId; }
    const SvIdentifier & GetSlotId() const;
    bool                GetExport() const;
    bool                GetHidden() const;
    bool                GetReadOnlyDoc() const;
    SvMetaType *        GetType() const;

    virtual bool        IsMethod() const;
    virtual bool        IsVariable() const;
    virtual OString     GetMangleName( bool bVariable ) const;


    virtual bool        Test( SvIdlDataBase &, SvTokenStream & rInStm ) override;
    virtual bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) override;
    sal_uLong           MakeSfx( OStringBuffer& rAtrrArray );
    virtual void        Insert( SvSlotElementList&, const OString& rPrefix,
                                SvIdlDataBase& );
};

enum { TYPE_METHOD, TYPE_STRUCT, TYPE_BASE, TYPE_ENUM,
      TYPE_CLASS };
class SvMetaType : public SvMetaExtern
{
    SvIdentifier                aCName;
    SvIdentifier                aBasicPostfix;
    SvIdentifier                aBasicName;
    SvRefMemberList<SvMetaAttribute *>* pAttrList;
    int                         nType;
    bool                        bIsItem;
    bool                        bIsShell;
    char                        cParserChar;

    void    WriteSfxItem( const OString& rItemName, SvIdlDataBase & rBase,
                        SvStream & rOutStm );
protected:
    bool        ReadNamesSvIdl( SvIdlDataBase & rBase,
                                         SvTokenStream & rInStm );
    virtual void ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) override;

    bool    ReadHeaderSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
public:
            SvMetaType();
            SvMetaType( const OString& rTypeName, char cParserChar,
                                const OString& rCName );
            SvMetaType( const OString& rTypeName,
                        char cParserChar,
                        const OString& rCName, const OString& rBasicName,
                        const OString& rBasicPostfix );

    virtual ~SvMetaType();

    SvRefMemberList<SvMetaAttribute *>& GetAttrList() const;
    sal_uLong           GetAttrCount() const
                        {
                            return pAttrList ? pAttrList->size() : 0L;
                        }

    void                SetType( int nT );
    int                 GetType() const { return nType; }
    SvMetaType *        GetBaseType() const;
    SvMetaType *        GetReturnType() const;
    bool                IsItem() const { return bIsItem; }
    bool                IsShell() const { return bIsShell; }

    void                SetBasicName(const OString& rName)
                        { aBasicName.setString(rName); }

    const OString&      GetBasicName() const;
    const OString&      GetCName() const;
    char                GetParserChar() const { return cParserChar; }

    virtual bool        SetName( const OString& rName, SvIdlDataBase * = nullptr ) override;


    virtual bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) override;

    sal_uLong           MakeSfx( OStringBuffer& rAtrrArray );
    virtual void        WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm );
    bool                ReadMethodArgs( SvIdlDataBase & rBase,
                                             SvTokenStream & rInStm );
    OString             GetParserString() const;
};

class SvMetaTypeString : public SvMetaType
{
public:
            SvMetaTypeString();
};

class SvMetaEnumValue : public SvMetaObject
{
public:
    SvMetaEnumValue();

    virtual bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) override;
};

class SvMetaTypeEnum : public SvMetaType
{
    SvRefMemberList<SvMetaEnumValue *> aEnumValueList;
    OString aPrefix;
protected:
    virtual void ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) override;
public:
            SvMetaTypeEnum();

    sal_uLong           Count() const { return aEnumValueList.size(); }
    const OString&      GetPrefix() const { return aPrefix; }
    SvMetaEnumValue *   GetObject( sal_uLong n ) const
                        { return aEnumValueList[n]; }

    virtual bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) override;
};

class SvMetaTypevoid : public SvMetaType
{
public:
            SvMetaTypevoid();
};


#endif // INCLUDED_IDL_INC_TYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
