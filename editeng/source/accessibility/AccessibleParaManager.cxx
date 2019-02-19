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


// Global header


#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/weakref.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>


// Project-local header


#include <editeng/unoedhlp.hxx>
#include <editeng/unopracc.hxx>
#include <editeng/unoedsrc.hxx>
#include <editeng/AccessibleParaManager.hxx>
#include <editeng/AccessibleEditableTextPara.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;


namespace accessibility
{
    AccessibleParaManager::AccessibleParaManager() :
        maChildren(1),
        maEEOffset( 0, 0 ),
        mnFocusedChild( -1 ),
        mbActive( false )
    {
    }

    AccessibleParaManager::~AccessibleParaManager()
    {
        // owner is responsible for possible child death
    }

    void AccessibleParaManager::SetAdditionalChildStates( const VectorOfStates& rChildStates )
    {
        maChildStates = rChildStates;
    }

    void AccessibleParaManager::SetNum( sal_Int32 nNumParas )
    {
        if( static_cast<size_t>(nNumParas) < maChildren.size() )
            Release( nNumParas, maChildren.size() );

        maChildren.resize( nNumParas );

        if( mnFocusedChild >= nNumParas )
            mnFocusedChild = -1;
    }

    sal_Int32 AccessibleParaManager::GetNum() const
    {
        size_t nSize = maChildren.size();
        if (nSize > SAL_MAX_INT32)
        {
            SAL_WARN( "editeng", "AccessibleParaManager::GetNum - overflow " << nSize);
            return SAL_MAX_INT32;
        }
        return static_cast<sal_Int32>(nSize);
    }

    AccessibleParaManager::VectorOfChildren::iterator AccessibleParaManager::begin()
    {
        return maChildren.begin();
    }

    AccessibleParaManager::VectorOfChildren::iterator AccessibleParaManager::end()
    {
        return maChildren.end();
    }

    void AccessibleParaManager::FireEvent( sal_Int32 nPara,
                                           const sal_Int16 nEventId ) const
    {
        DBG_ASSERT( 0 <= nPara && maChildren.size() > static_cast<size_t>(nPara),
                "AccessibleParaManager::FireEvent: invalid index" );

        if( 0 <= nPara && maChildren.size() > static_cast<size_t>(nPara) )
        {
            auto aChild( GetChild( nPara ).first.get() );
            if( aChild.is() )
                aChild->FireEvent( nEventId );
        }
    }

    bool AccessibleParaManager::IsReferencable(
        rtl::Reference<AccessibleEditableTextPara> const & aChild)
    {
        return aChild.is();
    }

    bool AccessibleParaManager::IsReferencable( sal_Int32 nChild ) const
    {
        DBG_ASSERT( 0 <= nChild && maChildren.size() > static_cast<size_t>(nChild),
                "AccessibleParaManager::IsReferencable: invalid index" );

        if( 0 <= nChild && maChildren.size() > static_cast<size_t>(nChild) )
        {
            // retrieve hard reference from weak one
            return IsReferencable( GetChild( nChild ).first.get() );
        }
        else
        {
            return false;
        }
    }

    AccessibleParaManager::WeakChild AccessibleParaManager::GetChild( sal_Int32 nParagraphIndex ) const
    {
        DBG_ASSERT( 0 <= nParagraphIndex && maChildren.size() > static_cast<size_t>(nParagraphIndex),
                "AccessibleParaManager::GetChild: invalid index" );

        if( 0 <= nParagraphIndex && maChildren.size() > static_cast<size_t>(nParagraphIndex) )
        {
            return maChildren[ nParagraphIndex ];
        }
        else
        {
            return WeakChild();
        }
    }

