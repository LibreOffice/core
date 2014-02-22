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
#include <osl/mutex.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <editeng/flditem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <comphelper/accessibleeventnotifier.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <vcl/unohelp.hxx>
#include <vcl/settings.hxx>

#include <editeng/editeng.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/unoipset.hxx>
#include <editeng/outliner.hxx>
#include <svl/intitem.hxx>


//

//


#include <com/sun/star/beans/PropertyState.hpp>

#include <editeng/unolingu.hxx>
#include <editeng/unopracc.hxx>
#include "editeng/AccessibleEditableTextPara.hxx"
#include "AccessibleHyperlink.hxx"

#include <svtools/colorcfg.hxx>
using namespace std;
#include "editeng.hrc"
#include <editeng/eerdll.hxx>
#include <editeng/numitem.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;


//

//


namespace accessibility
{
    const SvxItemPropertySet* ImplGetSvxCharAndParaPropertiesSet()
    {
        
        static const SfxItemPropertyMapEntry aPropMap[] =
        {
            SVX_UNOEDIT_OUTLINER_PROPERTIES,
            SVX_UNOEDIT_CHAR_PROPERTIES,
            SVX_UNOEDIT_PARA_PROPERTIES,
            SVX_UNOEDIT_NUMBERING_PROPERTIE,
            {OUString("TextUserDefinedAttributes"),     EE_CHAR_XMLATTRIBS,     ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
            {OUString("ParaUserDefinedAttributes"),     EE_PARA_XMLATTRIBS,     ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
            { OUString(), 0, css::uno::Type(), 0, 0 }
        };
        static SvxItemPropertySet aPropSet( aPropMap, EditEngine::GetGlobalItemPool() );
        return &aPropSet;
    }

    
    AccessibleEditableTextPara::AccessibleEditableTextPara(
                                const uno::Reference< XAccessible >& rParent,
                                const AccessibleParaManager* _pParaManager )
        : AccessibleTextParaInterfaceBase( m_aMutex ),
          mnParagraphIndex( 0 ),
          mnIndexInParent( 0 ),
          mpEditSource( NULL ),
          maEEOffset( 0, 0 ),
          mxParent( rParent ),
          
          
          
          
          
          mnNotifierClientId(::comphelper::AccessibleEventNotifier::registerClient()),
          
          mpParaManager( _pParaManager )
    {
#ifdef DBG_UTIL
        OSL_TRACE( "AccessibleEditableTextPara received ID: %d", mnNotifierClientId );
#endif

        try
        {
            
            ::utl::AccessibleStateSetHelper* pStateSet  = new ::utl::AccessibleStateSetHelper ();
            mxStateSet = pStateSet;

            
            pStateSet->AddState( AccessibleStateType::MULTI_LINE );
            pStateSet->AddState( AccessibleStateType::FOCUSABLE );
            pStateSet->AddState( AccessibleStateType::VISIBLE );
            pStateSet->AddState( AccessibleStateType::SHOWING );
            pStateSet->AddState( AccessibleStateType::ENABLED );
            pStateSet->AddState( AccessibleStateType::SENSITIVE );
        }
        catch (const uno::Exception&)
        {
        }
    }

    AccessibleEditableTextPara::~AccessibleEditableTextPara()
    {
        
        if( getNotifierClientId() != -1 )
        {
            try
            {
                ::comphelper::AccessibleEventNotifier::revokeClient( getNotifierClientId() );
#ifdef DBG_UTIL
                OSL_TRACE( "AccessibleEditableTextPara revoked ID: %d", mnNotifierClientId );
#endif
            }
            catch (const uno::Exception&)
            {
            }
        }
    }

    OUString AccessibleEditableTextPara::implGetText()
    {
        return GetTextRange( 0, GetTextLen() );
    }

    ::com::sun::star::lang::Locale AccessibleEditableTextPara::implGetLocale()
    {
        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= SAL_MAX_INT32,
                   "AccessibleEditableTextPara::getLocale: paragraph index value overflow");

        
        return LanguageTag(GetTextForwarder().GetLanguage( GetParagraphIndex(), 0 )).getLocale();
    }

    void AccessibleEditableTextPara::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
    {
        sal_uInt16 nStart, nEnd;

        if( GetSelection( nStart, nEnd ) )
        {
            nStartIndex = nStart;
            nEndIndex = nEnd;
        }
        else
        {
            
            nStartIndex = -1;
            nEndIndex = -1;
        }
    }

    void AccessibleEditableTextPara::implGetParagraphBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 /*nIndex*/ )
    {
        DBG_WARNING( "AccessibleEditableTextPara::implGetParagraphBoundary: only a base implementation, ignoring the index" );

        rBoundary.startPos = 0;
        
        OUString sText( implGetText() );
        sal_Int32 nLength = sText.getLength();
        rBoundary.endPos = nLength;
    }

