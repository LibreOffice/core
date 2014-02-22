/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */



//

//


#include <limits.h>
#include <vector>
#include <algorithm>
#include <boost/ref.hpp>
#include <osl/mutex.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/sequenceasvector.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>


//

//


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
    sal_Unicode cNewLine(0x0a);
    
    //
    
    //
    
    ESelection MakeSelection( sal_Int32 nStartPara, sal_Int32 nStartIndex,
                              sal_Int32 nEndPara, sal_Int32 nEndIndex )
    {
        DBG_ASSERT(nStartPara >= 0 && nStartPara <= SAL_MAX_INT32 &&
                   nStartIndex >= 0 && nStartIndex <= USHRT_MAX &&
                   nEndPara >= 0 && nEndPara <= SAL_MAX_INT32 &&
                   nEndIndex >= 0 && nEndIndex <= USHRT_MAX ,
                   "AccessibleStaticTextBase_Impl::MakeSelection: index value overflow");

        return ESelection( nStartPara, static_cast< sal_uInt16 >(nStartIndex),
                           nEndPara, static_cast< sal_uInt16 >(nEndIndex) );
    }

    
    //
    
    //
    


    /** AccessibleStaticTextBase_Impl

        This class implements the AccessibleStaticTextBase
        functionality, mainly by forwarding the calls to an aggregated
        AccessibleEditableTextPara. As this is a therefore non-trivial
        adapter, factoring out the common functionality from
        AccessibleEditableTextPara might be a profitable future task.
     */
    class AccessibleStaticTextBase_Impl
    {
        friend class AccessibleStaticTextBase;
    public:

        
        AccessibleStaticTextBase_Impl();
        ~AccessibleStaticTextBase_Impl();

        SvxEditSourceAdapter& GetEditSource() const SAL_THROW((uno::RuntimeException))
        {

            return maEditSource;
        }
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        void SetEditSource( ::std::auto_ptr< SvxEditSource > pEditSource ) SAL_THROW((uno::RuntimeException));
        SAL_WNODEPRECATED_DECLARATIONS_POP

        void SetEventSource( const uno::Reference< XAccessible >& rInterface )
        {

            mxThis = rInterface;
        }
        uno::Reference< XAccessible > GetEventSource() const
        {

            return mxThis;
        }

        void SetOffset( const Point& );
        Point GetOffset() const
        {

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

            return ImpCalcInternal( nFlatIndex, false );
        }

        EPosition                   Range2Internal( sal_Int32 nFlatIndex ) const
        {

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
        sal_Bool                    RemoveLineBreakCount( sal_Int32& rIndex );

    private:

        EPosition                   ImpCalcInternal( sal_Int32 nFlatIndex, bool bExclusive ) const;

        
        
        
        uno::Reference< XAccessible > mxThis;

        
        mutable AccessibleEditableTextPara* mpTextParagraph;

        uno::Reference< XAccessible > mxParagraph;

        
        mutable SvxEditSourceAdapter maEditSource;

        
        mutable ::osl::Mutex maMutex;

        /
        Point maOffset;

    };

    
    //
    
    //
    

    AccessibleStaticTextBase_Impl::AccessibleStaticTextBase_Impl() :
        mxThis( NULL ),
        mpTextParagraph( new AccessibleEditableTextPara(NULL) ),
        mxParagraph( mpTextParagraph ),
        maEditSource(),
        maMutex(),
        maOffset(0,0)
    {

        
        
    }

    AccessibleStaticTextBase_Impl::~AccessibleStaticTextBase_Impl()
    {
    }

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    void AccessibleStaticTextBase_Impl::SetEditSource( ::std::auto_ptr< SvxEditSource > pEditSource ) SAL_THROW((uno::RuntimeException))
    {

        maEditSource.SetEditSource( pEditSource );
        if( mpTextParagraph )
            mpTextParagraph->SetEditSource( &maEditSource );
    }
    SAL_WNODEPRECATED_DECLARATIONS_POP

    void AccessibleStaticTextBase_Impl::SetOffset( const Point& rPoint )
    {

        
        {
            ::osl::MutexGuard aGuard( maMutex );
            maOffset = rPoint;
        }

        if( mpTextParagraph )
            mpTextParagraph->SetEEOffset( rPoint );

        
        UpdateChildren();
    }

    void AccessibleStaticTextBase_Impl::UpdateChildren()
    {

        
    }

    void AccessibleStaticTextBase_Impl::Dispose()
    {

        
        if( mpTextParagraph )
            mpTextParagraph->Dispose();

        
        mxParagraph = NULL;
        mxThis = NULL;
        mpTextParagraph = NULL;
    }

#ifdef DBG_UTIL
    void AccessibleStaticTextBase_Impl::CheckInvariants() const
    {
        
    }
#endif

    AccessibleEditableTextPara& AccessibleStaticTextBase_Impl::GetParagraph( sal_Int32 nPara ) const
    {

        if( !mpTextParagraph )
            throw lang::DisposedException ("object has been already disposed", mxThis );

        
        
        mpTextParagraph->SetParagraphIndex( nPara );

        return *mpTextParagraph;
    }

    sal_Int32 AccessibleStaticTextBase_Impl::GetParagraphCount() const
    {

        if( !mpTextParagraph )
            return 0;
        else
            return mpTextParagraph->GetTextForwarder().GetParagraphCount();
    }

    sal_Int32 AccessibleStaticTextBase_Impl::Internal2Index( EPosition nEEIndex ) const
    {
        
        sal_Int32 aRes(0);
        for(sal_Int32 i=0; i<nEEIndex.nPara; ++i)
        {
            sal_Int32 nCount = GetParagraph(i).getCharacterCount();
            if (SAL_MAX_INT32 - aRes > nCount)
                return SAL_MAX_INT32;
            aRes += nCount;
        }

        if (SAL_MAX_INT32 - aRes > nEEIndex.nIndex)
            return SAL_MAX_INT32;
        return aRes + nEEIndex.nIndex;
    }

    void AccessibleStaticTextBase_Impl::CorrectTextSegment( TextSegment&    aTextSegment,
                                                            int             nPara   ) const
    {
        
        if( aTextSegment.SegmentStart != -1 &&
            aTextSegment.SegmentEnd != -1 )
        {
            
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

        if( nFlatIndex < 0 )
            throw lang::IndexOutOfBoundsException("AccessibleStaticTextBase_Impl::Index2Internal: character index out of bounds",
                                                  mxThis);
        

        sal_Int32 nCurrPara, nCurrIndex, nParas, nCurrCount;
        for( nCurrPara=0, nParas=GetParagraphCount(), nCurrCount=0, nCurrIndex=0; nCurrPara<nParas; ++nCurrPara )
        {
            nCurrCount = GetParagraph( nCurrPara ).getCharacterCount();
            nCurrIndex += nCurrCount;
            if( nCurrIndex >= nFlatIndex )
            {
                
                DBG_ASSERT(nCurrPara >= 0 && nCurrPara <= SAL_MAX_INT32 &&
                           nFlatIndex - nCurrIndex + nCurrCount >= 0 && nFlatIndex - nCurrIndex + nCurrCount <= USHRT_MAX ,
                           "AccessibleStaticTextBase_Impl::Index2Internal: index value overflow");

                return EPosition( nCurrPara, static_cast< sal_uInt16 >(nFlatIndex - nCurrIndex + nCurrCount) );
            }
        }

        
        if( bExclusive && nCurrIndex == nFlatIndex )
        {
            
            DBG_ASSERT(nCurrPara > 0 && nCurrPara <= SAL_MAX_INT32 &&
                       nFlatIndex - nCurrIndex + nCurrCount >= 0 && nFlatIndex - nCurrIndex + nCurrCount <= USHRT_MAX ,
                       "AccessibleStaticTextBase_Impl::Index2Internal: index value overflow");

            return EPosition( nCurrPara-1, static_cast< sal_uInt16 >(nFlatIndex - nCurrIndex + nCurrCount) );
        }

        
        throw lang::IndexOutOfBoundsException("AccessibleStaticTextBase_Impl::Index2Internal: character index out of bounds",
                                              mxThis);
    }

    sal_Bool AccessibleStaticTextBase_Impl::SetSelection( sal_Int32 nStartPara, sal_Int32 nStartIndex,
                                                          sal_Int32 nEndPara, sal_Int32 nEndIndex )
    {

        if( !mpTextParagraph )
            return sal_False;

        try
        {
            SvxEditViewForwarder& rCacheVF = mpTextParagraph->GetEditViewForwarder( true );
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

        if( !mpTextParagraph )
            return sal_False;

        try
        {
            SvxEditViewForwarder& rCacheVF = mpTextParagraph->GetEditViewForwarder( true );
            mpTextParagraph->GetTextForwarder();    
            sal_Bool aRetVal;

            
            ESelection aOldSelection;

            rCacheVF.GetSelection( aOldSelection );
            rCacheVF.SetSelection( MakeSelection(nStartPara, nStartIndex, nEndPara, nEndIndex) );
            aRetVal = rCacheVF.Copy();
            rCacheVF.SetSelection( aOldSelection ); 

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
    
    
    
    sal_Bool AccessibleStaticTextBase_Impl::RemoveLineBreakCount( sal_Int32& rIndex )
    {
        
        sal_Int32 i, nCount, nParas;
        for( i=0, nCount=0, nParas=GetParagraphCount(); i<nParas; ++i )
            nCount += GetParagraph(i).getCharacterCount();
        nCount = nCount + (nParas-1);
        if( nCount == 0 &&  rIndex == 0) return sal_False;


        sal_Int32 nCurrPara, nCurrCount;
        sal_Int32 nLineBreakPos = 0, nLineBreakCount = 0;
        sal_Int32 nParaCount = GetParagraphCount();
        for ( nCurrCount = 0, nCurrPara = 0; nCurrPara < nParaCount; nCurrPara++ )
        {
            nCurrCount += GetParagraph( nCurrPara ).getCharacterCount();
            nLineBreakPos = nCurrCount++;
            if ( rIndex == nLineBreakPos )
            {
                rIndex -= (++nLineBreakCount);
                if ( rIndex < 0)
                {
                    rIndex = 0;
                }
                
                
                if ( (nCurrPara+1) == nParaCount )
                {
                    rIndex++;
                    return sal_False;
                }
                else
                {
                    return sal_True;
                }
            }
            else if ( rIndex < nLineBreakPos )
            {
                rIndex -= nLineBreakCount;
                return sal_False;
            }
            else
            {
                nLineBreakCount++;
            }
        }
        return sal_False;
    }
    
    //
    
    //
    

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    AccessibleStaticTextBase::AccessibleStaticTextBase( ::std::auto_ptr< SvxEditSource >        pEditSource ) :
        mpImpl( new AccessibleStaticTextBase_Impl() )
    {
        SolarMutexGuard aGuard;

        SetEditSource( pEditSource );
    }
    SAL_WNODEPRECATED_DECLARATIONS_POP

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

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    void AccessibleStaticTextBase::SetEditSource( ::std::auto_ptr< SvxEditSource > pEditSource ) SAL_THROW((::com::sun::star::uno::RuntimeException))
    {
#ifdef DBG_UTIL
        
        DBG_TESTSOLARMUTEX();

        mpImpl->CheckInvariants();

        mpImpl->SetEditSource( pEditSource );

        mpImpl->CheckInvariants();
#else
        mpImpl->SetEditSource( pEditSource );
#endif
    }
    SAL_WNODEPRECATED_DECLARATIONS_POP

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

    
    sal_Int32 SAL_CALL AccessibleStaticTextBase::getAccessibleChildCount() throw (uno::RuntimeException)
    {
        
        return 0;
    }

    uno::Reference< XAccessible > SAL_CALL AccessibleStaticTextBase::getAccessibleChild( sal_Int32 /*i*/ ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        
        return uno::Reference< XAccessible >();
    }

    uno::Reference< XAccessible > SAL_CALL AccessibleStaticTextBase::getAccessibleAtPoint( const awt::Point& /*_aPoint*/ ) throw (uno::RuntimeException)
    {
        
        return uno::Reference< XAccessible >();
    }

    
    sal_Int32 SAL_CALL AccessibleStaticTextBase::getCaretPosition() throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

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
        SolarMutexGuard aGuard;

        EPosition aPos( mpImpl->Index2Internal(nIndex) );

        return mpImpl->GetParagraph( aPos.nPara ).getCharacter( aPos.nIndex );
    }

    uno::Sequence< beans::PropertyValue > SAL_CALL AccessibleStaticTextBase::getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< OUString >& aRequestedAttributes ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        
        mpImpl->RemoveLineBreakCount( nIndex );

        EPosition aPos( mpImpl->Index2Internal(nIndex) );

        return mpImpl->GetParagraph( aPos.nPara ).getCharacterAttributes( aPos.nIndex, aRequestedAttributes );
    }

    awt::Rectangle SAL_CALL AccessibleStaticTextBase::getCharacterBounds( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        
        
        EPosition aPos( mpImpl->Range2Internal(nIndex) );

        
        AccessibleEditableTextPara& rPara = mpImpl->GetParagraph( aPos.nPara );
        awt::Rectangle aParaBounds( rPara.getBounds() );
        awt::Rectangle aBounds( rPara.getCharacterBounds( aPos.nIndex ) );
        aBounds.X += aParaBounds.X;
        aBounds.Y += aParaBounds.Y;

        return aBounds;
    }

    sal_Int32 SAL_CALL AccessibleStaticTextBase::getCharacterCount() throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        sal_Int32 i, nCount, nParas;
        for( i=0, nCount=0, nParas=mpImpl->GetParagraphCount(); i<nParas; ++i )
            nCount += mpImpl->GetParagraph(i).getCharacterCount();
        
        nCount = nCount + (nParas-1);
        return nCount;
    }

    sal_Int32 SAL_CALL AccessibleStaticTextBase::getIndexAtPoint( const awt::Point& rPoint ) throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        const sal_Int32 nParas( mpImpl->GetParagraphCount() );
        sal_Int32 nIndex;
        int i;
        for( i=0; i<nParas; ++i )
        {
            
            

            
            AccessibleEditableTextPara& rPara = mpImpl->GetParagraph( i );
            awt::Rectangle aParaBounds( rPara.getBounds() );
            awt::Point aPoint( rPoint );
            aPoint.X -= aParaBounds.X;
            aPoint.Y -= aParaBounds.Y;

            
            if ( ( nIndex = rPara.getIndexAtPoint( aPoint ) ) != -1 )
                return mpImpl->Internal2Index( EPosition(sal::static_int_cast<sal_uInt16>(i),
                                                         sal::static_int_cast<sal_uInt16>(nIndex)) );
        }

        return -1;
    }

    OUString SAL_CALL AccessibleStaticTextBase::getSelectedText() throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        sal_Int32 nStart( getSelectionStart() );
        sal_Int32 nEnd( getSelectionEnd() );

        
        if( nStart < 0 || nEnd < 0 )
            return OUString();

        return getTextRange( nStart, nEnd );
    }

    sal_Int32 SAL_CALL AccessibleStaticTextBase::getSelectionStart() throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

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
        SolarMutexGuard aGuard;

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
        SolarMutexGuard aGuard;

        EPosition aStartIndex( mpImpl->Range2Internal(nStartIndex) );
        EPosition aEndIndex( mpImpl->Range2Internal(nEndIndex) );

        return mpImpl->SetSelection( aStartIndex.nPara, aStartIndex.nIndex,
                                     aEndIndex.nPara, aEndIndex.nIndex );
    }

    OUString SAL_CALL AccessibleStaticTextBase::getText() throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        sal_Int32 i, nParas;
        OUString aRes;
        for( i=0, nParas=mpImpl->GetParagraphCount(); i<nParas; ++i )
            aRes += mpImpl->GetParagraph(i).getText();

        return aRes;
    }

    OUString SAL_CALL AccessibleStaticTextBase::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        if( nStartIndex > nEndIndex )
            ::std::swap(nStartIndex, nEndIndex);
        
        if ( nStartIndex == nEndIndex )
        {
            return OUString();
        }
        sal_Bool bStart = mpImpl->RemoveLineBreakCount( nStartIndex );
        
        if ( bStart )
        {
            nStartIndex++;
        }
        
        
        sal_Int32 nTemp = nEndIndex - 1;
        sal_Bool bEnd = mpImpl->RemoveLineBreakCount( nTemp );
        sal_Bool bTemp = mpImpl->RemoveLineBreakCount( nEndIndex );
        
        
        if ( bStart && bEnd && ( nStartIndex == nEndIndex) )
        {
            bEnd = sal_False;
        }
        
        
        
        if ( bTemp )
        {
            nEndIndex++;
        }
        OUString aRes;
        EPosition aStartIndex( mpImpl->Range2Internal(nStartIndex) );
        EPosition aEndIndex( mpImpl->Range2Internal(nEndIndex) );

        
        if( aStartIndex.nPara == aEndIndex.nPara )
        {
            
            aRes = mpImpl->GetParagraph( aStartIndex.nPara ).getTextRange( aStartIndex.nIndex, aEndIndex.nIndex );
        }
        else
        {
            sal_Int32 i( aStartIndex.nPara );
            aRes = mpImpl->GetParagraph(i).getTextRange( aStartIndex.nIndex,
                                                         mpImpl->GetParagraph(i).getCharacterCount()/*-1*/);
            ++i;

            
            for( ; i<aEndIndex.nPara; ++i )
            {
                aRes += OUString(cNewLine);
                aRes += mpImpl->GetParagraph(i).getText();
            }

            if( i<=aEndIndex.nPara )
            {
                
                
                if ( !bEnd && aEndIndex.nIndex )
                {
                    aRes += OUString(cNewLine);
                }
                aRes += mpImpl->GetParagraph(i).getTextRange( 0, aEndIndex.nIndex );
            }
        }
        
        
        if ( bStart )
        {
            aRes = OUString(cNewLine) + aRes;
        }
        if ( bEnd )
        {
            aRes += OUString(cNewLine);
        }
        return aRes;
    }

    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleStaticTextBase::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        sal_Bool bLineBreak = mpImpl->RemoveLineBreakCount( nIndex );
        EPosition aPos( mpImpl->Range2Internal(nIndex) );

        ::com::sun::star::accessibility::TextSegment aResult;

        if( AccessibleTextType::PARAGRAPH == aTextType )
        {
            
            
            
            
            

            
            aResult.SegmentText = mpImpl->GetParagraph( aPos.nPara ).getText();

            
            aResult.SegmentStart = mpImpl->Internal2Index( EPosition( aPos.nPara, 0 ) );
            aResult.SegmentEnd = aResult.SegmentStart + aResult.SegmentText.getLength();
        }
        else if ( AccessibleTextType::ATTRIBUTE_RUN == aTextType )
        {
              SvxAccessibleTextAdapter& rTextForwarder = mpImpl->GetParagraph( aPos.nIndex ).GetTextForwarder();
              sal_Int32 nStartIndex, nEndIndex;
              if ( rTextForwarder.GetAttributeRun( nStartIndex, nEndIndex, aPos.nPara, aPos.nIndex, sal_True ) )
              {
                     aResult.SegmentText = getTextRange( nStartIndex, nEndIndex );
                     aResult.SegmentStart = nStartIndex;
                     aResult.SegmentEnd = nEndIndex;
              }
        }
        else
        {
            
            aResult = mpImpl->GetParagraph( aPos.nPara ).getTextAtIndex( aPos.nIndex, aTextType );

            
            mpImpl->CorrectTextSegment( aResult, aPos.nPara );
            if ( bLineBreak )
            {
                aResult.SegmentText = OUString(cNewLine);
            }
        }

        return aResult;
    }

    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleStaticTextBase::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        sal_Int32 nOldIdx = nIndex;
        sal_Bool bLineBreak =  mpImpl->RemoveLineBreakCount( nIndex );
        EPosition aPos( mpImpl->Range2Internal(nIndex) );

        ::com::sun::star::accessibility::TextSegment aResult;

        if( AccessibleTextType::PARAGRAPH == aTextType )
        {
            if( aPos.nIndex == mpImpl->GetParagraph( aPos.nPara ).getCharacterCount() )
            {
                
                aResult.SegmentText = mpImpl->GetParagraph( aPos.nPara ).getText();

                
                aResult.SegmentStart = mpImpl->Internal2Index( EPosition( aPos.nPara, 0 ) );
            }
            else if( aPos.nPara > 0 )
            {
                aResult.SegmentText = mpImpl->GetParagraph( aPos.nPara - 1 ).getText();

                
                aResult.SegmentStart = mpImpl->Internal2Index( EPosition( aPos.nPara - 1, 0 ) );
            }

            aResult.SegmentEnd = aResult.SegmentStart + aResult.SegmentText.getLength();
        }
        else
        {
            
            aResult = mpImpl->GetParagraph( aPos.nPara ).getTextBeforeIndex( aPos.nIndex, aTextType );

            
            mpImpl->CorrectTextSegment( aResult, aPos.nPara );
            if ( bLineBreak && (nOldIdx-1) >= 0)
            {
                aResult = getTextAtIndex( nOldIdx-1, aTextType );
            }
        }

        return aResult;
    }

    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleStaticTextBase::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        sal_Int32 nTemp = nIndex+1;
        sal_Bool bLineBreak = mpImpl->RemoveLineBreakCount( nTemp );
        mpImpl->RemoveLineBreakCount( nIndex );
        EPosition aPos( mpImpl->Range2Internal(nIndex) );

        ::com::sun::star::accessibility::TextSegment aResult;

        if( AccessibleTextType::PARAGRAPH == aTextType )
        {
            
            
            
            if( aPos.nPara + 1 < mpImpl->GetParagraphCount() )
            {
                aResult.SegmentText = mpImpl->GetParagraph( aPos.nPara + 1 ).getText();

                
                aResult.SegmentStart = mpImpl->Internal2Index( EPosition( aPos.nPara + 1, 0 ) );
                aResult.SegmentEnd = aResult.SegmentStart + aResult.SegmentText.getLength();
            }
        }
        else
        {
            
            aResult = mpImpl->GetParagraph( aPos.nPara ).getTextBehindIndex( aPos.nIndex, aTextType );

            
            mpImpl->CorrectTextSegment( aResult, aPos.nPara );
            if ( bLineBreak )
            {
                aResult.SegmentText = OUString(cNewLine) + aResult.SegmentText;
            }
       }

        return aResult;
    }

    sal_Bool SAL_CALL AccessibleStaticTextBase::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        if( nStartIndex > nEndIndex )
            ::std::swap(nStartIndex, nEndIndex);

        EPosition aStartIndex( mpImpl->Range2Internal(nStartIndex) );
        EPosition aEndIndex( mpImpl->Range2Internal(nEndIndex) );

        return mpImpl->CopyText( aStartIndex.nPara, aStartIndex.nIndex,
                                 aEndIndex.nPara, aEndIndex.nIndex );
    }

    
    uno::Sequence< beans::PropertyValue > AccessibleStaticTextBase::getDefaultAttributes( const uno::Sequence< OUString >& RequestedAttributes ) throw (uno::RuntimeException)
    {
        

        SolarMutexGuard aGuard;

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

    uno::Sequence< beans::PropertyValue > SAL_CALL AccessibleStaticTextBase::getRunAttributes( sal_Int32 nIndex, const uno::Sequence< OUString >& RequestedAttributes ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        
        

        SolarMutexGuard aGuard;

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

}  



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