    AccessibleParaManager::Child AccessibleParaManager::CreateChild( sal_Int32                              nChild,
                                                                     const uno::Reference< XAccessible >&   xFrontEnd,
                                                                     SvxEditSourceAdapter&                  rEditSource,
                                                                     sal_Int32                              nParagraphIndex )
    {
        DBG_ASSERT( 0 <= nParagraphIndex && maChildren.size() > static_cast<size_t>(nParagraphIndex),
                "AccessibleParaManager::CreateChild: invalid index" );

        if( 0 <= nParagraphIndex && maChildren.size() > static_cast<size_t>(nParagraphIndex) )
        {
            // retrieve hard reference from weak one
            auto aChild( GetChild( nParagraphIndex ).first.get() );

            if( !IsReferencable( nParagraphIndex ) )
            {
                // there is no hard reference available, create object then
                // #i27138#
                aChild = new AccessibleEditableTextPara(xFrontEnd, this);

                InitChild( *aChild, rEditSource, nChild, nParagraphIndex );

                maChildren[ nParagraphIndex ] = WeakChild( aChild, aChild->getBounds() );
            }

            return Child( aChild.get(), GetChild( nParagraphIndex ).second );
        }
        else
        {
            return Child();
        }
    }

    void AccessibleParaManager::SetEEOffset( const Point& rOffset )
    {
        maEEOffset = rOffset;

        MemFunAdapter< const Point& > aAdapter( &::accessibility::AccessibleEditableTextPara::SetEEOffset, rOffset );
        std::for_each( begin(), end(), aAdapter );
    }

    void AccessibleParaManager::SetActive( bool bActive )
    {
        mbActive = bActive;

        if( bActive )
        {
            SetState( AccessibleStateType::ACTIVE );
            SetState( AccessibleStateType::EDITABLE );
        }
        else
        {
            UnSetState( AccessibleStateType::ACTIVE );
            UnSetState( AccessibleStateType::EDITABLE );
        }
    }

    void AccessibleParaManager::SetFocus( sal_Int32 nChild )
    {
        if( mnFocusedChild != -1 )
            UnSetState( mnFocusedChild, AccessibleStateType::FOCUSED );

        mnFocusedChild = nChild;

        if( mnFocusedChild != -1 )
            SetState( mnFocusedChild, AccessibleStateType::FOCUSED );
    }

    void AccessibleParaManager::InitChild( AccessibleEditableTextPara&  rChild,
                                           SvxEditSourceAdapter&        rEditSource,
                                           sal_Int32                    nChild,
                                           sal_Int32                    nParagraphIndex ) const
    {
        rChild.SetEditSource( &rEditSource );
        rChild.SetIndexInParent( nChild );
        rChild.SetParagraphIndex( nParagraphIndex );

        rChild.SetEEOffset( maEEOffset );

        if( mbActive )
        {
            rChild.SetState( AccessibleStateType::ACTIVE );
            rChild.SetState( AccessibleStateType::EDITABLE );
        }

        if( mnFocusedChild == nParagraphIndex )
            rChild.SetState( AccessibleStateType::FOCUSED );

        // add states passed from outside
        for( const auto& rState : maChildStates )
            rChild.SetState( rState );
    }

    void AccessibleParaManager::SetState( sal_Int32 nChild, const sal_Int16 nStateId )
    {
        MemFunAdapter< const sal_Int16 > aFunc( &AccessibleEditableTextPara::SetState,
                                                nStateId );
        aFunc( GetChild(nChild) );
    }

    void AccessibleParaManager::SetState( const sal_Int16 nStateId )
    {
        std::for_each( begin(), end(),
                         MemFunAdapter< const sal_Int16 >( &AccessibleEditableTextPara::SetState,
                                                           nStateId ) );
    }

    void AccessibleParaManager::UnSetState( sal_Int32 nChild, const sal_Int16 nStateId )
    {
        MemFunAdapter< const sal_Int16 > aFunc( &AccessibleEditableTextPara::UnSetState,
                                                nStateId );
        aFunc( GetChild(nChild) );
    }

    void AccessibleParaManager::UnSetState( const sal_Int16 nStateId )
    {
        std::for_each( begin(), end(),
                         MemFunAdapter< const sal_Int16 >( &AccessibleEditableTextPara::UnSetState,
                                                           nStateId ) );
    }

