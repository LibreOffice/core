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

#include <limits.h>
#include <vector>
#include <algorithm>
#include <boost/bind.hpp>
#include <vos/mutex.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/sequenceasvector.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>

//------------------------------------------------------------------------
//
// Project-local header
//
//------------------------------------------------------------------------

#include <editeng/editdata.hxx>
#include <editeng/unopracc.hxx>
#include "editeng/unoedprx.hxx"
#include <editeng/AccessibleStaticTextBase.hxx>
#include "editeng/AccessibleEditableTextPara.hxx"


using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

/* TODO:
   =====

   - separate adapter functionality from AccessibleStaticText class

   - refactor common loops into templates, using mem_fun

 */

namespace accessibility
{
    typedef ::comphelper::SequenceAsVector< beans::PropertyValue > PropertyValueVector;

    class PropertyValueEqualFunctor : public ::std::binary_function< beans::PropertyValue, beans::PropertyValue, bool >
    {
    public:
        PropertyValueEqualFunctor()
        {}
        bool operator() ( const beans::PropertyValue& lhs, const beans::PropertyValue& rhs ) const
        {
            return ( lhs.Name == rhs.Name && lhs.Value == rhs.Value );
        }
    };

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

        return ESelection( static_cast< sal_uInt16 >(nStartPara), static_cast< sal_uInt16 >(nStartIndex),
                           static_cast< sal_uInt16 >(nEndPara), static_cast< sal_uInt16 >(nEndIndex) );
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
        sal_Int32                   GetParagraphIndex() const;
        sal_Int32                   GetLineCount( sal_Int32 nParagraph ) const;

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

        sal_Int32                   Internal2Index( EPosition nEEIndex ) const;

        void                        CorrectTextSegment( TextSegment&    aTextSegment,
                                                        int             nPara   ) const;

        sal_Bool                    SetSelection( sal_Int32 nStartPara, sal_Int32 nStartIndex,
                                                  sal_Int32 nEndPara, sal_Int32 nEndIndex );
        sal_Bool                    CopyText( sal_Int32 nStartPara, sal_Int32 nStartIndex,
                                              sal_Int32 nEndPara, sal_Int32 nEndIndex );

        Rectangle                   GetParagraphBoundingBox() const;

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

    sal_Int32 AccessibleStaticTextBase_Impl::GetParagraphIndex() const
    {
        DBG_CHKTHIS( AccessibleStaticTextBase_Impl, NULL );

        sal_Int32 nIndex = -1;
        if( mpTextParagraph )
            nIndex = mpTextParagraph->GetParagraphIndex();
        return nIndex;
    }

    sal_Int32 AccessibleStaticTextBase_Impl::GetLineCount( sal_Int32 nParagraph ) const
    {
        DBG_CHKTHIS( AccessibleStaticTextBase_Impl, NULL );

        sal_Int32 nIndex = 0;
        if( mpTextParagraph )
            nIndex = mpTextParagraph->GetTextForwarder().GetLineCount( static_cast< sal_uInt16 >(nParagraph) );
        return nIndex;
    }

    sal_Int32 AccessibleStaticTextBase_Impl::Internal2Index( EPosition nEEIndex ) const
    {
        sal_Int32 aRes(0);
        int i;
        for(i=0; i<nEEIndex.nPara; ++i)
            aRes += GetParagraph(i).getCharacterCount();

        return aRes + nEEIndex.nIndex;
    }

