/*************************************************************************
 *
 *  $RCSfile: style.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:01 $
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

#include <smplhint.hxx>
#include <poolitem.hxx>
#include <itemset.hxx>
#include <itempool.hxx>
#include <poolio.hxx>
#include <filerec.hxx>
#include <itemiter.hxx>
#include "style.hxx"

#define STYLESTREAM             "SfxStyleSheets"
#define STYLESTREAM_VERSION     USHORT(50)



TYPEINIT0(SfxStyleSheetBase)

TYPEINIT3(SfxStyleSheet, SfxStyleSheetBase, SfxListener, SfxBroadcaster)


//=========================================================================

TYPEINIT1(SfxStyleSheetHint, SfxHint);
TYPEINIT1(SfxStyleSheetHintExtended, SfxStyleSheetHint);
TYPEINIT1(SfxStyleSheetPoolHint, SfxHint);

SfxStyleSheetHintExtended::SfxStyleSheetHintExtended
(
    USHORT              nAction,        // SFX_STYLESHEET_... (s.o.)
    const String&       rOldName,
    SfxStyleSheetBase&  rStyleSheet     // geh"ort weiterhin dem Aufrufer
)
:   SfxStyleSheetHint( nAction, rStyleSheet ),
    aName( rOldName )
{}

//-------------------------------------------------------------------------

SfxStyleSheetHint::SfxStyleSheetHint
(
    USHORT              nAction,        // SFX_STYLESHEET_... (s.o.)
    SfxStyleSheetBase&  rStyleSheet     // geh"ort weiterhin dem Aufrufer
)
:   pStyleSh( &rStyleSheet ),
    nHint( nAction )
{}

//=========================================================================

class SfxStyleSheetBasePool_Impl
{
  public:
    SfxStyles aStyles;
    SfxStyleSheetIterator *pIter;
    SfxStyleSheetBasePool_Impl() : pIter(0){}
    ~SfxStyleSheetBasePool_Impl(){delete pIter;}
};


//////////////////////////// SfxStyleSheetBase ///////////////////////////////

// Konstruktoren

SfxStyleSheetBase::SfxStyleSheetBase
    ( const XubString& rName, SfxStyleSheetBasePool& r, SfxStyleFamily eFam, USHORT mask )
    : aName( rName ), aParent(), aFollow( rName ),
    nFamily( eFam ), nMask(mask), rPool( r ),
    nHelpId( 0 ), pSet( NULL ), bMySet( FALSE )
{
}

SfxStyleSheetBase::SfxStyleSheetBase( const SfxStyleSheetBase& r )
    : aName( r.aName ), aParent( r.aParent ), aFollow( r.aFollow ),
      nFamily( r.nFamily ),
      nMask( r.nMask ),
      rPool( r.rPool ),
      nHelpId( r.nHelpId ), aHelpFile( r.aHelpFile ), bMySet( r.bMySet )
{
    if( r.pSet )
        pSet = bMySet ? new SfxItemSet( *r.pSet ) : r.pSet;
    else
        pSet = NULL;
}

SfxStyleSheetBase::~SfxStyleSheetBase()
{
    if( bMySet )
    {
        delete pSet;
        pSet = 0;
    }
}

USHORT SfxStyleSheetBase::GetVersion() const
{
    return 0x0000;
}

// Namen aendern

const XubString& SfxStyleSheetBase::GetName() const
{
    return aName;
}

BOOL SfxStyleSheetBase::SetName( const XubString& rName )
{
    if(rName.Len() == 0)
        return FALSE;
    if( aName != rName )
    {
        String aOldName = aName;
        SfxStyleSheetBase *pOther = rPool.Find( rName, nFamily ) ;
        if ( pOther && pOther != this )
            return FALSE;

        SfxStyleFamily eTmpFam=rPool.GetSearchFamily();
        USHORT nTmpMask=rPool.GetSearchMask();

        rPool.SetSearchMask(nFamily);

        if ( aName.Len() )
            rPool.ChangeParent( aName, rName, FALSE );
        if ( aFollow.Equals( aName ) )
            aFollow = rName;
        aName = rName;
        rPool.SetSearchMask(eTmpFam, nTmpMask);
        rPool.Broadcast( SfxStyleSheetHintExtended(
            SFX_STYLESHEET_MODIFIED, aOldName, *this ) );
    }
    return TRUE;
}

// Parent aendern

const XubString& SfxStyleSheetBase::GetParent() const
{
    return aParent;
}

BOOL SfxStyleSheetBase::SetParent( const XubString& rName )
{
    if( aParent != rName )
    {
        SfxStyleSheetBase* pIter = rPool.Find(rName, nFamily);
        if( rName.Len() && !pIter )
        {
            DBG_ERROR( "StyleSheet-Parent nicht gefunden" );
            return FALSE;
        }
        // rekursive Verknuepfungen verhindern
        if( aName.Len() )
            while(pIter)
            {
                if(pIter->GetName() == aName && aName != rName)
                    return FALSE;
                pIter = rPool.Find(pIter->GetParent(), nFamily);
            }
        aParent = rName;
    }
    rPool.Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_MODIFIED, *this ) );
    return TRUE;
}

// Follow aendern

const XubString& SfxStyleSheetBase::GetFollow() const
{
    return aFollow;
}

BOOL SfxStyleSheetBase::SetFollow( const XubString& rName )
{
    if( aFollow != rName )
    {
        if( !rPool.Find( rName, nFamily ) )
        {
            DBG_ERROR( "StyleSheet-Follow nicht gefunden" );
            return FALSE;
        }
        aFollow = rName;
    }
    rPool.Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_MODIFIED, *this ) );
    return TRUE;
}

// Itemset setzen. Die Dflt-Implementation legt ein neues Set an.

SfxItemSet& SfxStyleSheetBase::GetItemSet()
{
    if( !pSet )
    {
        pSet = new SfxItemSet( rPool.GetPool() );
        bMySet = TRUE;
    }
    return *pSet;
}

// Hilfe-Datei und -ID setzen und abfragen

ULONG SfxStyleSheetBase::GetHelpId( String& rFile )
{
    rFile = aHelpFile;
    return nHelpId;
}

void SfxStyleSheetBase::SetHelpId( const String& rFile, ULONG nId )
{
    aHelpFile = rFile;
    nHelpId = nId;
}

// Folgevorlage m"oglich? Default: Ja

BOOL SfxStyleSheetBase::HasFollowSupport() const
{
    return TRUE;
}

// Basisvorlage m"oglich? Default: Ja

BOOL SfxStyleSheetBase::HasParentSupport() const
{
    return TRUE;
}

// Basisvorlage uf NULL setzen m"oglich? Default: Nein

BOOL SfxStyleSheetBase::HasClearParentSupport() const
{
    return FALSE;
}

// Defaultmaessig sind alle StyleSheets Used

BOOL SfxStyleSheetBase::IsUsed() const
{
    return TRUE;
}

// eingestellte Attribute ausgeben


XubString SfxStyleSheetBase::GetDescription()
{
#if SUPD >= 368
    return GetDescription( SFX_MAPUNIT_CM );

/*!!!
    if ( !pSet )
        GetItemSet();
    SfxItemIter aIter( *pSet );
    XubString aDesc;
    const SfxPoolItem* pItem = aIter.FirstItem();

    while ( pItem )
    {
        XubString aItemPresentation;

        if ( !IsInvalidItem( pItem ) &&
             rPool.GetPool().GetPresentation(
                *pItem, SFX_ITEM_PRESENTATION_COMPLETE,
                SFX_MAPUNIT_CM, aItemPresentation ) )
        {
            if ( aDesc.Len() && aItemPresentation.Len() )
#ifndef ENABLEUNICODE
                aDesc += " + ";
#else
                aDesc += L" + ";
#endif
            if ( aItemPresentation.Len() )
                aDesc += aItemPresentation;
        }
        pItem = aIter.NextItem();
    }
    return aDesc;
*/
}

