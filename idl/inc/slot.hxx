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

#ifndef _SLOT_HXX
#define _SLOT_HXX

#include <types.hxx>
#include <command.hxx>

class SvMetaSlotMemberList;
class SvMetaSlot : public SvMetaAttribute
{
    SvMetaTypeRef   aSlotType;
    SvMetaAttributeRef aMethod;
    SvIdentifier    aGroupId;
    SvBOOL          aHasCoreId;
    SvIdentifier    aConfigId;
    SvIdentifier    aExecMethod;
    SvIdentifier    aStateMethod;
    SvIdentifier    aDefault;
    SvBOOL          aPseudoSlots;
    SvBOOL          aGet;
    SvBOOL          aSet;

    SvBOOL          aCachable;  // exclusive
    SvBOOL          aVolatile;
    SvBOOL          aToggle;
    SvBOOL          aAutoUpdate;

    SvBOOL          aSynchron;  // exclusive
    SvBOOL          aAsynchron;

    SvBOOL          aRecordPerItem;// exclusive
    SvBOOL          aRecordPerSet;
    SvBOOL          aRecordManual;
    SvBOOL          aNoRecord;
    SvBOOL          aRecordAbsolute;

    SvBOOL          aHasDialog;
    SvBOOL          aMenuConfig;
    SvBOOL          aToolBoxConfig;
    SvBOOL          aStatusBarConfig;
    SvBOOL          aAccelConfig;
    SvBOOL          aFastCall;
    SvBOOL          aContainer;
    SvBOOL          aImageRotation;
    SvBOOL          aImageReflection;
    SvIdentifier    aPseudoPrefix;
    SvString        aDisableFlags;
    SvMetaSlot*     pLinkedSlot;
    SvMetaSlot*     pNextSlot;
    sal_uLong           nListPos;
    SvMetaEnumValue* pEnumValue;
    SvString    aUnoName;

#ifdef IDL_COMPILER
    void            WriteSlot( const ByteString & rShellName,
                            sal_uInt16 nCount, const ByteString & rSlotId,
                            SvSlotElementList &rList,
                            const ByteString & rPrefix,
                               SvIdlDataBase & rBase, SvStream & rOutStm );
    virtual void    Write( SvIdlDataBase & rBase,
                            SvStream & rOutStm, sal_uInt16 nTab,
                             WriteType, WriteAttribute = 0 );

    void            SetEnumValue(SvMetaEnumValue *p)
                    { pEnumValue = p; }
#endif
protected:
    void    SetCachable( sal_Bool bSet )
            {
                aCachable = bSet;
                if( bSet )
                    aVolatile = sal_False;
            }
    void    SetVolatile( sal_Bool bSet )
            {
                aVolatile = bSet;
                if( bSet )
                    aCachable = sal_False;
            }
    void    SetToggle( sal_Bool bSet )
            {
                aToggle = bSet;
            }
    void    SetAutoUpdate( sal_Bool bSet )
            {
                aAutoUpdate = bSet;
            }

    void    SetSynchron( sal_Bool bSet )
            {
                aSynchron = bSet;
                if( bSet )
                    aAsynchron = sal_False;
            }
    void    SetAsynchron( sal_Bool bSet )
            {
                aAsynchron = bSet;
                if( bSet )
                    aSynchron = sal_False;
            }

    void    SetRecordPerItem( sal_Bool bSet )
            {
                aRecordPerItem = bSet;
                if( bSet )
                    aRecordPerSet = aRecordManual = aNoRecord = sal_False;
            }
    void    SetRecordPerSet( sal_Bool bSet )
            {
                aRecordPerSet = bSet;
                if( bSet )
                    aRecordPerItem = aRecordManual = aNoRecord = sal_False;
            }
    void    SetRecordManual( sal_Bool bSet )
            {
                aRecordManual = bSet;
                if( bSet )
                    aRecordPerItem = aRecordPerSet = aNoRecord = sal_False;
            }
    void    SetNoRecord( sal_Bool bSet )
            {
                aNoRecord = bSet;
                if( bSet )
                    aRecordPerItem = aRecordPerSet = aRecordManual = sal_False;
            }
    void    SetRecordAbsolute( sal_Bool bSet )
            { aRecordAbsolute = bSet; }
    void    SetHasDialog( sal_Bool bSet )
            { aHasDialog = bSet; }
    void    SetMenuConfig( sal_Bool bSet )
            { aMenuConfig = bSet; }
    void    SetToolBoxConfig( sal_Bool bSet )
            { aToolBoxConfig = bSet; }
    void    SetStatusBarConfig( sal_Bool bSet )
            { aStatusBarConfig = bSet; }
    void    SetAccelConfig( sal_Bool bSet )
            { aAccelConfig = bSet; }
    void    SetAllConfig( sal_Bool bSet )
            {
                aMenuConfig     = bSet;
                aToolBoxConfig  = bSet;
                aStatusBarConfig = bSet;
                aAccelConfig    = bSet;
            }
    void    SetFastCall( sal_Bool bSet )
            { aFastCall = bSet; }
    void    SetContainer( sal_Bool bSet )
            { aContainer = bSet; }
    void    SetImageRotation( sal_Bool bSet )
            { aImageRotation = bSet; }
    void    SetImageReflection( sal_Bool bSet )
            { aImageReflection = bSet; }

public:
            SV_DECL_META_FACTORY1( SvMetaSlot, SvMetaReference, 11 )
            SvMetaObject *  MakeClone() const;
            SvMetaSlot *Clone() const { return (SvMetaSlot *)MakeClone(); }

