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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"

#include <com/sun/star/lang/XComponent.hpp>

#define _SVSTDARR_STRINGS
#define _SVSTDARR_STRINGSSORTDTOR
#define _SVSTDARR_BYTESTRINGS
#define _SVSTDARR_BYTESTRINGSSORTDTOR

#include <rtl/uuid.h>
#include <tools/tenccvt.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/intlwrapper.hxx>
#include <svl/smplhint.hxx>
#include <svl/poolitem.hxx>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <poolio.hxx>
#include <svl/filerec.hxx>
#include <svl/itemiter.hxx>
#include <svl/style.hxx>
#include <svl/svstdarr.hxx>
#include <unotools/syslocale.hxx>
#include <algorithm>

#define STYLESTREAM             "SfxStyleSheets"
#define STYLESTREAM_VERSION     sal_uInt16(50)

#ifdef DBG_UTIL
class DbgStyleSheetReferences
{
public:
    DbgStyleSheetReferences() : mnStyles(0), mnPools(0) {}
    ~DbgStyleSheetReferences()
    {
        OSL_TRACE("DbgStyleSheetReferences\nSfxStyleSheetBase left %ld\nSfxStyleSheetBasePool left %ld\n", mnStyles, mnPools );
    }

    sal_uInt32 mnStyles;
    sal_uInt32 mnPools;
}
aDbgStyleSheetReferences;

#endif

TYPEINIT0(SfxStyleSheetBase)

TYPEINIT3(SfxStyleSheet, SfxStyleSheetBase, SfxListener, SfxBroadcaster)


//=========================================================================

TYPEINIT1(SfxStyleSheetHint, SfxHint);
TYPEINIT1(SfxStyleSheetHintExtended, SfxStyleSheetHint);
TYPEINIT1(SfxStyleSheetPoolHint, SfxHint);

SfxStyleSheetHintExtended::SfxStyleSheetHintExtended
(
    sal_uInt16              nAction,        // SFX_STYLESHEET_... (s.o.)
    const String&       rOldName
)
:   SfxStyleSheetHint( nAction ),
    aName( rOldName )
{}
SfxStyleSheetHintExtended::SfxStyleSheetHintExtended
(
    sal_uInt16              nAction,        // SFX_STYLESHEET_... (s.o.)
    const String&       rOldName,
    SfxStyleSheetBase&  rStyleSheet     // geh"ort weiterhin dem Aufrufer
)
:   SfxStyleSheetHint( nAction, rStyleSheet ),
    aName( rOldName )
{}

//-------------------------------------------------------------------------

SfxStyleSheetHint::SfxStyleSheetHint
(
    sal_uInt16              nAction,        // SFX_STYLESHEET_... (s.o.)
    SfxStyleSheetBase&  rStyleSheet     // geh"ort weiterhin dem Aufrufer
)
:   pStyleSh( &rStyleSheet ),
    nHint( nAction )
{}

SfxStyleSheetHint::SfxStyleSheetHint
(
    sal_uInt16              nAction     // SFX_STYLESHEET_... (s.o.)
)
:   pStyleSh( NULL ),
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

SfxStyleSheetBase::SfxStyleSheetBase( const XubString& rName, SfxStyleSheetBasePool& r, SfxStyleFamily eFam, sal_uInt16 mask )
    : rPool( r )
    , nFamily( eFam )
    , aName( rName )
    , aParent()
    , aFollow( rName )
    , pSet( NULL )
    , nMask(mask)
    , nHelpId( 0 )
    , bMySet( sal_False )
{
#ifdef DBG_UTIL
    aDbgStyleSheetReferences.mnStyles++;
#endif
}

SfxStyleSheetBase::SfxStyleSheetBase( const SfxStyleSheetBase& r )
    : comphelper::OWeakTypeObject()
    , rPool( r.rPool )
    , nFamily( r.nFamily )
    , aName( r.aName )
    , aParent( r.aParent )
    , aFollow( r.aFollow )
    , aHelpFile( r.aHelpFile )
    , nMask( r.nMask )
    , nHelpId( r.nHelpId )
    , bMySet( r.bMySet )
{
#ifdef DBG_UTIL
    aDbgStyleSheetReferences.mnStyles++;
#endif
    if( r.pSet )
        pSet = bMySet ? new SfxItemSet( *r.pSet ) : r.pSet;
    else
        pSet = NULL;
}

static SfxStyleSheetBasePool& implGetStaticPool()
{
    static SfxStyleSheetBasePool* pSheetPool = 0;
    static SfxItemPool* pBasePool = 0;
    if( !pSheetPool )
    {
        UniString aName;
        pBasePool = new SfxItemPool( aName, 0, 0, 0 );
        pSheetPool = new SfxStyleSheetBasePool(*pBasePool);
    }
    return *pSheetPool;
}

SfxStyleSheetBase::SfxStyleSheetBase()
: comphelper::OWeakTypeObject()
, rPool( implGetStaticPool() )
{
}

SfxStyleSheetBase::~SfxStyleSheetBase()
{
#ifdef DBG_UTIL
    --aDbgStyleSheetReferences.mnStyles;
#endif

    if( bMySet )
    {
        delete pSet;
        pSet = 0;
    }
}

