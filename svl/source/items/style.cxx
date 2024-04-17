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
#include <osl/diagnose.h>
#include <unotools/intlwrapper.hxx>
#include <svl/hint.hxx>
#include <svl/poolitem.hxx>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <svl/IndexedStyleSheets.hxx>
#include <svl/itemiter.hxx>
#include <unotools/syslocale.hxx>
#include <comphelper/servicehelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <utility>

#ifdef DBG_UTIL
namespace {

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

}

static DbgStyleSheetReferences aDbgStyleSheetReferences;
#endif


SfxStyleSheetModifiedHint::SfxStyleSheetModifiedHint
(
    OUString            aOldName,
    SfxStyleSheetBase&  rStyleSheet     // Remains with the caller
)
:   SfxStyleSheetHint( SfxHintId::StyleSheetModifiedExtended, rStyleSheet ),
    aName(std::move( aOldName ))
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
        mxIndexedStyleSheets(std::make_shared<svl::IndexedStyleSheets>()) {}
};


SfxStyleSheetBase::SfxStyleSheetBase( const OUString& rName, SfxStyleSheetBasePool* p, SfxStyleFamily eFam, SfxStyleSearchBits mask )
    : m_pPool( p )
    , nFamily( eFam )
    , aName( rName )
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
    : WeakImplHelper()
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

    if( aName == rName )
        return true;

    OUString aOldName = aName;
    SfxStyleSheetBase *pOther = m_pPool->Find( rName, nFamily ) ;
    if ( pOther && pOther != this )
        return false;

    if ( !aName.isEmpty() )
        m_pPool->ChangeParent(aName, rName, nFamily, false);

    if ( aFollow == aName )
        aFollow = rName;
    aName = rName;
    if (bReIndexNow)
        m_pPool->ReindexOnNameChange(aOldName, rName);

    m_pPool->Broadcast( SfxStyleSheetModifiedHint( aOldName, *this ) );
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

std::optional<SfxItemSet> SfxStyleSheetBase::GetItemSetForPreview()
{
    return GetItemSet();
}

/**
 * Set help file and ID and return it
 */
sal_uInt32 SfxStyleSheetBase::GetHelpId( OUString& rFile )
{
    rFile = aHelpFile;
    return nHelpId;
}

