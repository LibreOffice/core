/*************************************************************************
 *
 *  $RCSfile: stlpool.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:46 $
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

#pragma hdrstop

#ifndef NOOLDSV //autogen
#include <vcl/system.hxx>
#endif
#ifndef _EEITEMID_HXX //autogen
#include <svx/eeitemid.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_ITEM_HXX //autogen
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX //autogen
#include <svx/shdditem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef SVX_XFILLIT0_HXX //autogen
#include <svx/xfillit0.hxx>
#endif
#ifndef _SVX_XLINEIT0_HXX //autogen
#include <svx/xlineit0.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_NUMITEM_HXX //autogen
#include <svx/numitem.hxx>
#endif
#define ITEMID_BRUSH    0
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _MyEDITENG_HXX //autogen
#include <svx/editeng.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif


#include <svx/svdattr.hxx>
#include "eetext.hxx"
#include <svx/xtable.hxx>           // fuer RGB_Color
#include <svx/bulitem.hxx>
#include <svx/lrspitem.hxx>
#include <svx/adjitem.hxx>
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif

#define _SDR_POSITIVE
#define _SDR_ITEMS

#include "stlpool.hxx"
#include "sdresid.hxx"
#include "stlsheet.hxx"
#include "glob.hrc"
#include "glob.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "helpids.h"


/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

SdStyleSheetPool::SdStyleSheetPool(SfxItemPool& rPool, SdDrawDocument* pDocument) :
    SfxStyleSheetPool(rPool),
    pActualStyleSheet(NULL),
    pDoc(pDocument)
{
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

__EXPORT SdStyleSheetPool::~SdStyleSheetPool()
{
}

/*************************************************************************
|*
|* Vorlage erzeugen
|*
\************************************************************************/

SfxStyleSheetBase* __EXPORT SdStyleSheetPool::Create(const String& rName,
                                            SfxStyleFamily eFamily,
                                            USHORT nMask )
{
    return new SdStyleSheet(rName, *this, eFamily, nMask);
}


/*************************************************************************
|*
|* Vorlage nach Vorbild erzeugen
|*
\************************************************************************/

SfxStyleSheetBase* __EXPORT SdStyleSheetPool::Create(const SdStyleSheet& rStyle)
{
    return new SdStyleSheet(rStyle);
}

/*************************************************************************
|*
|* eine Titelvorlage fuer ein Praesentationslayout suchen
|*
\************************************************************************/

SfxStyleSheetBase* SdStyleSheetPool::GetTitleSheet(const String& rLayoutName)
{
    String aName(rLayoutName);
    aName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
    aName += String(SdResId(STR_LAYOUT_TITLE));
    SfxStyleSheetBase* pResult = Find(aName, SD_LT_FAMILY);
    return pResult;
}

/*************************************************************************
|*
|* LayoutName des ersten Layouts ermitteln
|*
\************************************************************************/

String SdStyleSheetPool::GetLayoutName() const
{
    String aName( SdResId(STR_LAYOUT_DEFAULT_NAME ) );
    ULONG  nCount = aStyles.Count();

    for( ULONG n = 0; n < nCount; n++ )
    {
        aName = aStyles.GetObject( n )->GetName();
        USHORT nPos = aName.SearchAscii( SD_LT_SEPARATOR );
        if( nPos != STRING_NOTFOUND )
            break;
    }

    USHORT nPos = aName.Search( sal_Unicode( ' ' ));
    if( nPos != STRING_NOTFOUND )
        aName.Erase( nPos );       // removing blanks and number (e.g. "Gliederung 1")

    return aName;
}

/*************************************************************************
|*
|* eine Liste der Gliederungstextvorlagen fuer ein Praesentationlayout
|* erstellen, der Aufrufer muss die Liste wieder loeschen
|*
\************************************************************************/

List* SdStyleSheetPool::CreateOutlineSheetList (const String& rLayoutName)
{
    String aName(rLayoutName);
    aName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
    aName += String(SdResId(STR_LAYOUT_OUTLINE));
    List* pList = new List;
    for (USHORT nSheet = 1; nSheet < 10; nSheet++)
    {
        String aFullName(aName);
        aFullName.Append( sal_Unicode( ' ' ));
        aFullName.Append( String::CreateFromInt32( (sal_Int32)nSheet ));
        SfxStyleSheetBase* pSheet = Find(aFullName, SD_LT_FAMILY);
        pList->Insert(pSheet, LIST_APPEND);
    }
    return pList;
}

/*************************************************************************
|*
|* StyleSheets mit Defaultweren fuer das genannte Praesentationslayout erzeugen
|*
\************************************************************************/