sal_uInt16 SfxStyleSheetBase::GetVersion() const
{
    return 0x0000;
}

// Namen aendern

const XubString& SfxStyleSheetBase::GetName() const
{
    return aName;
}

sal_Bool SfxStyleSheetBase::SetName( const XubString& rName )
{
    if(rName.Len() == 0)
        return sal_False;
    if( aName != rName )
    {
        String aOldName = aName;
        SfxStyleSheetBase *pOther = rPool.Find( rName, nFamily ) ;
        if ( pOther && pOther != this )
            return sal_False;

        SfxStyleFamily eTmpFam=rPool.GetSearchFamily();
        sal_uInt16 nTmpMask=rPool.GetSearchMask();

        rPool.SetSearchMask(nFamily);

        if ( aName.Len() )
            rPool.ChangeParent( aName, rName, sal_False );
        if ( aFollow.Equals( aName ) )
            aFollow = rName;
        aName = rName;
        rPool.SetSearchMask(eTmpFam, nTmpMask);
        rPool.Broadcast( SfxStyleSheetHintExtended(
            SFX_STYLESHEET_MODIFIED, aOldName, *this ) );
    }
    return sal_True;
}

rtl::OUString SfxStyleSheetBase::GetDisplayName() const
{
    if( maDisplayName.getLength() == 0 )
    {
        return aName;
    }
    else
    {
        return maDisplayName;
    }
}

void SfxStyleSheetBase::SetDisplayName( const rtl::OUString& rDisplayName )
{
    maDisplayName = rDisplayName;
}

// Parent aendern

const XubString& SfxStyleSheetBase::GetParent() const
{
    return aParent;
}

sal_Bool SfxStyleSheetBase::SetParent( const XubString& rName )
{
    if ( rName == aName )
        return sal_False;

    if( aParent != rName )
    {
        SfxStyleSheetBase* pIter = rPool.Find(rName, nFamily);
        if( rName.Len() && !pIter )
        {
            OSL_FAIL( "StyleSheet-Parent nicht gefunden" );
            return sal_False;
        }
        // rekursive Verknuepfungen verhindern
        if( aName.Len() )
            while(pIter)
            {
                if(pIter->GetName() == aName && aName != rName)
                    return sal_False;
                pIter = rPool.Find(pIter->GetParent(), nFamily);
            }
        aParent = rName;
    }
    rPool.Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_MODIFIED, *this ) );
    return sal_True;
}

// Follow aendern

const XubString& SfxStyleSheetBase::GetFollow() const
{
    return aFollow;
}

sal_Bool SfxStyleSheetBase::SetFollow( const XubString& rName )
{
    if( aFollow != rName )
    {
        if( !rPool.Find( rName, nFamily ) )
        {
            OSL_FAIL( "StyleSheet-Follow nicht gefunden" );
            return sal_False;
        }
        aFollow = rName;
    }
    rPool.Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_MODIFIED, *this ) );
    return sal_True;
}

// Itemset setzen. Die Dflt-Implementation legt ein neues Set an.

SfxItemSet& SfxStyleSheetBase::GetItemSet()
{
    if( !pSet )
    {
        pSet = new SfxItemSet( rPool.GetPool() );
        bMySet = sal_True;
    }
    return *pSet;
}

// Hilfe-Datei und -ID setzen und abfragen

sal_uLong SfxStyleSheetBase::GetHelpId( String& rFile )
{
    rFile = aHelpFile;
    return nHelpId;
}

void SfxStyleSheetBase::SetHelpId( const String& rFile, sal_uLong nId )
{
    aHelpFile = rFile;
    nHelpId = nId;
}

// Folgevorlage m"oglich? Default: Ja

sal_Bool SfxStyleSheetBase::HasFollowSupport() const
{
    return sal_True;
}

// Basisvorlage m"oglich? Default: Ja

sal_Bool SfxStyleSheetBase::HasParentSupport() const
{
    return sal_True;
}

// Basisvorlage uf NULL setzen m"oglich? Default: Nein

sal_Bool SfxStyleSheetBase::HasClearParentSupport() const
{
    return sal_False;
}

// Defaultmaessig sind alle StyleSheets Used

sal_Bool SfxStyleSheetBase::IsUsed() const
{
    return sal_True;
}

// eingestellte Attribute ausgeben


XubString SfxStyleSheetBase::GetDescription()
{
    return GetDescription( SFX_MAPUNIT_CM );
}

// eingestellte Attribute ausgeben

