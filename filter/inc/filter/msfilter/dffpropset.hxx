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
#ifndef _DFFPROPSET_HXX
#define _DFFPROPSET_HXX

#include <tools/solar.h>
#include <filter/msfilter/msfilterdllapi.h>
#include <filter/msfilter/dffrecordheader.hxx>
#include <tools/stream.hxx>
#include <vector>

struct DffPropFlags
{
    sal_uInt8   bSet        : 1;
    sal_uInt8   bComplex    : 1;
    sal_uInt8   bBlip       : 1;
    sal_uInt8   bSoftAttr   : 1;
};

struct DffPropSetEntry
{
    DffPropFlags aFlags;
    sal_uInt16   nComplexIndexOrFlagsHAttr;
    sal_uInt32   nContent;
};

class MSFILTER_DLLPUBLIC DffPropSet
{
    private:
        DffPropSetEntry*          mpPropSetEntries;
        std::vector< sal_uInt32 > maOffsets;

        void ReadPropSet( SvStream&, bool );

    public:
        explicit DffPropSet();
        ~DffPropSet();

        inline sal_Bool    IsProperty( sal_uInt32 nRecType ) const { return ( mpPropSetEntries[ nRecType & 0x3ff ].aFlags.bSet ); };
        sal_Bool        IsHardAttribute( sal_uInt32 nId ) const;
        sal_uInt32        GetPropertyValue( sal_uInt32 nId, sal_uInt32 nDefault = 0 ) const;
        /** Returns a boolean property by its real identifier. */
        bool        GetPropertyBool( sal_uInt32 nId, bool bDefault = false ) const;
        /** Returns a string property. */
        OUString GetPropertyString( sal_uInt32 nId, SvStream& rStrm ) const;
        sal_Bool        SeekToContent( sal_uInt32 nRecType, SvStream& rSt ) const;
        void        InitializePropSet( sal_uInt16 nPropSetType ) const;

        friend SvStream& operator>>( SvStream& rIn, DffPropSet& rPropSet );
        friend SvStream& operator|=( SvStream& rIn, DffPropSet& rPropSet );
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
