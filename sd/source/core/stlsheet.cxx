/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/style/XStyle.hpp>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/serviceinfohelper.hxx>
#include <boost/bind.hpp>

#include "eetext.hxx"
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svx/svdoattr.hxx>
#include <editeng/ulspitem.hxx>
#include <svl/smplhint.hxx>
#include <svl/itemset.hxx>

#include <svx/xflbmtit.hxx>
#include <svx/xflbstit.hxx>
#include <editeng/bulitem.hxx>
#include <editeng/lrspitem.hxx>
#include <svx/unoshprp.hxx>
#include <svx/unoshape.hxx>
#include <svx/svdpool.hxx>
#include "stlsheet.hxx"
#include "sdresid.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "stlpool.hxx"
#include "glob.hrc"
#include "app.hrc"
#include "glob.hxx"
#include "helpids.h"
#include "../ui/inc/DrawViewShell.hxx"
#include "../ui/inc/ViewShellBase.hxx"

using ::rtl::OUString;
using ::osl::MutexGuard;
using ::osl::ClearableMutexGuard;
using ::cppu::OInterfaceContainerHelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;

#define WID_STYLE_DISPNAME  7998
#define WID_STYLE_FAMILY    7999

static SvxItemPropertySet& GetStylePropertySet()
{
    static const SfxItemPropertyMapEntry aFullPropertyMap_Impl[] =
    {
        { RTL_CONSTASCII_STRINGPARAM("Family"),                 WID_STYLE_FAMILY,       &::getCppuType((const OUString*)0), PropertyAttribute::READONLY,    0},
        { RTL_CONSTASCII_STRINGPARAM("UserDefinedAttributes"),  SDRATTR_XMLATTRIBUTES,  &XNameContainer::static_type(), 0,     0},
        { RTL_CONSTASCII_STRINGPARAM("DisplayName"),            WID_STYLE_DISPNAME,     &::getCppuType((const OUString*)0), PropertyAttribute::READONLY,    0},

        SVX_UNOEDIT_NUMBERING_PROPERTIE,
        SHADOW_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        FILL_PROPERTIES
        EDGERADIUS_PROPERTIES
        TEXT_PROPERTIES_DEFAULTS
        CONNECTOR_PROPERTIES
        SPECIAL_DIMENSIONING_PROPERTIES_DEFAULTS
        {0,0,0,0,0,0}
    };

    static SvxItemPropertySet aPropSet( aFullPropertyMap_Impl, SdrObject::GetGlobalDrawObjectItemPool() );
    return aPropSet;
}

class ModifyListenerForewarder : public SfxListener
{
public:
    ModifyListenerForewarder( SdStyleSheet* pStyleSheet );

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

private:
    SdStyleSheet* mpStyleSheet;
};

ModifyListenerForewarder::ModifyListenerForewarder( SdStyleSheet* pStyleSheet )
: mpStyleSheet( pStyleSheet )
{
    if( pStyleSheet )
    {
        SfxBroadcaster& rBC = static_cast< SfxBroadcaster& >( *pStyleSheet );
        StartListening( rBC );
    }
}

void ModifyListenerForewarder::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& /*rHint*/)
{
    if( mpStyleSheet )
        mpStyleSheet->notifyModifyListener();
}

SdStyleSheet::SdStyleSheet(const OUString& rDisplayName, SfxStyleSheetBasePool& _rPool, SfxStyleFamily eFamily, sal_uInt16 _nMask)
: SdStyleSheetBase( UniString( rDisplayName ), _rPool, eFamily, _nMask)
, ::cppu::BaseMutex()
, msApiName( rDisplayName )
, mxPool( const_cast< SfxStyleSheetBasePool* >(&_rPool) )
, mrBHelper( m_aMutex )
{
}

SdStyleSheet::SdStyleSheet( const SdStyleSheet & r )
: SdStyleSheetBase( r )
, ::cppu::BaseMutex()
, msApiName( r.msApiName )
, mxPool( r.mxPool )
, mrBHelper( m_aMutex )
{
}

SdStyleSheet::~SdStyleSheet()
{
    delete pSet;
    pSet = NULL;    // damit nachfolgende Destruktoren eine Chance haben
}

void SdStyleSheet::SetApiName( const OUString& rApiName )
{
    msApiName = rApiName;
}

rtl::OUString SdStyleSheet::GetApiName() const
{
    if( !msApiName.isEmpty() )
        return msApiName;
    else
        return GetName();
}


void SdStyleSheet::Load (SvStream& rIn, sal_uInt16 nVersion)
{
    SfxStyleSheetBase::Load(rIn, nVersion);

    // Die Default-Maske war frueher 0xAFFE.
    // Aus dieser Default-Maske wurden die benoetigten Flags ausmaskiert.
    // Nun wurde das Flag SFXSTYLEBIT_READONLY eingefuehrt, was dazu
    // das alle StyleSheets read-only waren.
    // Da im Draw kein StyleSheet read-only sein soll, wird an dieser Stelle
    // das Flag zurueckgesetzt.
    nMask &= ~SFXSTYLEBIT_READONLY;
}

/*************************************************************************
|*
|* Store
|*
\************************************************************************/

void SdStyleSheet::Store(SvStream& rOut)
{
    SfxStyleSheetBase::Store(rOut);
}

/*************************************************************************
|*
|* Parent setzen
|*
\************************************************************************/

