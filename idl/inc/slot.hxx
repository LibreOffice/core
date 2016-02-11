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
    tools::SvRef<SvMetaType>      aSlotType;
    tools::SvRef<SvMetaSlot>      aMethod;
    SvIdentifier     aGroupId;
    SvIdentifier     aExecMethod;
    SvIdentifier     aStateMethod;
    SvBOOL           aPseudoSlots;

    SvBOOL           aToggle;
    SvBOOL           aAutoUpdate;

    SvBOOL           aAsynchron;

    SvBOOL           aRecordPerItem;// exclusive
    SvBOOL           aRecordPerSet;
    SvBOOL           aNoRecord;
    SvBOOL           aRecordAbsolute;

    SvBOOL           aMenuConfig;
    SvBOOL           aToolBoxConfig;
    SvBOOL           aAccelConfig;
    SvBOOL           aFastCall;
    SvBOOL           aContainer;
    SvBOOL           aImageRotation;
    SvBOOL           aImageReflection;
    SvIdentifier     aPseudoPrefix;
    OString          aDisableFlags;
    SvMetaSlot*      pLinkedSlot;
    SvMetaSlot*      pNextSlot;
    sal_uLong        nListPos;
    SvMetaEnumValue* pEnumValue;
    SvBOOL           aReadOnlyDoc;
    SvBOOL           aExport;

    void            WriteSlot( const OString& rShellName,
                            sal_uInt16 nCount, const OString& rSlotId,
                            SvSlotElementList &rList,
                            size_t nStart,
                            SvIdlDataBase & rBase, SvStream & rOutStm );

    void            SetEnumValue(SvMetaEnumValue *p) { pEnumValue = p; }
    OString         GetMangleName() const;
    bool            IsVariable() const;
    bool            IsMethod() const;

protected:
    void    SetToggle( bool bSet ) { aToggle = bSet; }
    void    SetAutoUpdate( bool bSet ) { aAutoUpdate = bSet; }
    void    SetAsynchron( bool bSet ) { aAsynchron = bSet; }
    void    SetRecordPerItem( bool bSet )
            {
                aRecordPerItem = bSet;
                if( bSet )
                    aRecordPerSet = aNoRecord = false;
            }
    void    SetRecordPerSet( bool bSet )
            {
                aRecordPerSet = bSet;
                if( bSet )
                    aRecordPerItem = aNoRecord = false;
            }
    void    SetNoRecord( bool bSet )
            {
                aNoRecord = bSet;
                if( bSet )
                    aRecordPerItem = aRecordPerSet = false;
            }
    void    SetRecordAbsolute( bool bSet ) { aRecordAbsolute = bSet; }

public:
            SvMetaObject *  MakeClone() const;
            SvMetaSlot *Clone() const { return static_cast<SvMetaSlot *>(MakeClone()); }

            SvMetaSlot();
            SvMetaSlot( SvMetaType * pType );

    SvMetaAttribute *   GetMethod() const;
    SvMetaType *        GetSlotType() const;
    const OString&      GetGroupId() const;
    const OString&      GetExecMethod() const;
    const OString&      GetStateMethod() const;
    const OString&      GetDisableFlags() const;
    bool                GetPseudoSlots() const;
    bool                GetToggle() const;
    bool                GetAutoUpdate() const;

    bool                GetAsynchron() const;

    bool                GetRecordPerItem() const;
    bool                GetRecordPerSet() const;
    bool                GetNoRecord() const;
    bool                GetRecordAbsolute() const;

    const OString&      GetPseudoPrefix() const;
    bool                GetMenuConfig() const;
    bool                GetToolBoxConfig() const;
    bool                GetAccelConfig() const;
    bool                GetFastCall() const;
    bool                GetContainer() const;
    bool                GetImageRotation() const;
    bool                GetImageReflection() const;
    bool                GetReadOnlyDoc() const;
    bool                GetExport() const;
    bool                GetHidden() const;

    sal_uLong           GetListPos() const
                        { return nListPos; }
    void                SetListPos(sal_uLong n)
                        { nListPos = n; }
    void                ResetSlotPointer()
                        { pNextSlot = pLinkedSlot = nullptr; }

    virtual bool        Test( SvTokenStream & rInStm ) override;
    virtual void        ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                             SvTokenStream & rInStm ) override;
    virtual bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) override;
    virtual void        Insert( SvSlotElementList&, const OString& rPrefix,
                                SvIdlDataBase& ) override;
    void                WriteSlotStubs( const OString& rShellName,
                                    ByteStringList & rList,
                                    SvStream & rOutStm );
    sal_uInt16          WriteSlotMap( const OString& rShellName,
                                    sal_uInt16 nCount,
                                    SvSlotElementList&,
                                    size_t nStart,
                                    SvIdlDataBase & rBase,
                                    SvStream & rOutStm );
    sal_uInt16          WriteSlotParamArray( SvIdlDataBase & rBase,
                                            SvStream & rOutStm );
};

#endif // INCLUDED_IDL_INC_SLOT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