    void AccessibleEditableTextPara::implGetLineBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex )
    {
        SvxTextForwarder&   rCacheTF = GetTextForwarder();
        const sal_Int32     nParaIndex = GetParagraphIndex();

        DBG_ASSERT(nParaIndex >= 0 && nParaIndex <= SAL_MAX_INT32,
                   "AccessibleEditableTextPara::implGetLineBoundary: paragraph index value overflow");

        const sal_Int32 nTextLen = rCacheTF.GetTextLen( nParaIndex );

        CheckPosition(nIndex);

        rBoundary.startPos = rBoundary.endPos = -1;

        const sal_Int32 nLineCount=rCacheTF.GetLineCount( nParaIndex );

        if( nIndex == nTextLen )
        {
            
            if( nLineCount <= 1 )
                rBoundary.startPos = 0;
            else
                rBoundary.startPos = nTextLen - rCacheTF.GetLineLen( nParaIndex,
                                                                     nLineCount-1 );

            rBoundary.endPos = nTextLen;
        }
        else
        {
            
            sal_Int32 nLine;
            sal_Int32 nCurIndex;
            for( nLine=0, nCurIndex=0; nLine<nLineCount; ++nLine )
            {
                nCurIndex += rCacheTF.GetLineLen( nParaIndex, nLine);

                if( nCurIndex > nIndex )
                {
                    rBoundary.startPos = nCurIndex - rCacheTF.GetLineLen( nParaIndex, nLine);
                    rBoundary.endPos = nCurIndex;
                    break;
                }
            }
        }
    }

    int AccessibleEditableTextPara::getNotifierClientId() const
    {
        return mnNotifierClientId;
    }

    void AccessibleEditableTextPara::SetIndexInParent( sal_Int32 nIndex )
    {
        mnIndexInParent = nIndex;
    }

    sal_Int32 AccessibleEditableTextPara::GetIndexInParent() const
    {
        return mnIndexInParent;
    }

    void AccessibleEditableTextPara::SetParagraphIndex( sal_Int32 nIndex )
    {
        sal_Int32 nOldIndex = mnParagraphIndex;

        mnParagraphIndex = nIndex;

        WeakBullet::HardRefType aChild( maImageBullet.get() );
        if( aChild.is() )
            aChild->SetParagraphIndex(mnParagraphIndex);

        try
        {
            if( nOldIndex != nIndex )
            {
                uno::Any aOldDesc;
                uno::Any aOldName;

                try
                {
                    aOldDesc <<= getAccessibleDescription();
                    aOldName <<= getAccessibleName();
                }
                catch (const uno::Exception&) 
                {
                }
                
                FireEvent( AccessibleEventId::DESCRIPTION_CHANGED, uno::makeAny( getAccessibleDescription() ), aOldDesc );
                FireEvent( AccessibleEventId::NAME_CHANGED, uno::makeAny( getAccessibleName() ), aOldName );
            }
        }
        catch (const uno::Exception&) 
        {
        }
    }

    sal_Int32 AccessibleEditableTextPara::GetParagraphIndex() const SAL_THROW((uno::RuntimeException))
    {
        return mnParagraphIndex;
    }

    void AccessibleEditableTextPara::Dispose()
    {
        int nClientId( getNotifierClientId() );

        
        mxParent = NULL;
        mnNotifierClientId = -1;
        mpEditSource = NULL;

        
        if( nClientId != -1 )
        {
            try
            {
                uno::Reference < XAccessibleContext > xThis = getAccessibleContext();

                
                ::comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( nClientId, xThis );
#ifdef DBG_UTIL
                OSL_TRACE( "Disposed ID: %d", nClientId );
#endif
            }
            catch (const uno::Exception&)
            {
            }
        }
    }

    void AccessibleEditableTextPara::SetEditSource( SvxEditSourceAdapter* pEditSource )
    {
        WeakBullet::HardRefType aChild( maImageBullet.get() );
        if( aChild.is() )
            aChild->SetEditSource(pEditSource);

        if( !pEditSource )
        {
            
            UnSetState( AccessibleStateType::SHOWING );
            UnSetState( AccessibleStateType::VISIBLE );
            SetState( AccessibleStateType::INVALID );
            SetState( AccessibleStateType::DEFUNC );

            Dispose();
        }
        mpEditSource = pEditSource;
        
        try
        {
            TextChanged();
        }
        catch (const uno::RuntimeException&)
        {
        }
    }

    ESelection AccessibleEditableTextPara::MakeSelection( sal_Int32 nStartEEIndex, sal_Int32 nEndEEIndex )
    {
        
        DBG_ASSERT(nStartEEIndex >= 0 && nStartEEIndex <= USHRT_MAX &&
                   nEndEEIndex >= 0 && nEndEEIndex <= USHRT_MAX &&
                   GetParagraphIndex() >= 0 && GetParagraphIndex() <= SAL_MAX_INT32,
                   "AccessibleEditableTextPara::MakeSelection: index value overflow");

        sal_Int32 nParaIndex = GetParagraphIndex();
        return ESelection( nParaIndex, static_cast< sal_uInt16 >( nStartEEIndex ),
                           nParaIndex, static_cast< sal_uInt16 >( nEndEEIndex ) );
    }

    ESelection AccessibleEditableTextPara::MakeSelection( sal_Int32 nEEIndex )
    {
        return MakeSelection( nEEIndex, nEEIndex+1 );
    }

    ESelection AccessibleEditableTextPara::MakeCursor( sal_Int32 nEEIndex )
    {
        return MakeSelection( nEEIndex, nEEIndex );
    }

    void AccessibleEditableTextPara::CheckIndex( sal_Int32 nIndex ) SAL_THROW((lang::IndexOutOfBoundsException, uno::RuntimeException))
    {
        if( nIndex < 0 || nIndex >= getCharacterCount() )
            throw lang::IndexOutOfBoundsException("AccessibleEditableTextPara: character index out of bounds",
                                                  uno::Reference< uno::XInterface >
                                                  ( static_cast< ::cppu::OWeakObject* > (this) ) ); 
    }

    void AccessibleEditableTextPara::CheckPosition( sal_Int32 nIndex ) SAL_THROW((lang::IndexOutOfBoundsException, uno::RuntimeException))
    {
        if( nIndex < 0 || nIndex > getCharacterCount() )
            throw lang::IndexOutOfBoundsException("AccessibleEditableTextPara: character position out of bounds",
                                                  uno::Reference< uno::XInterface >
                                                  ( static_cast< ::cppu::OWeakObject* > (this) ) ); 
    }

    void AccessibleEditableTextPara::CheckRange( sal_Int32 nStart, sal_Int32 nEnd ) SAL_THROW((lang::IndexOutOfBoundsException, uno::RuntimeException))
    {
        CheckPosition( nStart );
        CheckPosition( nEnd );
    }

    bool AccessibleEditableTextPara::GetSelection( sal_uInt16& nStartPos, sal_uInt16& nEndPos ) SAL_THROW((uno::RuntimeException))
    {
        ESelection aSelection;
        sal_Int32 nPara = GetParagraphIndex();
        if( !GetEditViewForwarder().GetSelection( aSelection ) )
            return false;

        if( aSelection.nStartPara < aSelection.nEndPara )
        {
            if( aSelection.nStartPara > nPara ||
                aSelection.nEndPara < nPara )
                return false;

            if( nPara == aSelection.nStartPara )
                nStartPos = aSelection.nStartPos;
            else
                nStartPos = 0;

            if( nPara == aSelection.nEndPara )
                nEndPos = aSelection.nEndPos;
            else
                nEndPos = GetTextLen();
        }
        else
        {
            if( aSelection.nStartPara < nPara ||
                aSelection.nEndPara > nPara )
                return false;

            if( nPara == aSelection.nStartPara )
                nStartPos = aSelection.nStartPos;
            else
                nStartPos = GetTextLen();

            if( nPara == aSelection.nEndPara )
                nEndPos = aSelection.nEndPos;
            else
                nEndPos = 0;
        }

        return true;
    }

    OUString AccessibleEditableTextPara::GetTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) SAL_THROW((uno::RuntimeException))
    {
        return GetTextForwarder().GetText( MakeSelection(nStartIndex, nEndIndex) );
    }

    sal_uInt16 AccessibleEditableTextPara::GetTextLen() const SAL_THROW((uno::RuntimeException))
    {
        return GetTextForwarder().GetTextLen( static_cast< sal_uInt16 >( GetParagraphIndex() ) );
    }

    SvxEditSourceAdapter& AccessibleEditableTextPara::GetEditSource() const SAL_THROW((uno::RuntimeException))
    {
        if( mpEditSource )
            return *mpEditSource;
        else
            throw uno::RuntimeException("No edit source, object is defunct",
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleEditableTextPara* > (this) ) ) ); 
    }

    SvxAccessibleTextAdapter& AccessibleEditableTextPara::GetTextForwarder() const SAL_THROW((uno::RuntimeException))
    {
        SvxEditSourceAdapter& rEditSource = GetEditSource();
        SvxAccessibleTextAdapter* pTextForwarder = rEditSource.GetTextForwarderAdapter();

        if( !pTextForwarder )
            throw uno::RuntimeException("Unable to fetch text forwarder, object is defunct",
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleEditableTextPara* > (this) ) ) ); 

        if( pTextForwarder->IsValid() )
            return *pTextForwarder;
        else
            throw uno::RuntimeException("Text forwarder is invalid, object is defunct",
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleEditableTextPara* > (this) ) ) ); 
    }

    SvxViewForwarder& AccessibleEditableTextPara::GetViewForwarder() const SAL_THROW((uno::RuntimeException))
    {
        SvxEditSource& rEditSource = GetEditSource();
        SvxViewForwarder* pViewForwarder = rEditSource.GetViewForwarder();

        if( !pViewForwarder )
        {
            throw uno::RuntimeException("Unable to fetch view forwarder, object is defunct",
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleEditableTextPara* > (this) ) ) ); 
        }

        if( pViewForwarder->IsValid() )
            return *pViewForwarder;
        else
            throw uno::RuntimeException("View forwarder is invalid, object is defunct",
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleEditableTextPara* > (this) )  ) );    
    }

    SvxAccessibleTextEditViewAdapter& AccessibleEditableTextPara::GetEditViewForwarder( bool bCreate ) const SAL_THROW((uno::RuntimeException))
    {
        SvxEditSourceAdapter& rEditSource = GetEditSource();
        SvxAccessibleTextEditViewAdapter* pTextEditViewForwarder = rEditSource.GetEditViewForwarderAdapter( bCreate );

        if( !pTextEditViewForwarder )
        {
            if( bCreate )
                throw uno::RuntimeException("Unable to fetch view forwarder, object is defunct",
                                            uno::Reference< uno::XInterface >
                                            ( static_cast< ::cppu::OWeakObject* >
                                              ( const_cast< AccessibleEditableTextPara* > (this) ) ) ); 
            else
                throw uno::RuntimeException("No view forwarder, object not in edit mode",
                                            uno::Reference< uno::XInterface >
                                            ( static_cast< ::cppu::OWeakObject* >
                                              ( const_cast< AccessibleEditableTextPara* > (this) ) ) ); 
        }

        if( pTextEditViewForwarder->IsValid() )
            return *pTextEditViewForwarder;
        else
        {
            if( bCreate )
                throw uno::RuntimeException("View forwarder is invalid, object is defunct",
                                            uno::Reference< uno::XInterface >
                                            ( static_cast< ::cppu::OWeakObject* >
                                              ( const_cast< AccessibleEditableTextPara* > (this) )  ) );    
            else
                throw uno::RuntimeException("View forwarder is invalid, object not in edit mode",
                                            uno::Reference< uno::XInterface >
                                            ( static_cast< ::cppu::OWeakObject* >
                                              ( const_cast< AccessibleEditableTextPara* > (this) )  ) );    
        }
    }

    bool AccessibleEditableTextPara::HaveEditView() const
    {
        SvxEditSource& rEditSource = GetEditSource();
        SvxEditViewForwarder* pViewForwarder = rEditSource.GetEditViewForwarder();

        if( !pViewForwarder )
            return false;

        if( !pViewForwarder->IsValid() )
            return false;

        return true;
    }

    bool AccessibleEditableTextPara::HaveChildren()
    {
        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= SAL_MAX_INT32,
                   "AccessibleEditableTextPara::HaveChildren: paragraph index value overflow");

        return GetTextForwarder().HaveImageBullet( GetParagraphIndex() );
    }

    Rectangle AccessibleEditableTextPara::LogicToPixel( const Rectangle& rRect, const MapMode& rMapMode, SvxViewForwarder& rForwarder )
    {
        
        return Rectangle( rForwarder.LogicToPixel( rRect.TopLeft(), rMapMode ),
                          rForwarder.LogicToPixel( rRect.BottomRight(), rMapMode ) );
    }

    const Point& AccessibleEditableTextPara::GetEEOffset() const
    {
        return maEEOffset;
    }

    void AccessibleEditableTextPara::SetEEOffset( const Point& rOffset )
    {
        WeakBullet::HardRefType aChild( maImageBullet.get() );
        if( aChild.is() )
            aChild->SetEEOffset(rOffset);

        maEEOffset = rOffset;
    }

    void AccessibleEditableTextPara::FireEvent(const sal_Int16 nEventId, const uno::Any& rNewValue, const uno::Any& rOldValue) const
    {
        uno::Reference < XAccessibleContext > xThis( const_cast< AccessibleEditableTextPara* > (this)->getAccessibleContext() );

        AccessibleEventObject aEvent(xThis, nEventId, rNewValue, rOldValue);

        
        if( nEventId == AccessibleEventId::STATE_CHANGED )
            vcl::unohelper::NotifyAccessibleStateEventGlobally( aEvent );

        
        if( getNotifierClientId() != -1 )
            ::comphelper::AccessibleEventNotifier::addEvent( getNotifierClientId(),
                                                             aEvent );
    }

    void AccessibleEditableTextPara::GotPropertyEvent( const uno::Any& rNewValue, const sal_Int16 nEventId ) const
    {
        FireEvent( nEventId, rNewValue );
    }

    void AccessibleEditableTextPara::LostPropertyEvent( const uno::Any& rOldValue, const sal_Int16 nEventId ) const
    {
        FireEvent( nEventId, uno::Any(), rOldValue );
    }

    void AccessibleEditableTextPara::SetState( const sal_Int16 nStateId )
    {
        ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
        if( pStateSet != NULL &&
            !pStateSet->contains(nStateId) )
        {
            pStateSet->AddState( nStateId );
            GotPropertyEvent( uno::makeAny( nStateId ), AccessibleEventId::STATE_CHANGED );
        }
    }

    void AccessibleEditableTextPara::UnSetState( const sal_Int16 nStateId )
    {
        ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
        if( pStateSet != NULL &&
            pStateSet->contains(nStateId) )
        {
            pStateSet->RemoveState( nStateId );
            LostPropertyEvent( uno::makeAny( nStateId ), AccessibleEventId::STATE_CHANGED );
        }
    }

    void AccessibleEditableTextPara::TextChanged()
    {
        OUString aCurrentString( OCommonAccessibleText::getText() );
        uno::Any aDeleted;
        uno::Any aInserted;
        if( OCommonAccessibleText::implInitTextChangedEvent( maLastTextString, aCurrentString,
                                                             aDeleted, aInserted) )
        {
            FireEvent( AccessibleEventId::TEXT_CHANGED, aInserted, aDeleted );
            maLastTextString = aCurrentString;
        }
    }

    bool AccessibleEditableTextPara::GetAttributeRun( sal_Int32& nStartIndex, sal_Int32& nEndIndex, sal_Int32 nIndex )
    {
        DBG_ASSERT(nIndex >= 0 && nIndex <= SAL_MAX_INT32,
                   "AccessibleEditableTextPara::GetAttributeRun: index value overflow");

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= SAL_MAX_INT32,
                   "AccessibleEditableTextPara::getLocale: paragraph index value overflow");

        return GetTextForwarder().GetAttributeRun( nStartIndex,
                                                   nEndIndex,
                                                   GetParagraphIndex(),
                                                   static_cast< sal_uInt16 >(nIndex) );
    }

    uno::Any SAL_CALL AccessibleEditableTextPara::queryInterface (const uno::Type & rType) throw (uno::RuntimeException)
    {
        uno::Any aRet;

        
        if ( rType == ::getCppuType((uno::Reference< XAccessibleText > *)0) )
        {
            uno::Reference< XAccessibleText > aAccText = static_cast< XAccessibleEditableText * >(this);
            aRet <<= aAccText;
        }
        else if ( rType == ::getCppuType((uno::Reference< XAccessibleEditableText > *)0) )
        {
            uno::Reference< XAccessibleEditableText > aAccEditText = this;
            aRet <<= aAccEditText;
        }
        else if ( rType == ::getCppuType((uno::Reference< XAccessibleHypertext > *)0) )
        {
            uno::Reference< XAccessibleHypertext > aAccHyperText = this;
            aRet <<= aAccHyperText;
        }
        else
        {
            aRet = AccessibleTextParaInterfaceBase::queryInterface(rType);
        }

        return aRet;
    }

    
    uno::Reference< XAccessibleContext > SAL_CALL AccessibleEditableTextPara::getAccessibleContext() throw (uno::RuntimeException)
    {
        
        return uno::Reference< XAccessibleContext > ( this );
    }

    
    sal_Int32 SAL_CALL AccessibleEditableTextPara::getAccessibleChildCount() throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        return HaveChildren() ? 1 : 0;
    }

    uno::Reference< XAccessible > SAL_CALL AccessibleEditableTextPara::getAccessibleChild( sal_Int32 i ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        if( !HaveChildren() )
            throw lang::IndexOutOfBoundsException("No children available",
                                                  uno::Reference< uno::XInterface >
                                                  ( static_cast< ::cppu::OWeakObject* > (this) ) ); 

        if( i != 0 )
            throw lang::IndexOutOfBoundsException("Invalid child index",
                                                  uno::Reference< uno::XInterface >
                                                  ( static_cast< ::cppu::OWeakObject* > (this) ) ); 

        WeakBullet::HardRefType aChild( maImageBullet.get() );

        if( !aChild.is() )
        {
            
            AccessibleImageBullet* pChild = new AccessibleImageBullet( uno::Reference< XAccessible >( this ) );
            uno::Reference< XAccessible > xChild( static_cast< ::cppu::OWeakObject* > (pChild), uno::UNO_QUERY );

            if( !xChild.is() )
                throw uno::RuntimeException("Child creation failed",
                                            uno::Reference< uno::XInterface >
                                            ( static_cast< ::cppu::OWeakObject* > (this) ) );

            aChild = WeakBullet::HardRefType( xChild, pChild );

            aChild->SetEditSource( &GetEditSource() );
            aChild->SetParagraphIndex( GetParagraphIndex() );
            aChild->SetIndexInParent( i );

            maImageBullet = aChild;
        }

        return aChild.getRef();
    }

    uno::Reference< XAccessible > SAL_CALL AccessibleEditableTextPara::getAccessibleParent() throw (uno::RuntimeException)
    {

#ifdef DBG_UTIL
        if( !mxParent.is() )
            OSL_TRACE( "AccessibleEditableTextPara::getAccessibleParent: no frontend set, did somebody forgot to call AccessibleTextHelper::SetEventSource()?");
#endif

        return mxParent;
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getAccessibleIndexInParent() throw (uno::RuntimeException)
    {
        return mnIndexInParent;
    }

    sal_Int16 SAL_CALL AccessibleEditableTextPara::getAccessibleRole() throw (uno::RuntimeException)
    {
        return AccessibleRole::PARAGRAPH;
    }

    OUString SAL_CALL AccessibleEditableTextPara::getAccessibleDescription() throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        
        
        
        
        OUString aLine;

        if( getCharacterCount() )
            aLine = getTextAtIndex(0, AccessibleTextType::LINE).SegmentText;

        
        OUString sStr(EditResId(RID_SVXSTR_A11Y_PARAGRAPH_DESCRIPTION));
        OUString sParaIndex = OUString::number(GetParagraphIndex());
        sStr = sStr.replaceFirst("$(ARG)", sParaIndex);

        if( aLine.getLength() > MaxDescriptionLen )
        {
            OUString aCurrWord;
            sal_Int32 i;

            
            for( aCurrWord=getTextAtIndex(MaxDescriptionLen, AccessibleTextType::WORD).SegmentText,
                     i=MaxDescriptionLen,
                     aLine=OUString();
                 i>=0;
                 --i )
            {
                if( getTextAtIndex(i, AccessibleTextType::WORD).SegmentText != aCurrWord )
                {
                    if( i == 0 )
                        
                        aLine = getTextAtIndex(0, AccessibleTextType::WORD).SegmentText;
                    else
                        aLine = getTextRange(0, i);
                }
            }
        }

        return OUString( sStr ) + aLine;
    }

    OUString SAL_CALL AccessibleEditableTextPara::getAccessibleName() throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        
        sal_Int32 nPara( GetParagraphIndex() );

        
        OUString sStr(EditResId(RID_SVXSTR_A11Y_PARAGRAPH_NAME));
        OUString sParaIndex = OUString::number(nPara);
        return sStr.replaceFirst("$(ARG)", sParaIndex);
    }

    uno::Reference< XAccessibleRelationSet > SAL_CALL AccessibleEditableTextPara::getAccessibleRelationSet() throw (uno::RuntimeException)
    {
        
        
        if ( mpParaManager )
        {
            utl::AccessibleRelationSetHelper* pAccRelSetHelper =
                                        new utl::AccessibleRelationSetHelper();
            sal_Int32 nMyParaIndex( GetParagraphIndex() );
            
            if ( nMyParaIndex > 0 &&
                 mpParaManager->IsReferencable( nMyParaIndex - 1 ) )
            {
                uno::Sequence<uno::Reference<XInterface> > aSequence(1);
                aSequence[0] =
                    mpParaManager->GetChild( nMyParaIndex - 1 ).first.get().getRef();
                AccessibleRelation aAccRel( AccessibleRelationType::CONTENT_FLOWS_FROM,
                                            aSequence );
                pAccRelSetHelper->AddRelation( aAccRel );
            }

            
            if ( (nMyParaIndex + 1) < (sal_Int32)mpParaManager->GetNum() &&
                 mpParaManager->IsReferencable( nMyParaIndex + 1 ) )
            {
                uno::Sequence<uno::Reference<XInterface> > aSequence(1);
                aSequence[0] =
                    mpParaManager->GetChild( nMyParaIndex + 1 ).first.get().getRef();
                AccessibleRelation aAccRel( AccessibleRelationType::CONTENT_FLOWS_TO,
                                            aSequence );
                pAccRelSetHelper->AddRelation( aAccRel );
            }

            return pAccRelSetHelper;
        }
        else
        {
            
            return uno::Reference< XAccessibleRelationSet >();
        }
    }

    static uno::Sequence< OUString > getAttributeNames()
    {
        static uno::Sequence< OUString >* pNames = NULL;

        if( pNames == NULL )
        {
            uno::Sequence< OUString >* pSeq = new uno::Sequence< OUString >( 21 );
            OUString* pStrings = pSeq->getArray();
            sal_Int32 i = 0;
            #define STR(x) pStrings[i++] = OUString(x)
            STR("CharColor");
            STR("CharContoured");
            STR("CharEmphasis");
            STR("CharEscapement");
            STR("CharFontName");
            STR("CharHeight");
            STR("CharPosture");
            STR("CharShadowed");
            STR("CharStrikeout");
            STR("CharCaseMap");
            STR("CharUnderline");
            STR("CharUnderlineColor");
            STR("CharWeight");
            STR("NumberingLevel");
            STR("NumberingRules");
            STR("ParaAdjust");
            STR("ParaBottomMargin");
            STR("ParaFirstLineIndent");
            STR("ParaLeftMargin");
            STR("ParaLineSpacing");
            STR("ParaRightMargin");
            STR("ParaTabStops");
            #undef STR
            DBG_ASSERT( i == pSeq->getLength(), "Please adjust length" );
            if( i != pSeq->getLength() )
                pSeq->realloc( i );
            pNames = pSeq;
        }
        return *pNames;
    }

    struct IndexCompare
    {
        const PropertyValue* pValues;
        IndexCompare( const PropertyValue* pVals ) : pValues(pVals) {}
        bool operator() ( const sal_Int32& a, const sal_Int32& b ) const
        {
            return (pValues[a].Name < pValues[b].Name) ? true : false;
        }
    };

    OUString AccessibleEditableTextPara::GetFieldTypeNameAtIndex(sal_Int32 nIndex)
    {
        OUString strFldType;
        SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();
        
        sal_Int32 nParaIndex = GetParagraphIndex();
        sal_Int32 nAllFieldLen = 0;
        sal_Int32 nField = rCacheTF.GetFieldCount(sal_uInt16(nParaIndex)), nFoundFieldIndex = -1;
        EFieldInfo ree;
        sal_Int32  reeBegin, reeEnd;
        sal_Int32 nFieldType = -1;
        for(sal_uInt16 j = 0; j < nField; j++)
        {
            ree = rCacheTF.GetFieldInfo(sal_uInt16(nParaIndex), j);
            reeBegin  = ree.aPosition.nIndex + nAllFieldLen;
            reeEnd = reeBegin + ree.aCurrentText.getLength();
            nAllFieldLen += (ree.aCurrentText.getLength() - 1);
            if( reeBegin > nIndex )
            {
                break;
            }
            if(  nIndex >= reeBegin && nIndex < reeEnd )
            {
                nFoundFieldIndex = j;
                break;
            }
        }
        if( nFoundFieldIndex >= 0  )
        {
            
            nFieldType = ree.pFieldItem->GetField()->GetClassId() ;
        }
        switch(nFieldType)
        {
        case text::textfield::Type::DATE:
            {
                const SvxDateField* pDateField = static_cast< const SvxDateField* >(ree.pFieldItem->GetField());
                if (pDateField)
                {
                    if (pDateField->GetType() == SVXDATETYPE_FIX)
                        strFldType = "date (fixed)";
                    else if (pDateField->GetType() == SVXDATETYPE_VAR)
                        strFldType = "date (variable)";
                }
            }
            break;
        case text::textfield::Type::PAGE:
            strFldType = "page-number";
            break;
        
        case text::textfield::Type::PAGES:
            strFldType = "page-count";
            break;
        case text::textfield::Type::TABLE:
            strFldType = "sheet-name";
            break;
        
        case text::textfield::Type::TIME:
            strFldType = "time";
            break;
        case text::textfield::Type::EXTENDED_TIME:
            {
                const SvxExtTimeField* pTimeField = static_cast< const SvxExtTimeField* >(ree.pFieldItem->GetField());
                if (pTimeField)
                {
                    if (pTimeField->GetType() == SVXTIMETYPE_FIX)
                        strFldType = "time (fixed)";
                    else if (pTimeField->GetType() == SVXTIMETYPE_VAR)
                        strFldType = "time (variable)";
                }
            }
            break;
        case text::textfield::Type::AUTHOR:
            strFldType = "author";
            break;
        case text::textfield::Type::EXTENDED_FILE:
        case text::textfield::Type::DOCINFO_TITLE:
            strFldType = "file name";
        default:
            break;
        }
        return strFldType;
    }

    uno::Reference< XAccessibleStateSet > SAL_CALL AccessibleEditableTextPara::getAccessibleStateSet() throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        
        ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());

        if( !pStateSet )
            return uno::Reference<XAccessibleStateSet>();
        uno::Reference<XAccessibleStateSet> xParentStates;
        if (getAccessibleParent().is())
        {
            uno::Reference<XAccessibleContext> xParentContext = getAccessibleParent()->getAccessibleContext();
            xParentStates = xParentContext->getAccessibleStateSet();
        }
        if (xParentStates.is() && xParentStates->contains(AccessibleStateType::EDITABLE) )
        {
            pStateSet->AddState(AccessibleStateType::EDITABLE);
        }
        return uno::Reference<XAccessibleStateSet>( new ::utl::AccessibleStateSetHelper (*pStateSet) );
    }

    lang::Locale SAL_CALL AccessibleEditableTextPara::getLocale() throw (IllegalAccessibleComponentStateException, uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        return implGetLocale();
    }

    void SAL_CALL AccessibleEditableTextPara::addAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) throw (uno::RuntimeException)
    {
        if( getNotifierClientId() != -1 )
            ::comphelper::AccessibleEventNotifier::addEventListener( getNotifierClientId(), xListener );
    }

    void SAL_CALL AccessibleEditableTextPara::removeAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) throw (uno::RuntimeException)
    {
        if( getNotifierClientId() != -1 )
            ::comphelper::AccessibleEventNotifier::removeEventListener( getNotifierClientId(), xListener );
    }

    
    sal_Bool SAL_CALL AccessibleEditableTextPara::containsPoint( const awt::Point& aTmpPoint ) throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= SAL_MAX_INT32,
                   "AccessibleEditableTextPara::contains: index value overflow");

        awt::Rectangle aTmpRect = getBounds();
        Rectangle aRect( Point(aTmpRect.X, aTmpRect.Y), Size(aTmpRect.Width, aTmpRect.Height) );
        Point aPoint( aTmpPoint.X, aTmpPoint.Y );

        return aRect.IsInside( aPoint );
    }

    uno::Reference< XAccessible > SAL_CALL AccessibleEditableTextPara::getAccessibleAtPoint( const awt::Point& _aPoint ) throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        if( HaveChildren() )
        {
            
            Point aPoint( _aPoint.X, _aPoint.Y );

            
            aPoint -= GetEEOffset();

            
            SvxTextForwarder& rCacheTF = GetTextForwarder();
            Point aLogPoint( GetViewForwarder().PixelToLogic( aPoint, rCacheTF.GetMapMode() ) );

            EBulletInfo aBulletInfo = rCacheTF.GetBulletInfo( static_cast< sal_uInt16 > (GetParagraphIndex()) );

            if( aBulletInfo.nParagraph != EE_PARA_NOT_FOUND &&
                aBulletInfo.bVisible &&
                aBulletInfo.nType == SVX_NUM_BITMAP )
            {
                Rectangle aRect = aBulletInfo.aBounds;

                if( aRect.IsInside( aLogPoint ) )
                    return getAccessibleChild(0);
            }
        }

        
        return uno::Reference< XAccessible >();
    }

    awt::Rectangle SAL_CALL AccessibleEditableTextPara::getBounds() throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= SAL_MAX_INT32,
                   "AccessibleEditableTextPara::getBounds: index value overflow");

        SvxTextForwarder& rCacheTF = GetTextForwarder();
        Rectangle aRect = rCacheTF.GetParaBounds( GetParagraphIndex() );

        
        Rectangle aScreenRect = AccessibleEditableTextPara::LogicToPixel( aRect,
                                                                          rCacheTF.GetMapMode(),
                                                                          GetViewForwarder() );

        
        Point aOffset = GetEEOffset();

        return awt::Rectangle( aScreenRect.Left() + aOffset.X(),
                               aScreenRect.Top() + aOffset.Y(),
                               aScreenRect.GetSize().Width(),
                               aScreenRect.GetSize().Height() );
    }

    awt::Point SAL_CALL AccessibleEditableTextPara::getLocation(  ) throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        awt::Rectangle aRect = getBounds();

        return awt::Point( aRect.X, aRect.Y );
    }

    awt::Point SAL_CALL AccessibleEditableTextPara::getLocationOnScreen(  ) throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        
        uno::Reference< XAccessible > xParent = getAccessibleParent();
        if( xParent.is() )
        {
            uno::Reference< XAccessibleComponent > xParentComponent( xParent, uno::UNO_QUERY );
            if( xParentComponent.is() )
            {
                awt::Point aRefPoint = xParentComponent->getLocationOnScreen();
                awt::Point aPoint = getLocation();
                aPoint.X += aRefPoint.X;
                aPoint.Y += aRefPoint.Y;

                return aPoint;
            }
            
            
            else
            {
                uno::Reference< XAccessibleContext > xParentContext = xParent->getAccessibleContext();
                if ( xParentContext.is() )
                {
                    uno::Reference< XAccessibleComponent > xParentContextComponent( xParentContext, uno::UNO_QUERY );
                    if( xParentContextComponent.is() )
                    {
                        awt::Point aRefPoint = xParentContextComponent->getLocationOnScreen();
                        awt::Point aPoint = getLocation();
                        aPoint.X += aRefPoint.X;
                        aPoint.Y += aRefPoint.Y;

                        return aPoint;
                    }
                }
            }
        }

        throw uno::RuntimeException("Cannot access parent",
                                    uno::Reference< uno::XInterface >
                                    ( static_cast< XAccessible* > (this) ) );   
    }

    awt::Size SAL_CALL AccessibleEditableTextPara::getSize(  ) throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        awt::Rectangle aRect = getBounds();

        return awt::Size( aRect.Width, aRect.Height );
    }

    void SAL_CALL AccessibleEditableTextPara::grabFocus(  ) throw (uno::RuntimeException)
    {
        
        setSelection(0,0);
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getForeground(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        
        svtools::ColorConfig aColorConfig;
        sal_uInt32 nColor = aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor;
        return static_cast<sal_Int32>(nColor);
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getBackground(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        
        Color aColor( Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor() );

        
        aColor.SetTransparency( 0xFF);

        return static_cast<sal_Int32>( aColor.GetColor() );
    }

    
    sal_Int32 SAL_CALL AccessibleEditableTextPara::getCaretPosition() throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        if( !HaveEditView() )
            return -1;

        ESelection aSelection;
        if( GetEditViewForwarder().GetSelection( aSelection ) &&
            GetParagraphIndex() == aSelection.nEndPara )
        {
            
            EBulletInfo aBulletInfo = GetTextForwarder().GetBulletInfo( static_cast< sal_uInt16 >(GetParagraphIndex()) );
            if( aBulletInfo.nParagraph != EE_PARA_NOT_FOUND &&
                aBulletInfo.bVisible &&
                aBulletInfo.nType != SVX_NUM_BITMAP )
            {
                sal_Int32 nBulletLen = aBulletInfo.aText.getLength();
                if( aSelection.nEndPos - nBulletLen >= 0 )
                    return aSelection.nEndPos - nBulletLen;
            }
            return aSelection.nEndPos;
        }

        
        return -1;
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::setCaretPosition( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        return setSelection(nIndex, nIndex);
    }

    sal_Unicode SAL_CALL AccessibleEditableTextPara::getCharacter( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getCharacter: index value overflow");

        return OCommonAccessibleText::getCharacter( nIndex );
    }

    uno::Sequence< beans::PropertyValue > SAL_CALL AccessibleEditableTextPara::getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< OUString >& rRequestedAttributes ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        
        SvxTextForwarder& rCacheTF = GetTextForwarder();
        EBulletInfo aBulletInfo = rCacheTF.GetBulletInfo( static_cast< sal_uInt16 >(GetParagraphIndex()) );
        if (aBulletInfo.bVisible)
            nIndex += aBulletInfo.aText.getLength();
        CheckIndex(nIndex); 

        bool bSupplementalMode = false;
        uno::Sequence< OUString > aPropertyNames = rRequestedAttributes;
        if (aPropertyNames.getLength() == 0)
        {
            bSupplementalMode = true;
            aPropertyNames = getAttributeNames();
        }

        
        ::comphelper::SequenceAsHashMap aPropHashMap( getDefaultAttributes( aPropertyNames ) );

        
        uno::Sequence< beans::PropertyValue > aRunAttribs( getRunAttributes( nIndex, aPropertyNames ) );
        sal_Int32 nRunAttribs = aRunAttribs.getLength();
        const beans::PropertyValue *pRunAttrib = aRunAttribs.getConstArray();
        for (sal_Int32 k = 0;  k < nRunAttribs;  ++k)
        {
            const beans::PropertyValue &rRunAttrib = pRunAttrib[k];
            aPropHashMap[ rRunAttrib.Name ] = rRunAttrib.Value; 
        }

        
        uno::Sequence< beans::PropertyValue > aRes;
        aPropHashMap >> aRes;

        
        
        
        sal_Int32 nRes = aRes.getLength();
        beans::PropertyValue *pRes = aRes.getArray();
        for (sal_Int32 i = 0;  i < nRes;  ++i)
        {
            beans::PropertyValue &rRes = pRes[i];
            sal_Bool bIsDirectVal = sal_False;
            for (sal_Int32 k = 0;  k < nRunAttribs && !bIsDirectVal;  ++k)
            {
                if (rRes.Name == pRunAttrib[k].Name)
                    bIsDirectVal = sal_True;
            }
            rRes.Handle = -1;
            rRes.State  = bIsDirectVal ? PropertyState_DIRECT_VALUE : PropertyState_DEFAULT_VALUE;
        }
        if( bSupplementalMode )
        {
            _correctValues( nIndex, aRes );
            
            nRes = aRes.getLength();
            aRes.realloc( nRes + 1 );
            pRes = aRes.getArray();
            beans::PropertyValue &rRes = pRes[nRes];
            rRes.Name = "NumberingPrefix";
            OUString numStr;
            if (aBulletInfo.nType != SVX_NUM_CHAR_SPECIAL && aBulletInfo.nType != SVX_NUM_BITMAP)
                numStr = aBulletInfo.aText;
            rRes.Value <<= numStr;
            rRes.Handle = -1;
            rRes.State = PropertyState_DIRECT_VALUE;
            
            OUString strFieldType = GetFieldTypeNameAtIndex(nIndex);
            if (!strFieldType.isEmpty())
            {
                nRes = aRes.getLength();
                aRes.realloc( nRes + 1 );
                pRes = aRes.getArray();
                beans::PropertyValue &rResField = pRes[nRes];
                beans::PropertyValue aFieldType;
                rResField.Name = "FieldType";
                rResField.Value <<= strFieldType.toAsciiLowerCase();
                rResField.Handle = -1;
                rResField.State = PropertyState_DIRECT_VALUE;
        }
        
        
        sal_Int32 nLength = aRes.getLength();
        const beans::PropertyValue* pPairs = aRes.getConstArray();
        sal_Int32* pIndices = new sal_Int32[nLength];
        sal_Int32 i = 0;
        for( i = 0; i < nLength; i++ )
            pIndices[i] = i;
        sort( &pIndices[0], &pIndices[nLength], IndexCompare(pPairs) );
        
        uno::Sequence<beans::PropertyValue> aNewValues( nLength );
        beans::PropertyValue* pNewValues = aNewValues.getArray();
        for( i = 0; i < nLength; i++ )
        {
            pNewValues[i] = pPairs[pIndices[i]];
        }
        delete[] pIndices;
        //
        return aNewValues;
        }
        return aRes;
    }

    awt::Rectangle SAL_CALL AccessibleEditableTextPara::getCharacterBounds( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getCharacterBounds: index value overflow");

        
        
        CheckPosition( nIndex );

        SvxTextForwarder& rCacheTF = GetTextForwarder();
        Rectangle aRect = rCacheTF.GetCharBounds( static_cast< sal_uInt16 >( GetParagraphIndex() ), static_cast< sal_uInt16 >( nIndex ) );

        
        Rectangle aScreenRect = AccessibleEditableTextPara::LogicToPixel( aRect,
                                                                          rCacheTF.GetMapMode(),
                                                                          GetViewForwarder() );
        
        
        
        awt::Rectangle aParaRect( getBounds() );
        aScreenRect.Move( -aParaRect.X, -aParaRect.Y );

        
        Point aOffset = GetEEOffset();

        return awt::Rectangle( aScreenRect.Left() + aOffset.X(),
                               aScreenRect.Top() + aOffset.Y(),
                               aScreenRect.GetSize().Width(),
                               aScreenRect.GetSize().Height() );
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getCharacterCount() throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getCharacterCount: index value overflow");

        return OCommonAccessibleText::getCharacterCount();
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getIndexAtPoint( const awt::Point& rPoint ) throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        sal_Int32 nPara;
        sal_Int32 nIndex;

        
        Point aOffset( GetEEOffset() );
        Point aPoint( rPoint.X - aOffset.X(), rPoint.Y - aOffset.Y() );

        
        SvxTextForwarder& rCacheTF = GetTextForwarder();
        Point aLogPoint( GetViewForwarder().PixelToLogic( aPoint, rCacheTF.GetMapMode() ) );

        
        Rectangle aParaRect = rCacheTF.GetParaBounds( GetParagraphIndex() );
        aLogPoint.Move( aParaRect.Left(), aParaRect.Top() );

        if( rCacheTF.GetIndexAtPoint( aLogPoint, nPara, nIndex ) &&
            GetParagraphIndex() == nPara )
        {
            
            try
            {
                awt::Rectangle aRect1( getCharacterBounds(nIndex) );
                Rectangle aRect2( aRect1.X, aRect1.Y,
                                  aRect1.Width + aRect1.X, aRect1.Height + aRect1.Y );
                if( aRect2.IsInside( Point( rPoint.X, rPoint.Y ) ) )
                    return nIndex;
                else
                    return -1;
            }
            catch (const lang::IndexOutOfBoundsException&)
            {
                
                return -1;
            }
        }
        else
        {
            
            return -1;
        }
    }

    OUString SAL_CALL AccessibleEditableTextPara::getSelectedText() throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getSelectedText: index value overflow");

        if( !HaveEditView() )
            return OUString();

        return OCommonAccessibleText::getSelectedText();
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getSelectionStart() throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getSelectionStart: index value overflow");

        if( !HaveEditView() )
            return -1;

        return OCommonAccessibleText::getSelectionStart();
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getSelectionEnd() throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getSelectionEnd: index value overflow");

        if( !HaveEditView() )
            return -1;

        return OCommonAccessibleText::getSelectionEnd();
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::setSelection: paragraph index value overflow");

        CheckRange(nStartIndex, nEndIndex);

        try
        {
            SvxEditViewForwarder& rCacheVF = GetEditViewForwarder( true );
            return rCacheVF.SetSelection( MakeSelection(nStartIndex, nEndIndex) );
        }
        catch (const uno::RuntimeException&)
        {
            return sal_False;
        }
    }

    OUString SAL_CALL AccessibleEditableTextPara::getText() throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getText: paragraph index value overflow");

        return OCommonAccessibleText::getText();
    }

    OUString SAL_CALL AccessibleEditableTextPara::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getTextRange: paragraph index value overflow");

        return OCommonAccessibleText::getTextRange(nStartIndex, nEndIndex);
    }

    void AccessibleEditableTextPara::_correctValues( const sal_Int32 /* nIndex */,
                                           uno::Sequence< PropertyValue >& rValues)
    {
        SvxTextForwarder& rCacheTF = GetTextForwarder();
        sal_Int32 nRes = rValues.getLength();
        beans::PropertyValue *pRes = rValues.getArray();
        for (sal_Int32 i = 0;  i < nRes;  ++i)
        {
            beans::PropertyValue &rRes = pRes[i];
            
            if (rRes.Name == "CharColor")
            {
                uno::Any &anyChar = rRes.Value;
                sal_uInt32 crChar = static_cast<sal_uInt32>( reinterpret_cast<sal_uIntPtr>(anyChar.pReserved));
                if (COL_AUTO == crChar )
                {
                    uno::Reference< ::com::sun::star::accessibility::XAccessibleComponent > xComponent;
                    if (mxParent.is())
                    {
                        xComponent.set(mxParent,uno::UNO_QUERY);
                    }
                    else
                    {
                        xComponent.set(m_xAccInfo,uno::UNO_QUERY);
                    }
                    if (xComponent.is())
                    {
                        uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > xContext(xComponent,uno::UNO_QUERY);
                        if (xContext->getAccessibleRole() == AccessibleRole::SHAPE
                            || xContext->getAccessibleRole() == AccessibleRole::TABLE_CELL)
                        {
                            anyChar <<= COL_BLACK;
                        }
                        else
                        {
                            Color cr(xComponent->getBackground());
                            crChar = cr.IsDark() ? COL_WHITE : COL_BLACK;
                            anyChar <<= crChar;
                        }
                    }
                }
                continue;
            }
            
            if (rRes.Name == "CharUnderline")
            {
                /*
                
                if (IsCurrentEditorEnableAutoSpell( mxParent ))
                {
                    try
                    {
                        SvxEditViewForwarder& rCacheVF = GetEditViewForwarder( false );
                        sal_Bool bWrong = rCacheVF.IsWrongSpelledWordAtPos( GetParagraphIndex(), nIndex );
                        if ( bWrong )
                        {
                            uno::Any &anyUnderLine = pRes[9].Value;
                            
                            anyUnderLine <<= (sal_uInt16)UNDERLINE_WAVE;
                        }
                    }
                    catch( const uno::RuntimeException& )
                    {
                    }
                }
                */
                continue;
            }
            
            if (rRes.Name == "CharUnderlineColor")
            {
                uno::Any &anyCharUnderLine = rRes.Value;
                sal_uInt32 crCharUnderLine = static_cast<sal_uInt32>( reinterpret_cast<sal_uIntPtr>( anyCharUnderLine.pReserved));
                if (COL_AUTO == crCharUnderLine )
                {
                    uno::Reference< ::com::sun::star::accessibility::XAccessibleComponent > xComponent;
                    if (mxParent.is())
                    {
                        xComponent.set(mxParent,uno::UNO_QUERY);
                    }
                    else
                    {
                        xComponent.set(m_xAccInfo,uno::UNO_QUERY);
                    }
                    if (xComponent.is())
                    {
                        uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > xContext(xComponent,uno::UNO_QUERY);
                        if (xContext->getAccessibleRole() == AccessibleRole::SHAPE
                            || xContext->getAccessibleRole() == AccessibleRole::TABLE_CELL)
                        {
                            anyCharUnderLine <<= COL_BLACK;
                        }
                        else
                        {
                            Color cr(xComponent->getBackground());
                            crCharUnderLine = cr.IsDark() ? COL_WHITE : COL_BLACK;
                            anyCharUnderLine <<= crCharUnderLine;
                        }
                    }
                }
                
                /*
                if (IsCurrentEditorEnableAutoSpell( mxParent ))
                {
                    try
                    {
                        SvxEditViewForwarder& rCacheVF = GetEditViewForwarder( false );
                        sal_Bool bWrong = rCacheVF.IsWrongSpelledWordAtPos( GetParagraphIndex(), nIndex );
                        if ( bWrong )
                        {
                            uno::Any &anyUnderLineColor = rRes.Value;
                            
                            anyUnderLineColor <<= COL_LIGHTRED;
                        }
                    }
                    catch( const uno::RuntimeException& )
                    {
                    }
                }
                */
                continue;
            }
            
            if (rRes.Name == "NumberingLevel")
            {
                const SvxNumBulletItem& rNumBullet = ( SvxNumBulletItem& )rCacheTF.GetParaAttribs(static_cast< sal_uInt16 >(GetParagraphIndex())).Get(EE_PARA_NUMBULLET);
                if(rNumBullet.GetNumRule()->GetLevelCount()==0)
                {
                    rRes.Value <<= (sal_Int16)-1;
                    rRes.Handle = -1;
                    rRes.State = PropertyState_DIRECT_VALUE;
                }
                else
                {


                    
                    SvxAccessibleTextPropertySet aPropSet( &GetEditSource(), ImplGetSvxTextPortionSvxPropertySet() );

                    aPropSet.SetSelection( MakeSelection( 0, GetTextLen() ) );
                    rRes.Value = aPropSet._getPropertyValue( rRes.Name, mnParagraphIndex );
                    rRes.State = aPropSet._getPropertyState( rRes.Name, mnParagraphIndex );
                    rRes.Handle = -1;
                }
                continue;
            }
            
            if (rRes.Name == "NumberingRules")
            {
                SfxItemSet aAttribs = rCacheTF.GetParaAttribs( static_cast< sal_uInt16 >(GetParagraphIndex()) );
                sal_Bool bVis = ((const SfxUInt16Item&)aAttribs.Get( EE_PARA_BULLETSTATE )).GetValue() ? sal_True : sal_False;
                if(bVis)
                {
                    rRes.Value <<= (sal_Int16)-1;
                    rRes.Handle = -1;
                    rRes.State = PropertyState_DIRECT_VALUE;
                }
                else
                {
                    
                    SvxAccessibleTextPropertySet aPropSet( &GetEditSource(), ImplGetSvxTextPortionSvxPropertySet() );
                    aPropSet.SetSelection( MakeSelection( 0, GetTextLen() ) );
                    rRes.Value = aPropSet._getPropertyValue( rRes.Name, mnParagraphIndex );
                    rRes.State = aPropSet._getPropertyState( rRes.Name, mnParagraphIndex );
                    rRes.Handle = -1;
                }
                continue;
            }
        }
    }
    sal_Int32 AccessibleEditableTextPara::SkipField(sal_Int32 nIndex, bool bForward)
    {
        sal_Int32 nParaIndex = GetParagraphIndex();
        SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();
        sal_Int32 nAllFieldLen = 0;
        sal_Int32 nField = rCacheTF.GetFieldCount(sal_uInt16(nParaIndex)), nFoundFieldIndex = -1;
        EFieldInfo ree;
        sal_Int32  reeBegin=0, reeEnd=0;
        for(sal_uInt16 j = 0; j < nField; j++)
        {
            ree = rCacheTF.GetFieldInfo(sal_uInt16(nParaIndex), j);
            reeBegin  = ree.aPosition.nIndex + nAllFieldLen;
            reeEnd = reeBegin + ree.aCurrentText.getLength();
            nAllFieldLen += (ree.aCurrentText.getLength() - 1);
            if( reeBegin > nIndex )
            {
                break;
            }
            if(  nIndex >= reeBegin && nIndex < reeEnd )
            {
                if(ree.pFieldItem->GetField()->GetClassId() != text::textfield::Type::URL)
                {
                    nFoundFieldIndex = j;
                    break;
                }
            }
        }
        if( nFoundFieldIndex >= 0  )
        {
            if( bForward )
                return reeEnd - 1;
            else
                return reeBegin;
        }
        return nIndex;
    }
    bool AccessibleEditableTextPara::ExtendByField( ::com::sun::star::accessibility::TextSegment& Segment )
    {
        sal_Int32 nParaIndex = GetParagraphIndex();
        SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();
        sal_Int32 nAllFieldLen = 0;
        sal_Int32 nField = rCacheTF.GetFieldCount(sal_uInt16(nParaIndex)), nFoundFieldIndex = -1;
        EFieldInfo ree;
        sal_Int32  reeBegin=0, reeEnd=0;
        for(sal_uInt16 j = 0; j < nField; j++)
        {
            ree = rCacheTF.GetFieldInfo(sal_uInt16(nParaIndex), j);
            reeBegin  = ree.aPosition.nIndex + nAllFieldLen;
            reeEnd = reeBegin + ree.aCurrentText.getLength();
            nAllFieldLen += (ree.aCurrentText.getLength() - 1);
            if( reeBegin > Segment.SegmentEnd )
            {
                break;
            }
            if(  (Segment.SegmentEnd > reeBegin && Segment.SegmentEnd <= reeEnd) ||
                  (Segment.SegmentStart >= reeBegin && Segment.SegmentStart < reeEnd)  )
            {
                if(ree.pFieldItem->GetField()->GetClassId() != text::textfield::Type::URL)
                {
                    nFoundFieldIndex = j;
                    break;
                }
            }
        }
        bool bExtend = false;
        if( nFoundFieldIndex >= 0 )
        {
            if( Segment.SegmentEnd < reeEnd )
            {
                Segment.SegmentEnd  = reeEnd;
                bExtend = true;
            }
            if( Segment.SegmentStart > reeBegin )
            {
                Segment.SegmentStart = reeBegin;
                bExtend = true;
            }
            if( bExtend )
            {
                
                EBulletInfo aBulletInfo = rCacheTF.GetBulletInfo(sal_uInt16(nParaIndex));
                sal_Int32 nBulletLen = aBulletInfo.aText.getLength();
                if (nBulletLen > 0)
                {
                    Segment.SegmentEnd += nBulletLen;
                    if (nFoundFieldIndex > 0)
                        Segment.SegmentStart += nBulletLen;
                    Segment.SegmentText = GetTextRange(Segment.SegmentStart, Segment.SegmentEnd);
                    
                    Segment.SegmentEnd -= nBulletLen;
                    if (nFoundFieldIndex > 0)
                        Segment.SegmentStart -= nBulletLen;
                }
                else
                    Segment.SegmentText = GetTextRange(Segment.SegmentStart, Segment.SegmentEnd);
            }
        }
        return bExtend;
    }

    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleEditableTextPara::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getTextAtIndex: paragraph index value overflow");

        ::com::sun::star::accessibility::TextSegment aResult;
        aResult.SegmentStart = -1;
        aResult.SegmentEnd = -1;

        switch( aTextType )
        {
            case AccessibleTextType::CHARACTER:
            case AccessibleTextType::WORD:
            {
                aResult = OCommonAccessibleText::getTextAtIndex( nIndex, aTextType );
                ExtendByField( aResult );
                break;
            }
            
            
            case AccessibleTextType::ATTRIBUTE_RUN:
            {
                const sal_Int32 nTextLen = GetTextForwarder().GetTextLen( GetParagraphIndex() );

                if( nIndex == nTextLen )
                {
                    
                    aResult.SegmentStart = aResult.SegmentEnd = nTextLen;
                }
                else
                {
                    sal_Int32 nStartIndex, nEndIndex;
                    
                    SvxTextForwarder&   rCacheTF = GetTextForwarder();
                    
                    EBulletInfo aBulletInfo = rCacheTF.GetBulletInfo( static_cast< sal_uInt16 >(GetParagraphIndex()) );
                    if (aBulletInfo.bVisible)
                        nIndex += aBulletInfo.aText.getLength();
                    if (nIndex != 0  && nIndex >= getCharacterCount())
                        nIndex = getCharacterCount()-1;
                    CheckPosition(nIndex);
                    if( GetAttributeRun(nStartIndex, nEndIndex, nIndex) )
                    {
                        aResult.SegmentText = GetTextRange(nStartIndex, nEndIndex);
                        if (aBulletInfo.bVisible)
                        {
                            nStartIndex -= aBulletInfo.aText.getLength();
                            nEndIndex -= aBulletInfo.aText.getLength();
                        }
                        aResult.SegmentStart = nStartIndex;
                        aResult.SegmentEnd = nEndIndex;
                    }
                }
                break;
            }
            case AccessibleTextType::LINE:
            {
                SvxTextForwarder&   rCacheTF = GetTextForwarder();
                sal_Int32           nParaIndex = GetParagraphIndex();
                CheckPosition(nIndex);
        if (nIndex != 0  && nIndex == getCharacterCount())
            --nIndex;
                sal_Int32 nLine, nLineCount=rCacheTF.GetLineCount( nParaIndex );
                sal_Int32 nCurIndex;
                
                
                
                sal_Int32 nBulletLen = 0;
                for( nLine=0, nCurIndex=0; nLine<nLineCount; ++nLine )
                {
                    if (nLine == 0)
                    {
                        EBulletInfo aBulletInfo = rCacheTF.GetBulletInfo( nParaIndex );
                        if (aBulletInfo.bVisible)
                        {
                            
                            nBulletLen = aBulletInfo.aText.getLength();
                        }
                    }
                    sal_Int32 nLineLen = rCacheTF.GetLineLen(nParaIndex, nLine);
                    if (nLine == 0)
                        nCurIndex += nLineLen - nBulletLen;
                    else
                        nCurIndex += nLineLen;
                    if( nCurIndex > nIndex )
                    {
                        if (nLine ==0)
                        {
                            aResult.SegmentStart = 0;
                            aResult.SegmentEnd = nCurIndex;
                            aResult.SegmentText = GetTextRange( aResult.SegmentStart, aResult.SegmentEnd + nBulletLen);
                            break;
                        }
                        else
                        {
                            aResult.SegmentStart = nCurIndex - nLineLen;
                            aResult.SegmentEnd = nCurIndex;
                            
                            aResult.SegmentText = GetTextRange( aResult.SegmentStart + nBulletLen, aResult.SegmentEnd + nBulletLen);
                            break;
                        }
                    }
                }
                break;
            }
            default:
                aResult = OCommonAccessibleText::getTextAtIndex( nIndex, aTextType );
                break;
        } /* end of switch( aTextType ) */

        return aResult;
    }

    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleEditableTextPara::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getTextBeforeIndex: paragraph index value overflow");

        ::com::sun::star::accessibility::TextSegment aResult;
        aResult.SegmentStart = -1;
        aResult.SegmentEnd = -1;
        i18n::Boundary aBoundary;
        switch( aTextType )
        {
            
            
            case AccessibleTextType::ATTRIBUTE_RUN:
            {
                const sal_Int32 nTextLen = GetTextForwarder().GetTextLen( GetParagraphIndex() );
                sal_Int32 nStartIndex, nEndIndex;

                if( nIndex == nTextLen )
                {
                    
                    if( nIndex > 0 &&
                        GetAttributeRun(nStartIndex, nEndIndex, nIndex-1) )
                    {
                        aResult.SegmentText = GetTextRange(nStartIndex, nEndIndex);
                        aResult.SegmentStart = nStartIndex;
                        aResult.SegmentEnd = nEndIndex;
                    }
                }
                else
                {
                    if( GetAttributeRun(nStartIndex, nEndIndex, nIndex) )
                    {
                        
                        
                        if( nStartIndex > 0 &&
                            GetAttributeRun(nStartIndex, nEndIndex, nStartIndex-1) )
                        {
                            aResult.SegmentText = GetTextRange(nStartIndex, nEndIndex);
                            aResult.SegmentStart = nStartIndex;
                            aResult.SegmentEnd = nEndIndex;
                        }
                    }
                }
                break;
            }
            case AccessibleTextType::LINE:
            {
                SvxTextForwarder&   rCacheTF = GetTextForwarder();
                sal_Int32           nParaIndex = GetParagraphIndex();

                CheckPosition(nIndex);

                sal_Int32 nLine, nLineCount=rCacheTF.GetLineCount( nParaIndex );
                
                
                
                sal_Int32 nCurIndex=0, nLastIndex=0, nCurLineLen=0;
                sal_Int32 nLastLineLen = 0, nBulletLen = 0;;
                
                for( nLine=0, nCurIndex=0, nLastIndex=0; nLine<nLineCount; ++nLine )
                {
                    nLastIndex = nCurIndex;
                    if (nLine == 0)
                    {
                        EBulletInfo aBulletInfo = rCacheTF.GetBulletInfo( static_cast< sal_uInt16 >(nParaIndex) );
                        if (aBulletInfo.bVisible)
                        {
                            
                            nBulletLen = aBulletInfo.aText.getLength();
                        }
                    }
                    if (nLine == 1)
                        nLastLineLen = nCurLineLen - nBulletLen;
                    else
                        nLastLineLen = nCurLineLen;
                    nCurLineLen = rCacheTF.GetLineLen( nParaIndex, nLine);
                    
                    if (nLine == 0)
                        nCurIndex += nCurLineLen - nBulletLen;
                    else
                        nCurIndex += nCurLineLen;

                    
                    
                    if (nCurIndex > nIndex)
                    {
                        if (nLine == 0)
                        {
                            break;
                        }
                        else if (nLine == 1)
                        {
                            aResult.SegmentStart = 0;
                            aResult.SegmentEnd = static_cast< sal_uInt16 >( nLastIndex );
                            aResult.SegmentText = GetTextRange( aResult.SegmentStart, aResult.SegmentEnd + nBulletLen);
                            break;
                        }
                        else
                        {
                            
                            aResult.SegmentStart = nLastIndex - nLastLineLen;
                            aResult.SegmentEnd = static_cast< sal_uInt16 >( nLastIndex );
                            aResult.SegmentText = GetTextRange( aResult.SegmentStart + nBulletLen, aResult.SegmentEnd + nBulletLen);
                            break;
                        }
                    }
                }

                break;
            }
            case AccessibleTextType::WORD:
            {
                nIndex = SkipField( nIndex, false);
                OUString sText( implGetText() );
                sal_Int32 nLength = sText.getLength();

                
                implGetWordBoundary( aBoundary, nIndex );


                
                
                sal_Int32 curWordStart , preWordStart;
                if( aBoundary.startPos == -1 || aBoundary.startPos > nIndex)
                    curWordStart = preWordStart = nIndex;
                else
                    curWordStart = preWordStart = aBoundary.startPos;

                

                sal_Bool bWord = sal_False;

                
                while ( (preWordStart >= 0 && !bWord ) || ( aBoundary.endPos > curWordStart ) )
                    {
                    preWordStart--;
                    bWord = implGetWordBoundary( aBoundary, preWordStart );
                }
                if ( bWord && implIsValidBoundary( aBoundary, nLength ) )
                {
                    aResult.SegmentText = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
                    aResult.SegmentStart = aBoundary.startPos;
                    aResult.SegmentEnd = aBoundary.endPos;
                    ExtendByField( aResult );
                }
            }
            break;
            case AccessibleTextType::CHARACTER:
            {
                nIndex = SkipField( nIndex, false);
                aResult = OCommonAccessibleText::getTextBeforeIndex( nIndex, aTextType );
                ExtendByField( aResult );
                break;
            }
            default:
                aResult = OCommonAccessibleText::getTextBeforeIndex( nIndex, aTextType );
                break;
        } /* end of switch( aTextType ) */

        return aResult;
    }

    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleEditableTextPara::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getTextBehindIndex: paragraph index value overflow");

        ::com::sun::star::accessibility::TextSegment aResult;
        aResult.SegmentStart = -1;
        aResult.SegmentEnd = -1;
        i18n::Boundary aBoundary;
        switch( aTextType )
        {
            case AccessibleTextType::ATTRIBUTE_RUN:
            {
                sal_Int32 nStartIndex, nEndIndex;

                if( GetAttributeRun(nStartIndex, nEndIndex, nIndex) )
                {
                    
                    if( nEndIndex < GetTextLen() )
                    {
                        if( GetAttributeRun(nStartIndex, nEndIndex, nEndIndex) )
                        {
                            aResult.SegmentText = GetTextRange(nStartIndex, nEndIndex);
                            aResult.SegmentStart = nStartIndex;
                            aResult.SegmentEnd = nEndIndex;
                        }
                    }
                }
                break;
            }

            case AccessibleTextType::LINE:
            {
                SvxTextForwarder&   rCacheTF = GetTextForwarder();
                sal_Int32           nParaIndex = GetParagraphIndex();
                

                CheckPosition(nIndex);

                sal_Int32 nLine, nLineCount = rCacheTF.GetLineCount( nParaIndex );
                sal_Int32 nCurIndex;
                
                
                
                sal_Int32 nBulletLen = 0;
                
                for( nLine=0, nCurIndex=0; nLine<nLineCount; ++nLine )
                {
                    if (nLine == 0)
                    {
                        EBulletInfo aBulletInfo = rCacheTF.GetBulletInfo( static_cast< sal_uInt16 >(nParaIndex) );
                        if (aBulletInfo.bVisible)
                        {
                            
                            nBulletLen = aBulletInfo.aText.getLength();
                        }
                    }
                    
                    sal_Int32 nLineLen = rCacheTF.GetLineLen( nParaIndex, nLine);

                    if (nLine == 0)
                        nCurIndex += nLineLen - nBulletLen;
                    else
                        nCurIndex += nLineLen;

                    if( nCurIndex > nIndex &&
                        nLine < nLineCount-1 )
                    {
                        aResult.SegmentStart = nCurIndex;
                        aResult.SegmentEnd = nCurIndex + rCacheTF.GetLineLen( nParaIndex, nLine+1);
                        aResult.SegmentText = GetTextRange( aResult.SegmentStart + nBulletLen, aResult.SegmentEnd + nBulletLen);
                        break;
                    }
                }

                break;
            }
            case AccessibleTextType::WORD:
            {
                nIndex = SkipField( nIndex, true);
                OUString sText( implGetText() );
                sal_Int32 nLength = sText.getLength();

                
                sal_Bool bWord = implGetWordBoundary( aBoundary, nIndex );

                
                sal_Int32 nextWord = nIndex;
                
                if( nIndex <= aBoundary.endPos )
                {
                    nextWord =  aBoundary.endPos;
                    if( sText.getStr()[nextWord] == sal_Unicode(' ') ) nextWord++;
                    bWord = implGetWordBoundary( aBoundary, nextWord );
                }

                if ( bWord && implIsValidBoundary( aBoundary, nLength ) )
                {
                    aResult.SegmentText = sText.copy( aBoundary.startPos, aBoundary.endPos - aBoundary.startPos );
                    aResult.SegmentStart = aBoundary.startPos;
                    aResult.SegmentEnd = aBoundary.endPos;

                    

                    ExtendByField( aResult );
                }
            }
            break;

            case AccessibleTextType::CHARACTER:
            {
                nIndex = SkipField( nIndex, true);
                aResult = OCommonAccessibleText::getTextBehindIndex( nIndex, aTextType );
                ExtendByField( aResult );
                break;
            }
            default:
                aResult = OCommonAccessibleText::getTextBehindIndex( nIndex, aTextType );
                break;
        } /* end of switch( aTextType ) */

        return aResult;
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        try
        {
            SvxEditViewForwarder& rCacheVF = GetEditViewForwarder( true );
            #if OSL_DEBUG_LEVEL > 0
            SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    
            (void)rCacheTF;
            #else
            GetTextForwarder();                                         
            #endif

            sal_Bool aRetVal;

            DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                       "AccessibleEditableTextPara::copyText: index value overflow");

            CheckRange(nStartIndex, nEndIndex);

            
            sal_Int32 nBulletLen = 0;
            EBulletInfo aBulletInfo = GetTextForwarder().GetBulletInfo( static_cast< sal_uInt16 >(GetParagraphIndex()) );
            if( aBulletInfo.nParagraph != EE_PARA_NOT_FOUND && aBulletInfo.bVisible )
                        nBulletLen = aBulletInfo.aText.getLength();
            
            ESelection aOldSelection;

            rCacheVF.GetSelection( aOldSelection );
            
            rCacheVF.SetSelection( MakeSelection(nStartIndex + nBulletLen, nEndIndex + nBulletLen) );
            aRetVal = rCacheVF.Copy();
            rCacheVF.SetSelection( aOldSelection ); 

            return aRetVal;
        }
        catch (const uno::RuntimeException&)
        {
            return sal_False;
        }
    }

    
    sal_Bool SAL_CALL AccessibleEditableTextPara::cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {

        SolarMutexGuard aGuard;

        try
        {
            SvxEditViewForwarder& rCacheVF = GetEditViewForwarder( true );
            SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    

            DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                       "AccessibleEditableTextPara::cutText: index value overflow");

            CheckRange(nStartIndex, nEndIndex);

            
            sal_Int32 nBulletLen = 0;
            EBulletInfo aBulletInfo = GetTextForwarder().GetBulletInfo( static_cast< sal_uInt16 >(GetParagraphIndex()) );
            if( aBulletInfo.nParagraph != EE_PARA_NOT_FOUND && aBulletInfo.bVisible )
                        nBulletLen = aBulletInfo.aText.getLength();
            ESelection aSelection = MakeSelection (nStartIndex + nBulletLen, nEndIndex + nBulletLen);
            
            if( !rCacheTF.IsEditable( aSelection ) )
                return sal_False; 

            
            
            rCacheVF.SetSelection( aSelection );

            return rCacheVF.Cut();
        }
        catch (const uno::RuntimeException&)
        {
            return sal_False;
        }
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::pasteText( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {

        SolarMutexGuard aGuard;

        try
        {
            SvxEditViewForwarder& rCacheVF = GetEditViewForwarder( true );
            SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    

            DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                       "AccessibleEditableTextPara::pasteText: index value overflow");

            CheckPosition(nIndex);

            
            sal_Int32 nBulletLen = 0;
            EBulletInfo aBulletInfo = GetTextForwarder().GetBulletInfo( static_cast< sal_uInt16 >(GetParagraphIndex()) );
            if( aBulletInfo.nParagraph != EE_PARA_NOT_FOUND && aBulletInfo.bVisible )
                        nBulletLen = aBulletInfo.aText.getLength();
            if( !rCacheTF.IsEditable( MakeSelection(nIndex + nBulletLen) ) )
                return sal_False; 

            
            
            rCacheVF.SetSelection( MakeCursor(nIndex + nBulletLen) );

            return rCacheVF.Paste();
        }
        catch (const uno::RuntimeException&)
        {
            return sal_False;
        }
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {

        SolarMutexGuard aGuard;

        try
        {
            
            
            GetEditViewForwarder( true );
            SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    

            DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                       "AccessibleEditableTextPara::deleteText: index value overflow");

            CheckRange(nStartIndex, nEndIndex);

            
            sal_Int32 nBulletLen = 0;
            EBulletInfo aBulletInfo = GetTextForwarder().GetBulletInfo( static_cast< sal_uInt16 >(GetParagraphIndex()) );
            if( aBulletInfo.nParagraph != EE_PARA_NOT_FOUND && aBulletInfo.bVisible )
                nBulletLen = aBulletInfo.aText.getLength();
            ESelection aSelection = MakeSelection (nStartIndex + nBulletLen, nEndIndex + nBulletLen);

            
            if( !rCacheTF.IsEditable( aSelection ) )
                return sal_False; 

            
            sal_Bool bRet = rCacheTF.Delete( aSelection );

            GetEditSource().UpdateData();

            return bRet;
        }
        catch (const uno::RuntimeException&)
        {
            return sal_False;
        }
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::insertText( const OUString& sText, sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {

        SolarMutexGuard aGuard;

        try
        {
            
            
            GetEditViewForwarder( true );
            SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    

            DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                       "AccessibleEditableTextPara::insertText: index value overflow");

            CheckPosition(nIndex);

            
            sal_Int32 nBulletLen = 0;
            EBulletInfo aBulletInfo = GetTextForwarder().GetBulletInfo( static_cast< sal_uInt16 >(GetParagraphIndex()) );
            if( aBulletInfo.nParagraph != EE_PARA_NOT_FOUND && aBulletInfo.bVisible )
                        nBulletLen = aBulletInfo.aText.getLength();

            if( !rCacheTF.IsEditable( MakeSelection(nIndex + nBulletLen) ) )
                return sal_False; 

            
            sal_Bool bRet = rCacheTF.InsertText( sText, MakeCursor(nIndex + nBulletLen) );

            rCacheTF.QuickFormatDoc();
            GetEditSource().UpdateData();

            return bRet;
        }
        catch (const uno::RuntimeException&)
        {
            return sal_False;
        }
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::replaceText( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const OUString& sReplacement ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {

        SolarMutexGuard aGuard;

        try
        {
            
            
            GetEditViewForwarder( true );
            SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    

            DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                       "AccessibleEditableTextPara::replaceText: index value overflow");

            CheckRange(nStartIndex, nEndIndex);

            
            sal_Int32 nBulletLen = 0;
            EBulletInfo aBulletInfo = GetTextForwarder().GetBulletInfo( static_cast< sal_uInt16 >(GetParagraphIndex()) );
            if( aBulletInfo.nParagraph != EE_PARA_NOT_FOUND && aBulletInfo.bVisible )
                        nBulletLen = aBulletInfo.aText.getLength();
            ESelection aSelection = MakeSelection (nStartIndex + nBulletLen, nEndIndex + nBulletLen);

            
            if( !rCacheTF.IsEditable( aSelection ) )
                return sal_False; 

            
            
            sal_Bool bRet = rCacheTF.InsertText( sReplacement, aSelection );

            rCacheTF.QuickFormatDoc();
            GetEditSource().UpdateData();

            return bRet;
        }
        catch (const uno::RuntimeException&)
        {
            return sal_False;
        }
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::setAttributes( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const uno::Sequence< beans::PropertyValue >& aAttributeSet ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {

        SolarMutexGuard aGuard;

        try
        {
            
            
            GetEditViewForwarder( true );
            SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    
            sal_uInt16 nPara = static_cast< sal_uInt16 >( GetParagraphIndex() );

            DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                       "AccessibleEditableTextPara::setAttributes: index value overflow");

            CheckRange(nStartIndex, nEndIndex);

            if( !rCacheTF.IsEditable( MakeSelection(nStartIndex, nEndIndex) ) )
                return sal_False; 

            
            
            SvxAccessibleTextPropertySet aPropSet( &GetEditSource(),
                                                   0 == nStartIndex &&
                                                   rCacheTF.GetTextLen(nPara) == nEndIndex ?
                                                   ImplGetSvxUnoOutlinerTextCursorSvxPropertySet() :
                                                   ImplGetSvxTextPortionSvxPropertySet() );

            aPropSet.SetSelection( MakeSelection(nStartIndex, nEndIndex) );

            
            sal_Int32 i, nLength( aAttributeSet.getLength() );
            const beans::PropertyValue* pPropArray = aAttributeSet.getConstArray();
            for(i=0; i<nLength; ++i)
            {
                try
                {
                    aPropSet.setPropertyValue(pPropArray->Name, pPropArray->Value);
                }
                catch (const uno::Exception&)
                {
                    OSL_FAIL("AccessibleEditableTextPara::setAttributes exception in setPropertyValue");
                }

                ++pPropArray;
            }

            rCacheTF.QuickFormatDoc();
            GetEditSource().UpdateData();

            return sal_True;
        }
        catch (const uno::RuntimeException&)
        {
            return sal_False;
        }
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::setText( const OUString& sText ) throw (uno::RuntimeException)
    {

        SolarMutexGuard aGuard;

        return replaceText(0, getCharacterCount(), sText);
    }

    
    uno::Sequence< beans::PropertyValue > SAL_CALL AccessibleEditableTextPara::getDefaultAttributes(
            const uno::Sequence< OUString >& rRequestedAttributes )
        throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        #if OSL_DEBUG_LEVEL > 0
        SvxAccessibleTextAdapter& rCacheTF =
        #endif
            GetTextForwarder();

        #if OSL_DEBUG_LEVEL > 0
        (void)rCacheTF;
        #endif

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getCharacterAttributes: index value overflow");

        
        
        SvxAccessibleTextPropertySet aPropSet( &GetEditSource(),
                ImplGetSvxCharAndParaPropertiesSet() );
        aPropSet.SetSelection( MakeSelection( 0, GetTextLen() ) );
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo = aPropSet.getPropertySetInfo();
        if (!xPropSetInfo.is())
            throw uno::RuntimeException("Cannot query XPropertySetInfo",
                        uno::Reference< uno::XInterface >
                        ( static_cast< XAccessible* > (this) ) );   

        
        sal_Int32 nLenReqAttr = rRequestedAttributes.getLength();
        uno::Sequence< beans::Property > aProperties;
        if (nLenReqAttr)
        {
            const OUString *pRequestedAttributes = rRequestedAttributes.getConstArray();

            aProperties.realloc( nLenReqAttr );
            beans::Property *pProperties = aProperties.getArray();
            sal_Int32 nCurLen = 0;
            for (sal_Int32 i = 0;  i < nLenReqAttr;  ++i)
            {
                beans::Property aProp;
                try
                {
                    aProp = xPropSetInfo->getPropertyByName( pRequestedAttributes[i] );
                }
                catch (const beans::UnknownPropertyException&)
                {
                    continue;
                }
                pProperties[ nCurLen++ ] = aProp;
            }
            aProperties.realloc( nCurLen );
        }
        else
            aProperties = xPropSetInfo->getProperties();

        sal_Int32 nLength = aProperties.getLength();
        const beans::Property *pProperties = aProperties.getConstArray();

        
        uno::Sequence< beans::PropertyValue > aOutSequence( nLength );
        beans::PropertyValue* pOutSequence = aOutSequence.getArray();
        sal_Int32 nOutLen = 0;
        for (sal_Int32 i = 0;  i < nLength;  ++i)
        {
            
            
            
            PropertyState eState = aPropSet._getPropertyState( pProperties->Name, mnParagraphIndex );
            if ( eState == PropertyState_AMBIGUOUS_VALUE )
            {
                OSL_FAIL( "ambiguous property value encountered" );
            }

            
            
            
            
            {
                pOutSequence->Name      = pProperties->Name;
                pOutSequence->Handle    = pProperties->Handle;
                pOutSequence->Value     = aPropSet._getPropertyValue( pProperties->Name, mnParagraphIndex );
                pOutSequence->State     = PropertyState_DEFAULT_VALUE;

                ++pOutSequence;
                ++nOutLen;
            }
            ++pProperties;
        }
        aOutSequence.realloc( nOutLen );

        return aOutSequence;
    }


    uno::Sequence< beans::PropertyValue > SAL_CALL AccessibleEditableTextPara::getRunAttributes(
            sal_Int32 nIndex,
            const uno::Sequence< OUString >& rRequestedAttributes )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {

        SolarMutexGuard aGuard;

        #if OSL_DEBUG_LEVEL > 0
        SvxAccessibleTextAdapter& rCacheTF =
        #endif
            GetTextForwarder();

        #if OSL_DEBUG_LEVEL > 0
        (void)rCacheTF;
        #endif

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getCharacterAttributes: index value overflow");

        if( getCharacterCount() > 0 )
            CheckIndex(nIndex);
        else
            CheckPosition(nIndex);

        SvxAccessibleTextPropertySet aPropSet( &GetEditSource(),
                                               ImplGetSvxCharAndParaPropertiesSet() );
        aPropSet.SetSelection( MakeSelection( nIndex ) );
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo = aPropSet.getPropertySetInfo();
        if (!xPropSetInfo.is())
            throw uno::RuntimeException("Cannot query XPropertySetInfo",
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< XAccessible* > (this) ) );   

        
        sal_Int32 nLenReqAttr = rRequestedAttributes.getLength();
        uno::Sequence< beans::Property > aProperties;
        if (nLenReqAttr)
        {
            const OUString *pRequestedAttributes = rRequestedAttributes.getConstArray();

            aProperties.realloc( nLenReqAttr );
            beans::Property *pProperties = aProperties.getArray();
            sal_Int32 nCurLen = 0;
            for (sal_Int32 i = 0;  i < nLenReqAttr;  ++i)
            {
                beans::Property aProp;
                try
                {
                    aProp = xPropSetInfo->getPropertyByName( pRequestedAttributes[i] );
                }
                catch (const beans::UnknownPropertyException&)
                {
                    continue;
                }
                pProperties[ nCurLen++ ] = aProp;
            }
            aProperties.realloc( nCurLen );
        }
        else
            aProperties = xPropSetInfo->getProperties();

        sal_Int32 nLength = aProperties.getLength();
        const beans::Property *pProperties = aProperties.getConstArray();

        
        uno::Sequence< beans::PropertyValue > aOutSequence( nLength );
        beans::PropertyValue* pOutSequence = aOutSequence.getArray();
        sal_Int32 nOutLen = 0;
        for (sal_Int32 i = 0;  i < nLength;  ++i)
        {
            
            PropertyState eState = aPropSet.getPropertyState( pProperties->Name );
            if (eState == PropertyState_DIRECT_VALUE)
            {
                pOutSequence->Name      = pProperties->Name;
                pOutSequence->Handle    = pProperties->Handle;
                pOutSequence->Value     = aPropSet.getPropertyValue( pProperties->Name );
                pOutSequence->State     = eState;

                ++pOutSequence;
                ++nOutLen;
            }
            ++pProperties;
        }
        aOutSequence.realloc( nOutLen );

        return aOutSequence;
    }

    
    ::sal_Int32 SAL_CALL AccessibleEditableTextPara::getHyperLinkCount(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        SvxAccessibleTextAdapter& rT = GetTextForwarder();
        const sal_Int32 nPara = GetParagraphIndex();

        sal_uInt16 nHyperLinks = 0;
        sal_uInt16 nFields = rT.GetFieldCount( nPara );
        for ( sal_uInt16 n = 0; n < nFields; n++ )
        {
            EFieldInfo aField = rT.GetFieldInfo( nPara, n );
            if ( aField.pFieldItem->GetField()->ISA( SvxURLField ) )
                nHyperLinks++;
        }
        return nHyperLinks;
    }

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleHyperlink > SAL_CALL AccessibleEditableTextPara::getHyperLink( ::sal_Int32 nLinkIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleHyperlink > xRef;

        SvxAccessibleTextAdapter& rT = GetTextForwarder();
        const sal_Int32 nPara = GetParagraphIndex();

        sal_uInt16 nHyperLink = 0;
        sal_uInt16 nFields = rT.GetFieldCount( nPara );
        for ( sal_uInt16 n = 0; n < nFields; n++ )
        {
            EFieldInfo aField = rT.GetFieldInfo( nPara, n );
            if ( aField.pFieldItem->GetField()->ISA( SvxURLField ) )
            {
                if ( nHyperLink == nLinkIndex )
                {
                    sal_uInt16 nEEStart = aField.aPosition.nIndex;

                    
                    sal_uInt16 nStart = rT.CalcEditEngineIndex( nPara, nEEStart );
                    sal_uInt16 nEnd = nStart + aField.aCurrentText.getLength();
                    xRef = new AccessibleHyperlink( rT, new SvxFieldItem( *aField.pFieldItem ), nPara, nEEStart, nStart, nEnd, aField.aCurrentText );
                    break;
                }
                nHyperLink++;
            }
        }

        return xRef;
    }

    ::sal_Int32 SAL_CALL AccessibleEditableTextPara::getHyperLinkIndex( ::sal_Int32 nCharIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        const sal_Int32 nPara = GetParagraphIndex();
        SvxAccessibleTextAdapter& rT = GetTextForwarder();





        const sal_uInt16 nEEIndex = rT.CalcEditEngineIndex( nPara, nCharIndex );
        sal_Int32 nHLIndex = -1; 
        sal_uInt16 nHyperLink = 0;
        sal_uInt16 nFields = rT.GetFieldCount( nPara );
        for ( sal_uInt16 n = 0; n < nFields; n++ )
        {
            EFieldInfo aField = rT.GetFieldInfo( nPara, n );
            if ( aField.pFieldItem->GetField()->ISA( SvxURLField ) )
            {
                if ( aField.aPosition.nIndex == nEEIndex )
                {
                    nHLIndex = nHyperLink;
                    break;
                }
                nHyperLink++;
            }
        }

        return nHLIndex;
    }

    
    sal_Int32 SAL_CALL AccessibleEditableTextPara::getLineNumberAtIndex( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {

        sal_Int32 nRes = -1;
        sal_Int32 nPara = GetParagraphIndex();

        SvxTextForwarder &rCacheTF = GetTextForwarder();
        const bool bValidPara = 0 <= nPara && nPara < rCacheTF.GetParagraphCount();
        DBG_ASSERT( bValidPara, "getLineNumberAtIndex: current paragraph index out of range" );
        if (bValidPara)
        {
            
            if (0 <= nIndex && nIndex <= rCacheTF.GetTextLen( nPara ))
                nRes = rCacheTF.GetLineNumberAtIndex( nPara, nIndex );
            else
                throw lang::IndexOutOfBoundsException();
        }
        return nRes;
    }

    
    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleEditableTextPara::getTextAtLineNumber( sal_Int32 nLineNo ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {

        ::com::sun::star::accessibility::TextSegment aResult;
        sal_Int32 nPara = GetParagraphIndex();
        SvxTextForwarder &rCacheTF = GetTextForwarder();
        const bool bValidPara = 0 <= nPara && nPara < rCacheTF.GetParagraphCount();
        DBG_ASSERT( bValidPara, "getTextAtLineNumber: current paragraph index out of range" );
        if (bValidPara)
        {
            if (0 <= nLineNo && nLineNo < rCacheTF.GetLineCount( nPara ))
            {
                sal_Int32 nStart = 0, nEnd = 0;
                rCacheTF.GetLineBoundaries( nStart, nEnd, nPara, nLineNo );
                if (nStart >= 0 && nEnd >=  0)
                {
                    try
                    {
                        aResult.SegmentText     = getTextRange( nStart, nEnd );
                        aResult.SegmentStart    = nStart;
                        aResult.SegmentEnd      = nEnd;
                    }
                    catch (const lang::IndexOutOfBoundsException&)
                    {
                        
                        DBG_ASSERT( false, "unexpected exception" );
                    }
                }
            }
            else
                throw lang::IndexOutOfBoundsException();
        }
        return aResult;
    }

    
    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleEditableTextPara::getTextAtLineWithCaret(  ) throw (uno::RuntimeException)
    {

        ::com::sun::star::accessibility::TextSegment aResult;
        try
        {
            aResult = getTextAtLineNumber( getNumberOfLineWithCaret() );
        }
        catch (const lang::IndexOutOfBoundsException&)
        {
            
        }
        return aResult;
    }

    
    sal_Int32 SAL_CALL AccessibleEditableTextPara::getNumberOfLineWithCaret(  ) throw (uno::RuntimeException)
    {

        sal_Int32 nRes = -1;
        try
        {
            nRes = getLineNumberAtIndex( getCaretPosition() );
        }
        catch (const lang::IndexOutOfBoundsException&)
        {
            
        }
        return nRes;
    }


    
    OUString SAL_CALL AccessibleEditableTextPara::getImplementationName (void) throw (uno::RuntimeException)
    {

        return OUString("AccessibleEditableTextPara");
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::supportsService (const OUString& sServiceName) throw (uno::RuntimeException)
    {

        return cppu::supportsService(this, sServiceName);
    }

    uno::Sequence< OUString> SAL_CALL AccessibleEditableTextPara::getSupportedServiceNames (void) throw (uno::RuntimeException)
    {

        const OUString sServiceName( getServiceName() );
        return uno::Sequence< OUString > (&sServiceName, 1);
    }

    
    OUString SAL_CALL AccessibleEditableTextPara::getServiceName (void) throw (uno::RuntimeException)
    {

        
        return OUString("com.sun.star.text.AccessibleParagraphView");
    }

}  



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