bool SdStyleSheet::SetParent(const String& rParentName)
{
    bool bResult = sal_False;

    if (SfxStyleSheet::SetParent(rParentName))
    {
        // PseudoStyleSheets haben keine eigenen ItemSets
        if (nFamily != SD_STYLE_FAMILY_PSEUDO)
        {
            if( rParentName.Len() )
            {
                SfxStyleSheetBase* pStyle = pPool->Find(rParentName, nFamily);
                if (pStyle)
                {
                    bResult = true;
                    SfxItemSet& rParentSet = pStyle->GetItemSet();
                    GetItemSet().SetParent(&rParentSet);
                    Broadcast( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
                }
            }
            else
            {
                bResult = true;
                GetItemSet().SetParent(NULL);
                Broadcast( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
            }
        }
        else
        {
            bResult = true;
        }
    }
    return bResult;
}

/*************************************************************************
|*
|* ItemSet ggfs. erzeugen und herausreichen
|*
\************************************************************************/

SfxItemSet& SdStyleSheet::GetItemSet()
{
    if (nFamily == SD_STYLE_FAMILY_GRAPHICS || nFamily == SD_STYLE_FAMILY_MASTERPAGE)
    {
        // ggfs. das ItemSet 'on demand' anlegen
        if (!pSet)
        {
            sal_uInt16 nWhichPairTable[] = { XATTR_LINE_FIRST,              XATTR_LINE_LAST,
                                         XATTR_FILL_FIRST,              XATTR_FILL_LAST,

                                        SDRATTR_SHADOW_FIRST,           SDRATTR_SHADOW_LAST,
                                        SDRATTR_TEXT_MINFRAMEHEIGHT,    SDRATTR_TEXT_CONTOURFRAME,

                                        SDRATTR_TEXT_WORDWRAP,          SDRATTR_TEXT_AUTOGROWSIZE,

                                        SDRATTR_EDGE_FIRST,             SDRATTR_EDGE_LAST,
                                        SDRATTR_MEASURE_FIRST,          SDRATTR_MEASURE_LAST,

                                        EE_PARA_START,                  EE_CHAR_END,

                                        SDRATTR_XMLATTRIBUTES,          SDRATTR_TEXT_USEFIXEDCELLHEIGHT,

                                        SDRATTR_3D_FIRST, SDRATTR_3D_LAST,
                                        0, 0 };

            pSet = new SfxItemSet(GetPool().GetPool(), nWhichPairTable);
        }

        return *pSet;
    }

    else if( nFamily == SD_STYLE_FAMILY_CELL )
    {
        if (!pSet)
        {
            sal_uInt16 nWhichPairTable[] = { XATTR_LINE_FIRST,              XATTR_LINE_LAST,
                                         XATTR_FILL_FIRST,              XATTR_FILL_LAST,

                                        SDRATTR_SHADOW_FIRST,           SDRATTR_SHADOW_LAST,
                                        SDRATTR_TEXT_MINFRAMEHEIGHT,    SDRATTR_TEXT_CONTOURFRAME,

                                        SDRATTR_TEXT_WORDWRAP,          SDRATTR_TEXT_AUTOGROWSIZE,

                                        EE_PARA_START,                  EE_CHAR_END,

                                        SDRATTR_TABLE_FIRST,            SDRATTR_TABLE_LAST,
                                        SDRATTR_XMLATTRIBUTES,          SDRATTR_XMLATTRIBUTES,

                                        0, 0 };

            pSet = new SfxItemSet(GetPool().GetPool(), nWhichPairTable);
        }

        return *pSet;
    }

    // dies ist eine Stellvertretervorlage fuer die interne Vorlage des
    // aktuellen Praesentationslayouts: dessen ItemSet returnieren
    else
    {

        SdStyleSheet* pSdSheet = GetRealStyleSheet();

        if (pSdSheet)
        {
            return(pSdSheet->GetItemSet());
        }
        else
        {
            if (!pSet)
            {
                sal_uInt16 nWhichPairTable[] = { XATTR_LINE_FIRST,              XATTR_LINE_LAST,
                                             XATTR_FILL_FIRST,              XATTR_FILL_LAST,

                                             SDRATTR_SHADOW_FIRST,          SDRATTR_SHADOW_LAST,
                                             SDRATTR_TEXT_MINFRAMEHEIGHT,   SDRATTR_TEXT_CONTOURFRAME,

                                             SDRATTR_TEXT_WORDWRAP,         SDRATTR_TEXT_AUTOGROWSIZE,

                                             SDRATTR_EDGE_FIRST,            SDRATTR_EDGE_LAST,
                                             SDRATTR_MEASURE_FIRST,         SDRATTR_MEASURE_LAST,

                                             EE_PARA_START,                 EE_CHAR_END,

                                            SDRATTR_XMLATTRIBUTES,          SDRATTR_TEXT_USEFIXEDCELLHEIGHT,

                                            SDRATTR_3D_FIRST, SDRATTR_3D_LAST,
                                             0, 0 };

                pSet = new SfxItemSet(GetPool().GetPool(), nWhichPairTable);
            }

            return(*pSet);
        }
    }
}

/*************************************************************************
|*
|* IsUsed(), eine Vorlage gilt als benutzt, wenn sie von eingefuegten Objekten
|*           oder von benutzten Vorlagen referenziert wird
|*
\************************************************************************/

bool SdStyleSheet::IsUsed() const
{
    bool bResult = false;

    sal_uInt16 nListenerCount = GetListenerCount();
    if (nListenerCount > 0)
    {
        for (sal_uInt16 n = 0; n < nListenerCount; n++)
        {
            SfxListener* pListener = GetListener(n);
            if( pListener == this )
                continue;

            // NULL-Pointer ist im Listener-Array erlaubt
            if (pListener && pListener->ISA(SdrAttrObj))
            {
                bResult = ((SdrAttrObj*)pListener)->IsInserted();
            }
            else if (pListener && pListener->ISA(SfxStyleSheet))
            {
                bResult = ((SfxStyleSheet*)pListener)->IsUsed();
            }
            if (bResult)
                break;
        }
    }

    if( !bResult )
    {
        MutexGuard aGuard( mrBHelper.rMutex );

        OInterfaceContainerHelper * pContainer = mrBHelper.getContainer( XModifyListener::static_type() );
        if( pContainer )
        {
            Sequence< Reference< XInterface > > aModifyListeners( pContainer->getElements() );
            Reference< XInterface > *p = aModifyListeners.getArray();
            sal_Int32 nCount = aModifyListeners.getLength();
            while( nCount-- && !bResult )
            {
                Reference< XStyle > xStyle( *p++, UNO_QUERY );
                if( xStyle.is() )
                    bResult = xStyle->isInUse();
            }
        }
    }
    return bResult;
}

/*************************************************************************
|*
|* das StyleSheet ermitteln, fuer das dieses StyleSheet steht
|*
\************************************************************************/

SdStyleSheet* SdStyleSheet::GetRealStyleSheet() const
{
    String aRealStyle;
    String aSep( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ));
    SdStyleSheet* pRealStyle = NULL;
    SdDrawDocument* pDoc = ((SdStyleSheetPool*)pPool)->GetDoc();

    ::sd::DrawViewShell* pDrawViewShell = 0;

    ::sd::ViewShellBase* pBase = dynamic_cast< ::sd::ViewShellBase* >( SfxViewShell::Current() );
    if( pBase )
        pDrawViewShell = dynamic_cast< ::sd::DrawViewShell* >( pBase->GetMainViewShell().get() );

    if (pDrawViewShell && pDrawViewShell->GetDoc() == pDoc)
    {
        SdPage* pPage = pDrawViewShell->getCurrentPage();
        if( pPage )
        {
            aRealStyle = pPage->GetLayoutName();
            // cut after seperator string
            aRealStyle.Erase(aRealStyle.Search(aSep) + aSep.Len());
        }
    }

    if (aRealStyle.Len() == 0)
    {
        SdPage* pPage = pDoc->GetSdPage(0, PK_STANDARD);

        if (pPage)
        {
            aRealStyle = pDoc->GetSdPage(0, PK_STANDARD)->GetLayoutName();
        }
        else
        {
            // Noch keine Seite vorhanden
            // Dieses kann beim Aktualisieren vonDokumentvorlagen vorkommen
            SfxStyleSheetIterator aIter(pPool, SD_STYLE_FAMILY_MASTERPAGE);
            SfxStyleSheetBase* pSheet = aIter.First();
            if( pSheet )
                aRealStyle = pSheet->GetName();
        }

        aRealStyle.Erase(aRealStyle.Search(aSep) + aSep.Len());
    }

    // jetzt vom Namen (landessprachlich angepasst) auf den internen
    // Namen (unabhaengig von der Landessprache) mappen
    String aInternalName;

    if (aName == String(SdResId(STR_PSEUDOSHEET_TITLE)))
    {
        aInternalName = String(SdResId(STR_LAYOUT_TITLE));
    }
    else if (aName == String(SdResId(STR_PSEUDOSHEET_SUBTITLE)))
    {
        aInternalName = String(SdResId(STR_LAYOUT_SUBTITLE));
    }
    else if (aName == String(SdResId(STR_PSEUDOSHEET_BACKGROUND)))
    {
        aInternalName = String(SdResId(STR_LAYOUT_BACKGROUND));
    }
    else if (aName == String(SdResId(STR_PSEUDOSHEET_BACKGROUNDOBJECTS)))
    {
        aInternalName = String(SdResId(STR_LAYOUT_BACKGROUNDOBJECTS));
    }
    else if (aName == String(SdResId(STR_PSEUDOSHEET_NOTES)))
    {
        aInternalName = String(SdResId(STR_LAYOUT_NOTES));
    }
    else
    {
        String aOutlineStr(SdResId(STR_PSEUDOSHEET_OUTLINE));
        sal_uInt16 nPos = aName.Search(aOutlineStr);
        if (nPos != STRING_NOTFOUND)
        {
            String aNumStr(aName.Copy(aOutlineStr.Len()));
            aInternalName = String(SdResId(STR_LAYOUT_OUTLINE));
            aInternalName += aNumStr;
        }
    }

    aRealStyle += aInternalName;
    pRealStyle = static_cast< SdStyleSheet* >( pPool->Find(aRealStyle, SD_STYLE_FAMILY_MASTERPAGE) );

#ifdef DBG_UTIL
    if( !pRealStyle )
    {
        SfxStyleSheetIterator aIter(pPool, SD_STYLE_FAMILY_MASTERPAGE);
        if( aIter.Count() > 0 )
            // StyleSheet not found, but pool already loaded
            DBG_ASSERT(pRealStyle, "Internal StyleSheet not found");
    }
#endif

    return pRealStyle;
}

/*************************************************************************
|*
|* das PseudoStyleSheet ermitteln, durch das dieses StyleSheet vertreten wird
|*
\************************************************************************/

SdStyleSheet* SdStyleSheet::GetPseudoStyleSheet() const
{
    SdStyleSheet* pPseudoStyle = NULL;
    String aSep( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ));
    String aStyleName(aName);
        // ohne Layoutnamen und Separator
    aStyleName.Erase(0, aStyleName.Search(aSep) + aSep.Len());

    if (aStyleName == String(SdResId(STR_LAYOUT_TITLE)))
    {
        aStyleName = String(SdResId(STR_PSEUDOSHEET_TITLE));
    }
    else if (aStyleName == String(SdResId(STR_LAYOUT_SUBTITLE)))
    {
        aStyleName = String(SdResId(STR_PSEUDOSHEET_SUBTITLE));
    }
    else if (aStyleName == String(SdResId(STR_LAYOUT_BACKGROUND)))
    {
        aStyleName = String(SdResId(STR_PSEUDOSHEET_BACKGROUND));
    }
    else if (aStyleName == String(SdResId(STR_LAYOUT_BACKGROUNDOBJECTS)))
    {
        aStyleName = String(SdResId(STR_PSEUDOSHEET_BACKGROUNDOBJECTS));
    }
    else if (aStyleName == String(SdResId(STR_LAYOUT_NOTES)))
    {
        aStyleName = String(SdResId(STR_PSEUDOSHEET_NOTES));
    }
    else
    {
        String aOutlineStr((SdResId(STR_LAYOUT_OUTLINE)));
        sal_uInt16 nPos = aStyleName.Search(aOutlineStr);
        if (nPos != STRING_NOTFOUND)
        {
            String aNumStr(aStyleName.Copy(aOutlineStr.Len()));
            aStyleName = String(SdResId(STR_PSEUDOSHEET_OUTLINE));
            aStyleName += aNumStr;
        }
    }

    pPseudoStyle = static_cast<SdStyleSheet*>(pPool->Find(aStyleName, SD_STYLE_FAMILY_PSEUDO));
    DBG_ASSERT(pPseudoStyle, "PseudoStyleSheet nicht gefunden");

    return pPseudoStyle;
}