XubString SfxStyleSheetBase::GetDescription( SfxMapUnit eMetric )
{
    SfxItemIter aIter( GetItemSet() );
    XubString aDesc;
    const SfxPoolItem* pItem = aIter.FirstItem();

    IntlWrapper aIntlWrapper(comphelper::getProcessServiceFactory(),
            SvtSysLocale().GetLanguage());
    while ( pItem )
    {
        XubString aItemPresentation;

        if ( !IsInvalidItem( pItem ) &&
             rPool.GetPool().GetPresentation(
                *pItem, SFX_ITEM_PRESENTATION_COMPLETE,
                eMetric, aItemPresentation, &aIntlWrapper ) )
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

inline sal_Bool SfxStyleSheetIterator::IsTrivialSearch()
{
    return nMask == 0xFFFF && GetSearchFamily() == SFX_STYLE_FAMILY_ALL;
}

sal_Bool SfxStyleSheetIterator::DoesStyleMatch(SfxStyleSheetBase *pStyle)
{
    return ((GetSearchFamily() == SFX_STYLE_FAMILY_ALL) ||
            ( pStyle->GetFamily() == GetSearchFamily() ))
        && (( pStyle->GetMask() & ( GetSearchMask() & ~SFXSTYLEBIT_USED )) ||
            ( bSearchUsed ? pStyle->IsUsed() : sal_False ) ||
            GetSearchMask() == SFXSTYLEBIT_ALL );
}


SfxStyleSheetIterator::SfxStyleSheetIterator(SfxStyleSheetBasePool *pBase,
                                             SfxStyleFamily eFam, sal_uInt16 n)
{
    pBasePool=pBase;
    nSearchFamily=eFam;
    bSearchUsed=sal_False;
        if((n != SFXSTYLEBIT_ALL ) && ((n & SFXSTYLEBIT_USED) == SFXSTYLEBIT_USED))
    {
        bSearchUsed = sal_True;
        n &= ~SFXSTYLEBIT_USED;
    }
    nMask=n;
}

SfxStyleSheetIterator::~SfxStyleSheetIterator()
{
}


sal_uInt16 SfxStyleSheetIterator::Count()
{
    sal_uInt16 n = 0;
    if( IsTrivialSearch())
        n = (sal_uInt16) pBasePool->aStyles.size();
    else
        for(sal_uInt16 i=0; i<pBasePool->aStyles.size(); i++)
        {
            SfxStyleSheetBase* pStyle = pBasePool->aStyles[i].get();
            if(DoesStyleMatch(pStyle))
                n++;
        }
    return n;
}

SfxStyleSheetBase* SfxStyleSheetIterator::operator[](sal_uInt16 nIdx)
{
    if( IsTrivialSearch())
        return pBasePool->aStyles[nIdx].get();

    sal_uInt16 z = 0;
    for(sal_uInt16 n=0; n<pBasePool->aStyles.size(); n++)
    {
        SfxStyleSheetBase* pStyle = pBasePool->aStyles[n].get();
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
    OSL_FAIL("falscher Index");
    return 0;
}

SfxStyleSheetBase* SfxStyleSheetIterator::First()
{
    sal_Int32 nIdx = -1;

    if ( IsTrivialSearch() && pBasePool->aStyles.size() )
        nIdx = 0;
    else
        for( sal_uInt16 n = 0; n < pBasePool->aStyles.size(); n++ )
        {
            SfxStyleSheetBase* pStyle = pBasePool->aStyles[n].get();

            if ( DoesStyleMatch( pStyle ) )
            {
                nIdx = n;
                break;
            }
        }

    if ( nIdx != -1 )
    {
        nAktPosition = (sal_uInt16)nIdx;
        return pAktStyle = pBasePool->aStyles[nIdx].get();
    }
    return 0;
}


SfxStyleSheetBase* SfxStyleSheetIterator::Next()
{
    sal_Int32 nIdx = -1;

    if ( IsTrivialSearch() &&
         (sal_uInt16)pBasePool->aStyles.size() > nAktPosition + 1 )
        nIdx = nAktPosition + 1;
    else
        for( sal_uInt16 n = nAktPosition + 1; n < pBasePool->aStyles.size(); n++ )
        {
            SfxStyleSheetBase* pStyle = pBasePool->aStyles[n].get();

            if ( DoesStyleMatch( pStyle ) )
            {
                nIdx = n;
                break;
            }
        }

    if ( nIdx != -1 )
    {
        nAktPosition = (sal_uInt16)nIdx;
        return pAktStyle = pBasePool->aStyles[nIdx].get();
    }
    return 0;
}


SfxStyleSheetBase* SfxStyleSheetIterator::Find(const XubString& rStr)
{
    for ( sal_uInt16 n = 0; n < pBasePool->aStyles.size(); n++ )
    {
        SfxStyleSheetBase* pStyle = pBasePool->aStyles[n].get();

        // #98454# performance: in case of bSearchUsed==sal_True it may be
        // significant to first compare the name and only if it matches to call
        // the style sheet IsUsed() method in DoesStyleMatch().
        if ( pStyle->GetName().Equals( rStr ) && DoesStyleMatch( pStyle ) )
        {
            nAktPosition = n;
            return pAktStyle = pStyle;
        }
    }
    return 0;
}


sal_uInt16 SfxStyleSheetIterator::GetSearchMask() const
{
    sal_uInt16 mask = nMask;

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
    if( !rpIter || (rpIter->GetSearchMask() != nMask) || (rpIter->GetSearchFamily() != nSearchFamily) )
    {
        delete rpIter;
        rpIter = CreateIterator( nSearchFamily, nMask );
    }
    return *rpIter;
}


SfxStyleSheetBasePool::SfxStyleSheetBasePool( SfxItemPool& r )
    : aAppName(r.GetName())
    , rPool(r)
    , nSearchFamily(SFX_STYLE_FAMILY_PARA)
    , nMask(0xFFFF)
{
#ifdef DBG_UTIL
    aDbgStyleSheetReferences.mnPools++;
#endif

    pImp = new SfxStyleSheetBasePool_Impl;
}

SfxStyleSheetBasePool::SfxStyleSheetBasePool( const SfxStyleSheetBasePool& r )
    : SfxBroadcaster( r )
    , comphelper::OWeakTypeObject()
    , aAppName(r.aAppName)
    , rPool(r.rPool)
    , nSearchFamily(r.nSearchFamily)
    , nMask( r.nMask )
{
#ifdef DBG_UTIL
    aDbgStyleSheetReferences.mnPools++;
#endif

    pImp = new SfxStyleSheetBasePool_Impl;
    *this += r;
}

SfxStyleSheetBasePool::~SfxStyleSheetBasePool()
{
#ifdef DBG_UTIL
    aDbgStyleSheetReferences.mnPools--;
#endif

    Broadcast( SfxSimpleHint(SFX_HINT_DYING) );
    Clear();
    delete pImp;
}

sal_Bool SfxStyleSheetBasePool::SetParent(SfxStyleFamily eFam, const XubString& rStyle, const XubString& rParent)
{
    SfxStyleSheetIterator aIter(this,eFam,SFXSTYLEBIT_ALL);
    SfxStyleSheetBase *pStyle =
        aIter.Find(rStyle);
    OSL_ENSURE(pStyle, "Vorlage nicht gefunden. Writer mit Solar <2541??");
    if(pStyle)
        return pStyle->SetParent(rParent);
    else
        return sal_False;
}


void SfxStyleSheetBasePool::SetSearchMask(SfxStyleFamily eFam, sal_uInt16 n)
{
    nSearchFamily = eFam; nMask = n;
}

sal_uInt16 SfxStyleSheetBasePool::GetSearchMask() const
{
    return nMask;
}


// Der Name des Streams

String SfxStyleSheetBasePool::GetStreamName()
{
    return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(STYLESTREAM));
}

/////////////////////////////////// Factory ////////////////////////////////



SfxStyleSheetIterator* SfxStyleSheetBasePool::CreateIterator
(
 SfxStyleFamily eFam,
 sal_uInt16 mask
)
{
    return new SfxStyleSheetIterator(this,eFam,mask);
}


SfxStyleSheetBase* SfxStyleSheetBasePool::Create
(
    const XubString& rName,
    SfxStyleFamily eFam,
    sal_uInt16 mask
)
{
    return new SfxStyleSheetBase( rName, *this, eFam, mask );
}

SfxStyleSheetBase* SfxStyleSheetBasePool::Create( const SfxStyleSheetBase& r )
{
    return new SfxStyleSheetBase( r );
}

SfxStyleSheetBase& SfxStyleSheetBasePool::Make( const XubString& rName, SfxStyleFamily eFam, sal_uInt16 mask, sal_uInt16 nPos)
{
    OSL_ENSURE( eFam != SFX_STYLE_FAMILY_ALL, "svl::SfxStyleSheetBasePool::Make(), FamilyAll is not a allowed Familie" );

    SfxStyleSheetIterator aIter(this, eFam, mask);
    rtl::Reference< SfxStyleSheetBase > xStyle( aIter.Find( rName ) );
    OSL_ENSURE( !xStyle.is(), "svl::SfxStyleSheetBasePool::Make(), StyleSheet already exists" );
    SfxStyleSheetIterator& rIter = GetIterator_Impl();

    if( !xStyle.is() )
    {
        xStyle = Create( rName, eFam, mask );
        if(0xffff == nPos || nPos == aStyles.size() || nPos == rIter.Count())
        {
            aStyles.push_back( xStyle );
        }
        else
        {
            rIter[nPos];
            aStyles.insert( aStyles.begin() + rIter.GetPos(), xStyle );
        }
        Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_CREATED, *xStyle.get() ) );
    }
    return *xStyle.get();
}

