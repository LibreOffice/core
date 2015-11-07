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
#ifndef INCLUDED_SW_INC_SWDDETBL_HXX
#define INCLUDED_SW_INC_SWDDETBL_HXX

#include "swtable.hxx"

class SwDDEFieldType;

class SwDDETable : public SwTable
{
    SwDepend aDepend;
public:

    // Ctor moves all lines/boxes from SwTable to it.
    // After that SwTable is empty and has to be deleted.
    SwDDETable( SwTable& rTable, SwDDEFieldType* pDDEType,
                bool bUpdate = true );
    virtual ~SwDDETable();

    void ChangeContent();
    bool  NoDDETable();

    SwDDEFieldType* GetDDEFieldType();
    inline const SwDDEFieldType* GetDDEFieldType() const;
protected:
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) override;
    virtual void SwClientNotify( const SwModify&, const SfxHint& ) override;
};

inline const SwDDEFieldType* SwDDETable::GetDDEFieldType() const
{
    return const_cast<SwDDETable*>(this)->GetDDEFieldType();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
