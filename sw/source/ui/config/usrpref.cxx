/*************************************************************************
 *
 *  $RCSfile: usrpref.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:33 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif


#include "swtypes.hxx"
#include "hintids.hxx"
#include "uitool.hxx"
#include "usrpref.hxx"
#include "crstate.hxx"
#include "cfgid.h"
#include "cfgstr.hrc"
#ifndef _OFA_OSPLCFG_HXX
#include <offmgr/osplcfg.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif

#ifndef _LINGU_LNGPROPS_HHX_
#include <lingu/lngprops.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::rtl;

#define C2U(cChar) OUString::createFromAscii(cChar)
#define C2S(cChar) String::CreateFromAscii(cChar)

#define VERSION_30B     ((sal_uInt16)250)   //Beta1 ??
#define VERSION_30B2    ((sal_uInt16)251)   //Beta2
#define VERSION_30B3    ((sal_uInt16)252)   //Beta2.9
#define VERSION_30B4    ((sal_uInt16)253)   //Beta3.1
#define VERSION_30B5    ((sal_uInt16)254)   //Beta4
#define VERSION_30B6    ((sal_uInt16)256)   //Beta6
#define VERSION_301B6   ((sal_uInt16)257)   //Beta6+OnlineSpelling
#define VERSION_301_03  ((sal_uInt16)258)   //Flag fuer BrowseMode
#define VERSION_301_04  ((sal_uInt16)259)   //Flag fuer Massstab beibehalten (Crop)
#define VERSION_391_05  ((sal_uInt16)260)   //Endlich das product/non-product chaos aufgeloest.
#define VERSION_302_01  ((sal_uInt16)313)   //Zusammenfassung der Optionen ab SV 313
#define VERSION_302_02  ((sal_uInt16)325)   //nTblDest neu
#define VERSION_326e    ((sal_uInt16)326)   //PrintPageBack neu
#define VERSION_327c    ((sal_uInt16)327)   //ExecHyperlinks neu
#define VERSION_328     ((sal_uInt16)328)   //HtmlMode
#define VERSION_332     ((sal_uInt16)332)   //DefaultBackground
#define VERSION_333     ((sal_uInt16)333)   //Symbolfont-Position
#define VERSION_346     ((sal_uInt16)346)   //nCore2Options neu
#define VERSION_349     ((sal_uInt16)349)   //TabDist und UndoCount raus
#define VERSION_4SP2a   ((sal_uInt16)400)   //HiddenPara, Fixpack II
#define VERSION_371     ((sal_uInt16)471)   //ShadowCrsr
#define VERSION_388     ((sal_uInt16)500)   //BackgroundBrush -> Background Color
#define VERSION_511     ((sal_uInt16)511)   //Bereichsbegrenzung, Cursor in gesch. Zonen
#define VERSION_512     ((sal_uInt16)512)   //index background flag and color


// mit der naechsten Version koennen die Dummies fuer die TabDist raus

#define USRPEF_VERSION VERSION_512


sal_Bool bNotLoadLayout = sal_False;    // fuer MD


void SwMasterUsrPref::SetUsrPref(const SwViewOption &rCopy)
{
    *((SwViewOption*)this) = rCopy;

#ifndef PRODUCT
    bNotLoadLayout = rCopy.IsTest1();       // MD Layout lesen
#endif
}

/*------------------------------------------------------------------------
 Beschreibung:  Schreiben / Lesen Konfiguration.
                Die einzelnen Bits des Bitfeldes muessen als
                char geschrieben werden, da die Bits eines Bytes
                auf den verschiedenen Plattformen unterschiedlich
                abgelegt werden.
------------------------------------------------------------------------*/


