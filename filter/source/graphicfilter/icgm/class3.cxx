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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <main.hxx>
#include <outact.hxx>

// ---------------------------------------------------------------

void CGM::ImplDoClass3()
{
    sal_uInt32  nUInteger;
    long    nI0, nI1;
    switch ( mnElementID )
    {
        case 0x01 : ComOut( CGM_LEVEL1, "VDC Integer Precision" )
        {
            switch( ImplGetI( pElement->nIntegerPrecision ) )
            {
                case 16 : pElement->nVDCIntegerPrecision = 2; break;
                case 32 : pElement->nVDCIntegerPrecision = 4; break;
                default : mbStatus = sal_False; break;
            }
        }
        break;
        case 0x02 : ComOut( CGM_LEVEL1, "VDC Real Precision" )
        {
            nUInteger = ImplGetUI16();
            nI0 = ImplGetI( pElement->nIntegerPrecision );  // exponent
            nI1 = ImplGetI( pElement->nIntegerPrecision );  // mantisse
            switch( nUInteger )
            {
                case 0 :
                    pElement->eVDCRealPrecision = RP_FLOAT;
                    switch ( nI0 )
                    {
                        case 9 :
                            if ( nI1 != 23 )
                                mbStatus = sal_False;
                            pElement->nVDCRealSize = 4;
                            break;
                        case 12 :
                            if ( nI1 != 52 )
                                mbStatus =sal_False;
                            pElement->nVDCRealSize = 8;
                            break;
                        default:
                            mbStatus = sal_False;
                            break;
                    }
                    break;
                case 1 :
                    pElement->eVDCRealPrecision = RP_FIXED;
                    if ( nI0 != nI1 )
                        mbStatus = sal_False;
                    if ( nI0 == 16 )
                        pElement->nVDCRealSize = 4;
                    else if ( nI0 == 32 )
                        pElement->nVDCRealSize = 8;
                    else
                        mbStatus = sal_False;
                    break;
                default :
                    mbStatus = sal_False; break;
            }
        }
        break;
        case 0x03 : ComOut( CGM_LEVEL1, "Auxiliary Colour" )
        {
            pElement->nAuxiliaryColor = ImplGetBitmapColor();
        }
        break;
        case 0x04 : ComOut( CGM_LEVEL1, "Transparency" )
        {
            switch( ImplGetUI16() )
            {
                case 0 : pElement->eTransparency = T_OFF; break;
                case 1 : pElement->eTransparency = T_ON; break;
                default : mbStatus = sal_False; break;
            }
        }
        break;
        case 0x05 : ComOut( CGM_LEVEL1, "Clip Rectangle" )
            ImplGetRectangle( pElement->aClipRect );
        break;
        case 0x06 : ComOut( CGM_LEVEL1, "Clip Indicator" )
        {
            switch( ImplGetUI16() )
            {
                case 0 : pElement->eClipIndicator = CI_OFF; break;
                case 1 : pElement->eClipIndicator = CI_ON; break;
                default : mbStatus = sal_False; break;
            }
        }
        break;
        case 0x07 : ComOut( CGM_LEVEL2, "Line Clipping Mode" ) break;               // NS
        case 0x08 : ComOut( CGM_LEVEL2, "Marker Clipping Mode" ) break;             // NS
        case 0x09 : ComOut( CGM_LEVEL2, "Edge Clipping Mode" ) break;               // NS
        case 0x0a : ComOut( CGM_LEVEL2, "New Region" )
            mpOutAct->NewRegion();
        break;
        case 0x0b : ComOut( CGM_LEVEL2, "Save Primitive Context" ) break;           // NS
        case 0x0c : ComOut( CGM_LEVEL2, "Restore Primitive Context" ) break;        // NS
        case 0x11 : ComOut( CGM_LEVEL3, "Protection Region Indicator" ) break;
        case 0x12 : ComOut( CGM_LEVEL3, "Generalized Text Path Mode" ) break;       // NS
        case 0x13 : ComOut( CGM_LEVEL3, "Mitre Limit" )
            pElement->nMitreLimit = ImplGetFloat( pElement->eRealPrecision, pElement->nRealSize );
        break;                                                                      // NS
        case 0x14 : ComOut( CGM_LEVEL3, "Transparent Cell Color" ) break;           // NS
        case 0xfc : ComOut( CGM_GDSF_ONLY, "Text Path Alignment Modes" ) break;
        case 0xfd : ComOut( CGM_GDSF_ONLY, "Pop Transformation Stack" ) break;
        case 0xfe : ComOut( CGM_GDSF_ONLY, "Push Transformation Stack" ) break;
        case 0xff : ComOut( CGM_GDSF_ONLY, "Set Patch ID" ) break;
        default: ComOut( CGM_UNKNOWN_COMMAND, "" ) break;
    }
};


