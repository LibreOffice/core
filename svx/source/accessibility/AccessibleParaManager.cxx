/*************************************************************************
 *
 *  $RCSfile: AccessibleParaManager.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: thb $ $Date: 2002-05-23 12:44:04 $
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

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <drafts/com/sun/star/accessibility/XAccessible.hpp>
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
using namespace ::drafts::com::sun::star::accessibility;



namespace accessibility
{
    AccessibleParaManager::AccessibleParaManager()
    {
    }

    AccessibleParaManager::~AccessibleParaManager()
    {
        // owner is responsible for possible child defuncs
    }

    void AccessibleParaManager::SetNum( sal_Int32 nNumParas )
    {
        maChildren.resize( nNumParas );
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

    void AccessibleParaManager::Release( sal_Int32 nPara )
    {
        ShutdownPara( maChildren[ nPara ] );

        // clear reference and rect
        maChildren[ nPara ] = WeakChild();
    }

    void AccessibleParaManager::FireEvent( sal_Int32 nPara,
                                           const sal_Int16 nEventId,
                                           const uno::Any& rNewValue,
                                           const uno::Any& rOldValue ) const
    {
        WeakPara::HardRefType maChild( maChildren[ nPara  ].first.get() );
        if( maChild.is() )
            maChild->FireEvent( nEventId, rNewValue, rOldValue );
    }

    sal_Bool AccessibleParaManager::IsReferencable( WeakPara::HardRefType aChild )
    {
        return aChild.is();
    }

    sal_Bool AccessibleParaManager::IsReferencable( sal_Int32 nChild ) const
    {
        // retrieve hard reference from weak one
        return IsReferencable( maChildren[ nChild ].first.get() );
    }

    AccessibleParaManager::WeakChild AccessibleParaManager::GetChild( sal_Int32 nParagraphIndex )
    {
        return maChildren[ nParagraphIndex ];
    }

    AccessibleParaManager::Child AccessibleParaManager::CreateChild( sal_Int32                          nChild,
                                                                     const uno::Reference< XAccessible >&   xFrontEnd,
                                                                     SvxEditSourceAdapter&              rEditSource,
                                                                     sal_Int32                          nParagraphIndex )
    {
        // retrieve hard reference from weak one
        WeakPara::HardRefType aChild( maChildren[ nParagraphIndex ].first.get() );

        if( !IsReferencable( nParagraphIndex ) )
        {
            // there is no hard reference available, create object then
            AccessibleEditableTextPara* pChild = new AccessibleEditableTextPara( xFrontEnd );
            uno::Reference< XAccessible > xChild( static_cast< ::cppu::OWeakObject* > (pChild), uno::UNO_QUERY );

            if( !xChild.is() )
                throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Child creation failed")), xFrontEnd);

            aChild = WeakPara::HardRefType( xChild, pChild );

            aChild->SetEditSource( &rEditSource );
            aChild->SetIndexInParent( nChild );
            aChild->SetParagraphIndex( nParagraphIndex );

            maChildren[ nParagraphIndex ] = WeakChild( aChild, pChild->getBounds() );
        }

        return Child( aChild.getRef(), maChildren[ nParagraphIndex ].second );
    }

    void AccessibleParaManager::SetEEOffset( const Point& rOffset )
    {
        MemFunAdapter< const Point& > aAdapter( &accessibility::AccessibleEditableTextPara::SetEEOffset, rOffset );
        ::std::for_each( begin(), end(), aAdapter );
    }

    void AccessibleParaManager::SetEditSource( SvxEditSourceAdapter* pEditSource )
    {
        MemFunAdapter< SvxEditSourceAdapter* > aAdapter( &accessibility::AccessibleEditableTextPara::SetEditSource, pEditSource );
        ::std::for_each( begin(), end(), aAdapter );
    }

    // not generic yet, too many method arguments...
    class StateChangeEvent : public ::std::unary_function< accessibility::AccessibleEditableTextPara&, void >
    {
    public:
        typedef void return_type;
        StateChangeEvent( const sal_Int16 nEventId,
                          const uno::Any& rNewValue,
                          const uno::Any& rOldValue ) :
            mnEventId( nEventId ),
            mrNewValue( rNewValue ),
            mrOldValue( rOldValue ) {}
        void operator()( accessibility::AccessibleEditableTextPara& rPara )
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
        VectorOfChildren::const_iterator front = maChildren.begin();
        VectorOfChildren::const_iterator back = front;

        ::std::advance( front, nStartPara );
        ::std::advance( back, nEndPara );

        StateChangeEvent aFunctor( nEventId, rNewValue, rOldValue );

        ::std::for_each( front, back, AccessibleParaManager::WeakChildAdapter< StateChangeEvent >( aFunctor ) );
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

    void AccessibleParaManager::Release( sal_Int32 nStartPara, sal_Int32 nEndPara )
    {
        VectorOfChildren::iterator front = maChildren.begin();
        VectorOfChildren::iterator back = front;

        ::std::advance( front, nStartPara );
        ::std::advance( back, nEndPara );

        ::std::transform( front, back, front, ReleaseChild() );
    }

    void AccessibleParaManager::ShutdownPara( const WeakChild& rChild )
    {
        WeakPara::HardRefType aChild( rChild.first.get() );

        if( IsReferencable( aChild ) )
            aChild->SetEditSource( NULL );
    }

}

//------------------------------------------------------------------------
