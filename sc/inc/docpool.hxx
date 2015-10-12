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

#ifndef INCLUDED_SC_INC_DOCPOOL_HXX
#define INCLUDED_SC_INC_DOCPOOL_HXX

#include <rtl/ustring.hxx>
#include <svl/itempool.hxx>
#include "scdllapi.h"

class ScStyleSheet;
class ScDocument;

class SC_DLLPUBLIC ScDocumentPool: public SfxItemPool
{
    SfxPoolItem**   ppPoolDefaults;
    SfxItemPool*    pSecondary;
    static sal_uInt16*  pVersionMap1;
    static sal_uInt16*  pVersionMap2;
    static sal_uInt16*  pVersionMap3;
    static sal_uInt16*  pVersionMap4;
    static sal_uInt16*  pVersionMap5;
    static sal_uInt16*  pVersionMap6;
    static sal_uInt16*  pVersionMap7;
    static sal_uInt16*  pVersionMap8;
    static sal_uInt16*  pVersionMap9;
    static sal_uInt16*  pVersionMap10;
    static sal_uInt16*  pVersionMap11;
    static sal_uInt16*  pVersionMap12;

public:
            ScDocumentPool( SfxItemPool* pSecPool = NULL );
protected:
            virtual ~ScDocumentPool();
public:

    virtual SfxItemPool*        Clone() const override;
    virtual SfxMapUnit          GetMetric( sal_uInt16 nWhich ) const override;

    virtual const SfxPoolItem&  Put( const SfxPoolItem&, sal_uInt16 nWhich = 0 ) override;
    virtual void                Remove( const SfxPoolItem& ) override;
    static void                 CheckRef( const SfxPoolItem& );

    void StyleDeleted( ScStyleSheet* pStyle );      // delete templates(?) in organizer
    void CellStyleCreated( const OUString& rName, ScDocument* pDoc );
    virtual bool GetPresentation(
                                        const SfxPoolItem&  rItem,
                                        SfxMapUnit          ePresentationMetric,
                                        OUString&           rText,
                                        const IntlWrapper* pIntl = 0 ) const override;

    static void InitVersionMaps();
    static void DeleteVersionMaps();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