// eingestellte Attribute ausgeben

XubString SfxStyleSheetBase::GetDescription( SfxMapUnit eMetric )
{
#endif
    SfxItemIter aIter( GetItemSet() );
    XubString aDesc;
    const SfxPoolItem* pItem = aIter.FirstItem();

    while ( pItem )
    {
        XubString aItemPresentation;

#if SUPD >= 368
        if ( !IsInvalidItem( pItem ) &&
             rPool.GetPool().GetPresentation(
                *pItem, SFX_ITEM_PRESENTATION_COMPLETE,
                eMetric, aItemPresentation ) )
#else
        if ( !IsInvalidItem( pItem ) &&
             rPool.GetPool().GetPresentation(
                *pItem, SFX_ITEM_PRESENTATION_COMPLETE,
                SFX_MAPUNIT_CM, aItemPresentation ) )
#endif
        {
            if ( aDesc.Len() && aItemPresentation.Len() )
                aDesc.AppendAscii(RTL_CONSTASCII_STRINGPARAM(" + "));
            if ( aItemPresentation.Len() )
                aDesc += aItemPresentation;
        }
        pItem = aIter.NextItem();
    }
    return aDesc;
}

/////////////////////////// SfxStyleSheetIterator ///////////////////////////////

SfxStyleFamily SfxStyleSheetIterator::GetSearchFamily() const
{
    return nSearchFamily;
}

