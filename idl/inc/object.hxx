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

#ifndef _OBJECT_HXX
#define _OBJECT_HXX

#define _SVSTDARR_ULONGS
#include <types.hxx>
#include <slot.hxx>
#include <vector>
#include <tools/list.hxx>

struct SvSlotElement
{
    SvMetaSlotRef   xSlot;
    ByteString          aPrefix;
            SvSlotElement( SvMetaSlot * pS, const ByteString & rPrefix )
                : xSlot( pS )
                , aPrefix( rPrefix )
             {}
};
DECLARE_LIST( SvSlotElementList, SvSlotElement* )

class SvMetaClass;
typedef ::std::vector< SvMetaClass* > SvMetaClassList;

SV_DECL_REF(SvMetaClass)
class SvClassElement : public SvPersistBase
{
    SvBOOL                      aAutomation;
    ByteString                      aPrefix;
    SvMetaClassRef              xClass;
public:
            SV_DECL_PERSIST1( SvClassElement, SvPersistBase, 1 )
            SvClassElement();

    void            SetPrefix( const ByteString & rPrefix )
                    { aPrefix = rPrefix; }
    const ByteString &  GetPrefix() const
                    { return aPrefix; }

    void            SetAutomation( BOOL rAutomation )
                    { aAutomation = rAutomation; }
    BOOL            GetAutomation() const
                    { return aAutomation; }

    void            SetClass( SvMetaClass * pClass )
                    { xClass = pClass; }
    SvMetaClass *   GetClass() const
                    { return xClass; }
};

SV_DECL_IMPL_REF(SvClassElement)
SV_DECL_IMPL_PERSIST_LIST(SvClassElement,SvClassElement *)

class SvMetaModule;
SV_DECL_PERSIST_LIST(SvMetaClass,SvMetaClass *)
class SvMetaClass : public SvMetaType
{
    SvMetaAttributeMemberList   aAttrList;
    SvClassElementMemberList    aClassList;
    SvMetaClassRef              aSuperClass;
    SvBOOL                      aAutomation;
    SvMetaClassRef              xAutomationInterface;

    BOOL                TestAttribute( SvIdlDataBase & rBase, SvTokenStream & rInStm,
                                     SvMetaAttribute & rAttr ) const;
#ifdef IDL_COMPILER
    void                WriteSlotStubs( const ByteString & rShellName,
                                        SvSlotElementList & rSlotList,
                                        ByteStringList & rList,
                                        SvStream & rOutStm );
    USHORT              WriteSlotParamArray( SvIdlDataBase & rBase,
                                            SvSlotElementList & rSlotList,
                                            SvStream & rOutStm );
    USHORT              WriteSlots( const ByteString & rShellName, USHORT nCount,
                                    SvSlotElementList & rSlotList,
                                    SvIdlDataBase & rBase,
                                    SvStream & rOutStm );

    void                InsertSlots( SvSlotElementList& rList, std::vector<ULONG>& rSuperList,
                                    SvMetaClassList & rClassList,
                                    const ByteString & rPrefix, SvIdlDataBase& rBase );

protected:
    virtual void    ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm );
    virtual void    WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                    SvStream & rOutStm, USHORT nTab );
    virtual void    ReadContextSvIdl( SvIdlDataBase &,
                                     SvTokenStream & rInStm );
    virtual void    WriteContextSvIdl( SvIdlDataBase & rBase,
                                     SvStream & rOutStm, USHORT nTab );
    void            WriteOdlMembers( ByteStringList & rSuperList,
                                    BOOL bVariable, BOOL bWriteTab,
                                    SvIdlDataBase & rBase,
                                    SvStream & rOutStm, USHORT nTab );
#endif
public:
            SV_DECL_META_FACTORY1( SvMetaClass, SvMetaType, 6 )
            SvMetaClass();

    BOOL                GetAutomation() const
                        { return aAutomation; }
    SvMetaClass *       GetSuperClass() const
                        { return aSuperClass; }

    void                FillClasses( SvMetaClassList & rList );

    const SvClassElementMemberList&
                        GetClassList() const
                        { return aClassList; }

#ifdef IDL_COMPILER
    virtual BOOL        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );
    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                                 USHORT nTab,
                                  WriteType, WriteAttribute = 0 );
    virtual void        WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm );
    virtual void        WriteHelpIds( SvIdlDataBase & rBase, SvStream & rOutStm,
                                Table* pTable );
    virtual void        WriteSrc( SvIdlDataBase & rBase, SvStream & rOutStm,
                                 Table * pTable );
    virtual void        WriteCxx( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );
    virtual void        WriteHxx( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );
#endif
};
SV_IMPL_REF(SvMetaClass)
SV_IMPL_PERSIST_LIST(SvMetaClass,SvMetaClass *)


#endif // _OBJECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