void SdStyleSheetPool::CreateLayoutStyleSheets(const String& rLayoutName)
{
    SfxStyleSheetBase* pSheet = NULL;

    String aPrefix(rLayoutName);
    String aSep( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ));
    aPrefix.Insert(aSep);

    // Font fuer Titel und Gliederung
    SvxFontItem aSvxFontItem;
    aSvxFontItem.GetFamily() = FAMILY_ROMAN;
    aSvxFontItem.GetFamilyName() = System::GetStandardFont(STDFONT_ROMAN).GetName();
    aSvxFontItem.GetCharSet() = gsl_getSystemTextEncoding();

    Font aBulletFont( GetBulletFont() );

    /**************************************************************************
    * Gliederungsebenen
    **************************************************************************/
    String aName(SdResId(STR_LAYOUT_OUTLINE));
    String aHelpFile;

    SfxStyleSheetBase* pParent = NULL;
    SvxLRSpaceItem aSvxLRSpaceItem;
    SvxULSpaceItem aSvxULSpaceItem;

    for (USHORT nLevel = 1; nLevel < 10; nLevel++)
    {
        String aLevelName(aName);
        aLevelName.Append( sal_Unicode( ' ' ));
        aLevelName.Append( String::CreateFromInt32( sal_Int32( nLevel )));

        aLevelName.Insert(aPrefix, 0);

        if (!Find(aLevelName, SD_LT_FAMILY))
        {
            pSheet = &Make(aLevelName, SD_LT_FAMILY);
            pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_OUTLINE + nLevel );

            pSheet->SetParent( String() );

            // Attributierung fuer Level 1, die anderen Ebenen "erben"
            if (nLevel == 1)
            {
                SfxItemSet& rSet = pSheet->GetItemSet();
                rSet.Put(aSvxFontItem);
                rSet.Put(SvxPostureItem(ITALIC_NONE));
                rSet.Put(SvxWeightItem(WEIGHT_NORMAL));
                rSet.Put(SvxUnderlineItem(UNDERLINE_NONE));
                rSet.Put(SvxCrossedOutItem(STRIKEOUT_NONE));
                rSet.Put(SvxShadowedItem(FALSE));
                rSet.Put(SvxContourItem(FALSE));
                rSet.Put(SvxColorItem( Color(COL_BLACK)));
                rSet.Put(SvxAdjustItem(SVX_ADJUST_LEFT));
                rSet.Put(XLineStyleItem(XLINE_NONE));
                rSet.Put(XFillStyleItem(XFILL_NONE));

                if( nLevel == 1 )
                {
                    Font aBulletFont( GetBulletFont() );
                    PutNumBulletItem( pSheet, aBulletFont );
                }
            }

            ULONG nFontSize = 20;
            short nFirstIndent = -600;
            USHORT nIndent = nLevel * 1200;
            USHORT nLower = 100;

            switch (nLevel)
            {
                case 1:
                {
                    nFontSize = 32;
                    nLower = 500;
                    nFirstIndent = -900;
                }
                break;

                case 2:
                {
                    nFontSize = 28;
                    nLower = 400;
                    nFirstIndent = -800;
                }
                break;

                case 3:
                {
                    nFontSize = 24;
                    nLower = 300;
                }
                break;

                case 4:
                {
                    nLower = 200;
                }
                break;
            }

            // FontSize
            nFontSize = (USHORT)((nFontSize * 2540L) / 72);  // Pt --> 1/100 mm
            pSheet->GetItemSet().Put(SvxFontHeightItem(nFontSize));

            // Einzuege
            aSvxLRSpaceItem.SetTxtFirstLineOfst(nFirstIndent);
            aSvxLRSpaceItem.SetTxtLeft(nIndent);
            aSvxLRSpaceItem.SetRight(0);
            aSvxLRSpaceItem.SetBulletFI(TRUE);
            pSheet->GetItemSet().Put(aSvxLRSpaceItem);

            // Zeilendurchschuss (Abstand nach unten)
            aSvxULSpaceItem.SetLower(nLower);
            pSheet->GetItemSet().Put(aSvxULSpaceItem);

            if (nLevel == 1)
            {
                SfxUInt16Item aBulletStateItem(EE_PARA_BULLETSTATE, 1); // Bullets sichtbar
                pSheet->GetItemSet().Put(aBulletStateItem);
            }

            // Gliederungsvorlagen haben die naechsthoehere Ebene als Parent
            if (pParent)
                pSheet->SetParent(pParent->GetName());
            pParent = pSheet;
        }
    }

    /**************************************************************************
    * Titel
    **************************************************************************/
    aName = String(SdResId(STR_LAYOUT_TITLE));
    aName.Insert(aPrefix, 0);

    if (!Find(aName, SD_LT_FAMILY))
    {
        pSheet = &Make(aName, SD_LT_FAMILY);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_TITLE );
        pSheet->SetParent(String());
        SfxItemSet& rTitleSet = pSheet->GetItemSet();
        rTitleSet.Put(XLineStyleItem(XLINE_NONE));
        rTitleSet.Put(XFillStyleItem(XFILL_NONE));
        rTitleSet.Put(aSvxFontItem);
        rTitleSet.Put(SvxPostureItem(ITALIC_NONE));
        rTitleSet.Put(SvxWeightItem(WEIGHT_NORMAL));
        rTitleSet.Put(SvxFontHeightItem(1552));                 // 44 pt
        rTitleSet.Put(SvxUnderlineItem(UNDERLINE_NONE));
        rTitleSet.Put(SvxCrossedOutItem(STRIKEOUT_NONE));
        rTitleSet.Put(SvxShadowedItem(FALSE));
        rTitleSet.Put(SvxContourItem(FALSE));
        rTitleSet.Put(SvxColorItem( Color(COL_BLACK)));
        rTitleSet.Put(SvxAdjustItem(SVX_ADJUST_CENTER));
        rTitleSet.Put( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
        rTitleSet.Put( SfxUInt16Item(EE_PARA_BULLETSTATE, 0) );
        aBulletFont.SetSize(Size(0,1552));                  // 44 pt
        PutNumBulletItem( pSheet, aBulletFont );
    }

    /**************************************************************************
    * Untertitel
    * Aenderugen auch in AdjustLRSpaceItems() vornehmen!
    **************************************************************************/
    aName = String(SdResId(STR_LAYOUT_SUBTITLE));
    aName.Insert(aPrefix, 0);

    if (!Find(aName, SD_LT_FAMILY))
    {
        pSheet = &Make(aName, SD_LT_FAMILY);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_SUBTITLE );
        pSheet->SetParent(String());
        SfxItemSet& rSubtitleSet = pSheet->GetItemSet();
        rSubtitleSet.Put(XLineStyleItem(XLINE_NONE));
        rSubtitleSet.Put(XFillStyleItem(XFILL_NONE));
        rSubtitleSet.Put(aSvxFontItem);
        rSubtitleSet.Put(SvxPostureItem(ITALIC_NONE));
        rSubtitleSet.Put(SvxWeightItem(WEIGHT_NORMAL));
        rSubtitleSet.Put(SvxFontHeightItem(1129));                  // 32 pt
        rSubtitleSet.Put(SvxUnderlineItem(UNDERLINE_NONE));
        rSubtitleSet.Put(SvxCrossedOutItem(STRIKEOUT_NONE));
        rSubtitleSet.Put(SvxShadowedItem(FALSE));
        rSubtitleSet.Put(SvxContourItem(FALSE));
        rSubtitleSet.Put(SvxColorItem( Color(COL_BLACK)));
        rSubtitleSet.Put(SvxAdjustItem(SVX_ADJUST_CENTER));
        rSubtitleSet.Put( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
        rSubtitleSet.Put( SfxUInt16Item(EE_PARA_BULLETSTATE, 0) );
        aSvxLRSpaceItem.SetTxtLeft(0);
        rSubtitleSet.Put(aSvxLRSpaceItem);
        Font aBulletFont( GetBulletFont() );
        aBulletFont.SetSize(Size(0, 1129));     // 32 pt
        PutNumBulletItem( pSheet, aBulletFont );
    }

    /**************************************************************************
    * Notizen
    **************************************************************************/
    aName = String(SdResId(STR_LAYOUT_NOTES));
    aName.Insert(aPrefix, 0);

    if (!Find(aName, SD_LT_FAMILY))
    {
        pSheet = &Make(aName, SD_LT_FAMILY);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_NOTES );
        pSheet->SetParent(String());
        SfxItemSet& rNotesSet = pSheet->GetItemSet();
        rNotesSet.Put(XLineStyleItem(XLINE_NONE));
        rNotesSet.Put(XFillStyleItem(XFILL_NONE));
        rNotesSet.Put(aSvxFontItem);
        rNotesSet.Put(SvxPostureItem(ITALIC_NONE));
        rNotesSet.Put(SvxWeightItem(WEIGHT_NORMAL));
        rNotesSet.Put(SvxFontHeightItem(846));     // 24 pt
        rNotesSet.Put(SvxUnderlineItem(UNDERLINE_NONE));
        rNotesSet.Put(SvxCrossedOutItem(STRIKEOUT_NONE));
        rNotesSet.Put(SvxShadowedItem(FALSE));
        rNotesSet.Put(SvxContourItem(FALSE));
        rNotesSet.Put(SvxColorItem( Color(COL_BLACK)));
        rNotesSet.Put( SfxUInt16Item(EE_PARA_BULLETSTATE, 0) );
    }

    /**************************************************************************
    * Hintergrundobjekte
    **************************************************************************/
    aName = String(SdResId(STR_LAYOUT_BACKGROUNDOBJECTS));
    aName.Insert(aPrefix, 0);

    if (!Find(aName, SD_LT_FAMILY))
    {
        pSheet = &Make(aName, SD_LT_FAMILY);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_BACKGROUNDOBJECTS );
        pSheet->SetParent(String());
        SfxItemSet& rBackgroundObjectsSet = pSheet->GetItemSet();
        rBackgroundObjectsSet.Put(SdrShadowItem(FALSE));
        rBackgroundObjectsSet.Put(SdrShadowColorItem(String(), Color(COL_GRAY)));
        rBackgroundObjectsSet.Put(SdrShadowXDistItem(300)); // 3 mm Schattendistanz
        rBackgroundObjectsSet.Put(SdrShadowYDistItem(300));
    }

    /**************************************************************************
    * Hintergrund
    **************************************************************************/
    aName = String(SdResId(STR_LAYOUT_BACKGROUND));
    aName.Insert(aPrefix, 0);

    if (!Find(aName, SD_LT_FAMILY))
    {
        pSheet = &Make(aName, SD_LT_FAMILY);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_BACKGROUND );
        pSheet->SetParent(String());
        SfxItemSet& rBackgroundSet = pSheet->GetItemSet();
        rBackgroundSet.Put(XLineStyleItem(XLINE_NONE));
        rBackgroundSet.Put(XFillStyleItem(XFILL_NONE));
    }
}

