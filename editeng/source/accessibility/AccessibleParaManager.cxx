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
#include "precompiled_editeng.hxx"

//------------------------------------------------------------------------
//
// Global header
//
//------------------------------------------------------------------------
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

//------------------------------------------------------------------------
//
// Project-local header
//
//------------------------------------------------------------------------

#include <editeng/unoedhlp.hxx>
#include <editeng/unopracc.hxx>
#include <editeng/unoedsrc.hxx>
#include "editeng/AccessibleParaManager.hxx"
#include "editeng/AccessibleEditableTextPara.hxx"


using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;



namespace accessibility
{
    AccessibleParaManager::AccessibleParaManager() :
        maChildren(1),
        maEEOffset( 0, 0 ),
        mnFocusedChild( -1 ),
        mbActive( sal_False )
    {
    }

    AccessibleParaManager::~AccessibleParaManager()
    {
        // owner is responsible for possible child defuncs
    }

    void AccessibleParaManager::SetAdditionalChildStates( const VectorOfStates& rChildStates )
    {
        maChildStates = rChildStates;
    }

    const AccessibleParaManager::VectorOfStates& AccessibleParaManager::GetAdditionalChildStates() const
    {
        return maChildStates;
    }

    void AccessibleParaManager::SetNum( sal_Int32 nNumParas )
    {
        if( (size_t)nNumParas < maChildren.size() )
            Release( nNumParas, maChildren.size() );

        maChildren.resize( nNumParas );

        if( mnFocusedChild >= nNumParas )
            mnFocusedChild = -1;
    }

    sal_uInt32 AccessibleParaManager::GetNum() const
    {
        return maChildren.size();
    }

    AccessibleParaManager::VectorOfChildren::iterator AccessibleParaManager::begin()
    {
        return maChildren.begin();
    }

    AccessibleParaManager::VectorOfChildren::iterator AccessibleParaManager::end()
    {
        return maChildren.end();
    }

    AccessibleParaManager::VectorOfChildren::const_iterator AccessibleParaManager::begin() const
    {
        return maChildren.begin();
    }

    AccessibleParaManager::VectorOfChildren::const_iterator AccessibleParaManager::end() const
    {
        return maChildren.end();
    }

    void AccessibleParaManager::Release( sal_uInt32 nPara )
    {
        DBG_ASSERT( maChildren.size() > nPara, "AccessibleParaManager::Release: invalid index" );

        if( maChildren.size() > nPara )
        {
            ShutdownPara( GetChild( nPara ) );

            // clear reference and rect
            maChildren[ nPara ] = WeakChild();
        }
    }

    void AccessibleParaManager::FireEvent( sal_uInt32 nPara,
                                           const sal_Int16 nEventId,
                                           const uno::Any& rNewValue,
                                           const uno::Any& rOldValue ) const
    {
        DBG_ASSERT( maChildren.size() > nPara, "AccessibleParaManager::FireEvent: invalid index" );

        if( maChildren.size() > nPara )
        {
            WeakPara::HardRefType maChild( GetChild( nPara ).first.get() );
            if( maChild.is() )
                maChild->FireEvent( nEventId, rNewValue, rOldValue );
        }
    }

    sal_Bool AccessibleParaManager::IsReferencable( WeakPara::HardRefType aChild )
    {
        return aChild.is();
    }

    sal_Bool AccessibleParaManager::IsReferencable( sal_uInt32 nChild ) const
    {
        DBG_ASSERT( maChildren.size() > nChild, "AccessibleParaManager::IsReferencable: invalid index" );

        if( maChildren.size() > nChild )
        {
            // retrieve hard reference from weak one
            return IsReferencable( GetChild( nChild ).first.get() );
        }
        else
        {
            return sal_False;
        }
    }

    AccessibleParaManager::WeakChild AccessibleParaManager::GetChild( sal_uInt32 nParagraphIndex ) const
    {
        DBG_ASSERT( maChildren.size() > nParagraphIndex, "AccessibleParaManager::GetChild: invalid index" );

        if( maChildren.size() > nParagraphIndex )
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
                                                                     sal_uInt32                             nParagraphIndex )
    {
        DBG_ASSERT( maChildren.size() > nParagraphIndex, "AccessibleParaManager::CreateChild: invalid index" );

        if( maChildren.size() > nParagraphIndex )
        {
            // retrieve hard reference from weak one
            WeakPara::HardRefType aChild( GetChild( nParagraphIndex ).first.get() );

            if( !IsReferencable( nParagraphIndex ) )
            {
                // there is no hard reference available, create object then
                // #i27138#
                AccessibleEditableTextPara* pChild = new AccessibleEditableTextPara( xFrontEnd, this );
                uno::Reference< XAccessible > xChild( static_cast< ::cppu::OWeakObject* > (pChild), uno::UNO_QUERY );

                if( !xChild.is() )
                    throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Child creation failed")), xFrontEnd);

                aChild = WeakPara::HardRefType( xChild, pChild );

                InitChild( *aChild, rEditSource, nChild, nParagraphIndex );

                maChildren[ nParagraphIndex ] = WeakChild( aChild, pChild->getBounds() );
            }

            return Child( aChild.getRef(), GetChild( nParagraphIndex ).second );
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
        ::std::for_each( begin(), end(), aAdapter );
    }

    void AccessibleParaManager::SetActive( sal_Bool bActive )
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
                                           sal_uInt32                   nParagraphIndex ) const
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