inline BOOL SfxStyleSheetIterator::IsTrivialSearch()
{
    return nMask == 0xFFFF && GetSearchFamily() == SFX_STYLE_FAMILY_ALL;
}

BOOL SfxStyleSheetIterator::DoesStyleMatch(SfxStyleSheetBase *pStyle)
{
    return ((GetSearchFamily() == SFX_STYLE_FAMILY_ALL) ||
            ( pStyle->GetFamily() == GetSearchFamily() ))
        && (( pStyle->GetMask() & ( GetSearchMask() & ~SFXSTYLEBIT_USED )) ||
            ( bSearchUsed ? pStyle->IsUsed() : FALSE ) ||
            GetSearchMask() == SFXSTYLEBIT_ALL );
}


SfxStyleSheetIterator::SfxStyleSheetIterator(SfxStyleSheetBasePool *pBase,
                                             SfxStyleFamily eFam, USHORT n)
{
    pBasePool=pBase;
    nSearchFamily=eFam;
    bSearchUsed=FALSE;
        int nn=SFXSTYLEBIT_USED;
        int nnn=SFXSTYLEBIT_ALL;
        if((n != SFXSTYLEBIT_ALL ) && ((n & SFXSTYLEBIT_USED) == SFXSTYLEBIT_USED))
    {
        bSearchUsed = TRUE;
        n &= ~SFXSTYLEBIT_USED;
    }
    nMask=n;
}

SfxStyleSheetIterator::~SfxStyleSheetIterator()
{
}


USHORT SfxStyleSheetIterator::Count()
{
    USHORT n = 0;
    if( IsTrivialSearch())
        n = (USHORT) pBasePool->aStyles.Count();
    else
        for(USHORT i=0; i<pBasePool->aStyles.Count(); i++)
        {
            SfxStyleSheetBase* pStyle = pBasePool->aStyles.GetObject(i);
            if(DoesStyleMatch(pStyle))
                n++;
        }
    return n;
}

SfxStyleSheetBase* SfxStyleSheetIterator::operator[](USHORT nIdx)
{
    if( IsTrivialSearch())
        return pBasePool->aStyles.GetObject(nIdx);

    USHORT z = 0;
    for(USHORT n=0; n<pBasePool->aStyles.Count(); n++)
    {
        SfxStyleSheetBase* pStyle = pBasePool->aStyles.GetObject(n);
        if( DoesStyleMatch(pStyle))
        {
            if(z == nIdx)
            {
                nAktPosition=n;
                return pAktStyle=pStyle;
            }
            ++z;
        }
    }
    DBG_ERROR("falscher Index");
    return 0;
}

SfxStyleSheetBase* SfxStyleSheetIterator::First()
{
    INT32 nIdx = -1;

    if ( IsTrivialSearch() && pBasePool->aStyles.Count() )
        nIdx = 0;
    else
        for( USHORT n = 0; n < pBasePool->aStyles.Count(); n++ )
        {
            SfxStyleSheetBase* pStyle = pBasePool->aStyles.GetObject(n);

            if ( DoesStyleMatch( pStyle ) )
            {
                nIdx = n;
                break;
            }
        }

    if ( nIdx != -1 )
    {
        nAktPosition = (USHORT)nIdx;
        return pAktStyle = pBasePool->aStyles.GetObject(nIdx);
    }
    return 0;
}


SfxStyleSheetBase* SfxStyleSheetIterator::Next()
{
    INT32 nIdx = -1;

    if ( IsTrivialSearch() &&
         (USHORT)pBasePool->aStyles.Count() > nAktPosition + 1 )
        nIdx = nAktPosition + 1;
    else
        for( USHORT n = nAktPosition + 1; n < pBasePool->aStyles.Count(); n++ )
        {
            SfxStyleSheetBase* pStyle = pBasePool->aStyles.GetObject(n);

            if ( DoesStyleMatch( pStyle ) )
            {
                nIdx = n;
                break;
            }
        }

    if ( nIdx != -1 )
    {
        nAktPosition = (USHORT)nIdx;
        return pAktStyle = pBasePool->aStyles.GetObject(nIdx);
    }
    return 0;
}


SfxStyleSheetBase* SfxStyleSheetIterator::Find(const XubString& rStr)
{
    for ( USHORT n = 0; n < pBasePool->aStyles.Count(); n++ )
    {
        SfxStyleSheetBase* pStyle = pBasePool->aStyles.GetObject(n);

        if ( DoesStyleMatch( pStyle ) &&
             pStyle->GetName().Equals( rStr ) )
        {
            nAktPosition = n;
            return pAktStyle = pStyle;
        }
    }
    return 0;
}