/*************************************************************************
|*
|* Notify
|*
\************************************************************************/

void SdStyleSheet::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    // erstmal die Basisklassenfunktionalitaet
    SfxStyleSheet::Notify(rBC, rHint);

    // wenn der Stellvertreter ein Notify bezueglich geaenderter Attribute
    // bekommt, sorgt er dafuer, dass das eigentlich gemeinte StyleSheet
    // broadcastet
    SfxSimpleHint* pSimple = PTR_CAST(SfxSimpleHint, &rHint);
    sal_uLong nId = pSimple == NULL ? 0 : pSimple->GetId();
    if (nId == SFX_HINT_DATACHANGED && nFamily == SD_STYLE_FAMILY_PSEUDO)
    {
        SdStyleSheet* pRealStyle = GetRealStyleSheet();
        if (pRealStyle)
            pRealStyle->Broadcast(rHint);
    }
}

/*************************************************************************
|* AdjustToFontHeight passt die Bulletbreite und den linken Texteinzug
|* des uebergebenen ItemSets dessen Fonthoehe an. Die neuen Werte werden so
|* berechnet, dass das Verhaeltnis zur Fonthoehe so ist wie im StyleSheet.
|*
|* bOnlyMissingItems legt fest, ob lediglich nicht gesetzte Items ergaenzt
|* (sal_True) oder explizit gesetzte Items ueberschreiben werden sollen (sal_False)
|*
\************************************************************************/

