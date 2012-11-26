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
#include "precompiled_sc.hxx"



//------------------------------------------------------------------------
#include "document.hxx"
#include "stlsheet.hxx"
#include "stlpool.hxx"

#include "scitems.hxx"
#include <editeng/boxitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/lrspitem.hxx>
#include <svx/pageitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/ulspitem.hxx>
#include <sfx2/printer.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/smplhint.hxx>
#include "attrib.hxx"


#include <vcl/svapp.hxx>    // GetSettings()

#include "globstr.hrc"
#include "sc.hrc"
//------------------------------------------------------------------------

#define TWO_CM      1134
#define HFDIST_CM   142

//========================================================================

ScStyleSheet::ScStyleSheet( const String&       rName,
                            ScStyleSheetPool&   rPoolP,
                            SfxStyleFamily      eFamily,
                            sal_uInt16              nMaskP )

    :   SfxStyleSheet   ( rName, rPoolP, eFamily, nMaskP )
    , eUsage( UNKNOWN )
{
}

//------------------------------------------------------------------------

ScStyleSheet::ScStyleSheet( const ScStyleSheet& rStyle )
    : SfxStyleSheet ( rStyle )
    , eUsage( UNKNOWN )
{
}

//------------------------------------------------------------------------

__EXPORT ScStyleSheet::~ScStyleSheet()
{
}

//------------------------------------------------------------------------

sal_Bool __EXPORT ScStyleSheet::HasFollowSupport() const
{
    return sal_False;
}

//------------------------------------------------------------------------

sal_Bool __EXPORT ScStyleSheet::HasParentSupport () const
{
    sal_Bool bHasParentSupport = sal_False;

    switch ( GetFamily() )
    {
        case SFX_STYLE_FAMILY_PARA: bHasParentSupport = sal_True;   break;
        case SFX_STYLE_FAMILY_PAGE: bHasParentSupport = sal_False;  break;
        default:
        {
            // added to avoid warnings
        }
    }

    return bHasParentSupport;
}

//------------------------------------------------------------------------

sal_Bool __EXPORT ScStyleSheet::SetParent( const String& rParentName )
{
    sal_Bool bResult = sal_False;
    String aEffName = rParentName;
    SfxStyleSheetBase* pStyle = rPool.Find( aEffName, nFamily );
    if (!pStyle)
    {
        SfxStyleSheetIteratorPtr pIter = rPool.CreateIterator( nFamily, SFXSTYLEBIT_ALL );
        pStyle = pIter->First();
        if (pStyle)
            aEffName = pStyle->GetName();
    }

    if ( pStyle && aEffName != GetName() )
    {
        bResult = SfxStyleSheet::SetParent( aEffName );
        if (bResult)
        {
            SfxItemSet& rParentSet = pStyle->GetItemSet();
            GetItemSet().SetParent( &rParentSet );

            // #i113491# Drag&Drop in the stylist's hierarchical view doesn't execute a slot,
            // so the repaint has to come from here (after modifying the ItemSet).
            // RepaintRange checks the document's IsVisible flag and locked repaints.
            ScDocument* pDoc = static_cast<ScStyleSheetPool&>(GetPool()).GetDocument();
            if (pDoc)
                pDoc->RepaintRange( ScRange( 0,0,0, MAXCOL,MAXROW,MAXTAB ) );
        }
    }

    return bResult;
}

//------------------------------------------------------------------------

