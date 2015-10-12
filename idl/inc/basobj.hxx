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

#ifndef INCLUDED_IDL_INC_BASOBJ_HXX
#define INCLUDED_IDL_INC_BASOBJ_HXX

#include <tools/ref.hxx>
#include <bastype.hxx>
#include <tools/pstm.hxx>
#include <functional>
class SvTokenStream;
class SvMetaObject;
class SvIdlDataBase;

typedef SvMetaObject * (*CreateMetaObjectType)();

#define C_PREF  "C_"

class SvMetaObjectMemberList : public SvRefMemberList<SvMetaObject *> {};

class SvMetaObject : public SvRttiBase
{
protected:
    SvString      aName;
    SvHelpContext aHelpContext;
    SvHelpText    aHelpText;
    SvString      aConfigName;
    SvString      aDescription;

    bool ReadNameSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
            void DoReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm,
                                     char c = '\0' );
    virtual void ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm );
public:
            TYPEINFO_OVERRIDE();
            SvMetaObject();

    static void         WriteTab( SvStream & rOutStm, sal_uInt16 nTab );
    static void         Back2Delemitter( SvStream & );
    static void         WriteStars( SvStream & );

    virtual bool                SetName( const OString& rName, SvIdlDataBase * = NULL  );
    const SvHelpContext&        GetHelpContext() const { return aHelpContext; }
    virtual const SvString &    GetName() const { return aName; }
    virtual const SvString &    GetHelpText() const { return aHelpText; }
    virtual const SvString &    GetConfigName() const{ return aConfigName; }
    virtual const SvString&     GetDescription() const{ return aDescription; }

    virtual bool        Test( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
};
template<class T> bool checkSvMetaObject(const SvMetaObject* pObject)
{
    return dynamic_cast<const T*>(pObject) != nullptr;
}

class SvMetaObjectMemberStack
{
    SvMetaObjectMemberList aList;
public:
            SvMetaObjectMemberStack() {;}

    void            Push( SvMetaObject * pObj )
                    { aList.push_back( pObj ); }
    SvMetaObject *  Pop() { return aList.pop_back(); }
    SvMetaObject *  Get( std::function<bool ( const SvMetaObject* )> isSvMetaObject )
                    {
                        for( SvMetaObjectMemberList::reverse_iterator it = aList.rbegin(); it != aList.rend(); ++it )
                            if( isSvMetaObject(*it) )
                                return *it;
                        return NULL;
                    }
};

class SvMetaNameMemberList : public SvRefMemberList<SvMetaObject *> {};

class SvMetaReference : public SvMetaObject
{
protected:
    tools::SvRef<SvMetaReference>  aRef;
public:
            TYPEINFO_OVERRIDE();
            SvMetaReference();

    const SvString &    GetName() const override
                        {
                            return ( !aRef.Is()
                                    || !SvMetaObject::GetName().getString().isEmpty() )
                                ? SvMetaObject::GetName()
                                : aRef->GetName();
                        }

    const SvString &    GetHelpText() const override
                        {
                            return ( !aRef.Is()
                                    || !SvMetaObject::GetHelpText().getString().isEmpty() )
                                ? SvMetaObject::GetHelpText()
                                : aRef->GetHelpText();
                        }

    const SvString &    GetConfigName() const override
                        {
                            return ( !aRef.Is()
                                    || !SvMetaObject::GetConfigName().getString().isEmpty() )
                                ? SvMetaObject::GetConfigName()
                                : aRef->GetConfigName();
                        }

    const SvString &    GetDescription() const override
                        {
                            return ( !aRef.Is()
                                    || !SvMetaObject::GetDescription().getString().isEmpty() )
                                ? SvMetaObject::GetDescription()
                                : aRef->GetDescription();
                        }
    SvMetaReference *   GetRef() const { return aRef; }
    void                SetRef( SvMetaReference * pRef  )
                        { aRef = pRef; }
};

class SvMetaReferenceMemberList : public SvRefMemberList<SvMetaReference *> {};


class SvMetaModule;
class SvMetaExtern : public SvMetaReference
{
    SvMetaModule *          pModule;    // included in which module

    SvUUId                  aUUId;
    SvVersion               aVersion;
    bool                    bReadUUId;
    bool                    bReadVersion;
public:
                        TYPEINFO_OVERRIDE();
                        SvMetaExtern();

    SvMetaModule *      GetModule() const;

    const SvGlobalName &GetUUId() const;
    void                SetModule( SvIdlDataBase & rBase );
    virtual bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) override;

protected:
    virtual void        ReadAttributesSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) override;
};

class SvMetaExternMemberList : public SvRefMemberList<SvMetaExtern *> {};

#endif // INCLUDED_IDL_INC_BASOBJ_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
