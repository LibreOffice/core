/*************************************************************************
 *
 *  $RCSfile: tblafmt.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:28 $
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
#ifndef _TBLAFMT_HXX
#define _TBLAFMT_HXX

/*************************************************************************
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

JP 20.07.95:

    Die akt. Struktur der Autoformatierung darf nicht mehr veraendert werden.
    Diese wird durch unterschiedlichen Code vom StartWriter und vom StarCalc
    eingelesen/geschrieben.
    Sollte sich doch mal eine Aenderung nicht vermeiden lassen, dann auf
    jedenfall in beiden Applikationen aendern.

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
**************************************************************************/

#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

#include "hintids.hxx"          //_immmer_ vor den solar-items!

#define ITEMID_HORJUSTIFY   0
#define ITEMID_VERJUSTIFY   0
#define ITEMID_ORIENTATION  0
#define ITEMID_MARGIN       0

#ifndef _SVX_ALGITEM_HXX //autogen
#include <svx/algitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_CNTRTITEM_HXX //autogen
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX //autogen
#include <svx/shdditem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_ROTMODIT_HXX //autogen
#include <svx/rotmodit.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif

struct SwAfVersions;

class SvNumberFormatter;

class SwBoxAutoFmt
{
    // gemeinsamer Teil von StarCalc und StarWriter
    SvxFontItem         aFont;

    SvxFontHeightItem   aHeight;
    SvxWeightItem       aWeight;
    SvxPostureItem      aPosture;
    SvxUnderlineItem    aUnderline;
    SvxCrossedOutItem   aCrossedOut;
    SvxContourItem      aContour;
    SvxShadowedItem     aShadowed;
    SvxColorItem        aColor;
    SvxBoxItem          aBox;
    SvxBrushItem        aBackground;

    // StarWriter spezifisches
    SvxAdjustItem       aAdjust;

    // StarCalc spezifisches
    SvxHorJustifyItem   aHorJustify;
    SvxVerJustifyItem   aVerJustify;
    SvxOrientationItem  aOrientation;
    SvxMarginItem       aMargin;
    SfxBoolItem         aLinebreak;
    SfxInt32Item        aRotateAngle;
    SvxRotateModeItem   aRotateMode;

        // und als letzes das Zahlenformat fuer die Box
    String              sNumFmtString;
    LanguageType        eSysLanguage, eNumFmtLanguage;

public:
    SwBoxAutoFmt();
    SwBoxAutoFmt( const SwBoxAutoFmt& rNew );
    ~SwBoxAutoFmt();

    int operator==( const SwBoxAutoFmt& rCmp ) const;
    SwBoxAutoFmt& operator=( const SwBoxAutoFmt& rNew );

    // die Get-Methoden
    const SvxFontItem       &GetFont() const        { return aFont; }
    const SvxFontHeightItem &GetHeight() const      { return aHeight; }
    const SvxWeightItem     &GetWeight() const      { return aWeight; }
    const SvxPostureItem    &GetPosture() const     { return aPosture; }
    const SvxUnderlineItem  &GetUnderline() const   { return aUnderline; }
    const SvxCrossedOutItem &GetCrossedOut() const  { return aCrossedOut; }
    const SvxContourItem    &GetContour() const     { return aContour; }
    const SvxShadowedItem   &GetShadowed() const    { return aShadowed; }
    const SvxColorItem      &GetColor() const       { return aColor; }
    const SvxAdjustItem     &GetAdjust() const      { return aAdjust; }
    const SvxBoxItem        &GetBox() const         { return aBox; }
    const SvxBrushItem      &GetBackground() const  { return aBackground; }
    void GetValueFormat( String& rFmt, LanguageType& rLng, LanguageType& rSys ) const
        { rFmt = sNumFmtString; rLng = eNumFmtLanguage; rSys = eSysLanguage; }

