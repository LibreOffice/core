/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <memory>
#include <svl/style.hxx>

#include <com/sun/star/lang/XComponent.hpp>

#include <sal/log.hxx>
#include <tools/tenccvt.hxx>
#include <osl/diagnose.h>
#include <unotools/intlwrapper.hxx>
#include <svl/hint.hxx>
#include <svl/poolitem.hxx>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <svl/IndexedStyleSheets.hxx>
#include <svl/itemiter.hxx>
#include <unotools/syslocale.hxx>
#include <algorithm>
#include <comphelper/servicehelper.hxx>
#include <o3tl/make_unique.hxx>

#include <string.h>

#ifdef DBG_UTIL
class DbgStyleSheetReferences
{
public:
    DbgStyleSheetReferences() : mnStyles(0), mnPools(0) {}
    ~DbgStyleSheetReferences()
    {
        SAL_WARN_IF(
            mnStyles != 0 || mnPools != 0, "svl.items",
            "SfxStyleSheetBase left " << mnStyles
                << "; SfxStyleSheetBasePool left " << mnPools);
    }

    sal_uInt32 mnStyles;
    sal_uInt32 mnPools;
};

static DbgStyleSheetReferences aDbgStyleSheetReferences;
#endif


SfxStyleSheetModifiedHint::SfxStyleSheetModifiedHint
(
    const OUString&     rOldName,
    SfxStyleSheetBase&  rStyleSheet     // Remains with the caller
)
:   SfxStyleSheetHint( SfxHintId::StyleSheetModified, rStyleSheet ),
    aName( rOldName )
{}


SfxStyleSheetHint::SfxStyleSheetHint
(
    SfxHintId           nAction,
    SfxStyleSheetBase&  rStyleSheet     // Remains with the caller
)
:   SfxHint(nAction), pStyleSh( &rStyleSheet )
{}


class SfxStyleSheetBasePool_Impl
{
private:
    SfxStyleSheetBasePool_Impl(const SfxStyleSheetBasePool_Impl&) = delete;
    SfxStyleSheetBasePool_Impl& operator=(const SfxStyleSheetBasePool_Impl&) = delete;
public:
    std::shared_ptr<SfxStyleSheetIterator> pIter;

    /** This member holds the indexed style sheets.
     *
     * @internal
     * This member is private and not protected in order to have more control which style sheets are added
     * where. Ideally, all calls which add/remove/change style sheets are done in the base class.
     */
    std::shared_ptr<svl::IndexedStyleSheets> mxIndexedStyleSheets;

    SfxStyleSheetBasePool_Impl() :
        mxIndexedStyleSheets(new svl::IndexedStyleSheets) {}
};


SfxStyleSheetBase::SfxStyleSheetBase( const OUString& rName, SfxStyleSheetBasePool* p, SfxStyleFamily eFam, SfxStyleSearchBits mask )
    : m_pPool( p )
    , nFamily( eFam )
    , aName( rName )
    , aParent()
    , aFollow( rName )
    , pSet( nullptr )
    , nMask(mask)
    , nHelpId( 0 )
    , bMySet( false )
    , bHidden( false )
{
#ifdef DBG_UTIL
    aDbgStyleSheetReferences.mnStyles++;
#endif
}

SfxStyleSheetBase::SfxStyleSheetBase( const SfxStyleSheetBase& r )
    : comphelper::OWeakTypeObject()
    , m_pPool( r.m_pPool )
    , nFamily( r.nFamily )
    , aName( r.aName )
    , aParent( r.aParent )
    , aFollow( r.aFollow )
    , aHelpFile( r.aHelpFile )
    , nMask( r.nMask )
    , nHelpId( r.nHelpId )
    , bMySet( r.bMySet )
    , bHidden( r.bHidden )
{
#ifdef DBG_UTIL
    aDbgStyleSheetReferences.mnStyles++;
#endif
    if( r.pSet )
        pSet = bMySet ? new SfxItemSet( *r.pSet ) : r.pSet;
    else
        pSet = nullptr;
}