USHORT SfxStyleSheetIterator::GetSearchMask() const
{
    USHORT mask = nMask;

    if ( bSearchUsed )
        mask |= SFXSTYLEBIT_USED;
    return mask;
}

/////////////////////////// SfxStyleSheetBasePool ///////////////////////////////


void SfxStyleSheetBasePool::Replace(
    SfxStyleSheetBase& rSource, SfxStyleSheetBase& rTarget )
{
    rTarget.SetFollow( rSource.GetFollow() );
    rTarget.SetParent( rSource.GetParent() );
    SfxItemSet& rSourceSet = rSource.GetItemSet();
    SfxItemSet& rTargetSet = rTarget.GetItemSet();
    rTargetSet.Intersect( rSourceSet );
    rTargetSet.Put( rSourceSet );
}

SfxStyleSheetIterator& SfxStyleSheetBasePool::GetIterator_Impl()
{
    SfxStyleSheetIterator*& rpIter = pImp->pIter;
    if( !rpIter || rpIter->GetSearchMask() != nMask ||
        rpIter->GetSearchFamily() != nSearchFamily )
    {
        delete rpIter;
#if SUPD > 363
        rpIter = CreateIterator( nSearchFamily, nMask );
#else
        rpIter = new SfxStyleSheetIterator( this, nSearchFamily, nMask );
#endif
    }
    return *rpIter;
}


SfxStyleSheetBasePool::SfxStyleSheetBasePool( SfxItemPool& r )
:   rPool(r), aAppName(r.GetName()),
    nMask(0xFFFF), nSearchFamily(SFX_STYLE_FAMILY_PARA)
{
    pImp = new SfxStyleSheetBasePool_Impl;
}

SfxStyleSheetBasePool::SfxStyleSheetBasePool( const SfxStyleSheetBasePool& r )
:   rPool(r.rPool),
    nMask( r.nMask ), nSearchFamily(r.nSearchFamily),aAppName(r.aAppName)
{
    pImp = new SfxStyleSheetBasePool_Impl;
    *this += r;
}

SfxStyleSheetBasePool::~SfxStyleSheetBasePool()
{
    Broadcast( SfxSimpleHint(SFX_HINT_DYING) );
    Clear();
    delete pImp;
}

BOOL SfxStyleSheetBasePool::SetParent(SfxStyleFamily eFam, const XubString& rStyle, const XubString& rParent)
{
    SfxStyleSheetIterator aIter(this,eFam,SFXSTYLEBIT_ALL);
    SfxStyleSheetBase *pStyle =
        aIter.Find(rStyle);
    DBG_ASSERT(pStyle, "Vorlage nicht gefunden. Writer mit Solar <2541??");
    if(pStyle)
        return pStyle->SetParent(rParent);
    else
        return FALSE;
}


void SfxStyleSheetBasePool::SetSearchMask(SfxStyleFamily eFam, USHORT n)
{
    nSearchFamily = eFam; nMask = n;
}

USHORT SfxStyleSheetBasePool::GetSearchMask() const
{
    return nMask;
}


// Der Name des Streams

String SfxStyleSheetBasePool::GetStreamName()
{
    return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(STYLESTREAM));
}

/////////////////////////////////// Factory ////////////////////////////////


#if SUPD > 363

SfxStyleSheetIterator* SfxStyleSheetBasePool::CreateIterator
(
 SfxStyleFamily eFam,
 USHORT mask
)
{
    return new SfxStyleSheetIterator(this,eFam,mask);
}

#endif

SfxStyleSheetBase* SfxStyleSheetBasePool::Create
(
    const XubString& rName,
    SfxStyleFamily eFam,
    USHORT mask
)
{
    return new SfxStyleSheetBase( rName, *this, eFam, mask );
}

SfxStyleSheetBase* SfxStyleSheetBasePool::Create( const SfxStyleSheetBase& r )
{
    return new SfxStyleSheetBase( r );
}

SfxStyleSheetBase& SfxStyleSheetBasePool::Make( const XubString& rName,
                            SfxStyleFamily eFam, USHORT mask, USHORT nPos)
{
    DBG_ASSERT( eFam != SFX_STYLE_FAMILY_ALL, "FamilyAll als Familie nicht erlaubt" );

    SfxStyleSheetIterator aIter(this, eFam, mask);
    SfxStyleSheetBase* p = aIter.Find( rName );
    DBG_ASSERT( !p, "StyleSheet bereits vorhanden" );
    SfxStyleSheetIterator& rIter = GetIterator_Impl();

    if( !p )
    {
        p = Create( rName, eFam, mask );
        if(0xffff == nPos || nPos == aStyles.Count() ||
           nPos == rIter.Count())
            aStyles.Insert( p, aStyles.Count() );
        else
        {
            rIter[nPos];
            aStyles.Insert(p, rIter.GetPos());
        }
        Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_CREATED, *p ) );
    }
    return *p;
}