    // not generic yet, no arguments...
    class AccessibleParaManager_DisposeChildren
    {
    public:
        AccessibleParaManager_DisposeChildren() {}
        void operator()( ::accessibility::AccessibleEditableTextPara& rPara )
        {
            rPara.Dispose();
        }
    };

    void AccessibleParaManager::Dispose()
    {
        AccessibleParaManager_DisposeChildren aFunctor;

        std::for_each( begin(), end(),
                         WeakChildAdapter< AccessibleParaManager_DisposeChildren > (aFunctor) );
    }

    // not generic yet, too many method arguments...
    class StateChangeEvent
    {
    public:
        typedef void return_type;
        StateChangeEvent( const sal_Int16 nEventId,
                          const uno::Any& rNewValue,
                          const uno::Any& rOldValue ) :
            mnEventId( nEventId ),
            mrNewValue( rNewValue ),
            mrOldValue( rOldValue ) {}
        void operator()( ::accessibility::AccessibleEditableTextPara const & rPara )
        {
            rPara.FireEvent( mnEventId, mrNewValue, mrOldValue );
        }

    private:
        const sal_Int16 mnEventId;
        const uno::Any& mrNewValue;
        const uno::Any& mrOldValue;
    };

    void AccessibleParaManager::FireEvent( sal_Int32 nStartPara,
                                           sal_Int32 nEndPara,
                                           const sal_Int16 nEventId,
                                           const uno::Any& rNewValue,
                                           const uno::Any& rOldValue ) const
    {
        DBG_ASSERT( 0 <= nStartPara && 0 <= nEndPara &&
                    maChildren.size() > static_cast<size_t>(nStartPara) &&
                    maChildren.size() >= static_cast<size_t>(nEndPara) &&
                    nEndPara >= nStartPara, "AccessibleParaManager::FireEvent: invalid index" );


        if( 0 <= nStartPara && 0 <= nEndPara &&
                maChildren.size() > static_cast<size_t>(nStartPara) &&
                maChildren.size() >= static_cast<size_t>(nEndPara) &&
                nEndPara >= nStartPara )
        {
            VectorOfChildren::const_iterator front = maChildren.begin();
            VectorOfChildren::const_iterator back = front;

            std::advance( front, nStartPara );
            std::advance( back, nEndPara );

            StateChangeEvent aFunctor( nEventId, rNewValue, rOldValue );

            std::for_each( front, back, AccessibleParaManager::WeakChildAdapter< StateChangeEvent >( aFunctor ) );
        }
    }

    class ReleaseChild
    {
    public:
        AccessibleParaManager::WeakChild operator()( const AccessibleParaManager::WeakChild& rPara )
        {
            AccessibleParaManager::ShutdownPara( rPara );

            // clear reference
            return AccessibleParaManager::WeakChild();
        }
    };

    void AccessibleParaManager::Release( sal_Int32 nStartPara, sal_Int32 nEndPara )
    {
        DBG_ASSERT( 0 <= nStartPara && 0 <= nEndPara &&
                    maChildren.size() > static_cast<size_t>(nStartPara) &&
                    maChildren.size() >= static_cast<size_t>(nEndPara),
                    "AccessibleParaManager::Release: invalid index" );

        if( 0 <= nStartPara && 0 <= nEndPara &&
                maChildren.size() > static_cast<size_t>(nStartPara) &&
                maChildren.size() >= static_cast<size_t>(nEndPara) )
        {
            VectorOfChildren::iterator front = maChildren.begin();
            VectorOfChildren::iterator back = front;

            std::advance( front, nStartPara );
            std::advance( back, nEndPara );

            std::transform( front, back, front, ReleaseChild() );
        }
    }

    void AccessibleParaManager::ShutdownPara( const WeakChild& rChild )
    {
        auto aChild( rChild.first.get() );

        if( IsReferencable( aChild ) )
            aChild->SetEditSource( nullptr );
    }

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
