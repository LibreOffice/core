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
#ifndef _SFX_SORGITM_HXX
#define _SFX_SORGITM_HXX

#include <svl/stritem.hxx>

// class SfxScriptOrganizerItem ---------------------------------------------

class SfxScriptOrganizerItem : public SfxStringItem
{
private:
    OUString aLanguage;

public:
    TYPEINFO();
    SfxScriptOrganizerItem();
    SfxScriptOrganizerItem( const SfxScriptOrganizerItem& );
    virtual ~SfxScriptOrganizerItem();

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual bool          QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool          PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
    OUString  getLanguage() { return aLanguage; };
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