/////////////////////////////// Kopieren ///////////////////////////////////

// Hilfsroutine: Falls eine Vorlage dieses Namens existiert, wird
// sie neu erzeugt. Alle Vorlagen, die diese Vorlage zum Parent haben,
// werden umgehaengt.

SfxStyleSheetBase& SfxStyleSheetBasePool::Add( SfxStyleSheetBase& rSheet )
{
    SfxStyleSheetIterator aIter(this, rSheet.GetFamily(), nMask);
    SfxStyleSheetBase* pOld = aIter.Find( rSheet.GetName() );
    Erase( pOld );
    SfxStyleSheetBase* pNew = Create( rSheet );
    aStyles.Insert( pNew, aStyles.Count() );
    Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_CHANGED, *pNew ) );
    return *pNew;
}

SfxStyleSheetBasePool& SfxStyleSheetBasePool::operator=( const SfxStyleSheetBasePool& r )
{
    if( &r != this )
    {
        Clear();
        *this += r;
    }
    return *this;
}

SfxStyleSheetBasePool& SfxStyleSheetBasePool::operator+=( const SfxStyleSheetBasePool& rP )
{
    if( &rP != this )
    {
        // kopieren
        SfxStyleSheetBasePool& r = (SfxStyleSheetBasePool&) rP;
        for( SfxStyleSheetBase* p = r.aStyles.First(); p; p = r.aStyles.Next() )
            Add(*p);

    }
    return *this;
}

//////////////////////////////// Suchen ////////////////////////////////////

USHORT SfxStyleSheetBasePool::Count()
{
    return GetIterator_Impl().Count();
}

SfxStyleSheetBase *SfxStyleSheetBasePool::operator[](USHORT nIdx)
{
    return GetIterator_Impl()[nIdx];
}

SfxStyleSheetBase* SfxStyleSheetBasePool::Find(const XubString& rName,
                                               SfxStyleFamily eFam,
                                               USHORT mask)
{
    SfxStyleSheetIterator aIter(this,eFam,mask);
    return aIter.Find(rName);
}

SfxStyleSheetBase* SfxStyleSheetBasePool::First()
{
    return GetIterator_Impl().First();
}

SfxStyleSheetBase* SfxStyleSheetBasePool::Next()
{
    return GetIterator_Impl().Next();
}

//////////////////////////////// Loeschen /////////////////////////////////

SfxStyleSheetBase* SfxStyleSheetBasePool::Remove( SfxStyleSheetBase* p )
{
    if( p )
    {
        // Alle Styles umsetzen, deren Parent dieser hier ist
        ChangeParent( p->GetName(), p->GetParent() );
        aStyles.Remove( p );
        Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_ERASED, *p ) );
    }
    return p;
}

void SfxStyleSheetBasePool::Erase( SfxStyleSheetBase* p )
{
    if( p )
    {
        Remove(p);
        delete p;
    }
}

void SfxStyleSheetBasePool::Insert( SfxStyleSheetBase* p )
{
    DBG_ASSERT( p, "Kein StyleSheet?" );
    SfxStyleSheetIterator aIter(
        this, p->GetFamily(), p->GetMask());
    SfxStyleSheetBase* pOld = aIter.Find( p->GetName() );
    DBG_ASSERT( !pOld, "StyleSheet bereits vorhanden" );
    if( p->GetParent().Len() )
    {
        pOld = aIter.Find( p->GetParent() );
        DBG_ASSERT( pOld, "Parent nicht mehr vorhanden" );
    }
    aStyles.Insert( p, aStyles.Count() );
    Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_CREATED, *p ) );
}

void SfxStyleSheetBasePool::Clear()
{
    while( aStyles.Count() )
    {
        SfxStyleSheetBase* p = aStyles.First();
        aStyles.Remove( p );
        Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_ERASED, *p ) );
        delete p;
    }
}

/////////////////////////// Parents umsetzen ////////////////////////////////

void SfxStyleSheetBasePool::ChangeParent(const XubString& rOld,
                                         const XubString& rNew,
                                         BOOL bVirtual)
{
    const ULONG nPos = aStyles.GetCurPos();
    const USHORT nTmpMask = GetSearchMask();
    SetSearchMask(GetSearchFamily(), 0xffff);
    for( SfxStyleSheetBase* p = First(); p; p = Next() )
    {
        if( p->GetParent().Equals( rOld ) )
        {
            if(bVirtual)
                p->SetParent( rNew );
            else
                p->aParent = rNew;
        }
    }
    SetSearchMask(GetSearchFamily(), nTmpMask);
    aStyles.Seek(nPos);
}