/*************************************************************************
|*
|* StyleSheets des genannten Praesentationslayouts loeschen
|*
\************************************************************************/

void SdStyleSheetPool::EraseLayoutStyleSheets(const String& rLayoutName)
{
    SfxStyleSheetBase* pSheet = NULL;

    List* pNameList = CreateLayoutSheetNames(rLayoutName);

    String* pName = (String*)pNameList->First();
    while (pName)
    {
        pSheet = Find(*pName, SD_LT_FAMILY);
        DBG_ASSERT(pSheet, "EraseLayoutStyleSheets: Vorlage nicht gefunden");
        if (pSheet)
            Erase(pSheet);
        delete pName;
        pName = (String*)pNameList->Next();
    }
    delete pNameList;
}


/*************************************************************************
|*
|* Graphik-StyleSheets  aus dem Quellpool in diesen Pool kopieren
|*
|* (rSourcePool kann nicht const sein, weil SfxStyleSheetPoolBase::Find
|*  nicht const ist)
|*
\************************************************************************/

void SdStyleSheetPool::CopyGraphicSheets(SdStyleSheetPool& rSourcePool)
{
    ULONG nCount = rSourcePool.aStyles.Count();

    for (ULONG n = 0; n < nCount; n++)
    {
        SfxStyleSheet* pSheet = (SfxStyleSheet*) rSourcePool.aStyles.GetObject(n);

        if( pSheet->GetFamily() == SFX_STYLE_FAMILY_PARA )
        {
            String aName( pSheet->GetName() );
            if ( !Find( aName, SFX_STYLE_FAMILY_PARA ) )
            {
                SfxStyleSheetBase& rNewSheet = Make( aName, SFX_STYLE_FAMILY_PARA );
                rNewSheet.GetItemSet().Put( pSheet->GetItemSet() );
            }
        }
    }
}


/*************************************************************************
|*
|* StyleSheets des genannten Praesentationslayouts aus dem Quellpool in diesen
|* Pool kopieren. Kopiert werden nur solche StyleSheets, die in diesem Pool
|* noch nicht vorhanden sind.
|* pCreatedSheets wird - wenn ungleich NULL - mit Zeigern auf die erzeugten
|* StyleSheets gefuellt.
|*
|* (rSourcePool kann nicht const sein, weil SfxStyleSheetPoolBase::Find
|*  nicht const ist)
|*
\************************************************************************/

void SdStyleSheetPool::CopyLayoutSheets(const String& rLayoutName,
                                        SdStyleSheetPool& rSourcePool,
                                        List* pCreatedSheets)
{
    SfxStyleSheetBase* pSheet = NULL;

    String aOutlineTag(SdResId(STR_LAYOUT_OUTLINE));

    List* pNameList = CreateLayoutSheetNames(rLayoutName);

    String* pName = (String*)pNameList->First();
    while (pName)
    {
        pSheet = Find(*pName, SD_LT_FAMILY);
        if (!pSheet)
        {
            SfxStyleSheetBase* pSourceSheet =
                                rSourcePool.Find(*pName, SD_LT_FAMILY);
            DBG_ASSERT(pSourceSheet,
                       "CopyLayoutSheets: Quellvorlage nicht gefunden");
            if (pSourceSheet)   // falls einer mit Methusalem-Doks. ankommt
            {
                SfxStyleSheetBase& rNewSheet = Make(*pName, SD_LT_FAMILY);
                rNewSheet.GetItemSet().Put(pSourceSheet->GetItemSet());
                if (pCreatedSheets)
                    pCreatedSheets->Insert(&rNewSheet, LIST_APPEND);
            }
        }
        delete pName;
        pName = (String*)pNameList->Next();
    }
    delete pNameList;

    // Sonderbehandlung fuer Gliederungsvorlagen: Parentbeziehungen aufbauen
    List* pOutlineSheets = CreateOutlineSheetList(rLayoutName);
    SfxStyleSheetBase* pParent = (SfxStyleSheetBase*)pOutlineSheets->First();
    pSheet = (SfxStyleSheetBase*)pOutlineSheets->Next();
    while (pSheet)
    {
        // kein Parent?
        if (pSheet->GetParent().Len() == 0)
            pSheet->SetParent(pParent->GetName());
        pParent = pSheet;
        pSheet = (SfxStyleSheetBase*)pOutlineSheets->Next();
    }
    delete pOutlineSheets;
}

/*************************************************************************
|*
|* Liste mit den Namen der Praesentationsvorlagen eines Layouts erzeugen.
|* Die Liste und die enthaltenen Strings gehoeren dem Caller!
|*
\************************************************************************/

List* SdStyleSheetPool::CreateLayoutSheetNames(const String& rLayoutName) const
{
    String aPrefix(rLayoutName);
    String aSep( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ));
    aPrefix.Insert(aSep);

    List* pNameList = new List;

    String aName(SdResId(STR_LAYOUT_OUTLINE));
    String* pName = NULL;

    for (USHORT nLevel = 1; nLevel < 10; nLevel++)
    {
        pName = new String(aName);
        pName->Append( sal_Unicode( ' ' ));
        pName->Append( String::CreateFromInt32( sal_Int32( nLevel )));
        pName->Insert(aPrefix, 0);
        pNameList->Insert(pName, LIST_APPEND);
    }

    pName = new String(SdResId(STR_LAYOUT_TITLE));
    pName->Insert(aPrefix, 0);
    pNameList->Insert(pName, LIST_APPEND);

    pName = new String(SdResId(STR_LAYOUT_SUBTITLE));
    pName->Insert(aPrefix, 0);
    pNameList->Insert(pName, LIST_APPEND);

    pName = new String(SdResId(STR_LAYOUT_NOTES));
    pName->Insert(aPrefix, 0);
    pNameList->Insert(pName, LIST_APPEND);

    pName = new String(SdResId(STR_LAYOUT_BACKGROUNDOBJECTS));
    pName->Insert(aPrefix, 0);
    pNameList->Insert(pName, LIST_APPEND);

    pName = new String(SdResId(STR_LAYOUT_BACKGROUND));
    pName->Insert(aPrefix, 0);
    pNameList->Insert(pName, LIST_APPEND);

    return pNameList;
}

/*************************************************************************
|*
|* Liste mit Zeigern auf Praesentationsvorlagen eines Layouts erzeugen.
|* Die Liste gehoert dem Caller!
|*
\************************************************************************/

