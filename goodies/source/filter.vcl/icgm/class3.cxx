/*************************************************************************
 *
 *  $RCSfile: class3.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
            if ( mnMode & CGM_IMPORT_IM )
                ALIGN4( mnParaSize );
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
                    if ( nI0 = 16 )
                        pElement->nVDCRealSize = 4;
                    else if ( nI0 = 32 )
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
            if ( mnMode & CGM_IMPORT_IM )                                       // PATCH
                mnParaSize = mnElementSize;
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