    // die SetMethoden
    void SetFont( const SvxFontItem& rNew )             { aFont = rNew; }
    void SetHeight( const SvxFontHeightItem& rNew )     { aHeight = rNew; }
    void SetWeight( const SvxWeightItem& rNew )         { aWeight = rNew; }
    void SetPosture( const SvxPostureItem& rNew )       { aPosture = rNew; }
    void SetUnderline( const SvxUnderlineItem& rNew )   { aUnderline = rNew; }
    void SetCrossedOut( const SvxCrossedOutItem& rNew ) { aCrossedOut = rNew; }
    void SetContour( const SvxContourItem& rNew )       { aContour = rNew; }
    void SetShadowed( const SvxShadowedItem& rNew )     { aShadowed = rNew; }
    void SetColor( const SvxColorItem& rNew )           { aColor = rNew; }
    void SetAdjust( const SvxAdjustItem& rNew )
        {
            aAdjust.SetAdjust( rNew.GetAdjust() );
            aAdjust.SetOneWord( rNew.GetOneWord() );
            aAdjust.SetLastBlock( rNew.GetLastBlock() );
        }
    void SetBox( const SvxBoxItem& rNew )               { aBox = rNew; }
    void SetBackground( const SvxBrushItem& rNew )      { aBackground = rNew; }
    void SetValueFormat( const String& rFmt, LanguageType eLng, LanguageType eSys )
        { sNumFmtString = rFmt; eNumFmtLanguage = eLng; eSysLanguage = eSys; }

    BOOL Load( SvStream& rStream, const SwAfVersions& rVersions, USHORT nVer );
    BOOL Save( SvStream& rStream ) const;
    BOOL SaveVerionNo( SvStream& rStream ) const;

#ifdef READ_OLDVERS
    // lade alte Version
    BOOL LoadOld( SvStream& rStream, USHORT aLoadVer[] );
#endif
};

class SwTableAutoFmt
{
    friend void _FinitCore();       // zum Zerstoeren des dflt. Pointers
    static SwBoxAutoFmt* pDfltBoxAutoFmt;

    String aName;
    USHORT nStrResId;
    BOOL bInclFont : 1;
    BOOL bInclJustify : 1;
    BOOL bInclFrame : 1;
    BOOL bInclBackground : 1;
    BOOL bInclValueFormat : 1;

    // StarCalc Spezifisches
    BOOL bInclWidthHeight : 1;

    SwBoxAutoFmt* aBoxAutoFmt[ 16 ];

public:
    SwTableAutoFmt( const String& rName );
    SwTableAutoFmt( const SwTableAutoFmt& rNew );
    ~SwTableAutoFmt();

    SwTableAutoFmt& operator=( const SwTableAutoFmt& rNew );

    void SetBoxFmt( const SwBoxAutoFmt& rNew, BYTE nPos );
    const SwBoxAutoFmt& GetBoxFmt( BYTE nPos ) const;

    void SetName( const String& rNew ) { aName = rNew; nStrResId = USHRT_MAX; }
    const String& GetName() const { return aName; }

    enum UpdateFlags { UPDATE_CHAR = 1, UPDATE_BOX = 2, UPDATE_ALL = 3 };
    SwBoxAutoFmt& UpdateFromSet( BYTE nPos, const SfxItemSet& rSet,
                                UpdateFlags eFlags, SvNumberFormatter* );
    void UpdateToSet( BYTE nPos, SfxItemSet& rSet, UpdateFlags eFlags,
                        SvNumberFormatter* ) const ;

    BOOL IsFont() const         { return bInclFont; }
    BOOL IsJustify() const      { return bInclJustify; }
    BOOL IsFrame() const        { return bInclFrame; }
    BOOL IsBackground() const   { return bInclBackground; }
    BOOL IsValueFormat() const  { return bInclValueFormat; }
    BOOL IsWidthHeight() const  { return bInclWidthHeight; }

