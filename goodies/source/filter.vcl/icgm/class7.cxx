/*************************************************************************
 *
 *  $RCSfile: class7.cxx,v $
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
#include <chart.hxx>
#include <outact.hxx>

// ---------------------------------------------------------------

void CGM::ImplDoClass7()
{
    switch ( mnElementID )
    {
        case 0x01 : ComOut( CGM_LEVEL1, "Message" ) break;
        case 0x02 :
        {
            sal_uInt8*  pAppData = mpSource + 12;
            sal_uInt16* pTemp = (sal_uInt16*)mpSource;
            sal_uInt16 nOpcode = pTemp[ 4 ];
            sal_uInt16 nOpcSize = pTemp[ 5 ];

            if ( mpChart || ( nOpcode == 0 ) )
            {
                switch ( nOpcode )
                {
                    case 0x000 : ComOut( CGM_LEVEL1, "AppData - Beginning of File Opcodes" )
                    {
                        if ( mpChart == NULL )
                            mpChart = new CGMChart( *this );
                        mpChart->mnCurrentFileType = pAppData[ 3 ];
                    }
                    break;
                    case 0x001 : ComOut( CGM_LEVEL1, "AppData - End of File Opcodes" ) break;
                    case 0x190 : ComOut( CGM_LEVEL1, "AppData - FDESC" ) break;
                    case 0x192 : ComOut( CGM_LEVEL1, "AppData - FNOTES" ) break;
                    case 0x1F4 : ComOut( CGM_LEVEL1, "AppData - BOGENFILE" ) break;
                    case 0x1F5 : ComOut( CGM_LEVEL1, "AppData - EOGENFILE" ) break;
                    case 0x1F8 : ComOut( CGM_LEVEL1, "AppData - BOCHTGROUP" ) break;
                    case 0x1F9 : ComOut( CGM_LEVEL1, "AppData - EOCHTGROUP" ) break;
                    case 0x1FC : ComOut( CGM_LEVEL1, "AppData - BOCHTDATA" ) break;
                    case 0x1FD : ComOut( CGM_LEVEL1, "AppData - EOCHTDATA" )
                    {
                        mpOutAct->DrawChart();
                    }
                    break;
                    case 0x200 : ComOut( CGM_LEVEL1, "AppData - BOSYMGROUP" ) break;
                    case 0x201 : ComOut( CGM_LEVEL1, "AppData - EOSYMGROUP" ) break;
                    case 0x204 : ComOut( CGM_LEVEL1, "AppData - BEGSYMBOL" ) break;
                    case 0x205 : ComOut( CGM_LEVEL1, "AppData - ENDSYMBOL" ) break;
                    case 0x208 : ComOut( CGM_LEVEL1, "AppData - BOSHWGROUP" ) break;
                    case 0x209 : ComOut( CGM_LEVEL1, "AppData - EOSHWGROUP" ) break;
                    case 0x260 : ComOut( CGM_LEVEL1, "AppData - BEGGROUP" ) break;
                    case 0x262 : ComOut( CGM_LEVEL1, "AppData - ENDGROUP" ) break;
                    case 0x264 : ComOut( CGM_LEVEL1, "AppData - DATANODE" )
                    {
                        mpChart->mDataNode[ 0 ] = *(DataNode*)( pAppData );
                        sal_Int8 nZoneEnum = mpChart->mDataNode[ 0 ].nZoneEnum;
                        if ( nZoneEnum && ( nZoneEnum <= 6 ) )
                            mpChart->mDataNode[ nZoneEnum ] = *(DataNode*)( pAppData );
                    }
                    break;
                    case 0x2BE : ComOut( CGM_LEVEL1, "AppData - SHWSLIDEREC" )
                    {
                        if ( mnMode & CGM_EXPORT_IMPRESS )
                        {
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
                    }
                    break;
                    case 0x2C0 : ComOut( CGM_LEVEL1, "AppData - SHWKEYTABLE" ) break;
                    case 0x2C2 : ComOut( CGM_LEVEL1, "AppData - SHWBUTTONTAB" ) break;
                    case 0x2C4 : ComOut( CGM_LEVEL1, "AppData - SHWGLOBAL" ) break;
                    case 0x2C6 : ComOut( CGM_LEVEL1, "AppData - SHWTITLE" ) break;
                    case 0x2CA : ComOut( CGM_LEVEL1, "AppData - SHWAPP" ) break;
                    case 0x320 : ComOut( CGM_LEVEL1, "AppData - TEXT" )
                    {
                        TextEntry* pTextEntry = new TextEntry;
                        pTextEntry->nTypeOfText = *((sal_uInt16*)( pAppData ) );
                        pTextEntry->nRowOrLineNum = *((sal_uInt16*)( pAppData + 2 ) );
                        pTextEntry->nColumnNum = *((sal_uInt16*)( pAppData + 4 ) );
                        sal_uInt16 nAttributes = *( (sal_uInt16*)( pAppData + 6 ) );
                        pTextEntry->nZoneSize = nAttributes & 0xff;
                        pTextEntry->nLineType = ( nAttributes >> 8 ) & 0xf;
                        nAttributes >>= 12;
                        pTextEntry->nAttributes = nAttributes;
                        pAppData += 8;
                        sal_uInt32 nLen = strlen( (char*)( pAppData ) ) + 1;
                        pTextEntry->pText = new char[ nLen ];
                        memcpy( pTextEntry->pText, pAppData, nLen );
                        pAppData += nLen;

                        TextAttribute* pTextOld;
                        for ( sal_uInt16 i = 0; i < nAttributes; i++ )
                        {
                            TextAttribute* pTextAttr = new TextAttribute;

                            *pTextAttr = *(TextAttribute*)( pAppData );

                            pTextAttr->pNextAttribute = NULL;
                            if ( i == 0 )
                                pTextEntry->pAttribute = pTextAttr;
                            else
                                pTextOld->pNextAttribute = pTextAttr;

                            pAppData += sizeof( TextAttribute ) - 4;
                            pTextOld = pTextAttr;
                        }
                        mpChart->InsertTextEntry( pTextEntry );
                    }
                    break;
                    case 0x321 : ComOut( CGM_LEVEL1, "AppData - IOC_TABS" ) break;
                    case 0x322 : ComOut( CGM_LEVEL1, "AppData - CHARTZONE" )
                    {
                        mpChart->mChartZone = *( ChartZone* )( pAppData );
                    }
                    break;
                    case 0x324 : ComOut( CGM_LEVEL1, "AppData - TITLEZONE" ) break;
                    case 0x328 : ComOut( CGM_LEVEL1, "AppData - FOOTNOTEZONE" ) break;
                    case 0x32A : ComOut( CGM_LEVEL1, "AppData - LEGENDZONE" ) break;
                    case 0x330 : ComOut( CGM_LEVEL1, "AppData - PAGEORIENTDIM" )
                    {
                        mpChart->mPageOrientDim = *( PageOrientDim*)( pAppData );
                    }
                    break;
                    case 0x334 : ComOut( CGM_LEVEL1, "AppData - CHTZONEOPTN" )
                    {
                        mpChart->mZoneOption = *( ZoneOption*)( pAppData );
                    }
                    break;
                    case 0x336 : ComOut( CGM_LEVEL1, "AppData - CHTINTL" )
                    {
                        mpChart->mIntSettings = *( IntSettings*)( pAppData );
                    }
                    break;
                    case 0x338 : ComOut( CGM_LEVEL1, "AppData - CHTLINESPC" ) break;
                    case 0x384 : ComOut( CGM_LEVEL1, "AppData - ORGGRIDSTATE" ) break;
                    case 0x386 : ComOut( CGM_LEVEL1, "AppData - ORGSCRSTATE" ) break;
                    case 0x388 : ComOut( CGM_LEVEL1, "AppData - ORGTREESTATE" ) break;
                    case 0x38A : ComOut( CGM_LEVEL1, "AppData - ORGTEXTOPTN" ) break;
                    case 0x38E : ComOut( CGM_LEVEL1, "AppData - ORGBOXOPTN" ) break;
                    case 0x390 : ComOut( CGM_LEVEL1, "AppData - ORGBOXDIM" ) break;
                    case 0x392 : ComOut( CGM_LEVEL1, "AppData - ORGBOX" ) break;
                    case 0x3EA : ComOut( CGM_LEVEL1, "AppData - TTLTEXTOPTN" ) break;
                    case 0x3EE : ComOut( CGM_LEVEL1, "AppData - TTLAUTOBUILD" ) break;
                    case 0x44E : ComOut( CGM_LEVEL1, "AppData - BULTEXTOPTN" ) break;
                    case 0x452 : ComOut( CGM_LEVEL1, "AppData - BULLETOPTN" )
                    {
                        mpChart->mBulletOption = *( BulletOption*)( pAppData );
                    }
                    break;
                    case 0x454 : ComOut( CGM_LEVEL1, "AppData - BULLETLINES" )
                    {
                        mpChart->mBulletLines = *( BulletLines*)( pAppData );
                    }
                    break;
                    case 0x456 : ComOut( CGM_LEVEL1, "AppData - BULAUTOBUILD" ) break;
                    case 0x4B2 : ComOut( CGM_LEVEL1, "AppData - TBLTEXTOPTN" ) break;
                    case 0x4B6 : ComOut( CGM_LEVEL1, "AppData - TBLOPTN" ) break;
                    case 0x4B8 : ComOut( CGM_LEVEL1, "AppData - TBLCOLOPTN" ) break;
                    case 0x4BA : ComOut( CGM_LEVEL1, "AppData - TBLLEGENDOPTN" ) break;
                    case 0x4BC : ComOut( CGM_LEVEL1, "AppData - TBLRANGEOPTN" ) break;
                    case 0x4BE : ComOut( CGM_LEVEL1, "AppData - TBLROWOPTN" ) break;
                    case 0x4C0 : ComOut( CGM_LEVEL1, "AppData - TBLAUTOBUILD" ) break;
                    case 0x518 : ComOut( CGM_LEVEL1, "AppData - PIECHARTOPTN" ) break;
                    case 0x51A : ComOut( CGM_LEVEL1, "AppData - PIELEGENDOPTN" ) break;
                    case 0x51C : ComOut( CGM_LEVEL1, "AppData - PIETEXTOPTN" ) break;
                    case 0x51E : ComOut( CGM_LEVEL1, "AppData - PIEOPTN" ) break;
                    case 0x520 : ComOut( CGM_LEVEL1, "AppData - PIEPCTLABOPTN" ) break;
                    case 0x522 : ComOut( CGM_LEVEL1, "AppData - PIEVALLABOPTN" ) break;
                    case 0x524 : ComOut( CGM_LEVEL1, "AppData - PIESLICE" ) break;
                    case 0x57A : ComOut( CGM_LEVEL1, "AppData - XYAXISOPTN" ) break;
                    case 0x57C : ComOut( CGM_LEVEL1, "AppData - XYGRIDOPTN" ) break;
                    case 0x57D : ComOut( CGM_LEVEL1, "AppData - XYGRIDSHOWFILL" ) break;
                    case 0x57E : ComOut( CGM_LEVEL1, "AppData - XYSERIESOPTN" ) break;
                    case 0x580 : ComOut( CGM_LEVEL1, "AppData - XYSTYLEOPTN" ) break;
                    case 0x582 : ComOut( CGM_LEVEL1, "AppData - XYTABLEOPTN" ) break;
                    case 0x584 : ComOut( CGM_LEVEL1, "AppData - XYTEXTOPTN" ) break;
                    case 0x586 : ComOut( CGM_LEVEL1, "AppData - XYDATAOPTN" ) break;
                    case 0x58A : ComOut( CGM_LEVEL1, "AppData - XYLEGENDOPN" ) break;
                    case 0x58C : ComOut( CGM_LEVEL1, "AppData - XYCALCULATION" ) break;
                    case 0x58E : ComOut( CGM_LEVEL1, "AppData - XYXVALUE" ) break;
                    case 0x590 : ComOut( CGM_LEVEL1, "AppData - XYYVALUE" ) break;
                    case 0x592 : ComOut( CGM_LEVEL1, "AppData - XYXEXTVALUE" ) break;
                    case 0x618 : ComOut( CGM_LEVEL1, "AppData - IOC_CHTCOLRTAB" ) break;
                    case 0x619 : ComOut( CGM_LEVEL1, "AppData - IOC_CHTFONTTAB" ) break;
                    case 0x1fff : ComOut( CGM_LEVEL1, "AppData - 0x1fff" ) break;
                    default : ComOut( CGM_LEVEL1, "UNKNOWN Application Data" ) break;
                }
            }
            mnParaSize = mnElementSize;
            break;
        }
        default: ComOut( CGM_UNKNOWN_COMMAND, "" ) break;
    }
};

