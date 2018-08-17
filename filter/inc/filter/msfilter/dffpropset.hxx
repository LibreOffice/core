/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
        inline sal_Bool IsComplex( sal_uInt32 nRecType ) const { return ( mpPropSetEntries[ nRecType & 0x3ff ].aFlags.bComplex ); };
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