void SdStyleSheet::AdjustToFontHeight(SfxItemSet& rSet, sal_Bool bOnlyMissingItems)
{
    // Bulletbreite und Texteinzug an neue Fonthoehe
    // anpassen, wenn sie nicht explizit gesetzt wurden
    SfxStyleFamily eFamily = nFamily;
    String aStyleName(aName);
    if (eFamily == SD_STYLE_FAMILY_PSEUDO)
    {
        SfxStyleSheet* pRealStyle = GetRealStyleSheet();
        eFamily = pRealStyle->GetFamily();
        aStyleName = pRealStyle->GetName();
    }

    if (eFamily == SD_STYLE_FAMILY_MASTERPAGE &&
        aStyleName.Search(String(SdResId(STR_LAYOUT_OUTLINE))) != STRING_NOTFOUND &&
        rSet.GetItemState(EE_CHAR_FONTHEIGHT) == SFX_ITEM_SET)
    {
        const SfxItemSet* pCurSet = &GetItemSet();
        sal_uInt32 nNewHeight = ((SvxFontHeightItem&)rSet.Get(EE_CHAR_FONTHEIGHT)).GetHeight();
        sal_uInt32 nOldHeight = ((SvxFontHeightItem&)pCurSet->Get(EE_CHAR_FONTHEIGHT)).GetHeight();

        if (rSet.GetItemState(EE_PARA_BULLET) != SFX_ITEM_SET || !bOnlyMissingItems)
        {
            const SvxBulletItem& rBItem = (const SvxBulletItem&)pCurSet->Get(EE_PARA_BULLET);
            double fBulletFraction = double(rBItem.GetWidth()) / nOldHeight;
            SvxBulletItem aNewBItem(rBItem);
            aNewBItem.SetWidth((sal_uInt32)(fBulletFraction * nNewHeight));
            rSet.Put(aNewBItem);
        }

        if (rSet.GetItemState(EE_PARA_LRSPACE) != SFX_ITEM_SET || !bOnlyMissingItems)
        {
            const SvxLRSpaceItem& rLRItem = (const SvxLRSpaceItem&)pCurSet->Get(EE_PARA_LRSPACE);
            double fIndentFraction = double(rLRItem.GetTxtLeft()) / nOldHeight;
            SvxLRSpaceItem aNewLRItem(rLRItem);
            aNewLRItem.SetTxtLeft(fIndentFraction * nNewHeight);
            double fFirstIndentFraction = double(rLRItem.GetTxtFirstLineOfst()) / nOldHeight;
            aNewLRItem.SetTxtFirstLineOfst((short)(fFirstIndentFraction * nNewHeight));
            rSet.Put(aNewLRItem);
        }

        if (rSet.GetItemState(EE_PARA_ULSPACE) != SFX_ITEM_SET || !bOnlyMissingItems)
        {
            const SvxULSpaceItem& rULItem = (const SvxULSpaceItem&)pCurSet->Get(EE_PARA_ULSPACE);
            SvxULSpaceItem aNewULItem(rULItem);
            double fLowerFraction = double(rULItem.GetLower()) / nOldHeight;
            aNewULItem.SetLower((sal_uInt16)(fLowerFraction * nNewHeight));
            double fUpperFraction = double(rULItem.GetUpper()) / nOldHeight;
            aNewULItem.SetUpper((sal_uInt16)(fUpperFraction * nNewHeight));
            rSet.Put(aNewULItem);
        }
    }
}

// --------------------------------------------------------------------

bool SdStyleSheet::HasFollowSupport() const
{
    return false;
}

// --------------------------------------------------------------------

bool SdStyleSheet::HasParentSupport() const
{
    return true;
}

// --------------------------------------------------------------------

bool SdStyleSheet::HasClearParentSupport() const
{
    return true;
}

// --------------------------------------------------------------------

bool SdStyleSheet::SetName( const UniString& rName )
{
    return SfxStyleSheet::SetName( rName );
}

// --------------------------------------------------------------------