        if( mnFocusedChild == static_cast<sal_Int32>(nParagraphIndex) )
            rChild.SetState( AccessibleStateType::FOCUSED );

        // add states passed from outside
        for( VectorOfStates::const_iterator aIt = maChildStates.begin(), aEnd = maChildStates.end(); aIt != aEnd; ++aIt )
            rChild.SetState( *aIt );
    }

    void AccessibleParaManager::SetState( sal_Int32 nChild, const sal_Int16 nStateId )
    {
        MemFunAdapter< const sal_Int16 > aFunc( &AccessibleEditableTextPara::SetState,
                                                nStateId );
        aFunc( GetChild(nChild) );
    }

    void AccessibleParaManager::SetState( const sal_Int16 nStateId )
    {
        ::std::for_each( begin(), end(),
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
        ::std::for_each( begin(), end(),
                         MemFunAdapter< const sal_Int16 >( &AccessibleEditableTextPara::UnSetState,
                                                           nStateId ) );
    }

    void AccessibleParaManager::SetEditSource( SvxEditSourceAdapter* pEditSource )
    {
        MemFunAdapter< SvxEditSourceAdapter* > aAdapter( &::accessibility::AccessibleEditableTextPara::SetEditSource, pEditSource );
        ::std::for_each( begin(), end(), aAdapter );
    }

    // not generic yet, no arguments...
    class AccessibleParaManager_DisposeChildren : public ::std::unary_function< ::accessibility::AccessibleEditableTextPara&, void >
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

        ::std::for_each( begin(), end(),
                         WeakChildAdapter< AccessibleParaManager_DisposeChildren > (aFunctor) );
    }

    // not generic yet, too many method arguments...
    class StateChangeEvent : public ::std::unary_function< ::accessibility::AccessibleEditableTextPara&, void >
    {
    public:
        typedef void return_type;
        StateChangeEvent( const sal_Int16 nEventId,
                          const uno::Any& rNewValue,
                          const uno::Any& rOldValue ) :
            mnEventId( nEventId ),
            mrNewValue( rNewValue ),
            mrOldValue( rOldValue ) {}
        void operator()( ::accessibility::AccessibleEditableTextPara& rPara )
        {
            rPara.FireEvent( mnEventId, mrNewValue, mrOldValue );
        }

    private:
        const sal_Int16 mnEventId;
        const uno::Any& mrNewValue;
        const uno::Any& mrOldValue;
    };

    void AccessibleParaManager::FireEvent( sal_uInt32 nStartPara,
                                           sal_uInt32 nEndPara,
                                           const sal_Int16 nEventId,
                                           const uno::Any& rNewValue,
                                           const uno::Any& rOldValue ) const
    {
        DBG_ASSERT( maChildren.size() > nStartPara &&
                    maChildren.size() >= nEndPara , "AccessibleParaManager::FireEvent: invalid index" );

        if( maChildren.size() > nStartPara &&
            maChildren.size() >= nEndPara )
        {
            VectorOfChildren::const_iterator front = maChildren.begin();
            VectorOfChildren::const_iterator back = front;

            ::std::advance( front, nStartPara );
            ::std::advance( back, nEndPara );

            StateChangeEvent aFunctor( nEventId, rNewValue, rOldValue );

            ::std::for_each( front, back, AccessibleParaManager::WeakChildAdapter< StateChangeEvent >( aFunctor ) );
        }
    }

    class ReleaseChild : public ::std::unary_function< const AccessibleParaManager::WeakChild&, AccessibleParaManager::WeakChild >
    {
    public:
        AccessibleParaManager::WeakChild operator()( const AccessibleParaManager::WeakChild& rPara )
        {
            AccessibleParaManager::ShutdownPara( rPara );

            // clear reference
            return AccessibleParaManager::WeakChild();
        }
    };

    void AccessibleParaManager::Release( sal_uInt32 nStartPara, sal_uInt32 nEndPara )
    {
        DBG_ASSERT( maChildren.size() > nStartPara &&
                    maChildren.size() >= nEndPara, "AccessibleParaManager::Release: invalid index" );

        if( maChildren.size() > nStartPara &&
            maChildren.size() >= nEndPara )
        {
            VectorOfChildren::iterator front = maChildren.begin();
            VectorOfChildren::iterator back = front;

            ::std::advance( front, nStartPara );
            ::std::advance( back, nEndPara );

            ::std::transform( front, back, front, ReleaseChild() );
        }
    }

    void AccessibleParaManager::ShutdownPara( const WeakChild& rChild )
    {
        WeakPara::HardRefType aChild( rChild.first.get() );

        if( IsReferencable( aChild ) )
            aChild->SetEditSource( NULL );
    }

}

//------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
