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

#ifndef SC_LOTFNTBF_HXX
#define SC_LOTFNTBF_HXX

#include <tools/solar.h>
#include <tools/string.hxx>
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

    void                    MakeFont( ENTRY* pEntry );
public:
	const static sal_uInt16	nSize = 8;
    void                    Fill( const sal_uInt8 nIndex, SfxItemSet& rItemSet );
    void                    SetName( const sal_uInt16 nIndex, const String& rName );
    void                    SetHeight( const sal_uInt16 nIndex, const sal_uInt16 nHeight );
    void                    SetType( const sal_uInt16 nIndex, const sal_uInt16 nType );
private:
	ENTRY					pData[ nSize ];
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
