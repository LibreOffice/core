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

#ifndef _SLOT_HXX
#define _SLOT_HXX

#include <types.hxx>
#include <command.hxx>

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

    void            WriteSlot( const OString& rShellName,
                            sal_uInt16 nCount, const OString& rSlotId,
                            SvSlotElementList &rList,
                            size_t nStart,
                            const OString& rPrefix,
                               SvIdlDataBase & rBase, SvStream & rOutStm );
    virtual void    Write( SvIdlDataBase & rBase,
                            SvStream & rOutStm, sal_uInt16 nTab,
                             WriteType, WriteAttribute = 0 ) SAL_OVERRIDE;

    void            SetEnumValue(SvMetaEnumValue *p)
                    { pEnumValue = p; }
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

    virtual sal_Bool    IsVariable() const SAL_OVERRIDE;
    virtual sal_Bool    IsMethod() const SAL_OVERRIDE;
    virtual OString GetMangleName( sal_Bool bVariable ) const SAL_OVERRIDE;

    SvMetaAttribute *   GetMethod() const;
    SvMetaType *        GetSlotType() const;
    sal_Bool                GetHasCoreId() const;
    const OString&     GetGroupId() const;
    const OString&     GetConfigId() const;
    const OString&     GetExecMethod() const;
    const OString&     GetStateMethod() const;
    const OString&     GetDefault() const;
    const OString&     GetDisableFlags() const;
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
    const OString&     GetPseudoPrefix() const;
    const OString&     GetUnoName() const;
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

    SvMetaEnumValue*    GetEnumValue() const
                        { return pEnumValue; }
    virtual sal_Bool        Test( SvIdlDataBase &, SvTokenStream & rInStm ) SAL_OVERRIDE;
    virtual void        ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                             SvTokenStream & rInStm ) SAL_OVERRIDE;
    virtual void        WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                              SvStream & rOutStm, sal_uInt16 nTab ) SAL_OVERRIDE;
    virtual sal_Bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) SAL_OVERRIDE;
    virtual void        WriteSvIdl( SvIdlDataBase & rBase,
                                    SvStream & rOutStm, sal_uInt16 nTab ) SAL_OVERRIDE;
    virtual void        Insert( SvSlotElementList&, const OString& rPrefix,
                                SvIdlDataBase& ) SAL_OVERRIDE;
    void                WriteSlotStubs( const OString& rShellName,
                                    ByteStringList & rList,
                                    SvStream & rOutStm );
    sal_uInt16          WriteSlotMap( const OString& rShellName,
                                    sal_uInt16 nCount,
                                    SvSlotElementList&,
                                    size_t nStart,
                                    const OString&,
                                    SvIdlDataBase & rBase,
                                    SvStream & rOutStm );
    sal_uInt16              WriteSlotParamArray( SvIdlDataBase & rBase,
                                            SvStream & rOutStm );
    virtual void        WriteHelpId( SvIdlDataBase & rBase, SvStream & rOutStm,
                                  HelpIdTable& rIdTable ) SAL_OVERRIDE;
    virtual void        WriteCSV( SvIdlDataBase&, SvStream& ) SAL_OVERRIDE;
};
typedef tools::SvRef<SvMetaSlot> SvMetaSlotRef;

class SvMetaSlotMemberList : public SvDeclPersistList<SvMetaSlot *> {};

#endif // _SLOT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