List* SdStyleSheetPool::CreateLayoutSheetList(const String& rLayoutName)
{
    List* pList = new List;             // die kriegt der Caller
    String aLayoutNameWithSep(rLayoutName);
    aLayoutNameWithSep.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
    USHORT nLen = aLayoutNameWithSep.Len();

    SfxStyleSheetIterator aIter(this, SD_LT_FAMILY);
    SfxStyleSheetBase* pSheet = aIter.First();

    while (pSheet)
    {
        if (pSheet->GetName().Match(aLayoutNameWithSep) == nLen)
            pList->Insert(pSheet, LIST_APPEND);
        pSheet = aIter.Next();
    }
    return pList;
}

/*************************************************************************
|*
|* ggfs. PseudoStyleSheets erzeugen
|*
\************************************************************************/

void SdStyleSheetPool::CreatePseudosIfNecessary()
{
    String aName;
    String aHelpFile;
    SfxStyleSheetBase* pSheet = NULL;
    SfxStyleSheetBase* pParent = NULL;

    //USHORT nMask = SFXSTYLEBIT_ALL & ~SFXSTYLEBIT_USERDEF;
    USHORT nMask = SFXSTYLEBIT_USED;

    aName = String(SdResId(STR_PSEUDOSHEET_TITLE));
    if (!(pSheet = Find(aName, SFX_STYLE_FAMILY_PSEUDO)))
    {
        pSheet = &Make(aName, SFX_STYLE_FAMILY_PSEUDO, nMask);
        pSheet->SetParent( String() );
        ((SfxStyleSheet*)pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_TITLE );

    aName = String(SdResId(STR_PSEUDOSHEET_SUBTITLE));
    if (!(pSheet = Find(aName, SFX_STYLE_FAMILY_PSEUDO)))
    {
        pSheet = &Make(aName, SFX_STYLE_FAMILY_PSEUDO, nMask);
        pSheet->SetParent(String());
        ((SfxStyleSheet*)pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_SUBTITLE );

    aName = String(SdResId(STR_PSEUDOSHEET_BACKGROUNDOBJECTS));
    if (!(pSheet = Find(aName, SFX_STYLE_FAMILY_PSEUDO)))
    {
        pSheet = &Make(aName, SFX_STYLE_FAMILY_PSEUDO, nMask);
        pSheet->SetParent( String() );
        ((SfxStyleSheet*)pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_BACKGROUNDOBJECTS );

    aName = String(SdResId(STR_PSEUDOSHEET_BACKGROUND));
    if (!(pSheet = Find(aName, SFX_STYLE_FAMILY_PSEUDO)))
    {
        pSheet = &Make(aName, SFX_STYLE_FAMILY_PSEUDO, nMask);
        pSheet->SetParent( String() );
        ((SfxStyleSheet*)pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_BACKGROUND );

    aName = String(SdResId(STR_PSEUDOSHEET_NOTES));
    if (!(pSheet = Find(aName, SFX_STYLE_FAMILY_PSEUDO)))
    {
        pSheet = &Make(aName, SFX_STYLE_FAMILY_PSEUDO, nMask);
        pSheet->SetParent( String() );
        ((SfxStyleSheet*)pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_NOTES );

    pParent = NULL;
    SetSearchMask(SFX_STYLE_FAMILY_PSEUDO);
    aName = String(SdResId(STR_PSEUDOSHEET_OUTLINE));
    for (USHORT nLevel = 1; nLevel < 10; nLevel++)
    {
        String aLevelName(aName);
        aLevelName.Append( sal_Unicode( ' ' ));
        aLevelName.Append( String::CreateFromInt32( sal_Int32( nLevel )));

        if (!(pSheet = Find(aLevelName, SFX_STYLE_FAMILY_PSEUDO)))
        {
            pSheet = &Make(aLevelName, SFX_STYLE_FAMILY_PSEUDO, nMask);

            if (pSheet)
            {
                if (pParent)
                    pSheet->SetParent(pParent->GetName());
                pParent = pSheet;
                ((SfxStyleSheet*)pSheet)->StartListening(*this);
            }
        }
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_OUTLINE + nLevel );
    }
}


/*************************************************************************
|*
|* Standard-Styles den richtigen Namen in der Programm-Sprache geben
|*
\************************************************************************/

void SdStyleSheetPool::UpdateStdNames()
{
    BOOL bNewHelpIds = FALSE;
    String aHelpFile;
    ULONG  nCount = aStyles.Count();
    List* pEraseList = NULL;

    for( ULONG n=0; n < nCount; n++ )
    {
        SfxStyleSheetBase* pStyle = aStyles.GetObject( n );

        if( !pStyle->IsUserDefined() )
        {
            String aOldName     = pStyle->GetName();
            ULONG nHelpId       = pStyle->GetHelpId( aHelpFile );
            SfxStyleFamily eFam = pStyle->GetFamily();

            BOOL bHelpKnown = TRUE;
            String aNewName;
            USHORT nNameId = 0;
            switch( nHelpId )
            {
                case HID_STANDARD_STYLESHEET_NAME:  nNameId = STR_STANDARD_STYLESHEET_NAME; break;
                case HID_POOLSHEET_OBJWITHARROW:    nNameId = STR_POOLSHEET_OBJWITHARROW;   break;
                case HID_POOLSHEET_OBJWITHSHADOW:   nNameId = STR_POOLSHEET_OBJWITHSHADOW;  break;
                case HID_POOLSHEET_OBJWITHOUTFILL:  nNameId = STR_POOLSHEET_OBJWITHOUTFILL; break;
                case HID_POOLSHEET_TEXT:            nNameId = STR_POOLSHEET_TEXT;           break;
                case HID_POOLSHEET_TEXTBODY:        nNameId = STR_POOLSHEET_TEXTBODY;       break;
                case HID_POOLSHEET_TEXTBODY_JUSTIFY:nNameId = STR_POOLSHEET_TEXTBODY_JUSTIFY;break;
                case HID_POOLSHEET_TEXTBODY_INDENT: nNameId = STR_POOLSHEET_TEXTBODY_INDENT;break;
                case HID_POOLSHEET_TITLE:           nNameId = STR_POOLSHEET_TITLE;          break;
                case HID_POOLSHEET_TITLE1:          nNameId = STR_POOLSHEET_TITLE1;         break;
                case HID_POOLSHEET_TITLE2:          nNameId = STR_POOLSHEET_TITLE2;         break;
                case HID_POOLSHEET_HEADLINE:        nNameId = STR_POOLSHEET_HEADLINE;       break;
                case HID_POOLSHEET_HEADLINE1:       nNameId = STR_POOLSHEET_HEADLINE1;      break;
                case HID_POOLSHEET_HEADLINE2:       nNameId = STR_POOLSHEET_HEADLINE2;      break;
                case HID_POOLSHEET_MEASURE:         nNameId = STR_POOLSHEET_MEASURE;        break;

                case HID_PSEUDOSHEET_TITLE:         nNameId = STR_PSEUDOSHEET_TITLE;        break;
                case HID_PSEUDOSHEET_SUBTITLE:      nNameId = STR_PSEUDOSHEET_SUBTITLE;     break;
                case HID_PSEUDOSHEET_OUTLINE1:
                case HID_PSEUDOSHEET_OUTLINE2:
                case HID_PSEUDOSHEET_OUTLINE3:
                case HID_PSEUDOSHEET_OUTLINE4:
                case HID_PSEUDOSHEET_OUTLINE5:
                case HID_PSEUDOSHEET_OUTLINE6:
                case HID_PSEUDOSHEET_OUTLINE7:
                case HID_PSEUDOSHEET_OUTLINE8:
                case HID_PSEUDOSHEET_OUTLINE9:      nNameId = STR_PSEUDOSHEET_OUTLINE;      break;
                case HID_PSEUDOSHEET_BACKGROUNDOBJECTS: nNameId = STR_PSEUDOSHEET_BACKGROUNDOBJECTS; break;
                case HID_PSEUDOSHEET_BACKGROUND:    nNameId = STR_PSEUDOSHEET_BACKGROUND;   break;
                case HID_PSEUDOSHEET_NOTES:         nNameId = STR_PSEUDOSHEET_NOTES;        break;

                default:
                    // 0 oder falsche (alte) HelpId
                    bHelpKnown = FALSE;
            }
            if( bHelpKnown )
            {
                if( nNameId )
                {
                    aNewName = String( SdResId( nNameId ) );
                    if( nNameId == STR_PSEUDOSHEET_OUTLINE )
                    {
                        aNewName.Append( sal_Unicode( ' ' ));
                        aNewName.Append( String::CreateFromInt32( sal_Int32( nHelpId - HID_PSEUDOSHEET_OUTLINE )));
                    }
                }

                if( aNewName.Len() && aNewName != aOldName )
                {
                    SfxStyleSheetBase* pSheetFound = Find( aNewName, eFam );

                    if ( !pSheetFound )
                    {
                        // Sheet existiert noch nicht: Altes Sheet wird umbenannt
                        pStyle->SetName( aNewName );    // setzt auch Parents um
                    }
                    else
                    {
                        // Sheet existiert schon: Altes Sheet muss entfernt werden
                        if( !pEraseList )
                        {
                            pEraseList = new List();
                        }

                        pEraseList->Insert( pStyle );
                    }
                }
            }
            else
            {
                //  falsche oder keine HelpId -> neue generieren

                DBG_WARNING("Default-Vorlage mit falscher oder keiner HelpId");

                // Da bisher nur deutsche Dokumente erzeugt worden sein koennen,
                // kann hier auch der deutsche Text abgefragt werden, damit man
                // nicht in einer anderssprachigen Version auf die deutsche
                // Version zurueckgreifen muss!

                USHORT nNewId = 0;
                if( eFam == SFX_STYLE_FAMILY_PARA )
                {
                    if( aOldName == String( SdResId( STR_STANDARD_STYLESHEET_NAME ) ) ||
                        aOldName.EqualsAscii( "Standard", 0, RTL_CONSTASCII_LENGTH( "Standard" )) )
                        nNewId = HID_STANDARD_STYLESHEET_NAME;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_OBJWITHARROW ) ) ||
                        aOldName.EqualsAscii( "Objekt mit Pfeilspitze", 0, RTL_CONSTASCII_LENGTH( "Objekt mit Pfeilspitze" )) )
                        nNewId = HID_POOLSHEET_OBJWITHARROW;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_OBJWITHSHADOW ) ) ||
                        aOldName.EqualsAscii( "Objekt mit Schatten", 0, RTL_CONSTASCII_LENGTH( "Objekt mit Schatten" )) )
                        nNewId = HID_POOLSHEET_OBJWITHSHADOW;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_OBJWITHOUTFILL ) ) ||
                        aOldName == String( SdResId( STR_POOLSHEET_OBJWITHOUTFILL_NT ) ) )
                        //aOldName.EqualsAscii( "Objekt ohne Füllung", 0, RTL_CONSTASCII_LENGTH( "Objekt ohne Füllung" )) )
                        nNewId = HID_POOLSHEET_OBJWITHOUTFILL;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_TEXT ) ) ||
                        aOldName.EqualsAscii( "Text", 0, RTL_CONSTASCII_LENGTH( "Text" )) )
                        nNewId = HID_POOLSHEET_TEXT;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_TEXTBODY ) ) ||
                        aOldName == String( SdResId( STR_POOLSHEET_TEXTBODY_NT ) ) )
                        //aOldName.EqualsAscii( "Textkörper", 0, RTL_CONSTASCII_LENGTH( "Textkörper" )) )
                        nNewId = HID_POOLSHEET_TEXTBODY;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_TEXTBODY_JUSTIFY ) ) ||
                        aOldName == String( SdResId( STR_POOLSHEET_TEXTBODY_JUSTIFY_NT ) ) )
                        //aOldName.EqualsAscii( "Textkörper Blocksatz", 0, RTL_CONSTASCII_LENGTH( "Textkörper Blocksatz" )) )
                        nNewId = HID_POOLSHEET_TEXTBODY_JUSTIFY;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_TEXTBODY_INDENT ) ) ||
                        aOldName == String( SdResId( STR_POOLSHEET_TEXTBODY_INDENT_NT ) ) )
                        //aOldName.EqualsAscii( "Textkörper Einzug", 0, RTL_CONSTASCII_LENGTH( "Textkörper Einzug" )) )
                        nNewId = HID_POOLSHEET_TEXTBODY_INDENT;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_TITLE ) ) ||
                        aOldName.EqualsAscii( "Titel", 0, RTL_CONSTASCII_LENGTH( "Titel" )) )
                        nNewId = HID_POOLSHEET_TITLE;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_TITLE1 ) ) ||
                        aOldName.EqualsAscii( "Titel1", 0, RTL_CONSTASCII_LENGTH( "Titel1" )) )
                        nNewId = HID_POOLSHEET_TITLE1;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_TITLE2 ) ) ||
                        aOldName.EqualsAscii( "Titel2", 0, RTL_CONSTASCII_LENGTH( "Titel2" )) )
                        nNewId = HID_POOLSHEET_TITLE2;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_HEADLINE ) ) ||
                        aOldName == String( SdResId( STR_POOLSHEET_HEADLINE_NT ) ) )
                        //aOldName.EqualsAscii( "Überschrift", 0, RTL_CONSTASCII_LENGTH( "Überschrift" )) )
                        nNewId = HID_POOLSHEET_HEADLINE;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_HEADLINE1 ) ) ||
                        aOldName == String( SdResId( STR_POOLSHEET_HEADLINE1_NT ) ) )
                        //aOldName.EqualsAscii( "Überschrift1", 0, RTL_CONSTASCII_LENGTH( "Überschrift1" )) )
                        nNewId = HID_POOLSHEET_HEADLINE1;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_HEADLINE2 ) ) ||
                        aOldName == String( SdResId( STR_POOLSHEET_HEADLINE2_NT ) ) )
                        //aOldName.EqualsAscii( "Überschrift2", 0, RTL_CONSTASCII_LENGTH( "Überschrift2" )) )
                        nNewId = HID_POOLSHEET_HEADLINE2;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_MEASURE ) ) ||
                        aOldName == String( SdResId( STR_POOLSHEET_MEASURE_NT ) ) )
                        //aOldName.EqualsAscii( "Maßlinie", 0, RTL_CONSTASCII_LENGTH( "Maßlinie" )) )
                        nNewId = HID_POOLSHEET_MEASURE;
                }
                else if( eFam == SFX_STYLE_FAMILY_PSEUDO )
                {
                    String aStr( SdResId( STR_PSEUDOSHEET_OUTLINE ) );
                    aStr.Append( sal_Unicode( ' ' ));
                    String aStr2( RTL_CONSTASCII_USTRINGPARAM( "Gliederung " ));
                    sal_Int32 nNumber = 0;

                    if( aOldName == String( SdResId( STR_PSEUDOSHEET_TITLE ) ) ||
                        aOldName.EqualsAscii( "Titel", 0, RTL_CONSTASCII_LENGTH( "Titel" )))
                        nNewId = HID_PSEUDOSHEET_TITLE;
                    else if( aOldName == String( SdResId( STR_PSEUDOSHEET_SUBTITLE ) ) ||
                        aOldName.EqualsAscii( "Untertitel", 0, RTL_CONSTASCII_LENGTH( "Untertitel" )) )
                        nNewId = HID_PSEUDOSHEET_SUBTITLE;
                    else if( aOldName.Len() == aStr.Len() + 1 &&
                             aOldName.Match( aStr ) == STRING_MATCH )
                        nNumber = String( aStr, aStr.Len(), 1 ).ToInt32();
                    else if( aOldName.Len() == aStr2.Len() + 1 &&
                             aOldName.Match( aStr2 ) == STRING_MATCH )
                        nNumber = String( aStr2, aStr2.Len(), 1 ).ToInt32();
                    else if( aOldName == String( SdResId( STR_PSEUDOSHEET_BACKGROUNDOBJECTS ) ) ||
                        aOldName.EqualsAscii( "Hintergrundobjekte", 0, RTL_CONSTASCII_LENGTH( "Hintergrundobjekte" )) )
                        nNewId = HID_PSEUDOSHEET_BACKGROUNDOBJECTS;
                    else if( aOldName == String( SdResId( STR_PSEUDOSHEET_BACKGROUND ) ) ||
                        aOldName.EqualsAscii( "Hintergrund", 0, RTL_CONSTASCII_LENGTH( "Hintergrund" )) )
                        nNewId = HID_PSEUDOSHEET_BACKGROUND;
                    else if( aOldName == String( SdResId( STR_PSEUDOSHEET_NOTES ) ) ||
                        aOldName.EqualsAscii( "Notizen", 0, RTL_CONSTASCII_LENGTH( "Notizen" )) )
                        nNewId = HID_PSEUDOSHEET_NOTES;

                    if( nNumber )
                    {
                        switch( nNumber )
                        {
                            case 1:
                                nNewId = HID_PSEUDOSHEET_OUTLINE1;
                                break;
                            case 2:
                                nNewId = HID_PSEUDOSHEET_OUTLINE2;
                                break;
                            case 3:
                                nNewId = HID_PSEUDOSHEET_OUTLINE3;
                                break;
                            case 4:
                                nNewId = HID_PSEUDOSHEET_OUTLINE4;
                                break;
                            case 5:
                                nNewId = HID_PSEUDOSHEET_OUTLINE5;
                                break;
                            case 6:
                                nNewId = HID_PSEUDOSHEET_OUTLINE6;
                                break;
                            case 7:
                                nNewId = HID_PSEUDOSHEET_OUTLINE7;
                                break;
                            case 8:
                                nNewId = HID_PSEUDOSHEET_OUTLINE8;
                                break;
                            case 9:
                                nNewId = HID_PSEUDOSHEET_OUTLINE9;
                                break;
                        }
                    }
                }

                if ( nNewId )
                {
                    bNewHelpIds = TRUE;
                    pStyle->SetHelpId( aHelpFile, nNewId );
                }
            }
        }
    }

    if ( pEraseList )
    {
        // Styles, welche nicht umbenannt werden konnten, muessen entfernt werden
        for ( ULONG i = 0; i < pEraseList->Count(); i++ )
        {
            SfxStyleSheetBase* pEraseSheet = ( SfxStyleSheetBase* ) pEraseList->GetObject( i );
            Erase( pEraseSheet );
        }

        delete pEraseList;
        pEraseList = NULL;
    }

    // Wenn neue HelpIDs gefunden wurden koennen die Namen auch gleich ersetzt werden
    if( bNewHelpIds )
        UpdateStdNames();
}