SfxItemSet& __EXPORT ScStyleSheet::GetItemSet()
{
    if ( !pSet )
    {
        switch ( GetFamily() )
        {
            case SFX_STYLE_FAMILY_PAGE:
                {
                    // Seitenvorlagen sollen nicht ableitbar sein,
                    // deshalb werden an dieser Stelle geeignete
                    // Werte eingestellt. (==Standard-Seitenvorlage)

                    SfxItemPool& rItemPool = GetPool().GetPool();
                    pSet = new SfxItemSet( rItemPool,
                                           ATTR_BACKGROUND, ATTR_BACKGROUND,
                                           ATTR_BORDER, ATTR_SHADOW,
                                           ATTR_LRSPACE, ATTR_PAGE_SCALETO,
                                           ATTR_WRITINGDIR, ATTR_WRITINGDIR,
                                           ATTR_USERDEF, ATTR_USERDEF,
                                           0 );

                    //  Wenn gerade geladen wird, wird auch der Set hinterher aus der Datei
                    //  gefuellt, es brauchen also keine Defaults gesetzt zu werden.
                    //  GetPrinter wuerde dann auch einen neuen Printer anlegen, weil der
                    //  gespeicherte Printer noch nicht geladen ist!

                    ScDocument* pDoc = ((ScStyleSheetPool&)GetPool()).GetDocument();
                    if ( pDoc )
                    {
                        // Setzen von sinnvollen Default-Werten:
                        SvxPageItem     aPageItem( ATTR_PAGE );
                        SvxSizeItem     aPaperSizeItem( ATTR_PAGE_SIZE, SvxPaperInfo::GetDefaultPaperSize() );

                        SvxSetItem      aHFSetItem(
                                            (const SvxSetItem&)
                                            rItemPool.GetDefaultItem(ATTR_PAGE_HEADERSET) );

                        SfxItemSet&     rHFSet = aHFSetItem.GetItemSet();
                        SvxSizeItem     aHFSizeItem( // 0,5 cm + Abstand
                                            ATTR_PAGE_SIZE,
                                            Size( 0, (long)( 500 / HMM_PER_TWIPS ) + HFDIST_CM ) );

                        SvxULSpaceItem  aHFDistItem ( HFDIST_CM,// nUp
                                                      HFDIST_CM,// nLow
                                                      ATTR_ULSPACE );

                        SvxLRSpaceItem  aLRSpaceItem( TWO_CM,   // nLeft
                                                      TWO_CM,   // nRight
                                                      TWO_CM,   // nTLeft
                                                      0,        // nFirstLineOffset
                                                      ATTR_LRSPACE );
                        SvxULSpaceItem  aULSpaceItem( TWO_CM,   // nUp
                                                      TWO_CM,   // nLow
                                                      ATTR_ULSPACE );
                        SvxBoxInfoItem  aBoxInfoItem( ATTR_BORDER_INNER );

                        aBoxInfoItem.SetTable( sal_False );
                        aBoxInfoItem.SetDist( sal_True );
                        aBoxInfoItem.SetValid( VALID_DISTANCE, sal_True );

                        // aPageItem.SetLandscape( ORIENTATION_LANDSCAPE == pPrinter->GetOrientation() );
                        aPageItem.SetLandscape( sal_False );

                        rHFSet.Put( aBoxInfoItem );
                        rHFSet.Put( aHFSizeItem );
                        rHFSet.Put( aHFDistItem );
                        rHFSet.Put( SvxLRSpaceItem( 0,0,0,0, ATTR_LRSPACE ) ); // Rand auf Null setzen

                        pSet->Put( aHFSetItem, ATTR_PAGE_HEADERSET );
                        pSet->Put( aHFSetItem, ATTR_PAGE_FOOTERSET );
                        pSet->Put( aBoxInfoItem ); // PoolDefault wg. Formatvorlagen
                                                   // nicht ueberschreiben!

                        //  Writing direction: not as pool default because the default for cells
                        //  must remain FRMDIR_ENVIRONMENT, and each page style's setting is
                        //  supposed to be saved in the file format.
                        //  The page default depends on the system language.
                        SvxFrameDirection eDirection = ScGlobal::IsSystemRTL() ?
                                        FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP;
                        pSet->Put( SvxFrameDirectionItem( eDirection, ATTR_WRITINGDIR ), ATTR_WRITINGDIR );

                        rItemPool.SetPoolDefaultItem( aPageItem );
                        rItemPool.SetPoolDefaultItem( aPaperSizeItem );
                        rItemPool.SetPoolDefaultItem( aLRSpaceItem );
                        rItemPool.SetPoolDefaultItem( aULSpaceItem );
                        rItemPool.SetPoolDefaultItem( SfxUInt16Item( ATTR_PAGE_SCALE, 100 ) );
                        ScPageScaleToItem aScaleToItem;
                        rItemPool.SetPoolDefaultItem( aScaleToItem );
                        rItemPool.SetPoolDefaultItem( SfxUInt16Item( ATTR_PAGE_SCALETOPAGES, 0 ) );
                    }
                }
                break;

            case SFX_STYLE_FAMILY_PARA:
            default:
                pSet = new SfxItemSet( GetPool().GetPool(),
                                       ATTR_PATTERN_START, ATTR_PATTERN_END,
                                       0 );
                break;
        }
        bMySet = sal_True;
    } // if ( !pSet )
    if ( nHelpId == HID_SC_SHEET_CELL_ERG1 )
    {
        if ( !pSet->Count() )
        {
            ScDocument* pDoc = ((ScStyleSheetPool&)GetPool()).GetDocument();
            if ( pDoc )
            {
                sal_uLong nNumFmt = pDoc->GetFormatTable()->GetStandardFormat( NUMBERFORMAT_CURRENCY,ScGlobal::eLnge );
                pSet->Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNumFmt ) );
            } // if ( pDoc && pDoc->IsLoadingDone() )
        }
    }

    return *pSet;
}