void SdStyleSheet::SetHelpId( const String& r, sal_uLong nId )
{
    SfxStyleSheet::SetHelpId( r, nId );

    if( (nId >= HID_PSEUDOSHEET_OUTLINE1) && ( nId <= HID_PSEUDOSHEET_OUTLINE9 ) )
    {
        msApiName = "outline";
        msApiName += OUString( (sal_Unicode)( '1' + (nId - HID_PSEUDOSHEET_OUTLINE1) ) );
    }
    else
    {
        static struct ApiNameMap
        {
            const sal_Char* mpApiName;
            sal_uInt32      mnApiNameLength;
            sal_uInt32      mnHelpId;
        }
        pApiNameMap[] =
        {
            { RTL_CONSTASCII_STRINGPARAM( "title" ),            HID_PSEUDOSHEET_TITLE },
            { RTL_CONSTASCII_STRINGPARAM( "subtitle" ),         HID_PSEUDOSHEET_SUBTITLE },
            { RTL_CONSTASCII_STRINGPARAM( "background" ),       HID_PSEUDOSHEET_BACKGROUND },
            { RTL_CONSTASCII_STRINGPARAM( "backgroundobjects" ),HID_PSEUDOSHEET_BACKGROUNDOBJECTS },
            { RTL_CONSTASCII_STRINGPARAM( "notes" ),            HID_PSEUDOSHEET_NOTES },
            { RTL_CONSTASCII_STRINGPARAM( "standard" ),         HID_STANDARD_STYLESHEET_NAME },
            { RTL_CONSTASCII_STRINGPARAM( "objectwitharrow" ),  HID_POOLSHEET_OBJWITHARROW },
            { RTL_CONSTASCII_STRINGPARAM( "objectwithshadow" ), HID_POOLSHEET_OBJWITHSHADOW },
            { RTL_CONSTASCII_STRINGPARAM( "objectwithoutfill" ),HID_POOLSHEET_OBJWITHOUTFILL },
            { RTL_CONSTASCII_STRINGPARAM( "text" ),             HID_POOLSHEET_TEXT },
            { RTL_CONSTASCII_STRINGPARAM( "textbody" ),         HID_POOLSHEET_TEXTBODY },
            { RTL_CONSTASCII_STRINGPARAM( "textbodyjustfied" ), HID_POOLSHEET_TEXTBODY_JUSTIFY },
            { RTL_CONSTASCII_STRINGPARAM( "textbodyindent" ),   HID_POOLSHEET_TEXTBODY_INDENT },
            { RTL_CONSTASCII_STRINGPARAM( "title" ),            HID_POOLSHEET_TITLE },
            { RTL_CONSTASCII_STRINGPARAM( "title1" ),           HID_POOLSHEET_TITLE1 },
            { RTL_CONSTASCII_STRINGPARAM( "title2" ),           HID_POOLSHEET_TITLE2 },
            { RTL_CONSTASCII_STRINGPARAM( "headline" ),         HID_POOLSHEET_HEADLINE },
            { RTL_CONSTASCII_STRINGPARAM( "headline1" ),        HID_POOLSHEET_HEADLINE1 },
            { RTL_CONSTASCII_STRINGPARAM( "headline2" ),        HID_POOLSHEET_HEADLINE2 },
            { RTL_CONSTASCII_STRINGPARAM( "measure" ),          HID_POOLSHEET_MEASURE },
            { 0, 0, 0 }
        };

        ApiNameMap* p = pApiNameMap;
        while( p->mpApiName )
        {
            if( nId == p->mnHelpId )
            {
                msApiName = OUString( p->mpApiName, p->mnApiNameLength, RTL_TEXTENCODING_ASCII_US );
                break;
            }
            p++;
        }
    }
}

// --------------------------------------------------------------------

OUString SdStyleSheet::GetFamilyString( SfxStyleFamily eFamily )
{
    switch( eFamily )
    {
    case SD_STYLE_FAMILY_CELL:
        return OUString( "cell" );
    default:
        OSL_FAIL( "SdStyleSheet::GetFamilyString(), illegal family!" );
    case SD_STYLE_FAMILY_GRAPHICS:
        return OUString( "graphics" );
    }
}

// --------------------------------------------------------------------

void SdStyleSheet::throwIfDisposed() throw (RuntimeException)
{
    if( !mxPool.is() )
        throw DisposedException();
}

// --------------------------------------------------------------------

SdStyleSheet* SdStyleSheet::CreateEmptyUserStyle( SfxStyleSheetBasePool& rPool, SfxStyleFamily eFamily )
{
    OUString aPrefix( "user" );
    OUString aName;
    sal_Int32 nIndex = 1;
    do
    {
        aName = aPrefix + OUString::valueOf( nIndex++ );
    }
    while( rPool.Find( aName, eFamily ) != 0 );

    return new SdStyleSheet(aName, rPool, eFamily, SFXSTYLEBIT_USERDEF);
}

// --------------------------------------------------------------------
// XInterface
// --------------------------------------------------------------------

