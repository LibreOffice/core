/*************************************************************************
 *
 *  $RCSfile: AccessibleStaticTextBase.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:00:26 $
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

#include <limits.h>
#include <vector>
#include <algorithm>

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLETEXTTYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleTextType.hpp>
#endif

//------------------------------------------------------------------------
//
// Project-local header
//
//------------------------------------------------------------------------

#include "editdata.hxx"
#include "unopracc.hxx"
#include "unoedprx.hxx"
#include "AccessibleStaticTextBase.hxx"
#include "AccessibleEditableTextPara.hxx"


using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

/* TODO:
   =====

   - separate adapter functionality from AccessibleStaticText class

   - refactor common loops into templates, using mem_fun

 */

namespace accessibility
{
    //------------------------------------------------------------------------
    //
    // Static Helper
    //
    //------------------------------------------------------------------------
    ESelection MakeSelection( sal_Int32 nStartPara, sal_Int32 nStartIndex,
                              sal_Int32 nEndPara, sal_Int32 nEndIndex )
    {
        DBG_ASSERT(nStartPara >= 0 && nStartPara <= USHRT_MAX &&
                   nStartIndex >= 0 && nStartIndex <= USHRT_MAX &&
                   nEndPara >= 0 && nEndPara <= USHRT_MAX &&
                   nEndIndex >= 0 && nEndIndex <= USHRT_MAX ,
                   "AccessibleStaticTextBase_Impl::MakeSelection: index value overflow");

        return ESelection( static_cast< USHORT >(nStartPara), static_cast< USHORT >(nStartIndex),
                           static_cast< USHORT >(nEndPara), static_cast< USHORT >(nEndIndex) );
    }

    //------------------------------------------------------------------------
    //
    // AccessibleStaticTextBase_Impl declaration
    //
    //------------------------------------------------------------------------

    DBG_NAME( AccessibleStaticTextBase_Impl );

    /** AccessibleStaticTextBase_Impl

        This class implements the AccessibleStaticTextBase
        functionality, mainly by forwarding the calls to an aggregated
        AccessibleEditableTextPara. As this is a therefore non-trivial
        adapter, factoring out the common functionality from
        AccessibleEditableTextPara might be a profitable future task.
     */
    class AccessibleStaticTextBase_Impl
    {

    public:

        // receive pointer to our frontend class and view window
        AccessibleStaticTextBase_Impl();
        ~AccessibleStaticTextBase_Impl();

        SvxEditSourceAdapter& GetEditSource() const SAL_THROW((uno::RuntimeException))
        {
            DBG_CHKTHIS( AccessibleStaticTextBase_Impl, NULL );

            return maEditSource;
        }
        void SetEditSource( ::std::auto_ptr< SvxEditSource > pEditSource ) SAL_THROW((uno::RuntimeException));

        void SetEventSource( const uno::Reference< XAccessible >& rInterface )
        {
            DBG_CHKTHIS( AccessibleStaticTextBase_Impl, NULL );

            mxThis = rInterface;
        }
        uno::Reference< XAccessible > GetEventSource() const
        {
            DBG_CHKTHIS( AccessibleStaticTextBase_Impl, NULL );

            return mxThis;
        }

        void SetOffset( const Point& );
        Point GetOffset() const
        {
            DBG_CHKTHIS( AccessibleStaticTextBase_Impl, NULL );

            ::osl::MutexGuard aGuard( maMutex ); Point aPoint( maOffset );
            return aPoint;
        }

        void UpdateChildren();
        void Dispose();

#ifdef DBG_UTIL
        void CheckInvariants() const;
#endif

        AccessibleEditableTextPara& GetParagraph( sal_Int32 nPara ) const;
        sal_Int32                   GetParagraphCount() const;
        EPosition                   Index2Internal( sal_Int32 nFlatIndex ) const
        {
            DBG_CHKTHIS( AccessibleStaticTextBase_Impl, NULL );

            return ImpCalcInternal( nFlatIndex, false );
        }
        EPosition                   Range2Internal( sal_Int32 nFlatIndex ) const
        {
            DBG_CHKTHIS( AccessibleStaticTextBase_Impl, NULL );

            return ImpCalcInternal( nFlatIndex, true );
        }
        sal_Bool                    SetSelection( sal_Int32 nStartPara, sal_Int32 nStartIndex,
                                                  sal_Int32 nEndPara, sal_Int32 nEndIndex );
        sal_Bool                    CopyText( sal_Int32 nStartPara, sal_Int32 nStartIndex,
                                              sal_Int32 nEndPara, sal_Int32 nEndIndex );