/////////////////////////////// Kopieren ///////////////////////////////////

// Hilfsroutine: Falls eine Vorlage dieses Namens existiert, wird
// sie neu erzeugt. Alle Vorlagen, die diese Vorlage zum Parent haben,
// werden umgehaengt.

SfxStyleSheetBase& SfxStyleSheetBasePool::Add( SfxStyleSheetBase& rSheet )
{
    SfxStyleSheetIterator aIter(this, rSheet.GetFamily(), nMask);
    SfxStyleSheetBase* pOld = aIter.Find( rSheet.GetName() );
    Remove( pOld );
    rtl::Reference< SfxStyleSheetBase > xNew( Create( rSheet ) );
    aStyles.push_back( xNew );
    Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_CHANGED, *xNew.get() ) );
    return *xNew.get();
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

SfxStyleSheetBasePool& SfxStyleSheetBasePool::operator+=( const SfxStyleSheetBasePool& r )
{
    if( &r != this )
    {
        SfxStyles::const_iterator aIter( r.aStyles.begin() );
        while( aIter != r.aStyles.end() )
        {
            Add(*(*aIter++).get());
        }
    }
    return *this;
}

//////////////////////////////// Suchen ////////////////////////////////////

sal_uInt16 SfxStyleSheetBasePool::Count()
{
    return GetIterator_Impl().Count();
}

