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
#include <tools/solar.h>
#include <tools/ref.hxx>
#include "basobj.hxx"

class SvMetaType;
class SvMetaSlot;
typedef SvRefMemberList< SvMetaSlot* > SvSlotElementList;

class SvMetaAttribute : public SvMetaReference
{
public:
    tools::SvRef<SvMetaType> aType;
    SvIdentifier             aSlotId;
                        SvMetaAttribute();
    SvMetaAttribute( SvMetaType * );

    void                SetSlotId( const SvIdentifier & rId )
                        { aSlotId = rId; }
    const SvIdentifier& GetSlotId() const;
    SvMetaType *        GetType() const;

    virtual bool        Test( SvTokenStream & rInStm );
    virtual bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) override;
    size_t           MakeSfx( OStringBuffer& rAtrrArray ) const;
    virtual void        Insert( SvSlotElementList& );
};

enum MetaTypeType { Method, Struct, Base, Enum, Interface, Shell };

class SvMetaType : public SvMetaReference
{
    SvRefMemberList<SvMetaAttribute *>  aAttrList;
    MetaTypeType                        nType;
    bool                                bIsItem;

    void                WriteSfxItem( std::string_view rItemName, SvIdlDataBase const & rBase,
                                      SvStream & rOutStm );
protected:
    bool                ReadHeaderSvIdl( SvTokenStream & rInStm );
public:
            SvMetaType();
            SvMetaType( const OString& rTypeName );

    virtual ~SvMetaType() override;

    virtual void        ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) override;

    SvRefMemberList<SvMetaAttribute *>&
                        GetAttrList() { return aAttrList; }
    size_t           GetAttrCount() const { return aAttrList.size(); }

    void                SetType( MetaTypeType nT );
    MetaTypeType        GetMetaTypeType() const { return nType; }
    SvMetaType *        GetBaseType() const;
    SvMetaType *        GetReturnType() const;
    void                SetItem(bool b) { bIsItem = b; }
    bool                IsItem() const { return bIsItem; }

    virtual bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) override;

    size_t           MakeSfx( OStringBuffer& rAtrrArray );
    virtual void        WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm );
    bool                ReadMethodArgs( SvIdlDataBase & rBase,
                                        SvTokenStream & rInStm );
};

class SvMetaTypeString final : public SvMetaType
{
public:
            SvMetaTypeString();
};

class SvMetaEnumValue final : public SvMetaObject
{
public:
    SvMetaEnumValue();
};

class SvMetaTypeEnum final : public SvMetaType
{
public:
    SvRefMemberList<SvMetaEnumValue *> aEnumValueList;
    OString                            aPrefix;
            SvMetaTypeEnum();
};

class SvMetaTypevoid final : public SvMetaType
{
public:
            SvMetaTypevoid();
};


#endif // INCLUDED_IDL_INC_TYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
