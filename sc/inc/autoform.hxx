/*************************************************************************
 *
 *  $RCSfile: autoform.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:47 $
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

#ifndef SC_AUTOFORM_HXX
#define SC_AUTOFORM_HXX

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

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif

#ifndef SC_COLLECT_HXX
#include "collect.hxx"
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

class SfxBoolItem;
class SvxHorJustifyItem;
class SvxVerJustifyItem;
class SvxMarginItem;
class SvxOrientationItem;
class SvxFontItem;
class SvxPostureItem;
class SvxWeightItem;
class SvxFontHeightItem;
class SvxUnderlineItem;
class SvxCrossedOutItem;
class SvxShadowedItem;
class SvxContourItem;
class SvxColorItem;
class SvxBoxItem;
class SvxAdjustItem;
class SfxInt32Item;
class SvxRotateModeItem;


class ScNumFormatAbbrev;
struct ScAfVersions;

class ScAutoFormatData : public DataObject
{
private:
    String                  aName;
    USHORT                  nStrResId;
    // gemeinsame Flags von StarCalc und StarWriter
    BOOL                    bIncludeFont : 1;
    BOOL                    bIncludeJustify : 1;
    BOOL                    bIncludeFrame : 1;
    BOOL                    bIncludeBackground : 1;

    // StarCalc spezifisches
    BOOL                    bIncludeValueFormat : 1;
    BOOL                    bIncludeWidthHeight : 1;

    // gemeinsame Attribute von StarCalc und StarWriter
    SvxFontItem*            pFont[16];
    SvxFontHeightItem*      pFontHeight[16];
    SvxWeightItem*          pFontWeight[16];
    SvxPostureItem*         pFontPosture[16];
    SvxUnderlineItem*       pFontUnderline[16];
    SvxCrossedOutItem*      pFontCrossedOut[16];
    SvxContourItem*         pFontContour[16];
    SvxShadowedItem*        pFontShadowed[16];
    SvxColorItem*           pFontColor[16];
    SvxBoxItem*             pBox[16];
    SvxBrushItem*           pBackground[16];

    // StarWriter spezifisches
    SvxAdjustItem*          pAdjust[16];

    // StarCalc spezifisches
    SvxHorJustifyItem*      pHorJustify[16];
    SvxVerJustifyItem*      pVerJustify[16];
    SvxOrientationItem*     pOrientation[16];
    SvxMarginItem*          pMargin[16];
    SfxBoolItem*            pLinebreak[16];
    // ab SO5, 504k, gedrehter Text
    SfxInt32Item*           pRotateAngle[16];
    SvxRotateModeItem*      pRotateMode[16];

    // Zahlenformat
    ScNumFormatAbbrev*      pNumFormat[16];

public:
            ScAutoFormatData();
            ScAutoFormatData( const ScAutoFormatData& rData );
    virtual ~ScAutoFormatData();

    virtual DataObject* Clone() const { return new ScAutoFormatData(*this); }

    void    SetName(const String& rName) { aName = rName; nStrResId = USHRT_MAX; }
    void    GetName(String& rName) const { rName = aName; }

    BOOL    GetIncludeValueFormat() const { return bIncludeValueFormat; }
    void    SetIncludeValueFormat(const BOOL bValueFormat) { bIncludeValueFormat = bValueFormat; }

    BOOL    GetIncludeFont() const { return bIncludeFont; }
    void    SetIncludeFont(const BOOL bFont) { bIncludeFont = bFont; }

    BOOL    GetIncludeJustify() const { return bIncludeJustify; }
    void    SetIncludeJustify(const BOOL bJustify) { bIncludeJustify = bJustify; }

    BOOL    GetIncludeFrame() const { return bIncludeFrame; }
    void    SetIncludeFrame(const BOOL bFrame) { bIncludeFrame = bFrame; }

    BOOL    GetIncludeBackground() const { return bIncludeBackground; }
    void    SetIncludeBackground(const BOOL bBackground) { bIncludeBackground = bBackground; }

    BOOL    GetIncludeWidthHeight() const { return bIncludeWidthHeight; }
    void    SetIncludeWidthHeight(const BOOL bWidthHeight) { bIncludeWidthHeight = bWidthHeight; }

    void    GetNumFormat(USHORT nIndex, ScNumFormatAbbrev& rNumFormat) const;
    void    SetNumFormat(USHORT nIndex, const ScNumFormatAbbrev& rNumFormat);

    void    GetFont(USHORT nIndex, SvxFontItem& rFont) const;
    void    SetFont(USHORT nIndex, const SvxFontItem& rFont);

    void    GetFontHeight(USHORT nIndex, SvxFontHeightItem& rFontHeight) const;
    void    SetFontHeight(USHORT nIndex, const SvxFontHeightItem& rFontHeight);

    void    GetFontWeight(USHORT nIndex, SvxWeightItem& rFontWeight) const;
    void    SetFontWeight(USHORT nIndex, const SvxWeightItem& rFontWeight);

    void    GetFontPosture(USHORT nIndex, SvxPostureItem& rFontPosture) const;
    void    SetFontPosture(USHORT nIndex, const SvxPostureItem& rFontPosture);

    void    GetFontUnderline(USHORT nIndex, SvxUnderlineItem& rFontUnderline) const;
    void    SetFontUnderline(USHORT nIndex, const SvxUnderlineItem& rFontUnderline);

    void    GetFontCrossedOut(USHORT nIndex, SvxCrossedOutItem& rFontCrossedOut) const;
    void    SetFontCrossedOut(USHORT nIndex, const SvxCrossedOutItem& rFontCrossedOut);

    void    GetFontContour(USHORT nIndex, SvxContourItem& rFontContour) const;
    void    SetFontContour(USHORT nIndex, const SvxContourItem& rFontContour);

    void    GetFontShadowed(USHORT nIndex, SvxShadowedItem& rFontShadowed) const;
    void    SetFontShadowed(USHORT nIndex, const SvxShadowedItem& rFontShadowed);

    void    GetFontColor(USHORT nIndex, SvxColorItem& rFontColor) const;
    void    SetFontColor(USHORT nIndex, const SvxColorItem& rFontColor);

    void    GetHorJustify(USHORT nIndex, SvxHorJustifyItem& rHorJustify) const;
    void    SetHorJustify(USHORT nIndex, const SvxHorJustifyItem& rHorJustify);

    void    GetVerJustify(USHORT nIndex, SvxVerJustifyItem& rVerJustify) const;
    void    SetVerJustify(USHORT nIndex, const SvxVerJustifyItem& rVerJustify);

    void    GetOrientation(USHORT nIndex, SvxOrientationItem& rOrientation) const;
    void    SetOrientation(USHORT nIndex, const SvxOrientationItem& rOrientation);

    void    GetLinebreak(USHORT nIndex, SfxBoolItem& rLinebreak) const;
    void    SetLinebreak(USHORT nIndex, const SfxBoolItem& rLinebreak);

    void    GetMargin(USHORT nIndex, SvxMarginItem& rMargin) const;
    void    SetMargin(USHORT nIndex, const SvxMarginItem& rMargin);

    void    GetBox(USHORT nIndex, SvxBoxItem& rBox) const;
    void    SetBox(USHORT nIndex, const SvxBoxItem& rBox);

    void    GetBackground(USHORT nIndex, SvxBrushItem& rBackground) const;
    void    SetBackground(USHORT nIndex, const SvxBrushItem& rBackground);

    void    GetRotateAngle( USHORT nIndex, SfxInt32Item& rRotateAngle ) const;
    void    SetRotateAngle( USHORT nIndex, const SfxInt32Item& rRotateAngle );

    void    GetRotateMode( USHORT nIndex, SvxRotateModeItem& rRotateMode ) const;
    void    SetRotateMode( USHORT nIndex, const SvxRotateModeItem& rRotateMode );

    const SfxPoolItem* GetItem( USHORT nIndex, USHORT nWhich ) const;
    void    PutItem( USHORT nIndex, const SfxPoolItem& rItem );

    BOOL    IsEqualData(USHORT nIndex1, USHORT nIndex2);

    BOOL    Load(SvStream& rStream, const ScAfVersions& rVersions);
    BOOL    Save(SvStream& rStream);

#ifdef READ_OLDVERS
    BOOL    LoadOld( SvStream& rStream, const ScAfVersions& rVersions );
#endif
};

class ScAutoFormat : public SortedCollection
{
private:
    BOOL    bSaveLater;

public:
            ScAutoFormat(USHORT nLim = 4, USHORT nDel = 4, BOOL bDup = FALSE);
            ScAutoFormat(const ScAutoFormat& AutoFormat);
    virtual ~ScAutoFormat();
    virtual DataObject*         Clone() const { return new ScAutoFormat(*this); }
            ScAutoFormatData*   operator[]( const USHORT nIndex) const {return (ScAutoFormatData*)At(nIndex);}
    virtual short               Compare(DataObject* pKey1, DataObject* pKey2) const;
            BOOL                Load();
            BOOL                Save();
            USHORT              FindIndexPerName( const String& rName ) const;
            void                SetSaveLater( BOOL bSet );
            BOOL                IsSaveLater() const         { return bSaveLater; }
};


#endif

