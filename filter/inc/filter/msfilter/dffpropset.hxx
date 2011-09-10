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

#ifndef _DFFPROPSET_HXX
#define _DFFPROPSET_HXX

#include <tools/solar.h>
#include <filter/msfilter/msfilterdllapi.h>
#include <filter/msfilter/dffrecordheader.hxx>
#include <tools/stream.hxx>
#include <vector>
#include <tools/table.hxx>

struct DffPropFlags
{
    sal_uInt16  bSet        : 1;
    sal_uInt16  bComplex    : 1;
    sal_uInt16  bBlip       : 1;
    sal_uInt16  bSoftAttr   : 1;
};

struct DffPropSetEntry
{
    DffPropFlags    aFlags;
    sal_uInt16      nComplexIndexOrFlagsHAttr;
    sal_uInt32      nContent;
};

class MSFILTER_DLLPUBLIC DffPropSet
{
    private :

        DffPropSetEntry*            mpPropSetEntries;
        std::vector< sal_uInt32 >   maOffsets;

        void ReadPropSet( SvStream&, bool );

    public :

        explicit DffPropSet();
        ~DffPropSet();

        inline sal_Bool IsProperty( sal_uInt32 nRecType ) const { return ( mpPropSetEntries[ nRecType & 0x3ff ].aFlags.bSet ); };
        sal_Bool        IsHardAttribute( sal_uInt32 nId ) const;
        sal_uInt32      GetPropertyValue( sal_uInt32 nId, sal_uInt32 nDefault = 0 ) const;
        /** Returns a boolean property by its real identifier. */
        bool        GetPropertyBool( sal_uInt32 nId, bool bDefault = false ) const;
        /** Returns a string property. */
        ::rtl::OUString GetPropertyString( sal_uInt32 nId, SvStream& rStrm ) const;
        void        SetPropertyValue( sal_uInt32 nId, sal_uInt32 nValue ) const;
        sal_Bool        SeekToContent( sal_uInt32 nRecType, SvStream& rSt ) const;
        void        InitializePropSet( sal_uInt16 nPropSetType ) const;

        friend SvStream& operator>>( SvStream& rIn, DffPropSet& rPropSet );
        friend SvStream& operator|=( SvStream& rIn, DffPropSet& rPropSet );
};

#endif