/////////////////////////// Laden/Speichern /////////////////////////////////

void SfxStyleSheetBase::Load( SvStream&, USHORT )
{
}

void SfxStyleSheetBase::Store( SvStream& )
{
}


BOOL SfxStyleSheetBasePool::Load( SvStream& rStream )
{
    // alte Version?
    if ( !rPool.IsVer2_Impl() )
        return Load1_Impl( rStream );

    // gesamten StyleSheetPool in neuer Version aus einem MiniRecord lesen
    SfxMiniRecordReader aPoolRec( &rStream, SFX_STYLES_REC );

    // Header-Record lesen
    short nCharSet;
    if ( !rStream.GetError() )
    {
        SfxSingleRecordReader aHeaderRec( &rStream, SFX_STYLES_REC_HEADER );
        if ( !aHeaderRec.IsValid() )
            return FALSE;

        aAppName = rPool.GetName();
        rStream >> nCharSet;
    }

    // Styles-Record lesen
    if ( !rStream.GetError() )
    {
        SfxMultiRecordReader aStylesRec( &rStream, SFX_STYLES_REC_STYLES );
        if ( !aStylesRec.IsValid() )
            return FALSE;

        USHORT nCount;
        for ( nCount = 0; aStylesRec.GetContent(); nCount++ )
        {
            // kann nicht mehr weiterlesen?
            if ( rStream.GetError() )
                break;

            // Globale Teile
            XubString aName, aParent, aFollow;
            String aHelpFile;
            USHORT nFamily, nMask,nCount;
            ULONG nHelpId;
            rStream.ReadByteString(aName, rtl_TextEncoding(nCharSet));
            rStream.ReadByteString(aParent, rtl_TextEncoding(nCharSet));
            rStream.ReadByteString(aFollow, rtl_TextEncoding(nCharSet));
            rStream >> nFamily >> nMask;
            SfxPoolItem::readByteString(rStream, aHelpFile);
            rStream >> nHelpId;

#ifndef ENABLEUNICODE
            // bei Unicode keine Konvertierung erforderlich
            aName.Convert( (CharSet) nCharSet );
            aParent.Convert( (CharSet) nCharSet );
            aFollow.Convert( (CharSet) nCharSet );

            //  #72939# When loading from a different CharSet, several style
            //  names may have been converted to the same string, so the styles
            //  can't be inserted. If a style with this name already exists,
            //  generate an internal name so the information from the styles
            //  isn't lost.
            if ( nCharSet != ::GetSystemCharSet() &&
                    Find( aName, (SfxStyleFamily)nFamily ) != NULL )
            {
                DBG_WARNING("style has to be renamed");
                USHORT nMax = aStyles.Count() + 1;
                for ( USHORT nAdd=1; nAdd<=nMax; nAdd++ )
                {
                    aName = '_';
                    aName += nAdd;
                    if ( Find( aName, (SfxStyleFamily)nFamily ) == NULL )
                        break;
                }
            }
#endif
            SfxStyleSheetBase& rSheet = Make( aName, (SfxStyleFamily)nFamily , nMask);
            rSheet.SetHelpId( aHelpFile, nHelpId );
            // Hier erst einmal Parent und Follow zwischenspeichern
            rSheet.aParent = aParent;
            rSheet.aFollow = aFollow;
            UINT32 nPos = rStream.Tell();
            rStream >> nCount;
            if(nCount)
            {
                rStream.Seek( nPos );
                // Das Laden des ItemSets bedient sich der Methode GetItemSet(),
                // damit eigene ItemSets untergeschoben werden koennen
                SfxItemSet& rSet = rSheet.GetItemSet();
                rSet.ClearItem();
    //!         SfxItemSet aTmpSet( *pTmpPool );
                /*!aTmpSet*/ rSet.Load( rStream );
                //! rSet.Put( aTmpSet );
            }
            // Lokale Teile
            UINT32 nSize;
            USHORT nVer;
            rStream >> nVer >> nSize;
            nPos = rStream.Tell() + nSize;
            rSheet.Load( rStream, nVer );
            rStream.Seek( nPos );
        }

        //  #72939# only loop through the styles that were really inserted
        nCount = aStyles.Count();

        //! delete pTmpPool;
        // Jetzt Parent und Follow setzen. Alle Sheets sind geladen.
        // Mit Setxxx() noch einmal den String eintragen, da diese
        // virtuellen Methoden evtl. ueberlagert sind.
        for ( USHORT i = 0; i < nCount; i++ )
        {
            SfxStyleSheetBase* p = aStyles.GetObject( i );
            XubString aText = p->aParent;
            p->aParent.Erase();
            p->SetParent( aText );
            aText = p->aFollow;
            p->aFollow.Erase();
            p->SetFollow( aText );
        }
    }

    // alles klar?
    return BOOL( rStream.GetError() == SVSTREAM_OK );
}

