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

#ifndef SC_SCDOCPOL_HXX
#define SC_SCDOCPOL_HXX

#include <svl/itempool.hxx>
#include "scdllapi.h"

class ScStyleSheet;

class SC_DLLPUBLIC ScDocumentPool: public SfxItemPool
{
    SfxPoolItem**   ppPoolDefaults;
    SfxItemPool*    pSecondary;
    static USHORT*  pVersionMap1;
    static USHORT*  pVersionMap2;
    static USHORT*  pVersionMap3;
    static USHORT*  pVersionMap4;
    static USHORT*  pVersionMap5;
    static USHORT*  pVersionMap6;
    static USHORT*  pVersionMap7;
    static USHORT*  pVersionMap8;
    static USHORT*  pVersionMap9;
    static USHORT*  pVersionMap10;
    static USHORT*  pVersionMap11;

public:
            ScDocumentPool( SfxItemPool* pSecPool = NULL, BOOL bLoadRefCounts = FALSE );
protected:
            virtual ~ScDocumentPool();
public:

    virtual SfxItemPool*        Clone() const;
    virtual SfxMapUnit          GetMetric( USHORT nWhich ) const;

    virtual const SfxPoolItem&  Put( const SfxPoolItem&, USHORT nWhich = 0 );
    virtual void                Remove( const SfxPoolItem& );
    static void                 CheckRef( const SfxPoolItem& );

    void StyleDeleted( ScStyleSheet* pStyle );      // delete templates(?) in organizer
    void CellStyleCreated( const String& rName );
    virtual SfxItemPresentation     GetPresentation(
                                        const SfxPoolItem&  rItem,
                                        SfxItemPresentation ePresentation,
                                        SfxMapUnit          ePresentationMetric,
                                        String&             rText,
                                        const IntlWrapper* pIntl = 0 ) const;

    static void InitVersionMaps();
    static void DeleteVersionMaps();
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