//------------------------------------------------------------------------

sal_Bool __EXPORT ScStyleSheet::IsUsed() const
{
    if ( GetFamily() == SFX_STYLE_FAMILY_PARA )
    {
        // Always query the document to let it decide if a rescan is necessary,
        // and store the state.
        ScDocument* pDoc = ((ScStyleSheetPool&)rPool).GetDocument();
        if ( pDoc && pDoc->IsStyleSheetUsed( *this, sal_True ) )
            eUsage = USED;
        else
            eUsage = NOTUSED;
        return eUsage == USED;
    }
    else
        return sal_True;
}

//------------------------------------------------------------------------

void __EXPORT ScStyleSheet::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SfxSimpleHint* pSfxSimpleHint = dynamic_cast< const SfxSimpleHint* >(&rHint);

    if(pSfxSimpleHint && SFX_HINT_DYING == pSfxSimpleHint->GetId())
            GetItemSet().SetParent( NULL );
}

//------------------------------------------------------------------------

//  #66123# schmutzige Tricks, um die Standard-Vorlage immer als "Standard" zu speichern,
//  obwohl der fuer den Benutzer sichtbare Name uebersetzt ist:

const String& ScStyleSheet::GetName() const
{
    const String& rBase = SfxStyleSheet::GetName();
    const String* pForceStdName = ((ScStyleSheetPool&)rPool).GetForceStdName();
    if ( pForceStdName && rBase == ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
        return *pForceStdName;
    else
        return rBase;
}

const String& ScStyleSheet::GetParent() const
{
    const String& rBase = SfxStyleSheet::GetParent();
    const String* pForceStdName = ((ScStyleSheetPool&)rPool).GetForceStdName();
    if ( pForceStdName && rBase == ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
        return *pForceStdName;
    else
        return rBase;
}

const String& ScStyleSheet::GetFollow() const
{
    const String& rBase = SfxStyleSheet::GetFollow();
    const String* pForceStdName = ((ScStyleSheetPool&)rPool).GetForceStdName();
    if ( pForceStdName && rBase == ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
        return *pForceStdName;
    else
        return rBase;
}

//  Verhindern, dass ein Style "Standard" angelegt wird, wenn das nicht der
//  Standard-Name ist, weil sonst beim Speichern zwei Styles denselben Namen haetten
//  (Beim Laden wird der Style direkt per Make mit dem Namen erzeugt, so dass diese
//  Abfrage dann nicht gilt)
//! Wenn irgendwann aus dem Laden SetName aufgerufen wird, muss fuer das Laden ein
//! Flag gesetzt und abgefragt werden.
//! Die ganze Abfrage muss raus, wenn fuer eine neue Datei-Version die Namens-Umsetzung wegfaellt.

sal_Bool ScStyleSheet::SetName( const String& rNew )
{
    String aFileStdName = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(STRING_STANDARD));
    if ( rNew == aFileStdName && aFileStdName != ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
        return sal_False;
    else
        return SfxStyleSheet::SetName( rNew );
}