/*************************************************************************
|*
|* Da sich die Bedeutung der SvxLRSpaceItems ab nFileFormatVersion = 16
|* geaendert hat (Bullet-Handling des Outliners, siehe auch
|* SdDrawDocument::NewOrLoadCompleted()), muss fuer aeltere Versionen
|* eine Korrektur vorgenommen werden.
|*
\************************************************************************/

void SdStyleSheetPool::AdjustLRSpaceItems()
{
    String aHelpFile;
    ULONG nCount = aStyles.Count();

    // #63254# Aenderungen nicht mehr broadcasten,statt dessen nach
    // AdjustLRSpaceItems() ein UpdateStyleSheets am Outliner

    if ( nCount > 0 )
    {
        /**************************************************************************
        * All LayoutStyleSheets loaded? (e.g. new Subtitle LayoutStyleSheet)
        **************************************************************************/
        USHORT nMasterPageCount = pDoc->GetMasterSdPageCount(PK_STANDARD);

        for ( USHORT i = 0; i < nMasterPageCount; i++ )
        {
            String aName(pDoc->GetMasterSdPage(i, PK_STANDARD)->GetLayoutName());
            aName.Erase( aName.SearchAscii( SD_LT_SEPARATOR ));
            CreateLayoutStyleSheets(aName);
        }
    }

    String aOutlineName( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ));
    aOutlineName += String(SdResId(STR_PSEUDOSHEET_OUTLINE));
    String aTitleName( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ));
    aTitleName += String(SdResId(STR_PSEUDOSHEET_TITLE));
    String aSubtitleName( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ));
    aSubtitleName += String(SdResId(STR_PSEUDOSHEET_SUBTITLE));
    String aStandardName(SdResId(STR_STANDARD_STYLESHEET_NAME));
    SfxStyleSheet* pSheetOutlineLevelOne = NULL;

    for (ULONG n = 0; n < nCount; n++)
    {
        SfxStyleSheet* pSheet = (SfxStyleSheet*)aStyles.GetObject(n);
        SfxItemSet& rSet = pSheet->GetItemSet();
        String aName( pSheet->GetName() );
        String aHelpFile;
        ULONG nHelpId = pSheet->GetHelpId( aHelpFile );

        if( nHelpId == 0)
        {
            if( aName.Len() == aOutlineName.Len() + 1 &&
                aName.Match( aOutlineName ) == STRING_MATCH )
            {
                sal_Int32 nNumber = String( aName, aName.Len(), 1 ).ToInt32();
                if( 1 <= nNumber && nNumber <= 9 )
                    nHelpId = HID_PSEUDOSHEET_OUTLINE + nNumber;
            }
        }

        if ( nHelpId &&
             ( HID_PSEUDOSHEET_OUTLINE + 1 <= nHelpId &&
               nHelpId <= HID_PSEUDOSHEET_OUTLINE + 9 ))
        {
            // Gliederungsvorlage
            // Bei Gliederungsobjektvorlagen Korrektur vornehmen
            // Es handelt sich hier um die RealStyleSheets und die Pseudos!
            // (siehe SdStyleSheet::GetRealStyleSheet())
            if ( nHelpId == HID_PSEUDOSHEET_OUTLINE + 1 )
            {
                if (rSet.GetItemState(EE_PARA_BULLETSTATE) != SFX_ITEM_ON)
                {
                    SfxUInt16Item aBulletStateItem(EE_PARA_BULLETSTATE, 1); // Bullets sichtbar
                    rSet.Put(aBulletStateItem);
                }
            }

            // Gliederungs-Vorlage
            const SvxBulletItem* pOldBullet = NULL;
            if ( rSet.GetItemState( EE_PARA_BULLET ) == SFX_ITEM_ON )
                pOldBullet = &(const SvxBulletItem&) rSet.Get(EE_PARA_BULLET);

            const SvxLRSpaceItem* pOldLRSpace = NULL;
            if ( rSet.GetItemState( EE_PARA_LRSPACE ) == SFX_ITEM_ON )
                pOldLRSpace = &(const SvxLRSpaceItem&) rSet.Get(EE_PARA_LRSPACE);

            USHORT nLevel = nHelpId - HID_PSEUDOSHEET_OUTLINE;

            if ( nHelpId == HID_PSEUDOSHEET_OUTLINE + 1 )
            {
                pSheetOutlineLevelOne = pSheet;
                SvxNumBulletItem aDefaultNumBullet( *(SvxNumBulletItem*) rSet.GetPool()->GetSecondaryPool()->GetPoolDefaultItem(EE_PARA_NUMBULLET) );
                aDefaultNumBullet.GetNumRule()->SetNumRuleType( SVX_RULETYPE_PRESENTATION_NUMBERING );
                EditEngine::ImportBulletItem( aDefaultNumBullet, nLevel, pOldBullet, pOldLRSpace );
                rSet.Put( aDefaultNumBullet );

                if (rSet.GetItemState( XATTR_FILLSTYLE ) != SFX_ITEM_ON )
                {
                    // Flaechenattribute sind nun unsichtbar
                    XFillStyleItem aFillStyleItem(XFILL_NONE);
                    rSet.Put(aFillStyleItem);
                }

                if (rSet.GetItemState( XATTR_LINESTYLE ) != SFX_ITEM_ON )
                {
                    // Linienattribute sind nun unsichtbar
                    XLineStyleItem aLineStyleItem(XLINE_NONE);
                    rSet.Put(aLineStyleItem);
                }
            }
            else
            {
                SfxItemSet& rSetOutlineLevelOne = pSheetOutlineLevelOne->GetItemSet();
                SvxNumBulletItem aNumBullet( (const SvxNumBulletItem&) rSetOutlineLevelOne.Get(EE_PARA_NUMBULLET) );
                EditEngine::ImportBulletItem( aNumBullet, nLevel, pOldBullet, pOldLRSpace );
                rSetOutlineLevelOne.Put( aNumBullet );

                // SvxNumBulletItems darf es nur in der ersten Gliederungsebene geben!
                if ( rSet.GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON )
                    rSet.ClearItem( EE_PARA_NUMBULLET );
            }

            if (rSet.GetItemState(EE_PARA_LRSPACE) == SFX_ITEM_ON)
            {
                const SvxLRSpaceItem& rLRItem = (const SvxLRSpaceItem&) rSet.Get(EE_PARA_LRSPACE);

                if (rLRItem.GetTxtFirstLineOfst() == 0)
                {
                    SvxLRSpaceItem aNewLRItem(rLRItem);
                    const SvxBulletItem& rBulletItem = (const SvxBulletItem&) rSet.Get(EE_PARA_BULLET);
                    short nFirstLineOfst = (short) -rBulletItem.GetWidth();
                    aNewLRItem.SetTxtFirstLineOfst(nFirstLineOfst);
                    rSet.Put(aNewLRItem);
                }
            }
        }
        else if ( nHelpId == HID_PSEUDOSHEET_TITLE               ||
                  nHelpId == HID_PSEUDOSHEET_SUBTITLE            ||
                  aName.Search(aTitleName)    != STRING_NOTFOUND ||
                  aName.Search(aSubtitleName) != STRING_NOTFOUND )
        {
            // Titel- oder Untertitel-Vorlage
            SfxItemSet& rSet = pSheet->GetItemSet();

            if (rSet.GetItemState(EE_PARA_BULLETSTATE) != SFX_ITEM_ON ||
                ((const SfxUInt16Item&) rSet.Get(EE_PARA_BULLETSTATE)).GetValue() == 1)
            {
                SfxUInt16Item aBulletStateItem(EE_PARA_BULLETSTATE, 0); // Bullets nicht sichtbar
                rSet.Put(aBulletStateItem);
            }

            if( nHelpId == HID_PSEUDOSHEET_TITLE ||
                aName.Search(aTitleName) != STRING_NOTFOUND )
            {
                Font aBulletFont( GetBulletFont() );
                aBulletFont.SetSize(Size(0,1552));                  // 44 pt
                PutNumBulletItem( pSheet, aBulletFont );
            }
            else if( nHelpId == HID_PSEUDOSHEET_SUBTITLE ||
                     aName.Search(aSubtitleName) != STRING_NOTFOUND )
            {
                Font aBulletFont( GetBulletFont() );
                aBulletFont.SetSize(Size(0, 1129));     // 32 pt
                PutNumBulletItem( pSheet, aBulletFont );
            }
        }
        else if ( aName == aStandardName )
        {
            // Standardvorlage
            Font aBulletFont( GetBulletFont() );
            aBulletFont.SetSize(Size(0,846));       // 24 pt
            PutNumBulletItem( pSheet, aBulletFont );
        }
        else if ( rSet.GetItemState( EE_PARA_LRSPACE ) == SFX_ITEM_ON )
        {
            // SvxLRSpaceItem hart gesetzt: NumBulletItem anpassen
            SvxNumBulletItem aNumBullet( (const SvxNumBulletItem&) rSet.Get(EE_PARA_NUMBULLET) );

            EditEngine::ImportBulletItem( aNumBullet, 0, NULL, &(const SvxLRSpaceItem&) rSet.Get( EE_PARA_LRSPACE ) );
            rSet.Put( aNumBullet );
        }
    }
}