BOOL SfxStyleSheetBasePool::Load1_Impl( SvStream& rStream )
{
    aAppName = rPool.GetName();
    USHORT nVersion;
    short nCharSet;
    rStream >> nVersion;

    if(nVersion!=STYLESTREAM_VERSION)
        nCharSet=nVersion;
    else
        rStream >> nCharSet;

    USHORT nCount;
    rStream >> nCount;
    USHORT i;
    for ( i = 0; i < nCount; i++ )
    {
        // kann nicht mehr weiterlesen?
        if ( rStream.GetError() )
        {
            nCount = i;
            break;
        }

        // Globale Teile
        XubString aName, aParent, aFollow;
        String aHelpFile;
        USHORT nFamily, nMask,nCount;
        ULONG nHelpId;
        rStream.ReadByteString(aName, rtl_TextEncoding(nCharSet));
        rStream.ReadByteString(aParent, rtl_TextEncoding(nCharSet));
        rStream.ReadByteString(aFollow, rtl_TextEncoding(nCharSet));
        rStream >> nFamily >> nMask;
        SfxPoolItem::readByteString(rStream, aHelpFile);
        if(nVersion!=STYLESTREAM_VERSION)
        {
            USHORT nTmpHelpId;
            rStream >> nTmpHelpId;
            nHelpId=nTmpHelpId;
        }
        else
            rStream >> nHelpId;

#ifndef ENABLEUNICODE
        // bei Unicode keine Konvertierung erforderlich
        aName.Convert( (CharSet) nCharSet );
        aParent.Convert( (CharSet) nCharSet );
        aFollow.Convert( (CharSet) nCharSet );
#endif
        SfxStyleSheetBase& rSheet = Make( aName, (SfxStyleFamily)nFamily , nMask);
        rSheet.SetHelpId( aHelpFile, nHelpId );
        // Hier erst einmal Parent und Follow zwischenspeichern
        rSheet.aParent = aParent;
        rSheet.aFollow = aFollow;
        UINT32 nPos = rStream.Tell();
        rStream >> nCount;
        if(nCount) {
            rStream.Seek( nPos );
            // Das Laden des ItemSets bedient sich der Methode GetItemSet(),
            // damit eigene ItemSets untergeschoben werden koennen
            SfxItemSet& rSet = rSheet.GetItemSet();
            rSet.ClearItem();
//!         SfxItemSet aTmpSet( *pTmpPool );
            /*!aTmpSet*/ rSet.Load( rStream );
            //! rSet.Put( aTmpSet );
        }
        // Lokale Teile
        UINT32 nSize;
        USHORT nVer;
        rStream >> nVer >> nSize;
        nPos = rStream.Tell() + nSize;
        rSheet.Load( rStream, nVer );
        rStream.Seek( nPos );
    }

    //! delete pTmpPool;
    // Jetzt Parent und Follow setzen. Alle Sheets sind geladen.
    // Mit Setxxx() noch einmal den String eintragen, da diese
    // virtuellen Methoden evtl. ueberlagert sind.
    for ( i = 0; i < nCount; i++ )
    {
        SfxStyleSheetBase* p = aStyles.GetObject( i );
        XubString aText = p->aParent;
        p->aParent.Erase();
        p->SetParent( aText );
        aText = p->aFollow;
        p->aFollow.Erase();
        p->SetFollow( aText );
    }
    return BOOL( rStream.GetError() == SVSTREAM_OK );
}