sal_Bool  SwMasterUsrPref::Store(SvStream& rStream)
{
    rStream << USRPEF_VERSION;

#ifdef PRODUCT
    rStream << sal_Int8(1);
#else
    rStream << sal_Int8(0);
#endif

    rStream << nCoreOptions;
    rStream << nCore2Options;
    rStream << nUIOptions;

    rStream << sal_uInt32(GetSnapSize().Width());
    rStream << sal_uInt32(GetSnapSize().Height());
    rStream << sal_uInt16(GetDivisionX());
    rStream << sal_uInt16(GetDivisionY());

    rStream << sal_uInt16(GetZoom());
    rStream << sal_uInt16(eZoom);
    rStream << sal_Int8(nTblDest);

    rStream << sal_uInt16(::GetDfltMetric(sal_False));

    rStream.WriteByteString(sSymbolFont, gsl_getSystemTextEncoding());

    rStream << sal_Int8(GetPagePrevRow());
    rStream << sal_Int8(GetPagePrevCol());

    rStream << sal_uInt32(GetRetoucheColor().GetColor());

    rStream << (sal_Int8)nShdwCrsrFillMode;
    rStream << (sal_uInt32)aShdwCrsrCol.GetColor();

    rStream << (sal_uInt32)aIdxBackgrndCol.GetColor();

#ifndef PRODUCT
    sal_Int8 nFlags3 = 0;
    if ( bTest1 ) nFlags3 |= 0x01;
    if ( bTest2 ) nFlags3 |= 0x02;
    if ( bTest3 ) nFlags3 |= 0x04;
    if ( bTest4 ) nFlags3 |= 0x08;
    if ( bTest5 ) nFlags3 |= 0x10;
    if ( bTest6 ) nFlags3 |= 0x20;
    if ( bTest7 ) nFlags3 |= 0x40;
    if ( bTest8 ) nFlags3 |= 0x80;
    rStream << nFlags3;
#endif

    return SfxConfigItem::ERR_OK;
}