    void AccessibleStaticTextBase_Impl::CorrectTextSegment( TextSegment&    aTextSegment,
                                                            int             nPara   ) const
    {
        // Keep 'invalid' values at the TextSegment
        if( aTextSegment.SegmentStart != -1 &&
            aTextSegment.SegmentStart != -1 )
        {
            // #112814# Correct TextSegment by paragraph offset
            sal_Int32 nOffset(0);
            int i;
            for(i=0; i<nPara; ++i)
                nOffset += GetParagraph(i).getCharacterCount();

            aTextSegment.SegmentStart += nOffset;
            aTextSegment.SegmentEnd += nOffset;
        }
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

                return EPosition( static_cast< sal_uInt16 >(nCurrPara), static_cast< sal_uInt16 >(nFlatIndex - nCurrIndex + nCurrCount) );
            }
        }

        // #102170# Allow one-past the end for ranges
        if( bExclusive && nCurrIndex == nFlatIndex )
        {
            // check overflow
            DBG_ASSERT(nCurrPara >= 0 && nCurrPara <= USHRT_MAX &&
                       nFlatIndex - nCurrIndex + nCurrCount >= 0 && nFlatIndex - nCurrIndex + nCurrCount <= USHRT_MAX ,
                       "AccessibleStaticTextBase_Impl::Index2Internal: index value overflow");

            return EPosition( static_cast< sal_uInt16 >(nCurrPara-1), static_cast< sal_uInt16 >(nFlatIndex - nCurrIndex + nCurrCount) );
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
            mpTextParagraph->GetTextForwarder();    // MUST be after GetEditViewForwarder(), see method docs
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

    Rectangle AccessibleStaticTextBase_Impl::GetParagraphBoundingBox() const
    {
        Rectangle aRect;
        if( mpTextParagraph )
        {
            awt::Rectangle aAwtRect = mpTextParagraph->getBounds();
            aRect = Rectangle( Point( aAwtRect.X, aAwtRect.Y ), Size( aAwtRect.Width, aAwtRect.Height ) );
        }
        else
        {
            aRect.SetEmpty();
        }
        return aRect;
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

    uno::Reference< XAccessible > SAL_CALL AccessibleStaticTextBase::getAccessibleChild( sal_Int32 /*i*/ ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        // no children at all
        return uno::Reference< XAccessible >();
    }

    uno::Reference< XAccessible > SAL_CALL AccessibleStaticTextBase::getAccessibleAtPoint( const awt::Point& /*_aPoint*/ ) throw (uno::RuntimeException)
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

    uno::Sequence< beans::PropertyValue > SAL_CALL AccessibleStaticTextBase::getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRequestedAttributes ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        EPosition aPos( mpImpl->Index2Internal(nIndex) );

        return mpImpl->GetParagraph( aPos.nPara ).getCharacterAttributes( aPos.nIndex, aRequestedAttributes );
    }

    awt::Rectangle SAL_CALL AccessibleStaticTextBase::getCharacterBounds( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        // #108900# Allow ranges for nIndex, as one-past-the-end
        // values are now legal, too.
        EPosition aPos( mpImpl->Range2Internal(nIndex) );

        // #i70916# Text in spread sheet cells return the wrong extents
        AccessibleEditableTextPara& rPara = mpImpl->GetParagraph( aPos.nPara );
        awt::Rectangle aParaBounds( rPara.getBounds() );
        awt::Rectangle aBounds( rPara.getCharacterBounds( aPos.nIndex ) );
        aBounds.X += aParaBounds.X;
        aBounds.Y += aParaBounds.Y;

        return aBounds;
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

        const sal_Int32 nParas( mpImpl->GetParagraphCount() );
        sal_Int32 nIndex;
        int i;
        for( i=0; i<nParas; ++i )
        {
            // TODO: maybe exploit the fact that paragraphs are
            // ordered vertically for early exit

            // #i70916# Text in spread sheet cells return the wrong extents
            AccessibleEditableTextPara& rPara = mpImpl->GetParagraph( i );
            awt::Rectangle aParaBounds( rPara.getBounds() );
            awt::Point aPoint( rPoint );
            aPoint.X -= aParaBounds.X;
            aPoint.Y -= aParaBounds.Y;

            // #112814# Use correct index offset
            if ( ( nIndex = rPara.getIndexAtPoint( aPoint ) ) != -1 )
                return mpImpl->Internal2Index( EPosition(sal::static_int_cast<sal_uInt16>(i),
                                                         sal::static_int_cast<sal_uInt16>(nIndex)) );
        }

        return -1;
    }

    ::rtl::OUString SAL_CALL AccessibleStaticTextBase::getSelectedText() throw (uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        sal_Int32 nStart( getSelectionStart() );
        sal_Int32 nEnd( getSelectionEnd() );

        // #104481# Return the empty string for 'no selection'
        if( nStart < 0 || nEnd < 0 )
            return ::rtl::OUString();

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

    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleStaticTextBase::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        EPosition aPos( mpImpl->Range2Internal(nIndex) );

        ::com::sun::star::accessibility::TextSegment aResult;

        if( AccessibleTextType::PARAGRAPH == aTextType )
        {
            // #106393# Special casing one behind last paragraph is
            // not necessary, since then, we return the content and
            // boundary of that last paragraph. Range2Internal is
            // tolerant against that, and returns the last paragraph
            // in aPos.nPara.

            // retrieve full text of the paragraph
            aResult.SegmentText = mpImpl->GetParagraph( aPos.nPara ).getText();

            // #112814# Adapt the start index with the paragraph offset
            aResult.SegmentStart = mpImpl->Internal2Index( EPosition( aPos.nPara, 0 ) );
            aResult.SegmentEnd = aResult.SegmentStart + aResult.SegmentText.getLength();
        }
        else
        {
            // No special handling required, forward to wrapped class
            aResult = mpImpl->GetParagraph( aPos.nPara ).getTextAtIndex( aPos.nIndex, aTextType );

            // #112814# Adapt the start index with the paragraph offset
            mpImpl->CorrectTextSegment( aResult, aPos.nPara );
        }

        return aResult;
    }

    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleStaticTextBase::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        EPosition aPos( mpImpl->Range2Internal(nIndex) );

        ::com::sun::star::accessibility::TextSegment aResult;

        if( AccessibleTextType::PARAGRAPH == aTextType )
        {
            if( aPos.nIndex == mpImpl->GetParagraph( aPos.nPara ).getCharacterCount() )
            {
                // #103589# Special casing one behind the last paragraph
                aResult.SegmentText = mpImpl->GetParagraph( aPos.nPara ).getText();

                // #112814# Adapt the start index with the paragraph offset
                aResult.SegmentStart = mpImpl->Internal2Index( EPosition( aPos.nPara, 0 ) );
            }
            else if( aPos.nPara > 0 )
            {
                aResult.SegmentText = mpImpl->GetParagraph( aPos.nPara - 1 ).getText();

                // #112814# Adapt the start index with the paragraph offset
                aResult.SegmentStart = mpImpl->Internal2Index( EPosition( aPos.nPara - 1, 0 ) );
            }

            aResult.SegmentEnd = aResult.SegmentStart + aResult.SegmentText.getLength();
        }
        else
        {
            // No special handling required, forward to wrapped class
            aResult = mpImpl->GetParagraph( aPos.nPara ).getTextBeforeIndex( aPos.nIndex, aTextType );

            // #112814# Adapt the start index with the paragraph offset
            mpImpl->CorrectTextSegment( aResult, aPos.nPara );
        }

        return aResult;
    }

    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleStaticTextBase::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        EPosition aPos( mpImpl->Range2Internal(nIndex) );

        ::com::sun::star::accessibility::TextSegment aResult;

        if( AccessibleTextType::PARAGRAPH == aTextType )
        {
            // Special casing one behind the last paragraph is not
            // necessary, this case is invalid here for
            // getTextBehindIndex
            if( aPos.nPara + 1 < mpImpl->GetParagraphCount() )
            {
                aResult.SegmentText = mpImpl->GetParagraph( aPos.nPara + 1 ).getText();

                // #112814# Adapt the start index with the paragraph offset
                aResult.SegmentStart = mpImpl->Internal2Index( EPosition( aPos.nPara + 1, 0 ) );
                aResult.SegmentEnd = aResult.SegmentStart + aResult.SegmentText.getLength();
            }
        }
        else
        {
            // No special handling required, forward to wrapped class
            aResult = mpImpl->GetParagraph( aPos.nPara ).getTextBehindIndex( aPos.nIndex, aTextType );

            // #112814# Adapt the start index with the paragraph offset
            mpImpl->CorrectTextSegment( aResult, aPos.nPara );
       }

        return aResult;
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

    // XAccessibleTextAttributes
    uno::Sequence< beans::PropertyValue > AccessibleStaticTextBase::getDefaultAttributes( const uno::Sequence< ::rtl::OUString >& RequestedAttributes ) throw (uno::RuntimeException)
    {
        // get the intersection of the default attributes of all paragraphs

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        PropertyValueVector aDefAttrVec( mpImpl->GetParagraph( 0 ).getDefaultAttributes( RequestedAttributes ) );

        const sal_Int32 nParaCount = mpImpl->GetParagraphCount();
        for ( sal_Int32 nPara = 1; nPara < nParaCount; ++nPara )
        {
            uno::Sequence< beans::PropertyValue > aSeq = mpImpl->GetParagraph( nPara ).getDefaultAttributes( RequestedAttributes );
            PropertyValueVector aIntersectionVec;

            PropertyValueVector::const_iterator aEnd = aDefAttrVec.end();
            for ( PropertyValueVector::const_iterator aItr = aDefAttrVec.begin(); aItr != aEnd; ++aItr )
            {
                const beans::PropertyValue* pItr = aSeq.getConstArray();
                const beans::PropertyValue* pEnd  = pItr + aSeq.getLength();
                const beans::PropertyValue* pFind = ::std::find_if( pItr, pEnd, ::std::bind2nd( PropertyValueEqualFunctor(), boost::cref( *aItr ) ) );
                if ( pFind != pEnd )
                {
                    aIntersectionVec.push_back( *pFind );
                }
            }

            aDefAttrVec.swap( aIntersectionVec );

            if ( aDefAttrVec.empty() )
            {
                break;
            }
        }

        return aDefAttrVec.getAsConstList();
    }

    uno::Sequence< beans::PropertyValue > SAL_CALL AccessibleStaticTextBase::getRunAttributes( sal_Int32 nIndex, const uno::Sequence< ::rtl::OUString >& RequestedAttributes ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        // get those default attributes of the paragraph, which are not part
        // of the intersection of all paragraphs and add them to the run attributes

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        EPosition aPos( mpImpl->Index2Internal( nIndex ) );
        AccessibleEditableTextPara& rPara = mpImpl->GetParagraph( aPos.nPara );
        uno::Sequence< beans::PropertyValue > aDefAttrSeq = rPara.getDefaultAttributes( RequestedAttributes );
        uno::Sequence< beans::PropertyValue > aRunAttrSeq = rPara.getRunAttributes( aPos.nIndex, RequestedAttributes );
        uno::Sequence< beans::PropertyValue > aIntersectionSeq = getDefaultAttributes( RequestedAttributes );
        PropertyValueVector aDiffVec;

        const beans::PropertyValue* pDefAttr = aDefAttrSeq.getConstArray();
        const sal_Int32 nLength = aDefAttrSeq.getLength();
        for ( sal_Int32 i = 0; i < nLength; ++i )
        {
            const beans::PropertyValue* pItr = aIntersectionSeq.getConstArray();
            const beans::PropertyValue* pEnd  = pItr + aIntersectionSeq.getLength();
            const beans::PropertyValue* pFind = ::std::find_if( pItr, pEnd, ::std::bind2nd( PropertyValueEqualFunctor(), boost::cref( pDefAttr[i] ) ) );
            if ( pFind == pEnd && pDefAttr[i].Handle != 0)
            {
                aDiffVec.push_back( pDefAttr[i] );
            }
        }

        return ::comphelper::concatSequences( aRunAttrSeq, aDiffVec.getAsConstList() );
    }

    Rectangle AccessibleStaticTextBase::GetParagraphBoundingBox() const
    {
        return mpImpl->GetParagraphBoundingBox();
    }

    sal_Int32 AccessibleStaticTextBase::GetParagraphIndex() const
    {
        return mpImpl->GetParagraphIndex();
    }

    sal_Int32 AccessibleStaticTextBase::GetParagraphCount() const
    {
        return mpImpl->GetParagraphCount();
    }

    sal_Int32 AccessibleStaticTextBase::GetLineCount( sal_Int32 nParagraph ) const
    {
        return mpImpl->GetLineCount( nParagraph );
    }

}  // end of namespace accessibility

//------------------------------------------------------------------------