SfxStyleSheetBase *SfxStyleSheetBasePool::operator[](sal_uInt16 nIdx)
{
    return GetIterator_Impl()[nIdx];
}

SfxStyleSheetBase* SfxStyleSheetBasePool::Find(const XubString& rName,
                                               SfxStyleFamily eFam,
                                               sal_uInt16 mask)
{
    SfxStyleSheetIterator aIter(this,eFam,mask);
    return aIter.Find(rName);
}

const SfxStyles& SfxStyleSheetBasePool::GetStyles()
{
    return aStyles;
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

void SfxStyleSheetBasePool::Remove( SfxStyleSheetBase* p )
{
    if( p )
    {
        SfxStyles::iterator aIter( std::find( aStyles.begin(), aStyles.end(), rtl::Reference< SfxStyleSheetBase >( p ) ) );
        if( aIter != aStyles.end() )
        {
            // Alle Styles umsetzen, deren Parent dieser hier ist
            ChangeParent( p->GetName(), p->GetParent() );

            com::sun::star::uno::Reference< com::sun::star::lang::XComponent > xComp( static_cast< ::cppu::OWeakObject* >((*aIter).get()), com::sun::star::uno::UNO_QUERY );
            if( xComp.is() ) try
            {
                xComp->dispose();
            }
            catch( com::sun::star::uno::Exception& )
            {
            }

            aStyles.erase(aIter);
            Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_ERASED, *p ) );
        }
    }
}

void SfxStyleSheetBasePool::Insert( SfxStyleSheetBase* p )
{
#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE( p, "svl::SfxStyleSheetBasePool::Insert(), no stylesheet?" );

    SfxStyleSheetIterator aIter(this, p->GetFamily(), p->GetMask());
    SfxStyleSheetBase* pOld = aIter.Find( p->GetName() );
    OSL_ENSURE( !pOld, "svl::SfxStyleSheetBasePool::Insert(), StyleSheet already inserted" );
    if( p->GetParent().Len() )
    {
        pOld = aIter.Find( p->GetParent() );
        OSL_ENSURE( pOld, "svl::SfxStyleSheetBasePool::Insert(), Parent not found!" );
    }
#endif
    aStyles.push_back( rtl::Reference< SfxStyleSheetBase >( p ) );
    Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_CREATED, *p ) );
}

void SfxStyleSheetBasePool::Clear()
{
    SfxStyles aClearStyles;
    aClearStyles.swap( aStyles );

    SfxStyles::iterator aIter( aClearStyles.begin() );
    while( aIter != aClearStyles.end() )
    {
        com::sun::star::uno::Reference< com::sun::star::lang::XComponent > xComp( static_cast< ::cppu::OWeakObject* >((*aIter).get()), com::sun::star::uno::UNO_QUERY );
        if( xComp.is() ) try
        {
            xComp->dispose();
        }
        catch( com::sun::star::uno::Exception& )
        {
        }

        Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_ERASED, *(*aIter++).get() ) );
    }
}

/////////////////////////// Parents umsetzen ////////////////////////////////

void SfxStyleSheetBasePool::ChangeParent(const XubString& rOld,
                                         const XubString& rNew,
                                         sal_Bool bVirtual)
{
    const sal_uInt16 nTmpMask = GetSearchMask();
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
}

/////////////////////////// Laden/Speichern /////////////////////////////////

void SfxStyleSheetBase::Load( SvStream&, sal_uInt16 )
{
}

void SfxStyleSheetBase::Store( SvStream& )
{
}