int  SwMasterUsrPref::Load(SvStream& rStream)
{
    SetDefault( sal_False );

    sal_uInt16 nVersion;
    rStream >> nVersion;

    sal_Int8 nProductFlag;
    if ( nVersion >= VERSION_391_05 )
        rStream >> nProductFlag;

    if( nVersion >= VERSION_30B6 && nVersion < VERSION_302_01 )
    {
        nCore2Options = VIEWOPT_CORE2_BLACKFONT;
        sal_uInt16 nFlags;
        rStream >> nFlags;
        nUIOptions = 0;
        nUIOptions |= (nFlags & 0x0001) ? VIEWOPT_2_TOOLBOX         : 0 ;
        nUIOptions |= (nFlags & 0x0002) ? VIEWOPT_2_TOOLBOXTEXT     : 0 ;
        nUIOptions |= (nFlags & 0x0004) ? VIEWOPT_2_TOOLBOXBITMAP   : 0 ;
        nUIOptions |= (nFlags & 0x0008) ? VIEWOPT_2_TOOLBOXBOTH     : 0 ;
        nUIOptions |= (nFlags & 0x0010) ? VIEWOPT_2_TOOLBOXLEFT     : 0 ;
        nUIOptions |= (nFlags & 0x0020) ? VIEWOPT_2_TOOLBOXRIGHT    : 0 ;
        nUIOptions |= (nFlags & 0x0040) ? VIEWOPT_2_TOOLBOXTOP      : 0 ;
        nUIOptions |= (nFlags & 0x0080) ? VIEWOPT_2_TOOLBOXBOTTOM   : 0 ;
        nUIOptions |= (nFlags & 0x0100) ? VIEWOPT_2_UNUSED1         : 0 ;
        nUIOptions |= (nFlags & 0x0200) ? VIEWOPT_2_RIBBONBAR       : 0 ;
        nUIOptions |= (nFlags & 0x0400) ? VIEWOPT_2_TABWIN          : 0 ;
        nUIOptions |= (nFlags & 0x0800) ? VIEWOPT_2_VSCROLLBAR      : 0 ;
        nUIOptions |= (nFlags & 0x1000) ? VIEWOPT_2_HSCROLLBAR      : 0 ;
        nUIOptions |= (nFlags & 0x2000) ? VIEWOPT_2_STATUSLINE      : 0 ;
        nUIOptions |= (nFlags & 0x4000) ? VIEWOPT_2_VLIN            : 0 ;
        nUIOptions |= (nFlags & 0x8000) ? VIEWOPT_2_SVLOOK          : 0 ;


        sal_uInt32 nFlags2;
        rStream >> nFlags2;
        nCoreOptions = VIEWOPT_1_IDLE;
        nCoreOptions |= (nFlags2 & 0x00000001) ? VIEWOPT_1_TAB        : 0;
        nCoreOptions |= (nFlags2 & 0x00000002) ? VIEWOPT_1_BLANK      : 0;
        nCoreOptions |= (nFlags2 & 0x00000004) ? VIEWOPT_1_HARDBLANK  : 0;
        nCoreOptions |= (nFlags2 & 0x00000008) ? VIEWOPT_1_PARAGRAPH  : 0;
        nCoreOptions |= (nFlags2 & 0x00000010) ? VIEWOPT_1_LINEBREAK  : 0;
        nCoreOptions |= (nFlags2 & 0x00000020) ? VIEWOPT_1_PAGEBREAK  : 0;
        nCoreOptions |= (nFlags2 & 0x00000040) ? VIEWOPT_1_COLUMNBREAK: 0;
        nCoreOptions |= (nFlags2 & 0x00000080) ? VIEWOPT_1_SOFTHYPH   : 0;
        nCoreOptions |= (nFlags2 & 0x00000100) ? VIEWOPT_1_TOX        : 0;
        nCoreOptions |= (nFlags2 & 0x00000200) ? VIEWOPT_1_REF        : 0;
        nCoreOptions |= (nFlags2 & 0x00000400) ? VIEWOPT_1_FLDNAME    : 0;
        nCoreOptions |= (nFlags2 & 0x00000800) ? VIEWOPT_1_FIELD      : 0;
        nCoreOptions |= (nFlags2 & 0x00001000) ? VIEWOPT_1_FOOTNOTE   : 0;
        nCoreOptions |= (nFlags2 & 0x00002000) ? VIEWOPT_1_POSTITS    : 0;
        nCoreOptions |= (nFlags2 & 0x00004000) ? VIEWOPT_1_HIDDEN     : 0;
        nCoreOptions |= (nFlags2 & 0x00008000) ? VIEWOPT_1_SUBSLINES  : 0;
        nCoreOptions |= (nFlags2 & 0x00010000) ? VIEWOPT_1_GRAPHIC    : 0;
        nCoreOptions |= (nFlags2 & 0x00020000) ? VIEWOPT_1_TABLE      : 0;
        nCoreOptions |= (nFlags2 & 0x00040000) ? VIEWOPT_1_SUBSTABLE  : 0;
        nCoreOptions |= (nFlags2 & 0x00080000) ? VIEWOPT_1_CROSSHAIR  : 0;
        nCoreOptions |= (nFlags2 & 0x00100000) ? VIEWOPT_1_SNAP       : 0;
        nCoreOptions |= (nFlags2 & 0x00200000) ? VIEWOPT_1_SYNCHRONIZE: 0;
        nCoreOptions |= (nFlags2 & 0x00400000) ? VIEWOPT_1_GRIDVISIBLE: 0;
        nCoreOptions |= (nFlags2 & 0x00800000) ? VIEWOPT_1_DRAW       : 0;

        sal_uInt32 lWidth, lHeight;
        rStream >> lWidth;
        rStream >> lHeight;
         Size aSz( lWidth, lHeight );
        SetSnapSize( aSz );

        sal_uInt16 sVal;
        rStream >> sVal;
        SetDivisionX( sVal );
        rStream >> sVal;
        SetDivisionY( sVal );

        sal_uInt16 nZoom;
        rStream >> nZoom;
        SetZoom( nZoom );
        rStream >> nZoom;
        eZoom = int(nZoom);
        nTblDest = TBL_DEST_CELL;

        sal_uInt16 nMet;
        //Default-Tabulator und Metric
        // werden noch gelesen/geschrieben, damit die Version bleiben kann
        rStream >> nMet;
        rStream >> nMet;

        sal_Int8 nPrev;
        rStream >> nPrev;
        SetPagePrevRow(nPrev);
        rStream >> nPrev;
        SetPagePrevCol(nPrev);

        sal_uInt16 nUndo;
        rStream >> nUndo;

        if( nVersion > VERSION_30B6 )
        {
            rStream >> nPrev;
            SetKeepRatio( nPrev );
        }
        else
            SetKeepRatio( sal_False );
        if( nVersion > VERSION_301_03 )
        {
            rStream >> nPrev;
            SetGrfKeepZoom( nPrev ) ;
        }
        else
            SetGrfKeepZoom( sal_False );

#ifndef PRODUCT
    if ( nVersion < VERSION_391_05 || nProductFlag == sal_Int8(0) )
    {
        sal_Int8 nFlags3;
        rStream >> nFlags3;
        bTest1 = sal_Int8( (nFlags3 & 0x01) != 0 );
        if(bNotLoadLayout && !bTest1)
            DBG_ERROR("Layout wird doch geladen - nochmal versuchen!")
        bNotLoadLayout  = bTest1;

        bTest2 = sal_Int8( (nFlags3 & 0x02) != 0 );
        bTest3 = sal_Int8( (nFlags3 & 0x04) != 0 );
        bTest4 = sal_Int8( (nFlags3 & 0x08) != 0 );
        bTest5 = sal_Int8( (nFlags3 & 0x10) != 0 );
        bTest6 = sal_Int8( (nFlags3 & 0x20) != 0 );
        bTest7 = sal_Int8( (nFlags3 & 0x40) != 0 );
        bTest8 = sal_Int8( (nFlags3 & 0x80) != 0 );
    }
    else
        bTest1 = bTest2 = bTest3 = bTest4 = bTest5 = bTest6 =
        bTest7 = bTest8 = bNotLoadLayout = sal_False;
#endif
    }
    else if( nVersion >= VERSION_302_01)
    {
        rStream >> nCoreOptions;
        nCoreOptions |= VIEWOPT_1_IDLE;
        if( nVersion < VERSION_326e )
            nCoreOptions |= VIEWOPT_1_PAGEBACK;
        if(nVersion >= VERSION_346)
        {
            rStream >> nCore2Options;
            if ( nVersion < VERSION_4SP2a )
                nCore2Options |= VIEWOPT_CORE2_HIDDENPARA;
        }
        else
            nCore2Options = VIEWOPT_CORE2_BLACKFONT | VIEWOPT_CORE2_HIDDENPARA;

        if(nVersion < VERSION_511)
        {
            nCore2Options &= ~VIEWOPT_CORE2_SECTION_BOUNDS;
            nCore2Options |= VIEWOPT_CORE2_CRSR_IN_PROT;
        }
        if(nVersion < VERSION_512)
        {
            nCore2Options |= VIEWOPT_CORE2_INDEX_BACKGROUND;
        }

        rStream >> nUIOptions;
        if( nVersion < VERSION_327c )
            nUIOptions |= VIEWOPT_2_EXECHYPERLINKS;
        sal_uInt32 lWidth, lHeight;
        rStream >> lWidth;
        rStream >> lHeight;
         Size aSz( lWidth, lHeight );
        SetSnapSize( aSz );

        sal_uInt16 sVal;
        rStream >> sVal;
        SetDivisionX( sVal );
        rStream >> sVal;
        SetDivisionY( sVal );

        sal_uInt16 nZoom;
        rStream >> nZoom;
        SetZoom( nZoom );
        rStream >> nZoom;
        eZoom = int(nZoom);
        if(nVersion >= VERSION_302_02)
        {
            sal_Int8 nDest;
            rStream >> nDest;
            nTblDest = Min((sal_Int8)TBL_DEST_TBL, nDest);
        }
        else
        {
            nTblDest = TBL_DEST_CELL;
        }


        sal_uInt16 nMet;
        //Default-Tabulator und Metric
        // werden noch gelesen/geschrieben, damit die Version bleiben kann
        rStream >> nMet;
        if(nVersion < VERSION_349)
            rStream >> nMet;

        if(nVersion >= VERSION_333)
            rStream.ReadByteString(sSymbolFont, gsl_getSystemTextEncoding());

        sal_Int8 nPrev;
        rStream >> nPrev;
        SetPagePrevRow(nPrev);
        rStream >> nPrev;
        SetPagePrevCol(nPrev);

        // Undo einlesen
        if(nVersion < VERSION_349)
        {
            sal_uInt16 nUndo;
            rStream >> nUndo;
        }

        if(nVersion >= VERSION_332)
        {
            sal_uInt32 nColor;
            if ( nVersion < VERSION_388 )
            {
                sal_Int8 nStyle;
                if(GetType() == CFG_USERPREF_ITEM)
                {
                    // fuer Textdokumente wird keine Farbe gesetzt!
                    rStream >> nColor;
                    rStream >> nColor;
                    rStream >> nStyle;
                    rStream >> nStyle;
                }
                else
                {
                    rStream >> nColor;
                    aRetoucheColor.SetColor( nColor );
                    rStream >> nColor;
                    rStream >> nStyle;
                    if ( nStyle == 0 /*BRUSH_NULL*/ )
                        aRetoucheColor.SetColor( COL_TRANSPARENT );
                    rStream >> nStyle;
                }
            }
            else
            {
                if ( GetType() == CFG_USERPREF_ITEM )
                    rStream >> nColor;
                else
                {
                    rStream >> nColor;
                    aRetoucheColor.SetColor( nColor );
                }
            }
        }
        if(nVersion >= VERSION_371)
        {
            sal_Int8 nMode;
            sal_uInt32 nColor;
            rStream >> nMode;
            rStream >> nColor;

            aShdwCrsrCol.SetColor( nColor );
            nShdwCrsrFillMode = nMode;
        }
        if(nVersion >= VERSION_512)
        {
            sal_uInt32 nColor;
            rStream >> nColor;
            aIdxBackgrndCol.SetColor(nColor);
        }

#ifndef PRODUCT
    if ( nVersion < VERSION_391_05 || nProductFlag == sal_Int8(0) )
    {
        sal_Int8 nFlags3;
        rStream >> nFlags3;
        bTest1 = sal_Int8( (nFlags3 & 0x01) != 0 );
        if(bNotLoadLayout && !bTest1)
            DBG_ERROR("Layout wird doch geladen - nochmal versuchen!")
        bNotLoadLayout  = bTest1;
        bTest2 = sal_Int8( (nFlags3 & 0x02) != 0 );
        bTest3 = sal_Int8( (nFlags3 & 0x04) != 0 );
        bTest4 = sal_Int8( (nFlags3 & 0x08) != 0 );
        bTest5 = sal_Int8( (nFlags3 & 0x10) != 0 );
        bTest6 = sal_Int8( (nFlags3 & 0x20) != 0 );
        bTest7 = sal_Int8( (nFlags3 & 0x40) != 0 );
        bTest8 = sal_Int8( (nFlags3 & 0x80) != 0 );
    }
    else
        bTest1 = bTest2 = bTest3 = bTest4 = bTest5 = bTest6 =
        bTest7 = bTest8 = bNotLoadLayout = sal_False;
#endif
    }
    else
        return SfxConfigItem::WARNING_VERSION;

    uno::Reference< beans::XPropertySet >  xProp( ::GetLinguPropertySet() );

    sal_Bool bVal;
    bVal = xProp.is() ?
            *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_SPELL_AUTO) ).getValue() : sal_False;
    SetOnlineSpell( bVal );
    bVal = xProp.is() ?
            *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_SPELL_HIDE) ).getValue() : sal_False;
    SetHideSpell( bVal );
    return SfxConfigItem::ERR_OK;
}


