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
#ifndef INCLUDED_SW_INC_CCOLL_HXX
#define INCLUDED_SW_INC_CCOLL_HXX

#include <rtl/ustring.hxx>
#include <svl/poolitem.hxx>
#include "swdllapi.h"

enum class Master_CollCondition;

#define COND_COMMAND_COUNT 28

struct CommandStruct
{
    Master_CollCondition nCnd;
    sal_uLong nSubCond;
};

sal_Int16       GetCommandContextIndex( const OUString &rContextName );
OUString   GetCommandContextByIndex( sal_Int16 nIndex );

class SW_DLLPUBLIC SwCondCollItem : public SfxPoolItem
{
    static const CommandStruct  aCmds[COND_COMMAND_COUNT];

    OUString                    m_sStyles[COND_COMMAND_COUNT];

public:
    SwCondCollItem();
    virtual ~SwCondCollItem() override;

    SwCondCollItem(SwCondCollItem const &) = default;
    SwCondCollItem(SwCondCollItem &&) = default;
    SwCondCollItem & operator =(SwCondCollItem const &) = default;
    SwCondCollItem & operator =(SwCondCollItem &&) = default;

    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool                operator==( const SfxPoolItem& ) const override;

    static const CommandStruct* GetCmds() { return aCmds; }

    OUString            GetStyle(sal_uInt16 nPos) const;
    void                SetStyle(const OUString* pStyle, sal_uInt16 nPos);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