void SAL_CALL SdStyleSheet::release(  ) throw ()
{
    if (osl_atomic_decrement( &m_refCount ) == 0)
    {
        // restore reference count:
        osl_atomic_increment( &m_refCount );
        if (! mrBHelper.bDisposed) try
        {
            dispose();
        }
        catch (RuntimeException const& exc)
        { // don't break throw ()
            OSL_FAIL(
                OUStringToOString(
                    exc.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
            static_cast<void>(exc);
        }
        OSL_ASSERT( mrBHelper.bDisposed );
        SdStyleSheetBase::release();
    }
}

// --------------------------------------------------------------------
// XComponent
// --------------------------------------------------------------------

void SAL_CALL SdStyleSheet::dispose(  ) throw (RuntimeException)
{
    ClearableMutexGuard aGuard( mrBHelper.rMutex );
    if (!mrBHelper.bDisposed && !mrBHelper.bInDispose)
    {
        mrBHelper.bInDispose = sal_True;
        aGuard.clear();
        try
        {
            // side effect: keeping a reference to this
            EventObject aEvt( static_cast< OWeakObject * >( this ) );
            try
            {
                mrBHelper.aLC.disposeAndClear( aEvt );
                disposing();
            }
            catch (...)
            {
                MutexGuard aGuard2( mrBHelper.rMutex );
                // bDisposed and bInDispose must be set in this order:
                mrBHelper.bDisposed = sal_True;
                mrBHelper.bInDispose = sal_False;
                throw;
            }
            MutexGuard aGuard2( mrBHelper.rMutex );
            // bDisposed and bInDispose must be set in this order:
            mrBHelper.bDisposed = sal_True;
            mrBHelper.bInDispose = sal_False;
        }
        catch (RuntimeException &)
        {
            throw;
        }
        catch (const Exception & exc)
        {
            throw RuntimeException( "unexpected UNO exception caught: "  +  exc.Message, Reference< XInterface >() );
        }
    }
}

// --------------------------------------------------------------------

void SdStyleSheet::disposing()
{
    mxPool.clear();
}

// --------------------------------------------------------------------

void SAL_CALL SdStyleSheet::addEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException)
{
    ClearableMutexGuard aGuard( mrBHelper.rMutex );
    if (mrBHelper.bDisposed || mrBHelper.bInDispose)
    {
        aGuard.clear();
        EventObject aEvt( static_cast< OWeakObject * >( this ) );
        xListener->disposing( aEvt );
    }
    else
    {
        mrBHelper.addListener( ::getCppuType( &xListener ), xListener );
    }
}

// --------------------------------------------------------------------

void SAL_CALL SdStyleSheet::removeEventListener( const Reference< XEventListener >& xListener  ) throw (RuntimeException)
{
    mrBHelper.removeListener( ::getCppuType( &xListener ), xListener );
}

//------------------------------------------------------------------------
// XModifyBroadcaster
//------------------------------------------------------------------------

void SAL_CALL SdStyleSheet::addModifyListener( const Reference< XModifyListener >& xListener ) throw (RuntimeException)
{
    ClearableMutexGuard aGuard( mrBHelper.rMutex );
    if (mrBHelper.bDisposed || mrBHelper.bInDispose)
    {
        aGuard.clear();
        EventObject aEvt( static_cast< OWeakObject * >( this ) );
        xListener->disposing( aEvt );
    }
    else
    {
        if( !mpModifyListenerForewarder.get() )
            mpModifyListenerForewarder.reset( new ModifyListenerForewarder( this ) );
        mrBHelper.addListener( XModifyListener::static_type(), xListener );
    }
}

//------------------------------------------------------------------------

void SAL_CALL SdStyleSheet::removeModifyListener( const Reference< XModifyListener >& xListener ) throw (RuntimeException)
{
    mrBHelper.removeListener( XModifyListener::static_type(), xListener );
}

//------------------------------------------------------------------------

void SdStyleSheet::notifyModifyListener()
{
    MutexGuard aGuard( mrBHelper.rMutex );

    OInterfaceContainerHelper * pContainer = mrBHelper.getContainer( XModifyListener::static_type() );
    if( pContainer )
    {
        EventObject aEvt( static_cast< OWeakObject * >( this ) );
        pContainer->forEach<XModifyListener>( boost::bind( &XModifyListener::modified, _1, boost::cref( aEvt ) ) );
    }
}


// --------------------------------------------------------------------
// XServiceInfo
// --------------------------------------------------------------------

OUString SAL_CALL SdStyleSheet::getImplementationName() throw(RuntimeException)
{
    return OUString( "SdStyleSheet" );
}

// --------------------------------------------------------------------

sal_Bool SAL_CALL SdStyleSheet::supportsService( const OUString& ServiceName ) throw(RuntimeException)
{
    return comphelper::ServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL SdStyleSheet::getSupportedServiceNames() throw(RuntimeException)
{
    Sequence< OUString > aNameSequence( 10 );
    OUString* pStrings = aNameSequence.getArray();

    *pStrings++ = "com.sun.star.style.Style";
    *pStrings++ = "com.sun.star.drawing.FillProperties";
    *pStrings++ = "com.sun.star.drawing.LineProperties";
    *pStrings++ = "com.sun.star.drawing.ShadowProperties";
    *pStrings++ = "com.sun.star.drawing.ConnectorProperties";
    *pStrings++ = "com.sun.star.drawing.MeasureProperties";
    *pStrings++ = "com.sun.star.style.ParagraphProperties";
    *pStrings++ = "com.sun.star.style.CharacterProperties";
    *pStrings++ = "com.sun.star.drawing.TextProperties";
    *pStrings++ = "com.sun.star.drawing.Text";

    return aNameSequence;
}

// --------------------------------------------------------------------
// XNamed
// --------------------------------------------------------------------

OUString SAL_CALL SdStyleSheet::getName() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    throwIfDisposed();
    return GetApiName();
}

// --------------------------------------------------------------------

void SAL_CALL SdStyleSheet::setName( const OUString& rName  ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    if( SetName( rName ) )
    {
        msApiName = rName;
        Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
    }
}

// --------------------------------------------------------------------
// XStyle
// --------------------------------------------------------------------

sal_Bool SAL_CALL SdStyleSheet::isUserDefined() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    throwIfDisposed();
    return IsUserDefined() ? sal_True : sal_False;
}

// --------------------------------------------------------------------

sal_Bool SAL_CALL SdStyleSheet::isInUse() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    throwIfDisposed();
    return IsUsed() ? sal_True : sal_False;
}

// --------------------------------------------------------------------

OUString SAL_CALL SdStyleSheet::getParentStyle() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    if( GetParent().Len() )
    {
        SdStyleSheet* pParentStyle = static_cast< SdStyleSheet* >( mxPool->Find( GetParent(), nFamily ) );
        if( pParentStyle )
            return pParentStyle->msApiName;
    }
    return OUString();
}

// --------------------------------------------------------------------

void SAL_CALL SdStyleSheet::setParentStyle( const OUString& rParentName  ) throw(NoSuchElementException, RuntimeException)
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    if( !rParentName.isEmpty() )
    {
        const SfxStyles& rStyles = mxPool->GetStyles();

        /* Use reverse iterator to find the parents quicker - most probably its inserted recently.
         * Also avoids/fixes the issue n#708518
         * To fix it completely its probably wiser to compare this->GetName() and pStyle->GetName() or use complete names for styles (?)
         */
        for( SfxStyles::const_reverse_iterator iter( rStyles.rbegin() ); iter != rStyles.rend(); ++iter )
        {
            SdStyleSheet* pStyle = static_cast< SdStyleSheet* >( (*iter).get() );
            if( pStyle && (pStyle->nFamily == nFamily) && (pStyle->msApiName == rParentName) )
            {
                if( pStyle != this )
                    SetParent( pStyle->GetName() );
                return;
            }
        }
        throw NoSuchElementException();
    }
    else
    {
        SetParent( rParentName );
    }
}

// --------------------------------------------------------------------
// XPropertySet
// --------------------------------------------------------------------

Reference< XPropertySetInfo > SdStyleSheet::getPropertySetInfo() throw(RuntimeException)
{
    throwIfDisposed();
    static Reference< XPropertySetInfo > xInfo;
    if( !xInfo.is() )
        xInfo = GetStylePropertySet().getPropertySetInfo();
    return xInfo;
}

// --------------------------------------------------------------------