SfxStyleSheetBase::~SfxStyleSheetBase()
{
#ifdef DBG_UTIL
    --aDbgStyleSheetReferences.mnStyles;
#endif

    if( bMySet )
    {
        delete pSet;
        pSet = nullptr;
    }
}

// Change name
const OUString& SfxStyleSheetBase::GetName() const
{
    return aName;
}

bool SfxStyleSheetBase::SetName(const OUString& rName, bool bReIndexNow)
{
    if(rName.isEmpty())
        return false;

    if( aName != rName )
    {
        OUString aOldName = aName;
        SfxStyleSheetBase *pOther = m_pPool->Find( rName, nFamily ) ;
        if ( pOther && pOther != this )
            return false;

        SfxStyleFamily eTmpFam = m_pPool->GetSearchFamily();
        SfxStyleSearchBits nTmpMask = m_pPool->GetSearchMask();

        m_pPool->SetSearchMask(nFamily);

        if ( !aName.isEmpty() )
            m_pPool->ChangeParent( aName, rName, false );

        if ( aFollow == aName )
            aFollow = rName;
        aName = rName;
        if (bReIndexNow)
            m_pPool->Reindex();
        m_pPool->SetSearchMask(eTmpFam, nTmpMask);
        m_pPool->Broadcast( SfxStyleSheetModifiedHint( aOldName, *this ) );
    }
    return true;
}

// Change Parent
const OUString& SfxStyleSheetBase::GetParent() const
{
    return aParent;
}

bool SfxStyleSheetBase::SetParent( const OUString& rName )
{
    if ( rName == aName )
        return false;

    if( aParent != rName )
    {
        SfxStyleSheetBase* pIter = m_pPool->Find(rName, nFamily);
        if( !rName.isEmpty() && !pIter )
        {
            OSL_FAIL( "StyleSheet-Parent not found" );
            return false;
        }
        // prevent recursive linkages
        if( !aName.isEmpty() )
        {
            while(pIter)
            {
                if(pIter->GetName() == aName)
                    return false;
                pIter = m_pPool->Find(pIter->GetParent(), nFamily);
            }
        }
        aParent = rName;
    }
    m_pPool->Broadcast( SfxStyleSheetHint( SfxHintId::StyleSheetModified, *this ) );
    return true;
}

void SfxStyleSheetBase::SetHidden( bool hidden )
{
    bHidden = hidden;
    m_pPool->Broadcast( SfxStyleSheetHint( SfxHintId::StyleSheetModified, *this ) );
}

/**
 * Change follow
 */
const OUString& SfxStyleSheetBase::GetFollow() const
{
    return aFollow;
}

bool SfxStyleSheetBase::SetFollow( const OUString& rName )
{
    if( aFollow != rName )
    {
        if( !m_pPool->Find( rName, nFamily ) )
        {
            SAL_WARN( "svl.items", "StyleSheet-Follow not found" );
            return false;
        }
        aFollow = rName;
    }
    m_pPool->Broadcast( SfxStyleSheetHint( SfxHintId::StyleSheetModified, *this ) );
    return true;
}

/**
 * Set Itemset
 * The default implementation creates a new set
 */
SfxItemSet& SfxStyleSheetBase::GetItemSet()
{
    if( !pSet )
    {
        pSet = new SfxItemSet( m_pPool->GetPool() );
        bMySet = true;
    }
    return *pSet;
}

std::unique_ptr<SfxItemSet> SfxStyleSheetBase::GetItemSetForPreview()
{
    return o3tl::make_unique<SfxItemSet>(GetItemSet());
}

/**
 * Set help file and ID and return it
 */
sal_uLong SfxStyleSheetBase::GetHelpId( OUString& rFile )
{
    rFile = aHelpFile;
    return nHelpId;
}

void SfxStyleSheetBase::SetHelpId( const OUString& rFile, sal_uLong nId )
{
    aHelpFile = rFile;
    nHelpId = nId;
}