void  SwMasterUsrPref::UseDefault()
{
    nUIOptions =    VIEWOPT_2_TOOLBOXBITMAP | VIEWOPT_2_TOOLBOXTOP |
                    VIEWOPT_2_TOOLBOX | VIEWOPT_2_RIBBONBAR |
                    VIEWOPT_2_TABWIN | VIEWOPT_2_STATUSLINE |
                    VIEWOPT_2_VSCROLLBAR | VIEWOPT_2_HSCROLLBAR |
                    VIEWOPT_2_SVLOOK| VIEWOPT_2_EXECHYPERLINKS;
    nCore2Options = VIEWOPT_CORE2_BLACKFONT | VIEWOPT_CORE2_HIDDENPARA|
                    VIEWOPT_CORE2_CRSR_IN_PROT| VIEWOPT_CORE2_INDEX_BACKGROUND|
                    VIEWOPT_CORE2_SECTION_BOUNDS;

    eZoom = 0;
    nTblDest = TBL_DEST_CELL;

    sSymbolFont = aEmptyStr;

    aShdwCrsrCol.SetColor( COL_BLUE );
    nShdwCrsrFillMode = FILL_TAB;

    aIdxBackgrndCol.SetColor( RGB_COLORDATA( 0xF0, 0xF0, 0xF0 ) );

    uno::Reference< beans::XPropertySet >  xProp( ::GetLinguPropertySet() );

    sal_Bool bVal;
    bVal = xProp.is() ?
            *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_SPELL_AUTO) ).getValue() : sal_False;
    SetOnlineSpell( bVal );
    bVal = xProp.is() ?
            *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_SPELL_HIDE) ).getValue() : sal_False;
    SetHideSpell( bVal );
    SfxConfigItem::UseDefault();
}


