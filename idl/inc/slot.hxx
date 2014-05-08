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

#ifndef INCLUDED_IDL_INC_SLOT_HXX
#define INCLUDED_IDL_INC_SLOT_HXX

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
    void    SetCachable( bool bSet )
            {
                aCachable = bSet;
                if( bSet )
                    aVolatile = false;
            }
    void    SetVolatile( bool bSet )
            {
                aVolatile = bSet;
                if( bSet )
                    aCachable = false;
            }
    void    SetToggle( bool bSet )
            {
                aToggle = bSet;
            }
    void    SetAutoUpdate( bool bSet )
            {
                aAutoUpdate = bSet;
            }

    void    SetSynchron( bool bSet )
            {
                aSynchron = bSet;
                if( bSet )
                    aAsynchron = false;
            }
    void    SetAsynchron( bool bSet )
            {
                aAsynchron = bSet;
                if( bSet )
                    aSynchron = false;
            }

    void    SetRecordPerItem( bool bSet )
            {
                aRecordPerItem = bSet;
                if( bSet )
                    aRecordPerSet = aRecordManual = aNoRecord = false;
            }
    void    SetRecordPerSet( bool bSet )
            {
                aRecordPerSet = bSet;
                if( bSet )
                    aRecordPerItem = aRecordManual = aNoRecord = false;
            }
    void    SetRecordManual( bool bSet )
            {
                aRecordManual = bSet;
                if( bSet )
                    aRecordPerItem = aRecordPerSet = aNoRecord = false;
            }
    void    SetNoRecord( bool bSet )
            {
                aNoRecord = bSet;
                if( bSet )
                    aRecordPerItem = aRecordPerSet = aRecordManual = false;
            }
    void    SetRecordAbsolute( bool bSet )
            { aRecordAbsolute = bSet; }
    void    SetHasDialog( bool bSet )
            { aHasDialog = bSet; }
    void    SetMenuConfig( bool bSet )
            { aMenuConfig = bSet; }
    void    SetToolBoxConfig( bool bSet )
            { aToolBoxConfig = bSet; }
    void    SetStatusBarConfig( bool bSet )
            { aStatusBarConfig = bSet; }
    void    SetAccelConfig( bool bSet )
            { aAccelConfig = bSet; }
    void    SetAllConfig( bool bSet )
            {
                aMenuConfig     = bSet;
                aToolBoxConfig  = bSet;
                aStatusBarConfig = bSet;
                aAccelConfig    = bSet;
            }
    void    SetFastCall( bool bSet )
            { aFastCall = bSet; }
    void    SetContainer( bool bSet )
            { aContainer = bSet; }
    void    SetImageRotation( bool bSet )
            { aImageRotation = bSet; }
    void    SetImageReflection( bool bSet )
            { aImageReflection = bSet; }

public:
            SV_DECL_META_FACTORY1( SvMetaSlot, SvMetaReference, 11 )
            SvMetaObject *  MakeClone() const;
            SvMetaSlot *Clone() const { return (SvMetaSlot *)MakeClone(); }

            SvMetaSlot();
            SvMetaSlot( SvMetaType * pType );

    virtual bool    IsVariable() const SAL_OVERRIDE;
    virtual bool    IsMethod() const SAL_OVERRIDE;
    virtual OString GetMangleName( bool bVariable ) const SAL_OVERRIDE;

    SvMetaAttribute *   GetMethod() const;
    SvMetaType *        GetSlotType() const;
    bool                GetHasCoreId() const;
    const OString&     GetGroupId() const;
    const OString&     GetConfigId() const;
    const OString&     GetExecMethod() const;
    const OString&     GetStateMethod() const;
    const OString&     GetDefault() const;
    const OString&     GetDisableFlags() const;
    bool                GetPseudoSlots() const;
    bool                GetCachable() const;
    bool                GetVolatile() const;
    bool                GetToggle() const;
    bool                GetAutoUpdate() const;

    bool                GetSynchron() const;
    bool                GetAsynchron() const;

    bool                GetRecordPerItem() const;
    bool                GetRecordPerSet() const;
    bool                GetRecordManual() const;
    bool                GetNoRecord() const;
    bool                GetRecordAbsolute() const;

    bool                GetHasDialog() const;
    const OString&     GetPseudoPrefix() const;
    const OString&     GetUnoName() const;
    bool                GetMenuConfig() const;
    bool                GetToolBoxConfig() const;
    bool                GetStatusBarConfig() const;
    bool                GetAccelConfig() const;
    bool                GetFastCall() const;
    bool                GetContainer() const;
    bool                GetImageRotation() const;
    bool                GetImageReflection() const;
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
    virtual bool        Test( SvIdlDataBase &, SvTokenStream & rInStm ) SAL_OVERRIDE;
    virtual void        ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                             SvTokenStream & rInStm ) SAL_OVERRIDE;
    virtual void        WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                              SvStream & rOutStm, sal_uInt16 nTab ) SAL_OVERRIDE;
    virtual bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) SAL_OVERRIDE;
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
    sal_uInt16          WriteSlotParamArray( SvIdlDataBase & rBase,
                                            SvStream & rOutStm );
    virtual void        WriteHelpId( SvIdlDataBase & rBase, SvStream & rOutStm,
                                  HelpIdTable& rIdTable ) SAL_OVERRIDE;
    virtual void        WriteCSV( SvIdlDataBase&, SvStream& ) SAL_OVERRIDE;
};
typedef tools::SvRef<SvMetaSlot> SvMetaSlotRef;

class SvMetaSlotMemberList : public SvDeclPersistList<SvMetaSlot *> {};

#endif // INCLUDED_IDL_INC_SLOT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
