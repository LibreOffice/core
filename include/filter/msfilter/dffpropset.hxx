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
#ifndef INCLUDED_FILTER_MSFILTER_DFFPROPSET_HXX
#define INCLUDED_FILTER_MSFILTER_DFFPROPSET_HXX

#include <filter/msfilter/msfilterdllapi.h>
#include <filter/msfilter/dffrecordheader.hxx>
#include <tools/stream.hxx>
#include <vector>

struct DffPropFlags
{
    bool   bSet        : 1;
    bool   bComplex    : 1;
    bool   bBlip       : 1;
    bool   bSoftAttr   : 1;
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

        inline bool IsProperty( sal_uInt32 nRecType ) const { return ( mpPropSetEntries[ nRecType & 0x3ff ].aFlags.bSet ); };
        bool        IsHardAttribute( sal_uInt32 nId ) const;
        sal_uInt32  GetPropertyValue( sal_uInt32 nId, sal_uInt32 nDefault = 0 ) const;
        /** Returns a boolean property by its real identifier. */
        bool        GetPropertyBool( sal_uInt32 nId, bool bDefault = false ) const;
        /** Returns a string property. */
        OUString    GetPropertyString( sal_uInt32 nId, SvStream& rStrm ) const;
        bool        SeekToContent( sal_uInt32 nRecType, SvStream& rSt ) const;
        void        InitializePropSet( sal_uInt16 nPropSetType ) const;
        static sal_uLong SanitizeEndPos(SvStream &rIn, sal_uLong nEndRecPos);

        friend SvStream& ReadDffPropSet( SvStream& rIn, DffPropSet& rPropSet );
        friend SvStream& operator|=( SvStream& rIn, DffPropSet& rPropSet );
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
