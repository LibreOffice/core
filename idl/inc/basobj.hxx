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

#ifndef _BASOBJ_HXX
#define _BASOBJ_HXX

#include <tools/ref.hxx>
#include <bastype.hxx>
#include <tools/pstm.hxx>

class SvTokenStream;
class SvMetaObject;
class SvIdlDataBase;

typedef SvMetaObject * (*CreateMetaObjectType)();
#define IDL_WRITE_COMPILER  0x8000
#define IDL_WRITE_BROWSER   0x4000
#define IDL_WRITE_CALLING   0x2000
#define IDL_WRITE_MASK      0xE000

#define C_PREF  "C_"

enum WriteType
{
    WRITE_IDL, WRITE_ODL, WRITE_SLOTMAP, WRITE_C_HEADER, WRITE_C_SOURCE,
    WRITE_CXX_HEADER, WRITE_CXX_SOURCE, WRITE_DOCU
};

enum
{
    WA_METHOD = 0x1,    WA_VARIABLE = 0x2,  WA_ARGUMENT = 0x4,
    WA_STRUCT = 0x8,    WA_READONLY = 0x10
};
typedef int WriteAttribute;

#define SV_DECL_META_FACTORY1( Class, Super1, CLASS_ID )                \
    SV_DECL_PERSIST1( Class, Super1, CLASS_ID )

#define SV_IMPL_META_FACTORY1( Class, Super1 )                          \
    SV_IMPL_PERSIST1( Class, Super1 )

class SvMetaObject : public SvPersistBase
{
public:
            SV_DECL_META_FACTORY1( SvMetaObject, SvPersistBase, 14 )
            SvMetaObject();

    static void         WriteTab( SvStream & rOutStm, sal_uInt16 nTab );
    static sal_Bool         TestAndSeekSpaceOnly( SvStream &, sal_uLong nBegPos );
    static void         Back2Delemitter( SvStream & );
    static void         WriteStars( SvStream & );

    virtual sal_Bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab );

    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab,
                                WriteType, WriteAttribute = 0 );
};
SV_DECL_IMPL_REF(SvMetaObject)

class SvMetaObjectMemberList : public SvDeclPersistList<SvMetaObject *> {};

class SvMetaObjectMemberStack
{
    SvMetaObjectMemberList aList;
public:
            SvMetaObjectMemberStack() {;}

    void            Push( SvMetaObject * pObj )
                    { aList.push_back( pObj ); }
    SvMetaObject *  Pop() { return aList.pop_back(); }
    SvMetaObject *  Top() const { return aList.back(); }
    void            Clear() { aList.clear(); }
    sal_uLong     Count() const { return aList.size(); }

    SvMetaObject *  Get( TypeId nType )
                    {
                        for( SvMetaObjectMemberList::reverse_iterator it = aList.rbegin(); it != aList.rend(); ++it )
                            if( (*it)->IsA( nType ) )
                                return *it;
                        return NULL;
                    }
};

class SvMetaName : public SvMetaObject
{
    SvString      aName;
    SvHelpContext aHelpContext;
    SvHelpText    aHelpText;
    SvString      aConfigName;
    SvString      aDescription;

protected:
    virtual sal_Bool ReadNameSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
            void DoReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm,
                                     char c = '\0' );
    virtual void ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void WriteContextSvIdl( SvIdlDataBase & rBase,
                                    SvStream & rOutStm, sal_uInt16 nTab );
    virtual void ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm );
    virtual void WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                       SvStream & rOutStm, sal_uInt16 nTab );
    virtual void WriteAttributes( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab,
                                        WriteType, WriteAttribute = 0);
    virtual void WriteContext( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab,
                                        WriteType, WriteAttribute = 0);
public:
            SV_DECL_META_FACTORY1( SvMetaName, SvMetaObject, 15 )
            SvMetaName();

    virtual sal_Bool            SetName( const rtl::OString& rName, SvIdlDataBase * = NULL  );
    void                        SetDescription( const rtl::OString& rText )
                                { aDescription.setString(rText); }
    const SvHelpContext&        GetHelpContext() const { return aHelpContext; }
    virtual const SvString &    GetName() const { return aName; }
    virtual const SvString &    GetHelpText() const { return aHelpText; }
    virtual const SvString &    GetConfigName() const{ return aConfigName; }
    virtual const SvString&     GetDescription() const{ return aDescription; }

    virtual sal_Bool        Test( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual sal_Bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab );
    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab,
                                       WriteType, WriteAttribute = 0);
    void                WriteDescription( SvStream& rOutStm );
};
SV_DECL_IMPL_REF(SvMetaName)

class SvMetaNameMemberList : public SvDeclPersistList<SvMetaName *> {};


SV_DECL_REF(SvMetaReference)
class SvMetaReference : public SvMetaName
{
protected:
    SvMetaReferenceRef  aRef;
public:
            SV_DECL_META_FACTORY1( SvMetaReference, SvMetaName, 17 )
            SvMetaReference();

    const SvString &    GetName() const
                        {
                            return ( !aRef.Is()
                                    || !SvMetaName::GetName().getString().isEmpty() )
                                ? SvMetaName::GetName()
                                : aRef->GetName();
                        }

    const SvString &    GetHelpText() const
                        {
                            return ( !aRef.Is()
                                    || !SvMetaName::GetHelpText().getString().isEmpty() )
                                ? SvMetaName::GetHelpText()
                                : aRef->GetHelpText();
                        }

    const SvString &    GetConfigName() const
                        {
                            return ( !aRef.Is()
                                    || !SvMetaName::GetConfigName().getString().isEmpty() )
                                ? SvMetaName::GetConfigName()
                                : aRef->GetConfigName();
                        }

    const SvString &    GetDescription() const
                        {
                            return ( !aRef.Is()
                                    || !SvMetaName::GetDescription().getString().isEmpty() )
                                ? SvMetaName::GetDescription()
                                : aRef->GetDescription();
                        }
    SvMetaReference *   GetRef() const { return aRef; }
    void                SetRef( SvMetaReference * pRef  )
                        { aRef = pRef; }
};
SV_IMPL_REF(SvMetaReference)

class SvMetaReferenceMemberList : public SvDeclPersistList<SvMetaReference *> {};


class SvMetaModule;
class SvMetaExtern : public SvMetaReference
{
    SvMetaModule *          pModule;    // included in which module

    SvUUId                  aUUId;
    SvVersion               aVersion;
    sal_Bool                    bReadUUId;
    sal_Bool                    bReadVersion;
public:
                        SV_DECL_META_FACTORY1( SvMetaExtern, SvMetaName, 16 )
                        SvMetaExtern();

    SvMetaModule *      GetModule() const;

    const SvGlobalName &GetUUId() const;
    const SvVersion &   GetVersion() const { return aVersion; }
    void                SetModule( SvIdlDataBase & rBase );
    virtual sal_Bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab );

    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab,
                                  WriteType, WriteAttribute = 0);
protected:
    virtual void        ReadAttributesSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                              SvStream & rOutStm, sal_uInt16 nTab );
    virtual void        WriteAttributes( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab,
                                          WriteType, WriteAttribute = 0);
};
SV_DECL_IMPL_REF(SvMetaExtern)

class SvMetaExternMemberList : public SvDeclPersistList<SvMetaExtern *> {};

#endif // _BASOBJ_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