void SfxStyleSheetBase::SetHelpId( const OUString& rFile, sal_uInt32 nId )
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
    OUStringBuffer aDesc;

    IntlWrapper aIntlWrapper(SvtSysLocale().GetUILanguageTag());
    for (const SfxPoolItem* pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
    {
        OUString aItemPresentation;

        if ( !IsInvalidItem( pItem ) &&
             m_pPool->GetPool().GetPresentation(
                *pItem, eMetric, aItemPresentation, aIntlWrapper ) )
        {
            if ( !aDesc.isEmpty() && !aItemPresentation.isEmpty() )
                aDesc.append(" + ");
            if ( !aItemPresentation.isEmpty() )
                aDesc.append(aItemPresentation);
        }
    }
    return aDesc.makeStringAndClear();
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
        bool bMatchVisibility = bSearchHidden || !styleSheet.IsHidden() || bUsed;
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

SfxStyleSheetIterator::SfxStyleSheetIterator(const SfxStyleSheetBasePool *pBase,
                                             SfxStyleFamily eFam, SfxStyleSearchBits n)
    : pBasePool(pBase)
    , pCurrentStyle(nullptr)
    , mnCurrentPosition(0)
{
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

sal_Int32 SfxStyleSheetIterator::Count()
{
    sal_Int32 n = 0;
    if( IsTrivialSearch())
    {
        n = static_cast<sal_uInt16>(pBasePool->pImpl->mxIndexedStyleSheets->GetNumberOfStyleSheets());
    }
    else if(nMask == SfxStyleSearchBits::All)
    {
        n = static_cast<sal_uInt16>(pBasePool->pImpl->mxIndexedStyleSheets->GetStyleSheetsByFamily(nSearchFamily).size());
    }
    else
    {
        DoesStyleMatchStyleSheetPredicate predicate(this);
        n = pBasePool->pImpl->mxIndexedStyleSheets->GetNumberOfStyleSheetsWithPredicate(predicate);
    }
    return n;
}

SfxStyleSheetBase* SfxStyleSheetIterator::operator[](sal_Int32 nIdx)
{
    SfxStyleSheetBase* retval = nullptr;
    if( IsTrivialSearch())
    {
        retval = pBasePool->pImpl->mxIndexedStyleSheets->GetStyleSheetByPosition(nIdx);
        mnCurrentPosition = nIdx;
    }
    else if(nMask == SfxStyleSearchBits::All)
    {
        retval = pBasePool->pImpl->mxIndexedStyleSheets->GetStyleSheetsByFamily(nSearchFamily).at(nIdx);
        mnCurrentPosition = nIdx;
    }
    else
    {
        DoesStyleMatchStyleSheetPredicate predicate(this);
        rtl::Reference< SfxStyleSheetBase > ref =
                pBasePool->pImpl->mxIndexedStyleSheets->GetNthStyleSheetThatMatchesPredicate(nIdx, predicate);
        if (ref)
        {
            mnCurrentPosition = pBasePool->pImpl->mxIndexedStyleSheets->FindStyleSheetPosition(*ref);
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
        sal_Int32 nStyleSheets = pBasePool->pImpl->mxIndexedStyleSheets->GetNumberOfStyleSheets();
        sal_Int32 newPosition = mnCurrentPosition + 1;
        if (nStyleSheets > newPosition)
        {
            mnCurrentPosition = newPosition;
            retval = pBasePool->pImpl->mxIndexedStyleSheets->GetStyleSheetByPosition(mnCurrentPosition);
        }
    }
    else if(nMask == SfxStyleSearchBits::All)
    {
        sal_Int32 newPosition = mnCurrentPosition + 1;
        const std::vector<SfxStyleSheetBase*>& familyVector
            =
            pBasePool->pImpl->mxIndexedStyleSheets->GetStyleSheetsByFamily(nSearchFamily);
        if (static_cast<sal_Int32>(familyVector.size()) > newPosition)
        {
            mnCurrentPosition = newPosition;
            retval = familyVector[newPosition];
        }
    }
    else
    {
        DoesStyleMatchStyleSheetPredicate predicate(this);
        rtl::Reference< SfxStyleSheetBase > ref =
                pBasePool->pImpl->mxIndexedStyleSheets->GetNthStyleSheetThatMatchesPredicate(
                        0, predicate, mnCurrentPosition+1);
        retval = ref.get();
        if (retval != nullptr) {
            mnCurrentPosition = pBasePool->pImpl->mxIndexedStyleSheets->FindStyleSheetPosition(*ref);
        }
    }
    pCurrentStyle = retval;
    return retval;
}

SfxStyleSheetBase* SfxStyleSheetIterator::Find(const OUString& rStr)
{
    DoesStyleMatchStyleSheetPredicate predicate(this);

    std::vector<sal_Int32> positions =
            pBasePool->pImpl->mxIndexedStyleSheets->FindPositionsByNameAndPredicate(rStr, predicate,
                    svl::IndexedStyleSheets::SearchBehavior::ReturnFirst);
    if (positions.empty()) {
        return nullptr;
    }

    sal_Int32 pos = positions.front();
    SfxStyleSheetBase* pStyle = pBasePool->pImpl->mxIndexedStyleSheets->GetStyleSheetByPosition(pos);
    mnCurrentPosition = pos;
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

SfxStyleSheetIterator* SfxStyleSheetBasePool::GetCachedIterator()
{
    return pImpl->pIter.get();
}

SfxStyleSheetIterator& SfxStyleSheetBasePool::GetIterator_Impl(SfxStyleFamily eFamily, SfxStyleSearchBits eMask)
{
    if (!pImpl->pIter || (pImpl->pIter->GetSearchMask() != eMask) || (pImpl->pIter->GetSearchFamily() != eFamily))
        pImpl->pIter = CreateIterator(eFamily, eMask);
    return *pImpl->pIter;
}

SfxStyleSheetBasePool::SfxStyleSheetBasePool( SfxItemPool& r ) :
    pImpl(new SfxStyleSheetBasePool_Impl),
    rPool(r)
{
#ifdef DBG_UTIL
    aDbgStyleSheetReferences.mnPools++;
#endif
}

SfxStyleSheetBasePool::SfxStyleSheetBasePool( const SfxStyleSheetBasePool& r ) :
    SfxBroadcaster( r ),
    WeakImplHelper(),
    pImpl(new SfxStyleSheetBasePool_Impl),
    rPool(r.rPool)
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

std::unique_ptr<SfxStyleSheetIterator> SfxStyleSheetBasePool::CreateIterator
(
 SfxStyleFamily eFam,
 SfxStyleSearchBits mask
)
{
    return std::make_unique<SfxStyleSheetIterator>(this,eFam,mask);
}

rtl::Reference<SfxStyleSheetBase> SfxStyleSheetBasePool::Create
(
    const OUString& rName,
    SfxStyleFamily eFam,
    SfxStyleSearchBits mask
)
{
    return new SfxStyleSheetBase( rName, this, eFam, mask );
}

rtl::Reference<SfxStyleSheetBase> SfxStyleSheetBasePool::Create( const SfxStyleSheetBase& r )
{
    return new SfxStyleSheetBase( r );
}

SfxStyleSheetBase& SfxStyleSheetBasePool::Make( const OUString& rName, SfxStyleFamily eFam, SfxStyleSearchBits mask)
{
    OSL_ENSURE( eFam != SfxStyleFamily::All, "svl::SfxStyleSheetBasePool::Make(), FamilyAll is not an allowed Family" );

    SfxStyleSheetIterator aIter(this, eFam, mask);
    rtl::Reference< SfxStyleSheetBase > xStyle( aIter.Find( rName ) );
    OSL_ENSURE( !xStyle.is(), "svl::SfxStyleSheetBasePool::Make(), StyleSheet already exists" );

    if( !xStyle.is() )
    {
        xStyle = Create( rName, eFam, mask );
        StoreStyleSheet(xStyle);
        Broadcast(SfxStyleSheetHint(SfxHintId::StyleSheetCreated, *xStyle));
    }
    return *xStyle;
}

/**
 * Helper function: If a template with this name exists it is created
 * anew. All templates that have this template as a parent are reconnected.
 */
void SfxStyleSheetBasePool::Add( const SfxStyleSheetBase& rSheet )
{
    SfxStyleSheetIterator aIter(this, rSheet.GetFamily(), SfxStyleSearchBits::All);
    SfxStyleSheetBase* pOld = aIter.Find( rSheet.GetName() );
    if (pOld) {
        Remove( pOld );
    }
    rtl::Reference< SfxStyleSheetBase > xNew( Create( rSheet ) );
    pImpl->mxIndexedStyleSheets->AddStyleSheet(xNew);
    Broadcast(SfxStyleSheetHint(SfxHintId::StyleSheetChanged, *xNew));
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

SfxStyleSheetBase* SfxStyleSheetBasePool::Find(const OUString& rName,
                                               SfxStyleFamily eFamily,
                                               SfxStyleSearchBits eMask)
{
    SfxStyleSheetIterator aIter(this, eFamily, eMask);
    return aIter.Find(rName);
}

SfxStyleSheetBase* SfxStyleSheetBasePool::First(SfxStyleFamily eFamily, SfxStyleSearchBits eMask)
{
    return GetIterator_Impl(eFamily, eMask).First();
}

SfxStyleSheetBase* SfxStyleSheetBasePool::Next()
{
    assert(pImpl->pIter && "Next called without a previous First");
    return pImpl->pIter->Next();
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

    // Adapt all styles which have this style as parent
    ChangeParent(p->GetName(), p->GetParent(), p->GetFamily());

    // #120015# Do not dispose, the removed StyleSheet may still be used in
    // existing SdrUndoAttrObj incarnations. Rely on refcounting for disposal,
    // this works well under normal conditions (checked breaking and counting
    // on SfxStyleSheetBase constructors and destructors)

    // css::uno::Reference< css::lang::XComponent > xComp( getXWeak((*aIter).get()), css::uno::UNO_QUERY );
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
        cppu::OWeakObject* weakObject = styleSheet.get();
        css::uno::Reference< css::lang::XComponent > xComp( weakObject, css::uno::UNO_QUERY );
        if( xComp.is() ) try
        {
            xComp->dispose();
        }
        catch( css::uno::Exception& )
        {
        }
        mPool->Broadcast(SfxStyleSheetHint(SfxHintId::StyleSheetErased, *styleSheet));
    }

    SfxStyleSheetBasePool* mPool;
};

}

void SfxStyleSheetBasePool::Clear()
{
    StyleSheetDisposerFunctor cleanup(this);
    pImpl->mxIndexedStyleSheets->Clear(cleanup);
}

void SfxStyleSheetBasePool::ChangeParent(std::u16string_view rOld,
                                         const OUString& rNew,
                                         SfxStyleFamily eFamily,
                                         bool bVirtual)
{
    for( SfxStyleSheetBase* p = First(eFamily); p; p = Next() )
    {
        if( p->GetParent() == rOld )
        {
            if(bVirtual)
                p->SetParent( rNew );
            else
                p->aParent = rNew;
        }
    }
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

bool SfxStyleSheet::IsSfxStyleSheet() const
{
    return true;
}

SfxStyleSheetPool::SfxStyleSheetPool( SfxItemPool const& rSet)
: SfxStyleSheetBasePool( const_cast< SfxItemPool& >( rSet ) )
{
}

rtl::Reference<SfxStyleSheetBase> SfxStyleSheetPool::Create( const OUString& rName,
                                              SfxStyleFamily eFam, SfxStyleSearchBits mask )
{
    return new SfxStyleSheet( rName, *this, eFam, mask );
}

SfxUnoStyleSheet::SfxUnoStyleSheet( const OUString& _rName, const SfxStyleSheetBasePool& _rPool, SfxStyleFamily _eFamily, SfxStyleSearchBits _nMask )
: cppu::ImplInheritanceHelper<SfxStyleSheet, css::style::XStyle>(_rName, _rPool, _eFamily, _nMask)
{
}

SfxUnoStyleSheet* SfxUnoStyleSheet::getUnoStyleSheet( const css::uno::Reference< css::style::XStyle >& xStyle )
{
    return dynamic_cast<SfxUnoStyleSheet*>(xStyle.get());
}

void
SfxStyleSheetBasePool::StoreStyleSheet(const rtl::Reference< SfxStyleSheetBase >& xStyle)
{
    pImpl->mxIndexedStyleSheets->AddStyleSheet(xStyle);
}

void
SfxStyleSheetBasePool::Reindex()
{
    pImpl->mxIndexedStyleSheets->Reindex();
}

void
SfxStyleSheetBasePool::ReindexOnNameChange(const OUString& rOldName, const OUString& rNewName)
{
    pImpl->mxIndexedStyleSheets->ReindexOnNameChange(rOldName, rNewName);
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