    private:

        EPosition                   ImpCalcInternal( sal_Int32 nFlatIndex, bool bExclusive ) const;

        // our frontend class (the one implementing the actual
        // interface). That's not necessarily the one containing the impl
        // pointer
        uno::Reference< XAccessible > mxThis;

        // implements our functionality, we're just an adapter (guarded by solar mutex)
        mutable AccessibleEditableTextPara* mpTextParagraph;

        uno::Reference< XAccessible > mxParagraph;

        // a wrapper for the text forwarders (guarded by solar mutex)
        mutable SvxEditSourceAdapter maEditSource;

        // guard for maOffset
        mutable ::osl::Mutex maMutex;

        /// our current offset to the containing shape/cell (guarded by maMutex)
        Point maOffset;

    };

    //------------------------------------------------------------------------
    //
    // AccessibleStaticTextBase_Impl implementation
    //
    //------------------------------------------------------------------------

    AccessibleStaticTextBase_Impl::AccessibleStaticTextBase_Impl() :
        mxThis( NULL ),
        mpTextParagraph( new AccessibleEditableTextPara(NULL) ),
        mxParagraph( mpTextParagraph ),
        maEditSource(),
        maMutex(),
        maOffset(0,0)
    {
        DBG_CTOR( AccessibleStaticTextBase_Impl, NULL );

        // TODO: this is still somewhat of a hack, all the more since
        // now the maTextParagraph has an empty parent reference set
    }

    AccessibleStaticTextBase_Impl::~AccessibleStaticTextBase_Impl()
    {
        DBG_DTOR( AccessibleStaticTextBase_Impl, NULL );
    }

    void AccessibleStaticTextBase_Impl::SetEditSource( ::std::auto_ptr< SvxEditSource > pEditSource ) SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleStaticTextBase_Impl, NULL );

        maEditSource.SetEditSource( pEditSource );
        if( mpTextParagraph )
            mpTextParagraph->SetEditSource( &maEditSource );
    }

    void AccessibleStaticTextBase_Impl::SetOffset( const Point& rPoint )
    {
        DBG_CHKTHIS( AccessibleStaticTextBase_Impl, NULL );

        // guard against non-atomic access to maOffset data structure
        {
            ::osl::MutexGuard aGuard( maMutex );
            maOffset = rPoint;
        }

        if( mpTextParagraph )
            mpTextParagraph->SetEEOffset( rPoint );

        // in all cases, check visibility afterwards.
        UpdateChildren();
    }

    void AccessibleStaticTextBase_Impl::UpdateChildren()
    {
        DBG_CHKTHIS( AccessibleStaticTextBase_Impl, NULL );

        // currently no children
    }

    void AccessibleStaticTextBase_Impl::Dispose()
    {
        DBG_CHKTHIS( AccessibleStaticTextBase_Impl, NULL );

        // we're the owner of the paragraph, so destroy it, too
        if( mpTextParagraph )
            mpTextParagraph->Dispose();

        // drop references
        mxParagraph = NULL;
        mxThis = NULL;
        mpTextParagraph = NULL;
    }

#ifdef DBG_UTIL
    void AccessibleStaticTextBase_Impl::CheckInvariants() const
    {
        // TODO
    }