void SAL_CALL SdStyleSheet::setPropertyValue( const OUString& aPropertyName, const Any& aValue ) throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    throwIfDisposed();

    const SfxItemPropertySimpleEntry* pEntry = getPropertyMapEntry( aPropertyName );
    if( pEntry == NULL )
    {
        throw UnknownPropertyException();
    }
    else
    {
        if( pEntry->nWID == SDRATTR_TEXTDIRECTION )
            return; // not yet implemented for styles

        if( pEntry->nWID == WID_STYLE_FAMILY )
            throw PropertyVetoException();

        if( (pEntry->nWID == EE_PARA_NUMBULLET) && (GetFamily() == SD_STYLE_FAMILY_MASTERPAGE) )
        {
            String aStr;
            const sal_uInt32 nTempHelpId = GetHelpId( aStr );

            if( (nTempHelpId >= HID_PSEUDOSHEET_OUTLINE2) && (nTempHelpId <= HID_PSEUDOSHEET_OUTLINE9) )
                return;
        }

        SfxItemSet &rStyleSet = GetItemSet();

        if( pEntry->nWID == OWN_ATTR_FILLBMP_MODE )
        {
            BitmapMode eMode;
            if( aValue >>= eMode )
            {
                rStyleSet.Put( XFillBmpStretchItem( eMode == BitmapMode_STRETCH ) );
                rStyleSet.Put( XFillBmpTileItem( eMode == BitmapMode_REPEAT ) );
                return;
            }
            throw IllegalArgumentException();
        }

        SfxItemSet aSet( GetPool().GetPool(),   pEntry->nWID, pEntry->nWID);
        aSet.Put( rStyleSet );

        if( !aSet.Count() )
        {
            if( EE_PARA_NUMBULLET == pEntry->nWID )
            {
                Font aBulletFont;
                SdStyleSheetPool::PutNumBulletItem( this, aBulletFont );
                aSet.Put( rStyleSet );
            }
            else
            {
                aSet.Put( GetPool().GetPool().GetDefaultItem( pEntry->nWID ) );
            }
        }

        if( pEntry->nMemberId == MID_NAME &&
            ( pEntry->nWID == XATTR_FILLBITMAP || pEntry->nWID == XATTR_FILLGRADIENT ||
              pEntry->nWID == XATTR_FILLHATCH || pEntry->nWID == XATTR_FILLFLOATTRANSPARENCE ||
              pEntry->nWID == XATTR_LINESTART || pEntry->nWID == XATTR_LINEEND || pEntry->nWID == XATTR_LINEDASH) )
        {
            OUString aTempName;
            if(!(aValue >>= aTempName ))
                throw IllegalArgumentException();

            SvxShape::SetFillAttribute( pEntry->nWID, aTempName, aSet );
        }
        else if(!SvxUnoTextRangeBase::SetPropertyValueHelper( aSet, pEntry, aValue, aSet ))
        {
            SvxItemPropertySet_setPropertyValue( GetStylePropertySet(), pEntry, aValue, aSet );
        }

        rStyleSet.Put( aSet );
        Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
    }
}

// --------------------------------------------------------------------

Any SAL_CALL SdStyleSheet::getPropertyValue( const OUString& PropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    throwIfDisposed();

    const SfxItemPropertySimpleEntry* pEntry = getPropertyMapEntry( PropertyName );
    if( pEntry == NULL )
    {
        throw UnknownPropertyException();
    }
    else
    {
        Any aAny;

        if( pEntry->nWID == WID_STYLE_FAMILY )
        {
            if( nFamily == SD_STYLE_FAMILY_MASTERPAGE )
            {
                const OUString aLayoutName( GetName() );
                aAny <<= aLayoutName.copy( 0, aLayoutName.indexOf( SD_LT_SEPARATOR) );
            }
            else
            {
                aAny <<= GetFamilyString(nFamily);
            }
        }
        else if( pEntry->nWID == WID_STYLE_DISPNAME )
        {
            aAny <<= maDisplayName;
        }
        else if( pEntry->nWID == SDRATTR_TEXTDIRECTION )
        {
            aAny <<= sal_False;
        }
        else if( pEntry->nWID == OWN_ATTR_FILLBMP_MODE )
        {
            SfxItemSet &rStyleSet = GetItemSet();

            XFillBmpStretchItem* pStretchItem = (XFillBmpStretchItem*)rStyleSet.GetItem(XATTR_FILLBMP_STRETCH);
            XFillBmpTileItem* pTileItem = (XFillBmpTileItem*)rStyleSet.GetItem(XATTR_FILLBMP_TILE);

            if( pStretchItem && pTileItem )
            {
                if( pTileItem->GetValue() )
                    aAny <<= BitmapMode_REPEAT;
                else if( pStretchItem->GetValue() )
                    aAny <<= BitmapMode_STRETCH;
                else
                    aAny <<= BitmapMode_NO_REPEAT;
            }
        }
        else
        {
            SfxItemSet aSet( GetPool().GetPool(),   pEntry->nWID, pEntry->nWID);

            const SfxPoolItem* pItem;
            SfxItemSet& rStyleSet = GetItemSet();

            if( rStyleSet.GetItemState( pEntry->nWID, sal_True, &pItem ) == SFX_ITEM_SET )
                aSet.Put(  *pItem );

            if( !aSet.Count() )
                aSet.Put( GetPool().GetPool().GetDefaultItem( pEntry->nWID ) );

            if(SvxUnoTextRangeBase::GetPropertyValueHelper( aSet, pEntry, aAny ))
                return aAny;

            // Hole Wert aus ItemSet
            aAny = SvxItemPropertySet_getPropertyValue( GetStylePropertySet(),pEntry, aSet );
        }

        if( *pEntry->pType != aAny.getValueType() )
        {
            // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
            if( ( *pEntry->pType == ::getCppuType((const sal_Int16*)0)) && aAny.getValueType() == ::getCppuType((const sal_Int32*)0) )
            {
                sal_Int32 nValue = 0;
                aAny >>= nValue;
                aAny <<= (sal_Int16)nValue;
            }
            else
            {
                OSL_FAIL("SvxShape::GetAnyForItem() Returnvalue has wrong Type!" );
            }
        }

        return aAny;
    }
}

// --------------------------------------------------------------------

