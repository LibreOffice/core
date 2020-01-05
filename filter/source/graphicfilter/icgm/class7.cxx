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


#include "main.hxx"
#include "chart.hxx"
#include "outact.hxx"


void CGM::ImplDoClass7()
{
    switch ( mnElementID )
    {
        case 0x01 : /*Message */break;
        case 0x02 :
        {
            if (mpEndValidSource - mpSource < 12)
                throw css::uno::Exception("attempt to read past end of input", nullptr);

            sal_uInt16* pTemp = reinterpret_cast<sal_uInt16*>(mpSource);
            sal_uInt16 nOpcode = pTemp[ 4 ];

            sal_uInt8* pAppData = mpSource + 12;

            if ( mpChart || ( nOpcode == 0 ) )
            {
                switch ( nOpcode )
                {
                    case 0x000 : /*AppData - Beginning of File Opcodes*/
                    {
                        if (mpEndValidSource - pAppData < 4)
                            throw css::uno::Exception("attempt to read past end of input", nullptr);

                        if ( mpChart == nullptr )
                            mpChart.reset( new CGMChart );
                        mpChart->mnCurrentFileType = pAppData[ 3 ];
                    }
                    break;
                    case 0x001 : /*AppData - End of File Opcodes */break;
                    case 0x190 : /*AppData - FDESC */break;
                    case 0x192 : /*AppData - FNOTES */break;
                    case 0x1F4 : /*AppData - BOGENFILE */break;
                    case 0x1F5 : /*AppData - EOGENFILE */break;
                    case 0x1F8 : /*AppData - BOCHTGROUP */break;
                    case 0x1F9 : /*AppData - EOCHTGROUP */break;
                    case 0x1FC : /*AppData - BOCHTDATA */break;
                    case 0x1FD : /*AppData - EOCHTDATA*/
                    {
                        // mpOutAct->DrawChart();
                    }
                    break;
                    case 0x200 : /*AppData - BOSYMGROUP */break;
                    case 0x201 : /*AppData - EOSYMGROUP */break;
                    case 0x204 : /*AppData - BEGSYMBOL */break;
                    case 0x205 : /*AppData - ENDSYMBOL */break;
                    case 0x208 : /*AppData - BOSHWGROUP */break;
                    case 0x209 : /*AppData - EOSHWGROUP */break;
                    case 0x260 : /*AppData - BEGGROUP */break;
                    case 0x262 : /*AppData - ENDGROUP */break;
                    case 0x264 : /*AppData - DATANODE*/
                    {
                        if (static_cast<size_t>(mpEndValidSource - pAppData) < sizeof(DataNode))
                            throw css::uno::Exception("attempt to read past end of input", nullptr);

                        mpChart->mDataNode[ 0 ] = *reinterpret_cast<DataNode*>( pAppData );
                        sal_Int8 nZoneEnum = mpChart->mDataNode[ 0 ].nZoneEnum;
                        if (nZoneEnum > 0 && nZoneEnum <= 6)
                            mpChart->mDataNode[ nZoneEnum ] = *reinterpret_cast<DataNode*>( pAppData );
                    }
                    break;
                    case 0x2BE : /*AppData - SHWSLIDEREC*/
                    {
                        if (mpEndValidSource - pAppData < 16)
                            throw css::uno::Exception("attempt to read past end of input", nullptr);

                        if ( pAppData[ 16 ] == 0 )      // a blank template ?
                        {
                            if ( pAppData[ 2 ] == 46 )
                            {
                                // this starts the document -> maybe we could insert a new document
                            }
                            else if ( pAppData[ 2 ] & 0x80 )
                            {
                                // this is a template
                            }
                            else
                            {
                                mpOutAct->InsertPage();
                            }
                        }
                        mpChart->ResetAnnotation();
                    }
                    break;
                    case 0x2C0 : /*AppData - SHWKEYTABLE */break;
                    case 0x2C2 : /*AppData - SHWBUTTONTAB */break;
                    case 0x2C4 : /*AppData - SHWGLOBAL */break;
                    case 0x2C6 : /*AppData - SHWTITLE */break;
                    case 0x2CA : /*AppData - SHWAPP */break;
                    case 0x320 : /*AppData - TEXT*/
                    {
                        if (mpEndValidSource - pAppData < 9)
                            throw css::uno::Exception("attempt to read past end of input", nullptr);

                        TextEntry* pTextEntry = new TextEntry;
                        pTextEntry->nTypeOfText = *reinterpret_cast<sal_uInt16*>( pAppData );
                        pTextEntry->nRowOrLineNum = *reinterpret_cast<sal_uInt16*>( pAppData + 2 );
                        pTextEntry->nColumnNum = *reinterpret_cast<sal_uInt16*>( pAppData + 4 );
                        sal_uInt16 nAttributes = *reinterpret_cast<sal_uInt16*>( pAppData + 6 );
                        pTextEntry->nZoneSize = nAttributes & 0xff;
                        pTextEntry->nLineType = ( nAttributes >> 8 ) & 0xf;
                        nAttributes >>= 12;
                        pTextEntry->nAttributes = nAttributes;
                        pAppData += 8;
                        auto nMaxLen = mpEndValidSource - pAppData;
                        sal_uInt32 nLen = strnlen(reinterpret_cast<char*>(pAppData), nMaxLen);
                        pTextEntry->pText = new char[nLen + 1];
                        memcpy( pTextEntry->pText, pAppData, nLen );
                        pTextEntry->pText[nLen] = 0;
                        pAppData += nLen;

                        mpChart->InsertTextEntry( pTextEntry );
                    }
                    break;
                    case 0x321 : /*AppData - IOC_TABS */break;
                    case 0x322 : /*AppData - CHARTZONE*/
                    break;
                    case 0x324 : /*AppData - TITLEZONE */break;
                    case 0x328 : /*AppData - FOOTNOTEZONE */break;
                    case 0x32A : /*AppData - LEGENDZONE */break;
                    case 0x330 : /*AppData - PAGEORIENTDIM*/
                    break;
                    case 0x334 : /*AppData - CHTZONEOPTN*/
                    break;
                    case 0x336 : /*AppData - CHTINTL*/
                    break;
                    case 0x338 : /*AppData - CHTLINESPC */break;
                    case 0x384 : /*AppData - ORGGRIDSTATE */break;
                    case 0x386 : /*AppData - ORGSCRSTATE */break;
                    case 0x388 : /*AppData - ORGTREESTATE */break;
                    case 0x38A : /*AppData - ORGTEXTOPTN */break;
                    case 0x38E : /*AppData - ORGBOXOPTN */break;
                    case 0x390 : /*AppData - ORGBOXDIM */break;
                    case 0x392 : /*AppData - ORGBOX */break;
                    case 0x3EA : /*AppData - TTLTEXTOPTN */break;
                    case 0x3EE : /*AppData - TTLAUTOBUILD */break;
                    case 0x44E : /*AppData - BULTEXTOPTN */break;
                    case 0x452 : /*AppData - BULLETOPTN*/
                    break;
                    case 0x454 : /*AppData - BULLETLINES*/break;
                    case 0x456 : /*AppData - BULAUTOBUILD */break;
                    case 0x4B2 : /*AppData - TBLTEXTOPTN */break;
                    case 0x4B6 : /*AppData - TBLOPTN */break;
                    case 0x4B8 : /*AppData - TBLCOLOPTN */break;
                    case 0x4BA : /*AppData - TBLLEGENDOPTN */break;
                    case 0x4BC : /*AppData - TBLRANGEOPTN */break;
                    case 0x4BE : /*AppData - TBLROWOPTN */break;
                    case 0x4C0 : /*AppData - TBLAUTOBUILD */break;
                    case 0x518 : /*AppData - PIECHARTOPTN */break;
                    case 0x51A : /*AppData - PIELEGENDOPTN */break;
                    case 0x51C : /*AppData - PIETEXTOPTN */break;
                    case 0x51E : /*AppData - PIEOPTN */break;
                    case 0x520 : /*AppData - PIEPCTLABOPTN */break;
                    case 0x522 : /*AppData - PIEVALLABOPTN */break;
                    case 0x524 : /*AppData - PIESLICE */break;
                    case 0x57A : /*AppData - XYAXISOPTN */break;
                    case 0x57C : /*AppData - XYGRIDOPTN */break;
                    case 0x57D : /*AppData - XYGRIDSHOWFILL */break;
                    case 0x57E : /*AppData - XYSERIESOPTN */break;
                    case 0x580 : /*AppData - XYSTYLEOPTN */break;
                    case 0x582 : /*AppData - XYTABLEOPTN */break;
                    case 0x584 : /*AppData - XYTEXTOPTN */break;
                    case 0x586 : /*AppData - XYDATAOPTN */break;
                    case 0x58A : /*AppData - XYLEGENDOPN */break;
                    case 0x58C : /*AppData - XYCALCULATION */break;
                    case 0x58E : /*AppData - XYXVALUE */break;
                    case 0x590 : /*AppData - XYYVALUE */break;
                    case 0x592 : /*AppData - XYXEXTVALUE */break;
                    case 0x618 : /*AppData - IOC_CHTCOLRTAB */break;
                    case 0x619 : /*AppData - IOC_CHTFONTTAB */break;
                    case 0x1fff : /*AppData - 0x1fff */break;
                    default : /*UNKNOWN Application Data */break;
                }
            }
            mnParaSize = mnElementSize;
            break;
        }
        default: break;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