sal_Bool SfxStyleSheetBasePool::Load( SvStream& rStream )
{
    // alte Version?
    if ( !rPool.IsVer2_Impl() )
        return Load1_Impl( rStream );

    // gesamten StyleSheetPool in neuer Version aus einem MiniRecord lesen
    SfxMiniRecordReader aPoolRec( &rStream, SFX_STYLES_REC );

    // Header-Record lesen
    short nCharSet = 0;
    if ( !rStream.GetError() )
    {
        SfxSingleRecordReader aHeaderRec( &rStream, SFX_STYLES_REC_HEADER );
        if ( !aHeaderRec.IsValid() )
            return sal_False;

        aAppName = rPool.GetName();
        rStream >> nCharSet;
    }

    // Styles-Record lesen
    if ( !rStream.GetError() )
    {
        SfxMultiRecordReader aStylesRec( &rStream, SFX_STYLES_REC_STYLES );
        if ( !aStylesRec.IsValid() )
            return sal_False;

        rtl_TextEncoding eEnc = GetSOLoadTextEncoding(
            (rtl_TextEncoding)nCharSet,
            sal::static_int_cast< sal_uInt16 >(rStream.GetVersion()) );
        rtl_TextEncoding eOldEnc = rStream.GetStreamCharSet();
        rStream.SetStreamCharSet( eEnc );

        sal_uInt16 nStyles;
        for ( nStyles = 0; aStylesRec.GetContent(); nStyles++ )
        {
            // kann nicht mehr weiterlesen?
            if ( rStream.GetError() )
                break;

            // Globale Teile
            XubString aName, aParent, aFollow;
            String aHelpFile;
            sal_uInt16 nFamily, nStyleMask,nCount;
            sal_uInt32 nHelpId;
            rStream.ReadByteString(aName, eEnc );
            rStream.ReadByteString(aParent, eEnc );
            rStream.ReadByteString(aFollow, eEnc );
            rStream >> nFamily >> nStyleMask;
            SfxPoolItem::readByteString(rStream, aHelpFile);
            rStream >> nHelpId;

            SfxStyleSheetBase& rSheet = Make( aName, (SfxStyleFamily)nFamily , nStyleMask);
            rSheet.SetHelpId( aHelpFile, nHelpId );
            // Hier erst einmal Parent und Follow zwischenspeichern
            rSheet.aParent = aParent;
            rSheet.aFollow = aFollow;
            sal_uInt32 nPos = rStream.Tell();
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
            sal_uInt32 nSize;
            sal_uInt16 nVer;
            rStream >> nVer >> nSize;
            nPos = rStream.Tell() + nSize;
            rSheet.Load( rStream, nVer );
            rStream.Seek( nPos );
        }

        //  #72939# only loop through the styles that were really inserted
        sal_uLong n = aStyles.size();

        //! delete pTmpPool;
        // Jetzt Parent und Follow setzen. Alle Sheets sind geladen.
        // Mit Setxxx() noch einmal den String eintragen, da diese
        // virtuellen Methoden evtl. ueberlagert sind.
        for ( sal_uLong i = 0; i < n; i++ )
        {
            SfxStyleSheetBase* p = aStyles[ i ].get();
            XubString aText = p->aParent;
            p->aParent.Erase();
            p->SetParent( aText );
            aText = p->aFollow;
            p->aFollow.Erase();
            p->SetFollow( aText );
        }

        rStream.SetStreamCharSet( eOldEnc );
    }

    // alles klar?
    return sal_Bool( rStream.GetError() == SVSTREAM_OK );
}

sal_Bool SfxStyleSheetBasePool::Load1_Impl( SvStream& rStream )
{
    aAppName = rPool.GetName();
    sal_uInt16 nVersion;
    short nCharSet;
    rStream >> nVersion;

    if(nVersion!=STYLESTREAM_VERSION)
        nCharSet=nVersion;
    else
        rStream >> nCharSet;

    rtl_TextEncoding eEnc = GetSOLoadTextEncoding(
        (rtl_TextEncoding)nCharSet,
        sal::static_int_cast< sal_uInt16 >(rStream.GetVersion()) );
    rtl_TextEncoding eOldEnc = rStream.GetStreamCharSet();
    rStream.SetStreamCharSet( eEnc );

    sal_uInt16 nStyles;
    rStream >> nStyles;
    sal_uInt16 i;
    for ( i = 0; i < nStyles; i++ )
    {
        // kann nicht mehr weiterlesen?
        if ( rStream.GetError() )
        {
            nStyles = i;
            break;
        }

        // Globale Teile
        XubString aName, aParent, aFollow;
        String aHelpFile;
        sal_uInt16 nFamily, nStyleMask,nCount;
        sal_uInt32 nHelpId;
        rStream.ReadByteString(aName, eEnc );
        rStream.ReadByteString(aParent, eEnc );
        rStream.ReadByteString(aFollow, eEnc );
        rStream >> nFamily >> nStyleMask;
        SfxPoolItem::readByteString(rStream, aHelpFile);
        if(nVersion!=STYLESTREAM_VERSION)
        {
            sal_uInt16 nTmpHelpId;
            rStream >> nTmpHelpId;
            nHelpId=nTmpHelpId;
        }
        else
            rStream >> nHelpId;

        SfxStyleSheetBase& rSheet = Make( aName, (SfxStyleFamily)nFamily , nStyleMask);
        rSheet.SetHelpId( aHelpFile, nHelpId );
        // Hier erst einmal Parent und Follow zwischenspeichern
        rSheet.aParent = aParent;
        rSheet.aFollow = aFollow;
        sal_uInt32 nPos = rStream.Tell();
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
        sal_uInt32 nSize;
        sal_uInt16 nVer;
        rStream >> nVer >> nSize;
        nPos = rStream.Tell() + nSize;
        rSheet.Load( rStream, nVer );
        rStream.Seek( nPos );
    }

    //! delete pTmpPool;
    // Jetzt Parent und Follow setzen. Alle Sheets sind geladen.
    // Mit Setxxx() noch einmal den String eintragen, da diese
    // virtuellen Methoden evtl. ueberlagert sind.
    for ( i = 0; i < nStyles; i++ )
    {
        SfxStyleSheetBase* p = aStyles[ i ].get();
        XubString aText = p->aParent;
        p->aParent.Erase();
        p->SetParent( aText );
        aText = p->aFollow;
        p->aFollow.Erase();
        p->SetFollow( aText );
    }

    rStream.SetStreamCharSet( eOldEnc );

    return sal_Bool( rStream.GetError() == SVSTREAM_OK );
}

