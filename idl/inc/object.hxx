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

#ifndef _OBJECT_HXX
#define _OBJECT_HXX

#include <types.hxx>
#include <slot.hxx>
#include <vector>

struct SvSlotElement
{
    SvMetaSlotRef   xSlot;
    OString aPrefix;
    SvSlotElement( SvMetaSlot * pS, const OString& rPrefix )
    : xSlot( pS )
    , aPrefix( rPrefix )
    {
    }
};
typedef std::vector< SvSlotElement* > SvSlotElementList;

class SvMetaClass;
typedef ::std::vector< SvMetaClass* > SvMetaClassList;

typedef tools::SvRef<SvMetaClass> SvMetaClassRef;
class SvClassElement : public SvPersistBase
{
    SvBOOL                      aAutomation;
    OString                aPrefix;
    SvMetaClassRef              xClass;
public:
            SV_DECL_PERSIST1( SvClassElement, SvPersistBase, 1 )
            SvClassElement();

    void            SetPrefix( const OString& rPrefix )
                    { aPrefix = rPrefix; }
    const OString&  GetPrefix() const
                    { return aPrefix; }

    void            SetAutomation( sal_Bool rAutomation )
                    { aAutomation = rAutomation; }
    sal_Bool            GetAutomation() const
                    { return aAutomation; }

    void            SetClass( SvMetaClass * pClass )
                    { xClass = pClass; }
    SvMetaClass *   GetClass() const
                    { return xClass; }
};

typedef tools::SvRef<SvClassElement> SvClassElementRef;

class SvClassElementMemberList : public SvDeclPersistList<SvClassElement *> {};

class SvMetaClassMemberList : public SvDeclPersistList<SvMetaClass *> {};

class SvMetaClass : public SvMetaType
{
    SvMetaAttributeMemberList   aAttrList;
    SvClassElementMemberList    aClassList;
    SvMetaClassRef              aSuperClass;
    SvBOOL                      aAutomation;
    SvMetaClassRef              xAutomationInterface;

    sal_Bool                TestAttribute( SvIdlDataBase & rBase, SvTokenStream & rInStm,
                                     SvMetaAttribute & rAttr ) const;
    void                WriteSlotStubs( const OString& rShellName,
                                        SvSlotElementList & rSlotList,
                                        ByteStringList & rList,
                                        SvStream & rOutStm );
    sal_uInt16              WriteSlotParamArray( SvIdlDataBase & rBase,
                                            SvSlotElementList & rSlotList,
                                            SvStream & rOutStm );
    sal_uInt16              WriteSlots( const OString& rShellName, sal_uInt16 nCount,
                                    SvSlotElementList & rSlotList,
                                    SvIdlDataBase & rBase,
                                    SvStream & rOutStm );

    void                InsertSlots( SvSlotElementList& rList, std::vector<sal_uLong>& rSuperList,
                                    SvMetaClassList & rClassList,
                                    const OString& rPrefix, SvIdlDataBase& rBase );

protected:
    virtual void    ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm ) SAL_OVERRIDE;
    virtual void    WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                    SvStream & rOutStm, sal_uInt16 nTab ) SAL_OVERRIDE;
    virtual void    ReadContextSvIdl( SvIdlDataBase &,
                                     SvTokenStream & rInStm ) SAL_OVERRIDE;
    virtual void    WriteContextSvIdl( SvIdlDataBase & rBase,
                                     SvStream & rOutStm, sal_uInt16 nTab ) SAL_OVERRIDE;
    void            WriteOdlMembers( ByteStringList & rSuperList,
                                    sal_Bool bVariable, sal_Bool bWriteTab,
                                    SvIdlDataBase & rBase,
                                    SvStream & rOutStm, sal_uInt16 nTab );
public:
            SV_DECL_META_FACTORY1( SvMetaClass, SvMetaType, 6 )
            SvMetaClass();

    sal_Bool                GetAutomation() const
                        { return aAutomation; }
    SvMetaClass *       GetSuperClass() const
                        { return aSuperClass; }

    void                FillClasses( SvMetaClassList & rList );

    const SvClassElementMemberList&
                        GetClassList() const
                        { return aClassList; }

    virtual sal_Bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) SAL_OVERRIDE;
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, sal_uInt16 nTab ) SAL_OVERRIDE;
    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                                 sal_uInt16 nTab,
                                  WriteType, WriteAttribute = 0 ) SAL_OVERRIDE;
    virtual void        WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm ) SAL_OVERRIDE;
    virtual void        WriteHelpIds( SvIdlDataBase & rBase, SvStream & rOutStm,
                                HelpIdTable& rTable );
};

#endif // _OBJECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
