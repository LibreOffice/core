/*************************************************************************
 *
 *  $RCSfile: AccessibleParaManager.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 16:54:18 $
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

//------------------------------------------------------------------------
//
// Global header
//
//------------------------------------------------------------------------

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

//------------------------------------------------------------------------
//
// Project-local header
//
//------------------------------------------------------------------------

#include "unoedhlp.hxx"
#include "unopracc.hxx"
#include "unoedsrc.hxx"
#include "AccessibleParaManager.hxx"
#include "AccessibleEditableTextPara.hxx"


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

    void AccessibleParaManager::SetNum( sal_uInt32 nNumParas )
    {
        if( nNumParas < maChildren.size() )
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
                AccessibleEditableTextPara* pChild = new AccessibleEditableTextPara( xFrontEnd );
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
