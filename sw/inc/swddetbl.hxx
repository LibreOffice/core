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
#ifndef _SWDDETBL_HXX
#define _SWDDETBL_HXX

#include "swtable.hxx"

class SwDDEFieldType;

class SwDDETable : public SwTable
{
    SwDepend aDepend;
public:
    TYPEINFO();
    // Constructor movet alle Lines/Boxen aus der SwTable zu sich.
    // Die SwTable ist danach Leer und muss geloescht werden.
    SwDDETable( SwTable& rTable, SwDDEFieldType* pDDEType,
                sal_Bool bUpdate = sal_True );
    ~SwDDETable();

    void ChangeContent();
    sal_Bool  NoDDETable();

    SwDDEFieldType* GetDDEFldType();
    inline const SwDDEFieldType* GetDDEFldType() const;
protected:
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );
    virtual void SwClientNotify( const SwModify&, const SfxHint& );
};


// ----------- Inlines -----------------------------

inline const SwDDEFieldType* SwDDETable::GetDDEFldType() const
{
    return ((SwDDETable*)this)->GetDDEFldType();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
