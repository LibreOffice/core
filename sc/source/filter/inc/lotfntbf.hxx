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



#ifndef SC_LOTFNTBF_HXX
#define SC_LOTFNTBF_HXX

#include <tools/solar.h>

#include "scitems.hxx"
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>

// ---------------------------------------------------- class LotusFontBuffer -

// Code in fontbuff.cxx (excel)

class LotusFontBuffer
{
private:
    struct ENTRY
    {
        String*             pTmpName;
        SvxFontItem*        pFont;
        SvxFontHeightItem*  pHeight;
        SvxColorItem*       pColor;
        sal_Int32               nType;      // < 0 -> undefiniert
        inline              ENTRY( void )
                            {
                                pTmpName = NULL;
                                pFont = NULL;
                                pHeight = NULL;
                                pColor = NULL;
                                nType = -1;
                            }
        inline              ~ENTRY()
                            {
                                if( pTmpName )
                                    delete pTmpName;
                                if( pFont )
                                    delete pFont;
                                if( pHeight )
                                    delete pHeight;
                                if( pColor )
                                    delete pColor;
                            }
        inline void         TmpName( const String &rNew )
                            {
                                if( pTmpName )
                                    *pTmpName = rNew;
                                else
                                    pTmpName = new String( rNew );
                            }
        inline void         Font( SvxFontItem& rNew )
                            {
                                if( pFont )
                                    delete pFont;
                                pFont = &rNew;
                            }
        inline void         Height( SvxFontHeightItem& rNew )
                            {
                                if( pHeight )
                                    delete pHeight;
                                pHeight = &rNew;
                            }
        inline void         Color( SvxColorItem& rNew )
                            {
                                if( pColor )
                                    delete pColor;
                                pColor = &rNew;
                            }
        inline void         Type( const sal_uInt16 nNew )       { nType = nNew; }
    };

    ENTRY                   pData[ 8 ];
    const static sal_uInt16     nSize;
    void                    MakeFont( ENTRY* pEntry );
public:
    void                    Fill( const sal_uInt8 nIndex, SfxItemSet& rItemSet );
    void                    SetName( const sal_uInt16 nIndex, const String& rName );
    void                    SetHeight( const sal_uInt16 nIndex, const sal_uInt16 nHeight );
    void                    SetType( const sal_uInt16 nIndex, const sal_uInt16 nType );
};



#endif