sal_Bool SfxStyleSheetBasePool::Store( SvStream& rStream, sal_Bool bUsed )
{
    // den ganzen StyleSheet-Pool in einen Mini-Record
    SfxMiniRecordWriter aPoolRec( &rStream, SFX_STYLES_REC );

    // Erst einmal die Dummies rauszaehlen; die werden nicht gespeichert
    sal_uInt16 nCount = 0;
    for( SfxStyleSheetBase* p = First(); p; p = Next() )
    {
        if(!bUsed || p->IsUsed())
            nCount++;
    }

    // einen Header-Record vorweg
    rtl_TextEncoding eEnc
        = ::GetSOStoreTextEncoding(
            rStream.GetStreamCharSet(),
            sal::static_int_cast< sal_uInt16 >(rStream.GetVersion()) );
    rtl_TextEncoding eOldEnc = rStream.GetStreamCharSet();
    rStream.SetStreamCharSet( eEnc );

    {
        SfxSingleRecordWriter aHeaderRec( &rStream,
                SFX_STYLES_REC_HEADER,
                STYLESTREAM_VERSION );
        rStream << (short) eEnc;
    }

    // die StyleSheets in einen MultiVarRecord
    {
        // Bug 79478:
        // make a check loop, to be shure, that the converted names are also
        // unique like the originals! In other cases we get a loop.
        SvStringsSortDtor aSortOrigNames( 0, 128 );
        SvStrings aOrigNames( 0, 128 );
        SvByteStringsSortDtor aSortConvNames( 0, 128 );
        SvByteStrings aConvNames( 0, 128 );

        {

            for( SfxStyleSheetBase* p = First(); p; p = Next() )
            {
                if(!bUsed || p->IsUsed())
                {
                    sal_uInt16 nFamily = (sal_uInt16)p->GetFamily();
                    String* pName = new String( p->GetName() );
                    ByteString* pConvName = new ByteString( *pName, eEnc );

                    pName->Insert( (sal_Unicode)nFamily, 0 );
                    pConvName->Insert( "  ", 0 );
                    pConvName->SetChar(
                        0,
                        sal::static_int_cast< char >(0xff & (nFamily >> 8)) );
                    pConvName->SetChar(
                        1, sal::static_int_cast< char >(0xff & nFamily) );

                    sal_uInt16 nInsPos, nAdd = aSortConvNames.Count();
                    while( !aSortConvNames.Insert( pConvName, nInsPos ) )
                        (pConvName->Append( '_' )).Append(
                                    ByteString::CreateFromInt32( nAdd++ ));
                    aOrigNames.Insert( pName, nInsPos );
                }
            }

            // now we have the list of the names, sorted by convertede names
            // But now we need the sorted list of orignames.
            {
                sal_uInt16 nInsPos, nEnd = aOrigNames.Count();
                const ByteStringPtr* ppB = aSortConvNames.GetData();
                for( sal_uInt16 n = 0; n < nEnd; ++n, ++ppB )
                {
                    String* p = aOrigNames.GetObject( n );
                    aSortOrigNames.Insert( p, nInsPos );
                    aConvNames.Insert( *ppB, nInsPos );
                }

            }
        }


        ByteString sEmpty;
        sal_uInt16 nFndPos;
        String sNm;
        SfxMultiVarRecordWriter aStylesRec( &rStream, SFX_STYLES_REC_STYLES, 0 );
        for( SfxStyleSheetBase* p = First(); p; p = Next() )
        {
            if(!bUsed || p->IsUsed())
            {
                aStylesRec.NewContent();

                // Globale Teile speichern
                String aHelpFile;
                sal_uInt32 nHelpId = p->GetHelpId( aHelpFile );
                sal_uInt16 nFamily = sal::static_int_cast< sal_uInt16 >(p->GetFamily());
                String sFamily( (sal_Unicode)nFamily );

                (sNm = sFamily) += p->GetName();
                if( aSortOrigNames.Seek_Entry( &sNm, &nFndPos ))
                    rStream.WriteByteString( aConvNames.GetObject( nFndPos )->Copy( 2 ));
                else
                    rStream.WriteByteString( sEmpty );

                (sNm = sFamily) += p->GetParent();
                if( aSortOrigNames.Seek_Entry( &sNm, &nFndPos ))
                    rStream.WriteByteString( aConvNames.GetObject( nFndPos )->Copy( 2 ));
                else
                    rStream.WriteByteString( sEmpty );

                (sNm = sFamily) += p->GetFollow();
                if( aSortOrigNames.Seek_Entry( &sNm, &nFndPos ))
                    rStream.WriteByteString( aConvNames.GetObject( nFndPos )->Copy( 2 ));
                else
                    rStream.WriteByteString( sEmpty );

                rStream << nFamily << p->GetMask();
                SfxPoolItem::writeByteString(rStream, aHelpFile);
                rStream << nHelpId;
                if(p->pSet)
                    p->pSet->Store( rStream );
                else
                    rStream << (sal_uInt16)0;

                // Lokale Teile speichern
                // Vor dem lokalen Teil wird die Laenge der lokalen Daten
                // als sal_uInt32 sowie die Versionsnummer gespeichert.
                rStream << (sal_uInt16) p->GetVersion();
                sal_uLong nPos1 = rStream.Tell();
                rStream << (sal_uInt32) 0;
                p->Store( rStream );
                sal_uLong nPos2 = rStream.Tell();
                rStream.Seek( nPos1 );
                rStream << (sal_uInt32) ( nPos2 - nPos1 - sizeof( sal_uInt32 ) );
                rStream.Seek( nPos2 );
                if( rStream.GetError() != SVSTREAM_OK )
                    break;
            }
        }
    }

    rStream.SetStreamCharSet( eOldEnc );

    return sal_Bool( rStream.GetError() == SVSTREAM_OK );
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
                             const SfxStyleSheetBasePool& r_Pool,
                             SfxStyleFamily eFam,
                             sal_uInt16 mask ):
    SfxStyleSheetBase(rName, const_cast< SfxStyleSheetBasePool& >( r_Pool ), eFam, mask)
{}