BOOL SfxStyleSheetBasePool::Store( SvStream& rStream, BOOL bUsed )
{
    // den ganzen StyleSheet-Pool in einen Mini-Record
    SfxMiniRecordWriter aPoolRec( &rStream, SFX_STYLES_REC );

    // Erst einmal die Dummies rauszaehlen; die werden nicht gespeichert
    USHORT nCount = 0;
    for( SfxStyleSheetBase* p = First(); p; p = Next() )
    {
        if(!bUsed || p->IsUsed())
            nCount++;
    }

    // einen Header-Record vorweg
    rtl_TextEncoding eCharSet
        = ::GetStoreCharSet( rStream.GetStreamCharSet() );
    {
        SfxSingleRecordWriter aHeaderRec( &rStream,
                SFX_STYLES_REC_HEADER,
                STYLESTREAM_VERSION );
        rStream << (short) eCharSet;
    }

    // die StyleSheets in einen MultiVarRecord
    {
        SfxMultiVarRecordWriter aStylesRec( &rStream, SFX_STYLES_REC_STYLES, 0 );
        for( SfxStyleSheetBase* p = First(); p; p = Next() )
        {
            if(!bUsed || p->IsUsed())
            {
                aStylesRec.NewContent();

                // Globale Teile speichern
                String aHelpFile;
                ULONG nHelpId = p->GetHelpId( aHelpFile );
                rStream.WriteByteString(p->GetName(), eCharSet);
                rStream.WriteByteString(p->GetParent(), eCharSet);
                rStream.WriteByteString(p->GetFollow(), eCharSet);
                rStream << (USHORT)p->GetFamily() << p->GetMask();
                SfxPoolItem::writeByteString(rStream, aHelpFile);
                rStream << nHelpId;
                if(p->pSet)
                    p->pSet->Store( rStream );
                else
                    rStream << (USHORT)0;

                // Lokale Teile speichern
                // Vor dem lokalen Teil wird die Laenge der lokalen Daten
                // als UINT32 sowie die Versionsnummer gespeichert.
                rStream << (USHORT) p->GetVersion();
                ULONG nPos1 = rStream.Tell();
                rStream << (UINT32) 0;
                p->Store( rStream );
                ULONG nPos2 = rStream.Tell();
                rStream.Seek( nPos1 );
                rStream << (UINT32) ( nPos2 - nPos1 - sizeof( UINT32 ) );
                rStream.Seek( nPos2 );
                if( rStream.GetError() != SVSTREAM_OK )
                    break;
            }
        }
    }

    return BOOL( rStream.GetError() == SVSTREAM_OK );
}

SfxItemPool& SfxStyleSheetBasePool::GetPool()
{
    return rPool;
}

const SfxItemPool& SfxStyleSheetBasePool::GetPool() const
{
    return rPool;
}

/////////////////////// SfxStyleSheet /////////////////////////////////

SfxStyleSheet::SfxStyleSheet(const XubString &rName,
                             SfxStyleSheetBasePool& rPool,
                             SfxStyleFamily eFam,
                             USHORT mask ):
    SfxStyleSheetBase(rName, rPool, eFam, mask)
{}

SfxStyleSheet::SfxStyleSheet(const SfxStyleSheet& rStyle) :
    SfxStyleSheetBase(rStyle)
{}

SfxStyleSheet::~SfxStyleSheet()
{
    Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_INDESTRUCTION, *this ) );
}


BOOL SfxStyleSheet::SetParent( const XubString& rName )
{
    if(aParent == rName)
        return TRUE;
    const XubString aOldParent(aParent);
    if(SfxStyleSheetBase::SetParent(rName)) {
            // aus der Benachrichtigungskette des alten
            // Parents gfs. austragen
        if(aOldParent.Len()) {
            SfxStyleSheet *pParent = (SfxStyleSheet *)rPool.Find(aOldParent, nFamily, 0xffff);
            if(pParent)
                EndListening(*pParent);
        }
            // in die Benachrichtigungskette des neuen
            // Parents eintragen
        if(aParent.Len()) {
            SfxStyleSheet *pParent = (SfxStyleSheet *)rPool.Find(aParent, nFamily, 0xffff);
            if(pParent)
                StartListening(*pParent);
        }
        return TRUE;
    }
    return FALSE;
}

// alle Zuhoerer benachtichtigen

void SfxStyleSheet::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
                           const SfxHint& rHint, const TypeId& rHintType )
{
    SFX_FORWARD(rBC, rBCType, rHint, rHintType);
}

//////////////////////// SfxStyleSheetPool ///////////////////////////////

SfxStyleSheetPool::SfxStyleSheetPool( SfxItemPool& rSet):
    SfxStyleSheetBasePool(rSet)
{}

/////////////////////////////////// Factory ////////////////////////////////

SfxStyleSheetBase* SfxStyleSheetPool::Create( const XubString& rName,
                                    SfxStyleFamily eFam, USHORT mask )
{
    return new SfxStyleSheet( rName, *this, eFam, mask );
}

SfxStyleSheetBase* SfxStyleSheetPool::Create( const SfxStyleSheet& r )
{
    return new SfxStyleSheet( r );
}
/*
BOOL SfxStyleSheetPool::CopyTo(SfxStyleSheetPool &, const String &)
{
    return FALSE;
}
*/