            SvMetaSlot();
            SvMetaSlot( SvMetaType * pType );

    virtual sal_Bool    IsVariable() const;
    virtual sal_Bool    IsMethod() const;
    virtual ByteString  GetMangleName( sal_Bool bVariable ) const;

    SvMetaAttribute *   GetMethod() const;
    SvMetaType *        GetSlotType() const;
    sal_Bool                GetHasCoreId() const;
    const ByteString &      GetGroupId() const;
    const ByteString &      GetConfigId() const;
    const ByteString &      GetExecMethod() const;
    const ByteString &      GetStateMethod() const;
    const ByteString &      GetDefault() const;
    const ByteString &      GetDisableFlags() const;
    sal_Bool                GetPseudoSlots() const;
    sal_Bool                GetCachable() const;
    sal_Bool                GetVolatile() const;
    sal_Bool                GetToggle() const;
    sal_Bool                GetAutoUpdate() const;

    sal_Bool                GetSynchron() const;
    sal_Bool                GetAsynchron() const;

    sal_Bool                GetRecordPerItem() const;
    sal_Bool                GetRecordPerSet() const;
    sal_Bool                GetRecordManual() const;
    sal_Bool                GetNoRecord() const;
    sal_Bool                GetRecordAbsolute() const;

    sal_Bool                GetHasDialog() const;
    const ByteString &      GetPseudoPrefix() const;
    const ByteString &      GetUnoName() const;
    sal_Bool                GetMenuConfig() const;
    sal_Bool                GetToolBoxConfig() const;
    sal_Bool                GetStatusBarConfig() const;
    sal_Bool                GetAccelConfig() const;
    sal_Bool                GetFastCall() const;
    sal_Bool                GetContainer() const;
    sal_Bool                GetImageRotation() const;
    sal_Bool                GetImageReflection() const;
    SvMetaSlot*         GetLinkedSlot() const
                        { return pLinkedSlot; }
    SvMetaSlot*         GetNextSlot() const
                        { return pNextSlot; }
    sal_uLong               GetListPos() const
                        { return nListPos; }
    void                SetListPos(sal_uLong n)
                        { nListPos = n; }
    void                ResetSlotPointer()
                        { pNextSlot = pLinkedSlot = 0; }

#ifdef IDL_COMPILER
    SvMetaEnumValue*    GetEnumValue() const
                        { return pEnumValue; }
    virtual sal_Bool        Test( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                             SvTokenStream & rInStm );
    virtual void        WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                              SvStream & rOutStm, sal_uInt16 nTab );
    virtual sal_Bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase,
                                    SvStream & rOutStm, sal_uInt16 nTab );
    virtual void        Insert( SvSlotElementList&, const ByteString & rPrefix,
                                SvIdlDataBase& );
    void                WriteSlotStubs( const ByteString & rShellName,
                                    ByteStringList & rList,
                                    SvStream & rOutStm );
    sal_uInt16              WriteSlotMap( const ByteString & rShellName,
                                    sal_uInt16 nCount,
                                    SvSlotElementList&,
                                    const ByteString &,
                                    SvIdlDataBase & rBase,
                                    SvStream & rOutStm );
    sal_uInt16              WriteSlotParamArray( SvIdlDataBase & rBase,
                                            SvStream & rOutStm );
    virtual void        WriteHelpId( SvIdlDataBase & rBase, SvStream & rOutStm,
                                  Table * pIdTable );
    virtual void        WriteSrc( SvIdlDataBase & rBase, SvStream & rOutStm,
                                  Table * pIdTable );
    virtual void        WriteCSV( SvIdlDataBase&, SvStream& );
#endif
};
SV_DECL_IMPL_REF(SvMetaSlot)
SV_DECL_IMPL_PERSIST_LIST(SvMetaSlot,SvMetaSlot *)

#endif // _SLOT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