/**
 * Next style possible?
 * Default: Yes
 */
bool SfxStyleSheetBase::HasFollowSupport() const
{
    return true;
}

/**
 * Base template possible?
 * Default: Yes
 */
bool SfxStyleSheetBase::HasParentSupport() const
{
    return true;
}

/**
 * Setting base template to NULL possible?
 * Default: No
 */
bool SfxStyleSheetBase::HasClearParentSupport() const
{
    return false;
}

/**
 * By default all stylesheets are set to used
 */
bool SfxStyleSheetBase::IsUsed() const
{
    return true;
}

/**
 * Return set attributes
 */
OUString SfxStyleSheetBase::GetDescription( MapUnit eMetric )
{
    SfxItemIter aIter( GetItemSet() );
    OUString aDesc;
    const SfxPoolItem* pItem = aIter.FirstItem();

    IntlWrapper aIntlWrapper(SvtSysLocale().GetUILanguageTag());
    while ( pItem )
    {
        OUString aItemPresentation;

        if ( !IsInvalidItem( pItem ) &&
             m_pPool->GetPool().GetPresentation(
                *pItem, eMetric, aItemPresentation, aIntlWrapper ) )
        {
            if ( !aDesc.isEmpty() && !aItemPresentation.isEmpty() )
                aDesc += " + ";
            if ( !aItemPresentation.isEmpty() )
                aDesc += aItemPresentation;
        }
        pItem = aIter.NextItem();
    }
    return aDesc;
}

SfxStyleFamily SfxStyleSheetIterator::GetSearchFamily() const
{
    return nSearchFamily;
}

inline bool SfxStyleSheetIterator::IsTrivialSearch() const
{
    return (( nMask & SfxStyleSearchBits::AllVisible ) == SfxStyleSearchBits::AllVisible) &&
        (GetSearchFamily() == SfxStyleFamily::All);
}

namespace {

struct DoesStyleMatchStyleSheetPredicate final : public svl::StyleSheetPredicate
{
    explicit DoesStyleMatchStyleSheetPredicate(SfxStyleSheetIterator *it)
            : mIterator(it) {}

    bool
    Check(const SfxStyleSheetBase& styleSheet) override
    {
        bool bMatchFamily = ((mIterator->GetSearchFamily() == SfxStyleFamily::All) ||
                ( styleSheet.GetFamily() == mIterator->GetSearchFamily() ));

        bool bUsed = mIterator->SearchUsed() && styleSheet.IsUsed( );

        bool bSearchHidden( mIterator->GetSearchMask() & SfxStyleSearchBits::Hidden );
        bool bMatchVisibility = !( !bSearchHidden && styleSheet.IsHidden() && !bUsed );
        bool bOnlyHidden = mIterator->GetSearchMask( ) == SfxStyleSearchBits::Hidden && styleSheet.IsHidden( );

        bool bMatches = bMatchFamily && bMatchVisibility
            && (( styleSheet.GetMask() & ( mIterator->GetSearchMask() & ~SfxStyleSearchBits::Used )) ||
                bUsed || bOnlyHidden ||
                ( mIterator->GetSearchMask() & SfxStyleSearchBits::AllVisible ) == SfxStyleSearchBits::AllVisible );
        return bMatches;
    }

    SfxStyleSheetIterator *mIterator;
};

}