SwMasterUsrPref::SwMasterUsrPref(sal_uInt16 nType) :
    SfxConfigItem( nType )
{}


String  SwMasterUsrPref::GetName() const
{
    String sName(SW_RES( STR_CFG_USRPREF ));
    if(GetType() != CFG_USERPREF_ITEM)
        sName.SearchAndReplace(C2S("StarOffice Writer"), C2S("StarOffice  Writer/Web"));
    return sName;
}
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.128  2000/09/18 16:05:18  willem.vandorp
    OpenOffice header added.

    Revision 1.127  2000/04/11 08:02:24  os
    UNICODE

    Revision 1.126  2000/03/23 08:17:22  os
    UNO III

    Revision 1.125  2000/03/03 15:16:59  os
    StarView remainders removed

    Revision 1.124  2000/02/24 17:26:00  hr
    43447#: gcc

    Revision 1.123  2000/01/19 18:24:06  jp
    Bug #72118#: change default of view sectionboundaries

    Revision 1.122  1999/12/10 13:07:24  tl
    #70383# SvxGetLinguPropertySet => ::GetLinguPropertySet

    Revision 1.121  1999/11/24 18:26:26  tl
    check for Service availability

    Revision 1.120  1999/11/19 16:40:22  os
    modules renamed

    Revision 1.119  1999/10/25 19:47:09  tl
    ongoing ONE_LINGU implementation

    Revision 1.118  1999/10/22 11:06:41  os
    index background with a lighter background

    Revision 1.117  1999/10/13 14:19:58  os
    index background in light gray

    Revision 1.116  1999/08/31 08:46:22  TL
    #if[n]def ONE_LINGU inserted (for transition of lingu to StarOne)


      Rev 1.115   31 Aug 1999 10:46:22   TL
   #if[n]def ONE_LINGU inserted (for transition of lingu to StarOne)

      Rev 1.114   09 Jun 1999 13:22:42   OS
   index background

      Rev 1.113   19 May 1999 13:32:56   OS
   #61245# DBG_ERROR, wenn Layout trotz CtrlAltL geladen wird

      Rev 1.112   27 Jan 1999 10:03:10   OS
   #58677# Cursor in Readonly-Bereichen

      Rev 1.111   08 Sep 1998 16:49:52   OS
   #56134# Metric fuer Text und HTML getrennt

      Rev 1.110   31 Aug 1998 11:19:14   OS
   #55750# OnlineSpelling immer aus der OFA uebernehmen

      Rev 1.109   12 Jun 1998 09:44:14   OS
   Konfigurationsnamen berichtigt

      Rev 1.108   29 Apr 1998 09:27:06   MA
   BackgroundBrush -> RetoucheColor

      Rev 1.107   24 Nov 1997 17:58:48   MA
   include

      Rev 1.106   11 Nov 1997 14:04:28   MA
   precomp entfernt

      Rev 1.105   03 Nov 1997 16:12:36   JP
   neu: Optionen/-Page/Basic-Schnittst. fuer ShadowCursor

      Rev 1.104   16 Sep 1997 13:23:10   OS
   Color fuer VCL nicht konvertieren, fuer TextDocs Brush nie laden #43824#

      Rev 1.103   08 Aug 1997 16:11:40   OS
   IDLE-Flag auch in der aktuellen Version im Load immer einschalten #42510#

      Rev 1.102   09 Jun 1997 14:28:44   MA
   chg: Browse-Flag nur noch am Doc

      Rev 1.101   06 Jun 1997 12:44:14   MA
   chg: versteckte Absaetze ausblenden

      Rev 1.100   31 Jan 1997 13:41:24   OS
   alten Code fuer Metric geloescht

      Rev 1.99   27 Jan 1997 16:30:04   OS
   HtmlMode entfernt

      Rev 1.98   13 Dec 1996 14:33:32   OS
   UndoCount wird aus der SfxApp besorgt

      Rev 1.97   10 Dec 1996 16:58:48   OS
   TabDist ab sofort in der OFA

      Rev 1.96   09 Dec 1996 17:02:02   OS
   Grenzwerte fuer TblDest und Metric beim Load ueberpruefen

      Rev 1.95   28 Nov 1996 15:20:24   OS
   neu: Schwarz drucken

      Rev 1.94   18 Nov 1996 10:26:22   OS
   includes berichtigt

      Rev 1.93   11 Nov 1996 09:22:12   MA
   ResMgr

      Rev 1.92   17 Oct 1996 16:43:44   OS
   Altlasten der BrowseUsrPref entfernt

      Rev 1.91   25 Sep 1996 11:11:48   OS
   IsTransparent wird jetzt auch gespeichert

      Rev 1.90   30 Aug 1996 08:44:06   OS
   neu: sSymbolFont

      Rev 1.89   26 Aug 1996 16:56:10   OS
   neu: Brush fuer BrowseView

      Rev 1.88   25 Jul 1996 15:35:36   OS
   neu : HTML-Mode

      Rev 1.87   22 Jul 1996 15:06:30   OS
   neu: ..EXECHYPERLINKS

      Rev 1.86   16 Jul 1996 20:19:50   MA
   includes

      Rev 1.85   16 Jul 1996 15:52:20   MA
   new: PrintPageBackground

      Rev 1.84   10 Jul 1996 12:01:26   OS
   neu: nTblDest : Ziel fuer Tabellenhintergrund

      Rev 1.83   28 Jun 1996 10:11:42   OS
   UseDefault: Basisklasse rufen

      Rev 1.82   07 May 1996 08:32:06   OS
   im Laden der UsrPref Browse-Optionen absichern

      Rev 1.81   27 Apr 1996 14:53:14   OS
   Idle-Flag im Load setzen

      Rev 1.80   26 Apr 1996 13:54:16   OS
   Versionspruefung korrigiert

      Rev 1.79   25 Apr 1996 16:21:58   OS
   ViewOptions ohne sal_Bool, dafuer massenweise defines

      Rev 1.78   24 Apr 1996 15:09:00   OS
   Umstellung UsrPref/ViewOption

      Rev 1.77   25 Mar 1996 19:45:50   MA
   wechsel product/non-product

      Rev 1.76   21 Mar 1996 12:51:50   OS
   neu: bGrfKeepZoom fuer CropPage

      Rev 1.75   03 Mar 1996 16:48:48   MA
   new: Flag fuer BrowseView

      Rev 1.74   27 Feb 1996 12:18:54   OS
   Metric nicht mehr in den UsrPref

      Rev 1.73   23 Feb 1996 13:25:28   OS
   neu: nUndoCount = UNDO_ACTION_COUNT

      Rev 1.72   20 Feb 1996 16:31:40   OS
   neu: bKeepAspectRatio fuer Abgleich im Grafik/OLE-Dialog

      Rev 1.71   20 Feb 1996 11:01:52   AMA
   New: bOnlineSpell und bHideSpell

      Rev 1.70   28 Nov 1995 22:36:04   JP
   UiSystem-Klasse aufgehoben, in initui/swtypes aufgeteilt

      Rev 1.69   24 Nov 1995 16:58:52   OM
   PCH->PRECOMPILED

      Rev 1.68   08 Nov 1995 12:48:16   OM
   Change->Set

      Rev 1.67   03 Nov 1995 18:54:54   OS
   vergessenes Table-Flag nachgetragen

      Rev 1.66   29 Sep 1995 16:58:40   OM
   HelpPI entfernt

      Rev 1.65   17 Sep 1995 20:00:58   OS
   neu: nUndoCount

      Rev 1.64   15 Sep 1995 12:40:20   OS
   GetName() implementiert und ColorSetItem entfernt

      Rev 1.63   15 Aug 1995 17:04:52   OM
   Preview-Zoomfaktor gesichert

      Rev 1.62   14 Jul 1995 09:57:22   MA
   fix: TabDist ab Version B3 nicht groesser B3

      Rev 1.61   13 Jul 1995 17:12:22   OS
   DefaultAttribut fuer nTabDist holen

      Rev 1.60   13 Jul 1995 09:05:02   MA
   fix: Tab zum x-ten, default fuer Load verbessert

      Rev 1.59   12 Jul 1995 11:31:00   MA
   fix: Load(), TabDist fuer alte Versionen defaulten

      Rev 1.58   12 Jul 1995 10:49:42   MA
   chg: MinTabDist ist MM50

      Rev 1.57   11 Jul 1995 15:48:06   OS
   neuer Member=>neue Version

      Rev 1.56   10 Jul 1995 19:27:32   OS
   +nTabDist in Load, ::com::sun::star::ucb::Store, UseDefault

      Rev 1.55   25 Jun 1995 13:20:22   MA
   opt: ViewOptions aufgeraeumt

      Rev 1.54   17 May 1995 17:26:06   MA
   UsrPref: gefixt, aufgeraumt, optimiert, includes gerichtet.

      Rev 1.53   03 Apr 1995 19:56:36   OS
   Einstellungen mehr als einmal laden

      Rev 1.52   20 Mar 1995 19:07:40   OS
   unbenutzte Funktionen entfernt

      Rev 1.51   08 Mar 1995 03:56:52   OS
   Raster-Einstellungen auch sichern

      Rev 1.50   19 Feb 1995 17:44:50   MA
   SEXPORT'iert.

      Rev 1.49   25 Oct 1994 16:13:02   ER
   add: PCH

      Rev 1.48   20 Oct 1994 12:54:42   JN
   Versionskontrolle bei Load/Store

      Rev 1.47   05 Oct 1994 10:07:56   SWG
   Anpassung Sfx

      Rev 1.46   15 Sep 1994 14:38:40   MS
   Reanimation die 1.

      Rev 1.45   15 Sep 1994 14:10:56   MS

      Rev 1.44   04 Aug 1994 10:36:00   SWG
   swg32: SED Size to SSize, LSize to Size etc.

------------------------------------------------------------------------*/