SfxStyleSheet::SfxStyleSheet(const SfxStyleSheet& rStyle) :
    SfxStyleSheetBase(rStyle),
    SfxListener( rStyle ),
    SfxBroadcaster( rStyle )
{}

SfxStyleSheet::SfxStyleSheet()
{
}

SfxStyleSheet::~SfxStyleSheet()
{
    Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_INDESTRUCTION, *this ) );
}


sal_Bool SfxStyleSheet::SetParent( const XubString& rName )
{
    if(aParent == rName)
        return sal_True;
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
        return sal_True;
    }
    return sal_False;
}

// alle Zuhoerer benachtichtigen

void SfxStyleSheet::Notify(SfxBroadcaster& rBC, const SfxHint& rHint )
{
    Forward(rBC, rHint);
}

//////////////////////// SfxStyleSheetPool ///////////////////////////////

SfxStyleSheetPool::SfxStyleSheetPool( SfxItemPool const& rSet)
: SfxStyleSheetBasePool( const_cast< SfxItemPool& >( rSet ) )
{
}

/////////////////////////////////// Factory ////////////////////////////////

SfxStyleSheetBase* SfxStyleSheetPool::Create( const XubString& rName,
                                    SfxStyleFamily eFam, sal_uInt16 mask )
{
    return new SfxStyleSheet( rName, *this, eFam, mask );
}

SfxStyleSheetBase* SfxStyleSheetPool::Create( const SfxStyleSheet& r )
{
    return new SfxStyleSheet( r );
}
/*
sal_Bool SfxStyleSheetPool::CopyTo(SfxStyleSheetPool &, const String &)
{
    return sal_False;
}
*/

// --------------------------------------------------------------------
// class SfxUnoStyleSheet
// --------------------------------------------------------------------

SfxUnoStyleSheet::SfxUnoStyleSheet( const UniString& _rName, const SfxStyleSheetBasePool& _rPool, SfxStyleFamily _eFamily, sal_uInt16 _nMaske )
: ::cppu::ImplInheritanceHelper2< SfxStyleSheet, ::com::sun::star::style::XStyle, ::com::sun::star::lang::XUnoTunnel >( _rName, _rPool, _eFamily, _nMaske )
{
}

// --------------------------------------------------------------------
SfxUnoStyleSheet::SfxUnoStyleSheet( const SfxStyleSheet& _rSheet )
: ::cppu::ImplInheritanceHelper2< SfxStyleSheet, ::com::sun::star::style::XStyle, ::com::sun::star::lang::XUnoTunnel >( _rSheet )
{
}

// --------------------------------------------------------------------

SfxUnoStyleSheet* SfxUnoStyleSheet::getUnoStyleSheet( const ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle >& xStyle )
{
    SfxUnoStyleSheet* pRet = dynamic_cast< SfxUnoStyleSheet* >( xStyle.get() );
    if( !pRet )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUT( xStyle, ::com::sun::star::uno::UNO_QUERY );
        if( xUT.is() )
            pRet = reinterpret_cast<SfxUnoStyleSheet*>(sal::static_int_cast<sal_uIntPtr>(xUT->getSomething( SfxUnoStyleSheet::getIdentifier())));
    }
    return pRet;
}

// --------------------------------------------------------------------
// XUnoTunnel
// --------------------------------------------------------------------

::sal_Int64 SAL_CALL SfxUnoStyleSheet::getSomething( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& rId ) throw (::com::sun::star::uno::RuntimeException)
{
    if( rId.getLength() == 16 && 0 == rtl_compareMemory( getIdentifier().getConstArray(), rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));
    }
    else
    {
        return 0;
    }
}

// --------------------------------------------------------------------

const ::com::sun::star::uno::Sequence< ::sal_Int8 >& SfxUnoStyleSheet::getIdentifier()
{
    static ::com::sun::star::uno::Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

// --------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