SfxStyleSheetIterator::SfxStyleSheetIterator(SfxStyleSheetBasePool *pBase,
                                             SfxStyleFamily eFam, SfxStyleSearchBits n)
    : pCurrentStyle(nullptr)
    , nCurrentPosition(0)
{
    pBasePool=pBase;
    nSearchFamily=eFam;
    bSearchUsed=false;
        if( (( n & SfxStyleSearchBits::AllVisible ) != SfxStyleSearchBits::AllVisible )
                && ((n & SfxStyleSearchBits::Used) == SfxStyleSearchBits::Used))
    {
        bSearchUsed = true;
        n &= ~SfxStyleSearchBits::Used;
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
    {
        n = static_cast<sal_uInt16>(pBasePool->pImpl->mxIndexedStyleSheets->GetNumberOfStyleSheets());
    }
    else if(nMask == SfxStyleSearchBits::All)
    {
        n = static_cast<sal_uInt16>(pBasePool->pImpl->mxIndexedStyleSheets->GetStyleSheetPositionsByFamily(nSearchFamily).size());
    }
    else
    {
        DoesStyleMatchStyleSheetPredicate predicate(this);
        n = pBasePool->pImpl->mxIndexedStyleSheets->GetNumberOfStyleSheetsWithPredicate(predicate);
    }
    return n;
}

SfxStyleSheetBase* SfxStyleSheetIterator::operator[](sal_uInt16 nIdx)
{
    SfxStyleSheetBase* retval = nullptr;
    if( IsTrivialSearch())
    {
        retval = pBasePool->pImpl->mxIndexedStyleSheets->GetStyleSheetByPosition(nIdx);
        nCurrentPosition = nIdx;
    }
    else if(nMask == SfxStyleSearchBits::All)
    {
        rtl::Reference< SfxStyleSheetBase > ref =
        pBasePool->pImpl->mxIndexedStyleSheets->GetStyleSheetByPosition(
                pBasePool->pImpl->mxIndexedStyleSheets->GetStyleSheetPositionsByFamily(nSearchFamily).at(nIdx))
                ;
        retval = ref.get();
        nCurrentPosition = nIdx;
    }
    else
    {
        DoesStyleMatchStyleSheetPredicate predicate(this);
        rtl::Reference< SfxStyleSheetBase > ref =
                pBasePool->pImpl->mxIndexedStyleSheets->GetNthStyleSheetThatMatchesPredicate(nIdx, predicate);
        if (ref.get() != nullptr)
        {
            nCurrentPosition = pBasePool->pImpl->mxIndexedStyleSheets->FindStyleSheetPosition(*ref);
            retval = ref.get();
        }
    }

    if (retval == nullptr)
    {
        OSL_FAIL("Incorrect index");
    }

    return retval;
}

SfxStyleSheetBase* SfxStyleSheetIterator::First()
{
    if (Count() != 0) {
        return operator[](0);
    }
    else {
        return nullptr;
    }
}


SfxStyleSheetBase* SfxStyleSheetIterator::Next()
{
    SfxStyleSheetBase* retval = nullptr;

    if ( IsTrivialSearch() )
    {
        unsigned nStyleSheets = pBasePool->pImpl->mxIndexedStyleSheets->GetNumberOfStyleSheets();
        unsigned newPosition = nCurrentPosition +1;
        if (nStyleSheets > newPosition)
        {
            nCurrentPosition = newPosition;
            retval = pBasePool->pImpl->mxIndexedStyleSheets->GetStyleSheetByPosition(nCurrentPosition);
        }
    }
    else if(nMask == SfxStyleSearchBits::All)
    {
        unsigned newPosition = nCurrentPosition +1;
        const std::vector<unsigned>& familyVector =
            pBasePool->pImpl->mxIndexedStyleSheets->GetStyleSheetPositionsByFamily(nSearchFamily);
        if (familyVector.size() > newPosition)
        {
            nCurrentPosition = newPosition;
            unsigned stylePosition = familyVector[newPosition];
            retval = pBasePool->pImpl->mxIndexedStyleSheets->GetStyleSheetByPosition(stylePosition);
        }
    }
    else
    {
        DoesStyleMatchStyleSheetPredicate predicate(this);
        rtl::Reference< SfxStyleSheetBase > ref =
                pBasePool->pImpl->mxIndexedStyleSheets->GetNthStyleSheetThatMatchesPredicate(
                        0, predicate, nCurrentPosition+1);
        retval = ref.get();
        if (retval != nullptr) {
            nCurrentPosition = pBasePool->pImpl->mxIndexedStyleSheets->FindStyleSheetPosition(*ref);
        }
    }
    pCurrentStyle = retval;
    return retval;
}

SfxStyleSheetBase* SfxStyleSheetIterator::Find(const OUString& rStr)
{
    DoesStyleMatchStyleSheetPredicate predicate(this);

    std::vector<unsigned> positions =
            pBasePool->pImpl->mxIndexedStyleSheets->FindPositionsByNameAndPredicate(rStr, predicate,
                    svl::IndexedStyleSheets::SearchBehavior::ReturnFirst);
    if (positions.empty()) {
        return nullptr;
    }

    unsigned pos = positions.front();
    SfxStyleSheetBase* pStyle = pBasePool->pImpl->mxIndexedStyleSheets->GetStyleSheetByPosition(pos);
    nCurrentPosition = pos;
    pCurrentStyle = pStyle;
    return pCurrentStyle;
}

SfxStyleSearchBits SfxStyleSheetIterator::GetSearchMask() const
{
    SfxStyleSearchBits mask = nMask;

    if ( bSearchUsed )
        mask |= SfxStyleSearchBits::Used;
    return mask;
}


SfxStyleSheetIterator& SfxStyleSheetBasePool::GetIterator_Impl()
{
    if( !pImpl->pIter || (pImpl->pIter->GetSearchMask() != nMask) || (pImpl->pIter->GetSearchFamily() != nSearchFamily) )
    {
        pImpl->pIter = CreateIterator( nSearchFamily, nMask );
    }

    return *pImpl->pIter;
}

SfxStyleSheetBasePool::SfxStyleSheetBasePool( SfxItemPool& r ) :
    pImpl(new SfxStyleSheetBasePool_Impl),
    rPool(r),
    nSearchFamily(SfxStyleFamily::Para),
    nMask(SfxStyleSearchBits::All)
{
#ifdef DBG_UTIL
    aDbgStyleSheetReferences.mnPools++;
#endif
}

SfxStyleSheetBasePool::SfxStyleSheetBasePool( const SfxStyleSheetBasePool& r ) :
    SfxBroadcaster( r ),
    comphelper::OWeakTypeObject(),
    pImpl(new SfxStyleSheetBasePool_Impl),
    rPool(r.rPool),
    nSearchFamily(r.nSearchFamily),
    nMask( r.nMask )
{
#ifdef DBG_UTIL
    aDbgStyleSheetReferences.mnPools++;
#endif

    *this += r;
}

SfxStyleSheetBasePool::~SfxStyleSheetBasePool()
{
#ifdef DBG_UTIL
    aDbgStyleSheetReferences.mnPools--;
#endif

    Broadcast( SfxHint(SfxHintId::Dying) );
    Clear();
}

bool SfxStyleSheetBasePool::SetParent(SfxStyleFamily eFam, const OUString& rStyle, const OUString& rParent)
{
    SfxStyleSheetIterator aIter(this,eFam,SfxStyleSearchBits::All);
    SfxStyleSheetBase *pStyle = aIter.Find(rStyle);
    OSL_ENSURE(pStyle, "Template not found. Writer with solar <2541?");
    if(pStyle)
        return pStyle->SetParent(rParent);
    else
        return false;
}


void SfxStyleSheetBasePool::SetSearchMask(SfxStyleFamily eFam, SfxStyleSearchBits n)
{
    nSearchFamily = eFam; nMask = n;
}


std::unique_ptr<SfxStyleSheetIterator> SfxStyleSheetBasePool::CreateIterator
(
 SfxStyleFamily eFam,
 SfxStyleSearchBits mask
)
{
    return o3tl::make_unique<SfxStyleSheetIterator>(this,eFam,mask);
}

SfxStyleSheetBase* SfxStyleSheetBasePool::Create
(
    const OUString& rName,
    SfxStyleFamily eFam,
    SfxStyleSearchBits mask
)
{
    return new SfxStyleSheetBase( rName, this, eFam, mask );
}

SfxStyleSheetBase* SfxStyleSheetBasePool::Create( const SfxStyleSheetBase& r )
{
    return new SfxStyleSheetBase( r );
}

SfxStyleSheetBase& SfxStyleSheetBasePool::Make( const OUString& rName, SfxStyleFamily eFam, SfxStyleSearchBits mask)
{
    OSL_ENSURE( eFam != SfxStyleFamily::All, "svl::SfxStyleSheetBasePool::Make(), FamilyAll is not a allowed Familie" );

    SfxStyleSheetIterator aIter(this, eFam, mask);
    rtl::Reference< SfxStyleSheetBase > xStyle( aIter.Find( rName ) );
    OSL_ENSURE( !xStyle.is(), "svl::SfxStyleSheetBasePool::Make(), StyleSheet already exists" );

    if( !xStyle.is() )
    {
        xStyle = Create( rName, eFam, mask );
        StoreStyleSheet(xStyle);
        Broadcast( SfxStyleSheetHint( SfxHintId::StyleSheetCreated, *xStyle.get() ) );
    }
    return *xStyle.get();
}

/**
 * Helper function: If a template with this name exists it is created
 * anew. All templates that have this template as a parent are reconnected.
 */
void SfxStyleSheetBasePool::Add( const SfxStyleSheetBase& rSheet )
{
    SfxStyleSheetIterator aIter(this, rSheet.GetFamily(), nMask);
    SfxStyleSheetBase* pOld = aIter.Find( rSheet.GetName() );
    if (pOld) {
        Remove( pOld );
    }
    rtl::Reference< SfxStyleSheetBase > xNew( Create( rSheet ) );
    pImpl->mxIndexedStyleSheets->AddStyleSheet(xNew);
    Broadcast( SfxStyleSheetHint( SfxHintId::StyleSheetChanged, *xNew.get() ) );
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

namespace {
struct AddStyleSheetCallback : svl::StyleSheetCallback
{
    explicit AddStyleSheetCallback(SfxStyleSheetBasePool *pool)
    : mPool(pool) {}

    void DoIt(const SfxStyleSheetBase& ssheet) override
    {
        mPool->Add(ssheet);
    }

    SfxStyleSheetBasePool *mPool;
};
}

SfxStyleSheetBasePool& SfxStyleSheetBasePool::operator+=( const SfxStyleSheetBasePool& r )
{
    if( &r != this )
    {
        AddStyleSheetCallback callback(this);
        pImpl->mxIndexedStyleSheets->ApplyToAllStyleSheets(callback);
    }
    return *this;
}

sal_uInt16 SfxStyleSheetBasePool::Count()
{
    return GetIterator_Impl().Count();
}

SfxStyleSheetBase *SfxStyleSheetBasePool::operator[](sal_uInt16 nIdx)
{
    return GetIterator_Impl()[nIdx];
}

SfxStyleSheetBase* SfxStyleSheetBasePool::Find(const OUString& rName,
                                               SfxStyleFamily eFam,
                                               SfxStyleSearchBits mask)
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

void SfxStyleSheetBasePool::Remove( SfxStyleSheetBase* p )
{
    if( !p )
        return;

    // Reference to keep p alive until after Broadcast call!
    rtl::Reference<SfxStyleSheetBase> xP(p);
    bool bWasRemoved = pImpl->mxIndexedStyleSheets->RemoveStyleSheet(xP);
    if( !bWasRemoved )
        return;

    // Adapt all styles which have this style as parant
    ChangeParent( p->GetName(), p->GetParent() );

    // #120015# Do not dispose, the removed StyleSheet may still be used in
    // existing SdrUndoAttrObj incarnations. Rely on refcounting for disposal,
    // this works well under normal conditions (checked breaking and counting
    // on SfxStyleSheetBase constructors and destructors)

    // css::uno::Reference< css::lang::XComponent > xComp( static_cast< ::cppu::OWeakObject* >((*aIter).get()), css::uno::UNO_QUERY );
    // if( xComp.is() ) try
    // {
    //  xComp->dispose();
    // }
    // catch( css::uno::Exception& )
    // {
    // }
    Broadcast( SfxStyleSheetHint( SfxHintId::StyleSheetErased, *p ) );
}

void SfxStyleSheetBasePool::Insert( SfxStyleSheetBase* p )
{
#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE( p, "svl::SfxStyleSheetBasePool::Insert(), no stylesheet?" );

    SfxStyleSheetIterator aIter(this, p->GetFamily(), p->GetMask());
    SfxStyleSheetBase* pOld = aIter.Find( p->GetName() );
    OSL_ENSURE( !pOld, "svl::SfxStyleSheetBasePool::Insert(), StyleSheet already inserted" );
    if( !p->GetParent().isEmpty() )
    {
        pOld = aIter.Find( p->GetParent() );
        OSL_ENSURE( pOld, "svl::SfxStyleSheetBasePool::Insert(), Parent not found!" );
    }
#endif
    StoreStyleSheet(rtl::Reference< SfxStyleSheetBase >( p ) );
    Broadcast( SfxStyleSheetHint( SfxHintId::StyleSheetCreated, *p ) );
}

namespace
{

struct StyleSheetDisposerFunctor final : public svl::StyleSheetDisposer
{
    explicit StyleSheetDisposerFunctor(SfxStyleSheetBasePool* pool)
            : mPool(pool) {}

    void
    Dispose(rtl::Reference<SfxStyleSheetBase> styleSheet) override
    {
        cppu::OWeakObject* weakObject = static_cast< ::cppu::OWeakObject* >(styleSheet.get());
        css::uno::Reference< css::lang::XComponent > xComp( weakObject, css::uno::UNO_QUERY );
        if( xComp.is() ) try
        {
            xComp->dispose();
        }
        catch( css::uno::Exception& )
        {
        }
        mPool->Broadcast( SfxStyleSheetHint( SfxHintId::StyleSheetErased, *styleSheet.get() ) );
    }

    SfxStyleSheetBasePool* mPool;
};

}

void SfxStyleSheetBasePool::Clear()
{
    StyleSheetDisposerFunctor cleanup(this);
    pImpl->mxIndexedStyleSheets->Clear(cleanup);
}

void SfxStyleSheetBasePool::ChangeParent(const OUString& rOld,
                                         const OUString& rNew,
                                         bool bVirtual)
{
    const SfxStyleSearchBits nTmpMask = GetSearchMask();
    SetSearchMask(GetSearchFamily());
    for( SfxStyleSheetBase* p = First(); p; p = Next() )
    {
        if( p->GetParent() == rOld )
        {
            if(bVirtual)
                p->SetParent( rNew );
            else
                p->aParent = rNew;
        }
    }
    SetSearchMask(GetSearchFamily(), nTmpMask);
}

void SfxStyleSheetBase::Load( SvStream&, sal_uInt16 )
{
}

SfxStyleSheet::SfxStyleSheet(const OUString &rName,
                             const SfxStyleSheetBasePool& r_Pool,
                             SfxStyleFamily eFam,
                             SfxStyleSearchBits mask )
    : SfxStyleSheetBase(rName, const_cast< SfxStyleSheetBasePool* >( &r_Pool ), eFam, mask)
{
}

SfxStyleSheet::SfxStyleSheet(const SfxStyleSheet& rStyle)
    : SfxStyleSheetBase(rStyle)
    , SfxListener( rStyle )
    , SfxBroadcaster( rStyle )
    , svl::StyleSheetUser()
{
}

SfxStyleSheet::~SfxStyleSheet()
{
    Broadcast( SfxStyleSheetHint( SfxHintId::StyleSheetInDestruction, *this ) );
}


bool SfxStyleSheet::SetParent( const OUString& rName )
{
    if(aParent == rName)
        return true;
    const OUString aOldParent(aParent);
    if(SfxStyleSheetBase::SetParent(rName))
    {
        // Remove from notification chain of the old parent if applicable
        if(!aOldParent.isEmpty())
        {
            SfxStyleSheet *pParent = static_cast<SfxStyleSheet *>(m_pPool->Find(aOldParent, nFamily));
            if(pParent)
                EndListening(*pParent);
        }
        // Add to the notification chain of the new parent
        if(!aParent.isEmpty())
        {
            SfxStyleSheet *pParent = static_cast<SfxStyleSheet *>(m_pPool->Find(aParent, nFamily));
            if(pParent)
                StartListening(*pParent);
        }
        return true;
    }
    return false;
}

/**
 * Notify all listeners
 */
void SfxStyleSheet::Notify(SfxBroadcaster& rBC, const SfxHint& rHint )
{
    Forward(rBC, rHint);
}

bool SfxStyleSheet::isUsedByModel() const
{
    return IsUsed();
}


SfxStyleSheetPool::SfxStyleSheetPool( SfxItemPool const& rSet)
: SfxStyleSheetBasePool( const_cast< SfxItemPool& >( rSet ) )
{
}

SfxStyleSheetBase* SfxStyleSheetPool::Create( const OUString& rName,
                                              SfxStyleFamily eFam, SfxStyleSearchBits mask )
{
    return new SfxStyleSheet( rName, *this, eFam, mask );
}

SfxUnoStyleSheet::SfxUnoStyleSheet( const OUString& _rName, const SfxStyleSheetBasePool& _rPool, SfxStyleFamily _eFamily, SfxStyleSearchBits _nMask )
: cppu::ImplInheritanceHelper<SfxStyleSheet, css::style::XStyle, css::lang::XUnoTunnel>(_rName, _rPool, _eFamily, _nMask)
{
}

SfxUnoStyleSheet* SfxUnoStyleSheet::getUnoStyleSheet( const css::uno::Reference< css::style::XStyle >& xStyle )
{
    SfxUnoStyleSheet* pRet = dynamic_cast< SfxUnoStyleSheet* >( xStyle.get() );
    if( !pRet )
    {
        css::uno::Reference< css::lang::XUnoTunnel > xUT( xStyle, css::uno::UNO_QUERY );
        if( xUT.is() )
            pRet = reinterpret_cast<SfxUnoStyleSheet*>(sal::static_int_cast<sal_uIntPtr>(xUT->getSomething( SfxUnoStyleSheet::getIdentifier())));
    }
    return pRet;
}

/**
 * XUnoTunnel
 */
::sal_Int64 SAL_CALL SfxUnoStyleSheet::getSomething( const css::uno::Sequence< ::sal_Int8 >& rId )
{
    if( rId.getLength() == 16 && 0 == memcmp( getIdentifier().getConstArray(), rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));
    }
    else
    {
        return 0;
    }
}

void
SfxStyleSheetBasePool::StoreStyleSheet(const rtl::Reference< SfxStyleSheetBase >& xStyle)
{
    pImpl->mxIndexedStyleSheets->AddStyleSheet(xStyle);
}

namespace
{
    class theSfxUnoStyleSheetIdentifier : public rtl::Static< UnoTunnelIdInit, theSfxUnoStyleSheetIdentifier > {};
}

const css::uno::Sequence< ::sal_Int8 >& SfxUnoStyleSheet::getIdentifier()
{
    return theSfxUnoStyleSheetIdentifier::get().getSeq();
}

void
SfxStyleSheetBasePool::Reindex()
{
    pImpl->mxIndexedStyleSheets->Reindex();
}

const svl::IndexedStyleSheets&
SfxStyleSheetBasePool::GetIndexedStyleSheets() const
{
    return *pImpl->mxIndexedStyleSheets;
}

SfxStyleSheetBase*
SfxStyleSheetBasePool::GetStyleSheetByPositionInIndex(unsigned pos)
{
    return pImpl->mxIndexedStyleSheets->GetStyleSheetByPosition(pos);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