#endif

    AccessibleEditableTextPara& AccessibleStaticTextBase_Impl::GetParagraph( sal_Int32 nPara ) const
    {
        DBG_CHKTHIS( AccessibleStaticTextBase_Impl, NULL );

        if( !mpTextParagraph )
            throw lang::DisposedException (
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("object has been already disposed")), mxThis );

        // TODO: Have a differnt method on AccessibleEditableTextPara
        // that does not care about state changes
        mpTextParagraph->SetParagraphIndex( nPara );

        return *mpTextParagraph;
    }

    sal_Int32 AccessibleStaticTextBase_Impl::GetParagraphCount() const
    {
        DBG_CHKTHIS( AccessibleStaticTextBase_Impl, NULL );

        if( !mpTextParagraph )
            return 0;
        else
            return mpTextParagraph->GetTextForwarder().GetParagraphCount();
    }

    EPosition AccessibleStaticTextBase_Impl::ImpCalcInternal( sal_Int32 nFlatIndex, bool bExclusive ) const
    {
        DBG_CHKTHIS( AccessibleStaticTextBase_Impl, NULL );

        if( nFlatIndex < 0 )
            throw lang::IndexOutOfBoundsException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleStaticTextBase_Impl::Index2Internal: character index out of bounds")),
                                                  mxThis);
        // gratuitously accepting larger indices here, AccessibleEditableTextPara will throw eventually

        sal_Int32 nCurrPara, nCurrIndex, nParas, nCurrCount;
        for( nCurrPara=0, nParas=GetParagraphCount(), nCurrCount=0, nCurrIndex=0; nCurrPara<nParas; ++nCurrPara )
        {
            nCurrCount = GetParagraph( nCurrPara ).getCharacterCount();
            nCurrIndex += nCurrCount;

            if( nCurrIndex > nFlatIndex )
            {
                // check overflow
                DBG_ASSERT(nCurrPara >= 0 && nCurrPara <= USHRT_MAX &&
                           nFlatIndex - nCurrIndex + nCurrCount >= 0 && nFlatIndex - nCurrIndex + nCurrCount <= USHRT_MAX ,
                           "AccessibleStaticTextBase_Impl::Index2Internal: index value overflow");

                return EPosition( static_cast< USHORT >(nCurrPara), static_cast< USHORT >(nFlatIndex - nCurrIndex + nCurrCount) );
            }
        }

        // #102170# Allow one-past the end for ranges
        if( bExclusive && nCurrIndex == nFlatIndex )
        {
            // check overflow
            DBG_ASSERT(nCurrPara >= 0 && nCurrPara <= USHRT_MAX &&
                       nFlatIndex - nCurrIndex + nCurrCount >= 0 && nFlatIndex - nCurrIndex + nCurrCount <= USHRT_MAX ,
                       "AccessibleStaticTextBase_Impl::Index2Internal: index value overflow");

            return EPosition( static_cast< USHORT >(nCurrPara-1), static_cast< USHORT >(nFlatIndex - nCurrIndex + nCurrCount) );
        }

        // not found? Out of bounds
        throw lang::IndexOutOfBoundsException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleStaticTextBase_Impl::Index2Internal: character index out of bounds")),
                                              mxThis);
    }

    sal_Bool AccessibleStaticTextBase_Impl::SetSelection( sal_Int32 nStartPara, sal_Int32 nStartIndex,
                                                          sal_Int32 nEndPara, sal_Int32 nEndIndex )
    {
        DBG_CHKTHIS( AccessibleStaticTextBase_Impl, NULL );

        if( !mpTextParagraph )
            return sal_False;

        try
        {
            SvxEditViewForwarder& rCacheVF = mpTextParagraph->GetEditViewForwarder( sal_True );
            return rCacheVF.SetSelection( MakeSelection(nStartPara, nStartIndex, nEndPara, nEndIndex) );
        }
        catch( const uno::RuntimeException& )
        {
            return sal_False;
        }
    }

    sal_Bool AccessibleStaticTextBase_Impl::CopyText( sal_Int32 nStartPara, sal_Int32 nStartIndex,
                                                      sal_Int32 nEndPara, sal_Int32 nEndIndex )
    {
        DBG_CHKTHIS( AccessibleStaticTextBase_Impl, NULL );

        if( !mpTextParagraph )
            return sal_False;

        try
        {
            SvxEditViewForwarder& rCacheVF = mpTextParagraph->GetEditViewForwarder( sal_True );
            SvxTextForwarder& rCacheTF = mpTextParagraph->GetTextForwarder();   // MUST be after GetEditViewForwarder(), see method docs
            sal_Bool aRetVal;

            // save current selection
            ESelection aOldSelection;

            rCacheVF.GetSelection( aOldSelection );
            rCacheVF.SetSelection( MakeSelection(nStartPara, nStartIndex, nEndPara, nEndIndex) );
            aRetVal = rCacheVF.Copy();
            rCacheVF.SetSelection( aOldSelection ); // restore

            return aRetVal;
        }
        catch( const uno::RuntimeException& )
        {
            return sal_False;
        }
    }

    //------------------------------------------------------------------------
    //
    // AccessibleStaticTextBase implementation
    //
    //------------------------------------------------------------------------

    AccessibleStaticTextBase::AccessibleStaticTextBase( ::std::auto_ptr< SvxEditSource >        pEditSource ) :
        mpImpl( new AccessibleStaticTextBase_Impl() )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        SetEditSource( pEditSource );
    }

    AccessibleStaticTextBase::~AccessibleStaticTextBase()
    {
    }

    const SvxEditSource& AccessibleStaticTextBase::GetEditSource() const SAL_THROW((::com::sun::star::uno::RuntimeException))
    {
#ifdef DBG_UTIL
        mpImpl->CheckInvariants();

        const SvxEditSource& aEditSource = mpImpl->GetEditSource();

        mpImpl->CheckInvariants();

        return aEditSource;
#else
        return mpImpl->GetEditSource();
#endif
    }

    void AccessibleStaticTextBase::SetEditSource( ::std::auto_ptr< SvxEditSource > pEditSource ) SAL_THROW((::com::sun::star::uno::RuntimeException))
    {
#ifdef DBG_UTIL
        // precondition: solar mutex locked
        DBG_TESTSOLARMUTEX();

        mpImpl->CheckInvariants();

        mpImpl->SetEditSource( pEditSource );

        mpImpl->CheckInvariants();
#else
        mpImpl->SetEditSource( pEditSource );
#endif
    }

    void AccessibleStaticTextBase::SetEventSource( const uno::Reference< XAccessible >& rInterface )
    {
#ifdef DBG_UTIL
        mpImpl->CheckInvariants();
#endif

        mpImpl->SetEventSource( rInterface );

#ifdef DBG_UTIL
        mpImpl->CheckInvariants();
#endif
    }

    uno::Reference< XAccessible > AccessibleStaticTextBase::GetEventSource() const
    {
#ifdef DBG_UTIL
        mpImpl->CheckInvariants();

        uno::Reference< XAccessible > xRet( mpImpl->GetEventSource() );

        mpImpl->CheckInvariants();

        return xRet;
#else
        return mpImpl->GetEventSource();
#endif
    }

    void AccessibleStaticTextBase::SetOffset( const Point& rPoint )
    {
#ifdef DBG_UTIL
        // precondition: solar mutex locked
        DBG_TESTSOLARMUTEX();

        mpImpl->CheckInvariants();

        mpImpl->SetOffset( rPoint );

        mpImpl->CheckInvariants();
#else
        mpImpl->SetOffset( rPoint );
#endif
    }

    Point AccessibleStaticTextBase::GetOffset() const
    {
#ifdef DBG_UTIL
        mpImpl->CheckInvariants();

        Point aPoint( mpImpl->GetOffset() );

        mpImpl->CheckInvariants();

        return aPoint;
#else
        return mpImpl->GetOffset();
#endif
    }

    void AccessibleStaticTextBase::UpdateChildren() SAL_THROW((::com::sun::star::uno::RuntimeException))
    {
#ifdef DBG_UTIL
        // precondition: solar mutex locked
        DBG_TESTSOLARMUTEX();

        mpImpl->CheckInvariants();

        mpImpl->UpdateChildren();

        mpImpl->CheckInvariants();
#else
        mpImpl->UpdateChildren();
#endif
    }

    void AccessibleStaticTextBase::Dispose()
    {
#ifdef DBG_UTIL
        mpImpl->CheckInvariants();
#endif

        mpImpl->Dispose();

#ifdef DBG_UTIL
        mpImpl->CheckInvariants();
#endif
    }

    // XAccessibleContext
    sal_Int32 SAL_CALL AccessibleStaticTextBase::getAccessibleChildCount() throw (uno::RuntimeException)
    {
        // no children at all
        return 0;
    }

    uno::Reference< XAccessible > SAL_CALL AccessibleStaticTextBase::getAccessibleChild( sal_Int32 i ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        // no children at all
        return uno::Reference< XAccessible >();
    }

    uno::Reference< XAccessible > SAL_CALL AccessibleStaticTextBase::getAccessibleAt( const awt::Point& _aPoint ) throw (uno::RuntimeException)
    {
        // no children at all
        return uno::Reference< XAccessible >();
    }

    // XAccessibleText
    sal_Int32 SAL_CALL AccessibleStaticTextBase::getCaretPosition() throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        sal_Int32 i, nPos, nParas;
        for( i=0, nPos=-1, nParas=mpImpl->GetParagraphCount(); i<nParas; ++i )
        {
            if( (nPos=mpImpl->GetParagraph(i).getCaretPosition()) != -1 )
                return nPos;
        }

        return nPos;
    }

    sal_Bool SAL_CALL AccessibleStaticTextBase::setCaretPosition( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        return setSelection(nIndex, nIndex);
    }

    sal_Unicode SAL_CALL AccessibleStaticTextBase::getCharacter( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        EPosition aPos( mpImpl->Index2Internal(nIndex) );

        return mpImpl->GetParagraph( aPos.nPara ).getCharacter( aPos.nIndex );
    }

    uno::Sequence< beans::PropertyValue > SAL_CALL AccessibleStaticTextBase::getCharacterAttributes( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        EPosition aPos( mpImpl->Index2Internal(nIndex) );

        return mpImpl->GetParagraph( aPos.nPara ).getCharacterAttributes( aPos.nIndex );
    }

    awt::Rectangle SAL_CALL AccessibleStaticTextBase::getCharacterBounds( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        EPosition aPos( mpImpl->Index2Internal(nIndex) );

        return mpImpl->GetParagraph( aPos.nPara ).getCharacterBounds( aPos.nIndex );
    }

    sal_Int32 SAL_CALL AccessibleStaticTextBase::getCharacterCount() throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        sal_Int32 i, nCount, nParas;
        for( i=0, nCount=0, nParas=mpImpl->GetParagraphCount(); i<nParas; ++i )
            nCount += mpImpl->GetParagraph(i).getCharacterCount();

        return nCount;
    }

    sal_Int32 SAL_CALL AccessibleStaticTextBase::getIndexAtPoint( const awt::Point& rPoint ) throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        sal_Int32 i, nIndex, nParas;
        for( i=0, nIndex=-1, nParas=mpImpl->GetParagraphCount(); i<nParas; ++i )
        {
            // TODO: maybe exploit the fact that paragraphs are
            // ordered vertically for early exit
            if( (nIndex=mpImpl->GetParagraph(i).getIndexAtPoint( rPoint )) != -1 )
                return nIndex;
        }

        return nIndex;
    }

    ::rtl::OUString SAL_CALL AccessibleStaticTextBase::getSelectedText() throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        sal_Int32 nStart( getSelectionStart() );
        sal_Int32 nEnd( getSelectionEnd() );

        // #104481# Return the empty string for 'no selection'
        if( nStart < 0 || nEnd < 0 )
            return ::rtl::OUString();
        else
            return getTextRange( nStart, nEnd );
    }

    sal_Int32 SAL_CALL AccessibleStaticTextBase::getSelectionStart() throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        sal_Int32 i, nPos, nParas;
        for( i=0, nPos=-1, nParas=mpImpl->GetParagraphCount(); i<nParas; ++i )
        {
            if( (nPos=mpImpl->GetParagraph(i).getSelectionStart()) != -1 )
                return nPos;
        }

        return nPos;
    }

    sal_Int32 SAL_CALL AccessibleStaticTextBase::getSelectionEnd() throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        sal_Int32 i, nPos, nParas;
        for( i=0, nPos=-1, nParas=mpImpl->GetParagraphCount(); i<nParas; ++i )
        {
            if( (nPos=mpImpl->GetParagraph(i).getSelectionEnd()) != -1 )
                return nPos;
        }

        return nPos;
    }

    sal_Bool SAL_CALL AccessibleStaticTextBase::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        EPosition aStartIndex( mpImpl->Range2Internal(nStartIndex) );
        EPosition aEndIndex( mpImpl->Range2Internal(nEndIndex) );

        return mpImpl->SetSelection( aStartIndex.nPara, aStartIndex.nIndex,
                                     aEndIndex.nPara, aEndIndex.nIndex );
    }

    ::rtl::OUString SAL_CALL AccessibleStaticTextBase::getText() throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        sal_Int32 i, nParas;
        ::rtl::OUString aRes;
        for( i=0, nParas=mpImpl->GetParagraphCount(); i<nParas; ++i )
            aRes += mpImpl->GetParagraph(i).getText();

        return aRes;
    }

    ::rtl::OUString SAL_CALL AccessibleStaticTextBase::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        if( nStartIndex > nEndIndex )
            ::std::swap(nStartIndex, nEndIndex);

        EPosition aStartIndex( mpImpl->Range2Internal(nStartIndex) );
        EPosition aEndIndex( mpImpl->Range2Internal(nEndIndex) );

        // #102170# Special case: start and end paragraph are identical
        if( aStartIndex.nPara == aEndIndex.nPara )
        {
            return mpImpl->GetParagraph( aStartIndex.nPara ).getTextRange( aStartIndex.nIndex, aEndIndex.nIndex );
        }
        else
        {
            sal_Int32 i( aStartIndex.nPara );
            ::rtl::OUString aRes( mpImpl->GetParagraph(i).getTextRange( aStartIndex.nIndex,
                                                                        mpImpl->GetParagraph(i).getCharacterCount()-1) );
            ++i;

            // paragraphs inbetween are fully included
            for( ; i<aEndIndex.nPara; ++i )
                aRes += mpImpl->GetParagraph(i).getText();

            if( i<=aEndIndex.nPara )
                aRes += mpImpl->GetParagraph(i).getTextRange( 0, aEndIndex.nIndex );

            return aRes;
        }
    }

    ::rtl::OUString SAL_CALL AccessibleStaticTextBase::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        EPosition aPos( mpImpl->Range2Internal(nIndex) );

        if( AccessibleTextType::PARAGRAPH == aTextType )
        {
            // #106393# Special casing one behind last paragraph
            if( aPos.nIndex == mpImpl->GetParagraph( aPos.nPara ).getCharacterCount() )
            {
                return ::rtl::OUString();
            }
            else
            {
                return mpImpl->GetParagraph( aPos.nPara ).getText();
            }
        }
        else
        {
            return mpImpl->GetParagraph( aPos.nPara ).getTextAtIndex( aPos.nIndex, aTextType );
        }
    }

    ::rtl::OUString SAL_CALL AccessibleStaticTextBase::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        EPosition aPos( mpImpl->Range2Internal(nIndex) );

        if( AccessibleTextType::PARAGRAPH == aTextType )
        {
            if( aPos.nIndex == mpImpl->GetParagraph( aPos.nPara ).getCharacterCount() )
            {
                // #103589# Special casing one behind the last paragraph
                return mpImpl->GetParagraph( aPos.nPara ).getText();
            }
            else if( aPos.nPara > 0 )
            {
                return mpImpl->GetParagraph( aPos.nPara - 1 ).getText();
            }
            else
            {
                return ::rtl::OUString();
            }
        }
        else
        {
            return mpImpl->GetParagraph( aPos.nPara ).getTextBeforeIndex( aPos.nIndex, aTextType );
        }
    }

    ::rtl::OUString SAL_CALL AccessibleStaticTextBase::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        EPosition aPos( mpImpl->Range2Internal(nIndex) );

        if( AccessibleTextType::PARAGRAPH == aTextType )
        {
            if( aPos.nPara + 1 < mpImpl->GetParagraphCount() )
            {
                return mpImpl->GetParagraph( aPos.nPara + 1 ).getText();
            }
            else
            {
                return ::rtl::OUString();
            }
        }
        else
        {
            return mpImpl->GetParagraph( aPos.nPara ).getTextBehindIndex( aPos.nIndex, aTextType );
        }
    }

    sal_Bool SAL_CALL AccessibleStaticTextBase::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        if( nStartIndex > nEndIndex )
            ::std::swap(nStartIndex, nEndIndex);

        EPosition aStartIndex( mpImpl->Range2Internal(nStartIndex) );
        EPosition aEndIndex( mpImpl->Range2Internal(nEndIndex) );

        return mpImpl->CopyText( aStartIndex.nPara, aStartIndex.nIndex,
                                 aEndIndex.nPara, aEndIndex.nIndex );
    }

}  // end of namespace accessibility

//------------------------------------------------------------------------
