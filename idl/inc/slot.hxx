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

#pragma once

#include "types.hxx"
#include <tools/solar.h>

class SvMetaSlot : public SvMetaAttribute
{
public:
    SvIdentifier     aGroupId;
    SvIdentifier     aExecMethod;
    SvIdentifier     aStateMethod;

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
    OString          aDisableFlags;
    SvMetaSlot*      pNextSlot;
    sal_uInt32       nListPos;
    SvBOOL           aReadOnlyDoc;

    void            WriteSlot( std::string_view rShellName,
                            sal_uInt16 nCount, std::string_view rSlotId,
                            SvSlotElementList &rList,
                            size_t nStart,
                            SvIdlDataBase & rBase, SvStream & rOutStm );

    bool            IsVariable() const;
    bool            IsMethod() const;

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

public:
            SvMetaSlot();
            SvMetaSlot( SvMetaType * pType );

    const OString&      GetGroupId() const;
    const OString&      GetExecMethod() const;
    const OString&      GetStateMethod() const;
    const OString&      GetDisableFlags() const;
    bool                GetToggle() const;
    bool                GetAutoUpdate() const;

    bool                GetAsynchron() const;

    bool                GetRecordPerItem() const;
    bool                GetRecordPerSet() const;
    bool                GetNoRecord() const;
    bool                GetRecordAbsolute() const;

    bool                GetMenuConfig() const;
    bool                GetToolBoxConfig() const;
    bool                GetAccelConfig() const;
    bool                GetFastCall() const;
    bool                GetContainer() const;
    bool                GetReadOnlyDoc() const;

    sal_uInt32           GetListPos() const
                        { return nListPos; }
    void                SetListPos(sal_uInt32 n)
                        { nListPos = n; }
    void                ResetSlotPointer()
                        { pNextSlot = nullptr; }

    virtual bool        Test( SvTokenStream & rInStm ) override;
    virtual void        ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                             SvTokenStream & rInStm ) override;
    virtual bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) override;
    virtual void        Insert( SvSlotElementList& ) override;
    void                WriteSlotStubs( std::string_view rShellName,
                                    std::vector<OString> & rList,
                                    SvStream & rOutStm ) const;
    sal_uInt16          WriteSlotMap( std::string_view rShellName,
                                    sal_uInt16 nCount,
                                    SvSlotElementList&,
                                    size_t nStart,
                                    SvIdlDataBase & rBase,
                                    SvStream & rOutStm );
    sal_uInt16          WriteSlotParamArray( SvIdlDataBase & rBase,
                                            SvStream & rOutStm ) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