/*************************************************************************
|*
|* Neues SvxNumBulletItem fuer das jeweilige StyleSheet setzen
|*
\************************************************************************/

void SdStyleSheetPool::PutNumBulletItem( SfxStyleSheetBase* pSheet,
                                         Font& rBulletFont ) const
{
    String aHelpFile;
    ULONG nHelpId = pSheet->GetHelpId( aHelpFile );
    SfxItemSet& rSet = pSheet->GetItemSet();

    switch ( nHelpId )
    {
        case HID_STANDARD_STYLESHEET_NAME :
        {
            // Standard-Vorlage
            SvxNumberFormat aNumberFormat(SVX_NUM_CHAR_SPECIAL);
            aNumberFormat.SetBulletFont(&rBulletFont);
            aNumberFormat.SetBulletChar( 0xF000 + 34 );
            aNumberFormat.SetBulletRelSize(45);
            aNumberFormat.SetBulletColor(Color(COL_BLACK));
            aNumberFormat.SetStart(1);
            aNumberFormat.SetNumAdjust(SVX_ADJUST_LEFT);

            SvxNumRule aNumRule( NUM_BULLET_REL_SIZE|NUM_BULLET_COLOR|NUM_CHAR_TEXT_DISTANCE, 10 , FALSE);
            aNumberFormat.SetLSpace( 0 );
            aNumberFormat.SetAbsLSpace( 0 );
            aNumberFormat.SetFirstLineOffset( 0 );
            aNumRule.SetLevel( 0, aNumberFormat );

            for( USHORT i = 1; i < 10; i++ )
            {
                const short nLSpace = (i + 1) * 600;
                aNumberFormat.SetLSpace(nLSpace);
                aNumberFormat.SetAbsLSpace(nLSpace);
                aNumberFormat.SetFirstLineOffset(-600);
                aNumRule.SetLevel( i, aNumberFormat );
            }

            rSet.Put( SvxNumBulletItem( aNumRule, EE_PARA_NUMBULLET ) );
            ((SfxStyleSheet*)pSheet)->Broadcast(SfxSimpleHint( SFX_HINT_DATACHANGED ) );

        }
        break;

        case HID_PSEUDOSHEET_TITLE:
        {
            // Titel-Vorlage

            SfxItemPool* pIP = rSet.GetPool()->GetSecondaryPool();
            SvxNumBulletItem* pI = (SvxNumBulletItem*) pIP->GetPoolDefaultItem(EE_PARA_NUMBULLET);

            SvxNumRule aNumRule(*((SvxNumBulletItem*) rSet.GetPool()->GetSecondaryPool()->GetPoolDefaultItem(EE_PARA_NUMBULLET))->GetNumRule());
            for(USHORT i=0; i < aNumRule.GetLevelCount(); i++)
            {
                SvxNumberFormat aFrmt( aNumRule.GetLevel(i));
                aFrmt.SetBulletRelSize(45);
                aFrmt.SetBulletChar( 0xF000 + 114 );
                aFrmt.SetBulletFont(&rBulletFont);
                aFrmt.SetLSpace( i * 1000 );
                aFrmt.SetAbsLSpace( i * 1000 );
                aFrmt.SetFirstLineOffset(0);

                aNumRule.SetLevel(i, aFrmt);
            }

            rSet.Put( SvxNumBulletItem( aNumRule, EE_PARA_NUMBULLET ) );
            ((SfxStyleSheet*)pSheet)->Broadcast(SfxSimpleHint( SFX_HINT_DATACHANGED ) );
        }
        break;

        case HID_PSEUDOSHEET_SUBTITLE :
        {
            // Untertitel-Vorlage
            SvxNumRule* pDefaultRule = ((SvxNumBulletItem*) rSet.GetPool()->GetSecondaryPool()->GetPoolDefaultItem(EE_PARA_NUMBULLET))->GetNumRule();
            DBG_ASSERT( pDefaultRule, "Wo ist mein Default? [CL]" );

            if(pDefaultRule)
            {
                SvxNumRule aNumRule(pDefaultRule->GetFeatureFlags(), 9, FALSE);
                for(USHORT i=0; i < aNumRule.GetLevelCount(); i++)
                {
                    SvxNumberFormat aFrmt( pDefaultRule->GetLevel(i) );
                    aFrmt.SetNumType(SVX_NUM_CHAR_SPECIAL);
                    aFrmt.SetStart(1);
                    aFrmt.SetBulletRelSize(45);
                    aFrmt.SetBulletChar( 0xF000 + 34 );
                    aFrmt.SetBulletFont(&rBulletFont);
                    aNumRule.SetLevel(i, aFrmt);
                }

                rSet.Put( SvxNumBulletItem( aNumRule, EE_PARA_NUMBULLET ) );
                ((SfxStyleSheet*)pSheet)->Broadcast(SfxSimpleHint( SFX_HINT_DATACHANGED ) );
            }
        }
        break;

        case HID_PSEUDOSHEET_OUTLINE + 1 :
        {
            // Gliederungs-Vorlage
            SvxNumberFormat aNumberFormat(SVX_NUM_CHAR_SPECIAL);
            aNumberFormat.SetBulletColor(Color(COL_BLACK));
            aNumberFormat.SetStart(1);
            aNumberFormat.SetNumAdjust(SVX_ADJUST_LEFT);

            SvxNumRule aNumRule( NUM_BULLET_REL_SIZE|NUM_BULLET_COLOR|NUM_CHAR_TEXT_DISTANCE|NUM_SYMBOL_ALIGNMENT,
                                 10, FALSE, SVX_RULETYPE_PRESENTATION_NUMBERING );
            for( UINT16 i = 0; i < 10; i++ )
            {
                aNumberFormat.SetBulletChar( 0xF000 + 34 );
                aNumberFormat.SetBulletRelSize(45);
                const short nLSpace = i ? i * 1200 : 1200;
                aNumberFormat.SetLSpace(nLSpace);
                aNumberFormat.SetAbsLSpace(nLSpace);
                USHORT nFirstLineOffset = -600;

                ULONG nFontSize = 20;
                switch(i)
                {
                    case 0:
                    case 1:
                    {
                        nFontSize = 32;
                        nFirstLineOffset = -900;
                    }
                    break;

                    case 2:
                    {
                        nFontSize = 28;
                        nFirstLineOffset = -800;
                        aNumberFormat.SetBulletChar( 0xF000 + 150 );
                        aNumberFormat.SetBulletRelSize(75);
                    }
                    break;

                    case 3:
                    {
                        nFontSize = 24;
                    }
                    break;

                    case 4:
                    {
                        aNumberFormat.SetBulletChar( 0xF000 + 150 );
                        aNumberFormat.SetBulletRelSize(75);
                    }
                    break;
                }

                aNumberFormat.SetFirstLineOffset(nFirstLineOffset);
                nFontSize = (USHORT)((nFontSize * 2540L) / 72);  // Pt --> 1/100 mm
                rBulletFont.SetSize(Size(0,846));       // 24 pt
                aNumberFormat.SetBulletFont(&rBulletFont);
                aNumRule.SetLevel( i, aNumberFormat );
            }

            rSet.Put( SvxNumBulletItem( aNumRule, EE_PARA_NUMBULLET ) );
            ((SfxStyleSheet*)pSheet)->Broadcast(SfxSimpleHint( SFX_HINT_DATACHANGED ) );
        }
        break;
    }
}

/*************************************************************************
|*
|* Standard Bullet-Font erzeugen (ohne Groesse)
|*
\************************************************************************/

Font SdStyleSheetPool::GetBulletFont() const
{
    Font aBulletFont( String( RTL_CONSTASCII_USTRINGPARAM( "StarBats" )), Size(0, 1000) );
    aBulletFont.SetCharSet(RTL_TEXTENCODING_SYMBOL);
    aBulletFont.SetWeight(WEIGHT_NORMAL);
    aBulletFont.SetUnderline(UNDERLINE_NONE);
    aBulletFont.SetStrikeout(STRIKEOUT_NONE);
    aBulletFont.SetItalic(ITALIC_NONE);
    aBulletFont.SetOutline(FALSE);
    aBulletFont.SetShadow(FALSE);
    aBulletFont.SetColor(Color(COL_BLACK));
    aBulletFont.SetTransparent(TRUE);

    return aBulletFont;
}