    void SetFont( const BOOL bNew )         { bInclFont = bNew; }
    void SetJustify( const  BOOL bNew )     { bInclJustify = bNew; }
    void SetFrame( const BOOL bNew )        { bInclFrame = bNew; }
    void SetBackground( const BOOL bNew )   { bInclBackground = bNew; }
    void SetValueFormat( const BOOL bNew )  { bInclValueFormat = bNew; }
    void SetWidthHeight( const BOOL bNew )  { bInclWidthHeight = bNew; }

    BOOL Load( SvStream& rStream, const SwAfVersions& );
    BOOL Save( SvStream& rStream ) const;

#ifdef READ_OLDVERS
    // lade alte Version
    BOOL LoadOld( SvStream& rStream, USHORT aLoadVer[] );
#endif
};
typedef SwTableAutoFmt* SwTableAutoFmtPtr ;

SV_DECL_PTRARR_DEL( _SwTableAutoFmtTbl, SwTableAutoFmtPtr, 1, 5 )

class SwTableAutoFmtTbl : public _SwTableAutoFmtTbl
{
    BOOL Load( SvStream& rStream );
    BOOL Save( SvStream& rStream ) const;
public:
    SwTableAutoFmtTbl();

    BOOL Load();
    BOOL Save() const;
};



/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/inc/tblafmt.hxx,v 1.1.1.1 2000-09-18 17:14:28 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.22  2000/09/18 16:03:31  willem.vandorp
      OpenOffice header added.

      Revision 1.21  2000/09/08 13:24:29  willem.vandorp
      Header and footer replaced

      Revision 1.20  1999/10/11 15:23:51  jp
      TableautoFormatnames with resource ids

      Revision 1.19  1998/09/29 08:12:48  NN
      #53381# bei Zahlformat Systemsprache beim Erzeugen mitspeichern


      Rev 1.18   29 Sep 1998 10:12:48   NN
   #53381# bei Zahlformat Systemsprache beim Erzeugen mitspeichern

      Rev 1.17   15 Sep 1998 21:23:02   ER
   #47738# gedrehter Text in Tabellen-AutoFormat

      Rev 1.16   02 Jun 1998 10:35:02   JP
   TabellenAutoFormat: Load/Save ohne Stream - erzeugen diesen selbst

      Rev 1.15   14 May 1998 15:16:48   JP
   neu: Numberformat vom Boxen unterstuetzen

      Rev 1.14   12 May 1998 23:34:52   JP
   neu: Zuweisungsoperator fuer SwTableAutoFmt

      Rev 1.13   04 Sep 1997 09:02:58   MA
   includes

      Rev 1.12   09 Aug 1997 12:45:06   OS
   paraitem/frmitems/textitem aufgeteilt

      Rev 1.11   07 Aug 1997 14:43:12   OM
   Headerfile-Umstellung

      Rev 1.10   24 Jun 1996 18:35:28   MA
   includes

      Rev 1.9   13 Dec 1995 09:55:02   MA
   opt: Bessere Defaults fuer Arrays

      Rev 1.8   11 Dec 1995 13:38:48   HJS
   anpassung fuer unix

      Rev 1.7   30 Nov 1995 20:22:30   JP
   DefaultTblAutoFmt erst auf Bedarf anlegen

      Rev 1.6   31 Jul 1995 10:17:50   AMA
   International-Umstellung wg. Update 253

      Rev 1.5   20 Jul 1995 19:39:48   JP
   Aederung fuer gemeinsamen Import/Export der Tabellen-Autoformatierung von Calc/Writer

      Rev 1.4   10 Jun 1995 16:05:20   JP
   Save/Load nochmals umgestellt

      Rev 1.3   10 Jun 1995 12:51:26   JP
   SwBoxAutoFmt::Load - Load-Version mit geben

      Rev 1.2   06 Mar 1995 05:07:02   JP
   Load & Save als Dummy

      Rev 1.1   04 Mar 1995 20:42:22   JP
   Adjust nachgetragen

      Rev 1.0   04 Mar 1995 15:46:14   JP
   Initial revision.

*************************************************************************/
    // #ifndef _...
#endif