void SAL_CALL SdStyleSheet::addPropertyChangeListener( const OUString& , const Reference< XPropertyChangeListener >&  ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException) {}
void SAL_CALL SdStyleSheet::removePropertyChangeListener( const OUString& , const Reference< XPropertyChangeListener >&  ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException) {}
void SAL_CALL SdStyleSheet::addVetoableChangeListener( const OUString& , const Reference< XVetoableChangeListener >&  ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException) {}
void SAL_CALL SdStyleSheet::removeVetoableChangeListener( const OUString& , const Reference< XVetoableChangeListener >&  ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException) {}

// --------------------------------------------------------------------
// XPropertyState
// --------------------------------------------------------------------

PropertyState SAL_CALL SdStyleSheet::getPropertyState( const OUString& PropertyName ) throw(UnknownPropertyException, RuntimeException)
{
    SolarMutexGuard aGuard;

    throwIfDisposed();

    const SfxItemPropertySimpleEntry* pEntry = getPropertyMapEntry( PropertyName );

    if( pEntry == NULL )
        throw UnknownPropertyException();

    if( pEntry->nWID == WID_STYLE_FAMILY )
    {
        return PropertyState_DIRECT_VALUE;
    }
    else if( pEntry->nWID == SDRATTR_TEXTDIRECTION )
    {
        return PropertyState_DEFAULT_VALUE;
    }
    else if( pEntry->nWID == OWN_ATTR_FILLBMP_MODE )
    {
        const SfxItemSet& rSet = GetItemSet();

        if( rSet.GetItemState( XATTR_FILLBMP_STRETCH, false ) == SFX_ITEM_SET ||
            rSet.GetItemState( XATTR_FILLBMP_TILE, false ) == SFX_ITEM_SET )
        {
            return PropertyState_DIRECT_VALUE;
        }
        else
        {
            return PropertyState_AMBIGUOUS_VALUE;
        }
    }
    else
    {
        SfxItemSet &rStyleSet = GetItemSet();

        PropertyState eState;

        switch( rStyleSet.GetItemState( pEntry->nWID, sal_False ) )
        {
        case SFX_ITEM_READONLY:
        case SFX_ITEM_SET:
            eState = PropertyState_DIRECT_VALUE;
            break;
        case SFX_ITEM_DEFAULT:
            eState = PropertyState_DEFAULT_VALUE;
            break;
        default:
            eState = PropertyState_AMBIGUOUS_VALUE;
            break;
        }

        // if a item is set, this doesn't mean we want it :)
        if( ( PropertyState_DIRECT_VALUE == eState ) )
        {
            switch( pEntry->nWID )
            {
            case XATTR_FILLBITMAP:
            case XATTR_FILLGRADIENT:
            case XATTR_FILLHATCH:
            case XATTR_FILLFLOATTRANSPARENCE:
            case XATTR_LINEEND:
            case XATTR_LINESTART:
            case XATTR_LINEDASH:
                {
                    NameOrIndex* pItem = (NameOrIndex*)rStyleSet.GetItem((sal_uInt16)pEntry->nWID);
                    if( ( pItem == NULL ) || ( pItem->GetName().Len() == 0) )
                        eState = PropertyState_DEFAULT_VALUE;
                }
            }
        }

        return eState;
    }
}

// --------------------------------------------------------------------

Sequence< PropertyState > SAL_CALL SdStyleSheet::getPropertyStates( const Sequence< OUString >& aPropertyName ) throw(UnknownPropertyException, RuntimeException)
{
    SolarMutexGuard aGuard;

    throwIfDisposed();

    sal_Int32 nCount = aPropertyName.getLength();
    const OUString* pNames = aPropertyName.getConstArray();

    Sequence< PropertyState > aPropertyStateSequence( nCount );
    PropertyState* pState = aPropertyStateSequence.getArray();

    while( nCount-- )
        *pState++ = getPropertyState( *pNames++ );

    return aPropertyStateSequence;
}

// --------------------------------------------------------------------

void SAL_CALL SdStyleSheet::setPropertyToDefault( const OUString& PropertyName ) throw(UnknownPropertyException, RuntimeException)
{
    SolarMutexGuard aGuard;

    throwIfDisposed();

    const SfxItemPropertySimpleEntry* pEntry = getPropertyMapEntry( PropertyName );
    if( pEntry == NULL )
        throw UnknownPropertyException();

    SfxItemSet &rStyleSet = GetItemSet();

    if( pEntry->nWID == OWN_ATTR_FILLBMP_MODE )
    {
        rStyleSet.ClearItem( XATTR_FILLBMP_STRETCH );
        rStyleSet.ClearItem( XATTR_FILLBMP_TILE );
    }
    else
    {
        rStyleSet.ClearItem( pEntry->nWID );
    }
    Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
}

// --------------------------------------------------------------------

Any SAL_CALL SdStyleSheet::getPropertyDefault( const OUString& aPropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    throwIfDisposed();

    const SfxItemPropertySimpleEntry* pEntry = getPropertyMapEntry( aPropertyName );
    if( pEntry == NULL )
        throw UnknownPropertyException();
    Any aRet;
    if( pEntry->nWID == WID_STYLE_FAMILY )
    {
        aRet <<= GetFamilyString(nFamily);
    }
    else if( pEntry->nWID == SDRATTR_TEXTDIRECTION )
    {
        aRet <<= sal_False;
    }
    else if( pEntry->nWID == OWN_ATTR_FILLBMP_MODE )
    {
        aRet <<= BitmapMode_REPEAT;
    }
    else
    {
        SfxItemPool& rMyPool = GetPool().GetPool();
        SfxItemSet aSet( rMyPool,   pEntry->nWID, pEntry->nWID);
        aSet.Put( rMyPool.GetDefaultItem( pEntry->nWID ) );
        aRet = SvxItemPropertySet_getPropertyValue( GetStylePropertySet(), pEntry, aSet );
    }
    return aRet;
}

// --------------------------------------------------------------------

/** this is used because our property map is not sorted yet */
const SfxItemPropertySimpleEntry* SdStyleSheet::getPropertyMapEntry( const OUString& rPropertyName ) const throw()
{
    return GetStylePropertySet().getPropertyMapEntry(rPropertyName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
