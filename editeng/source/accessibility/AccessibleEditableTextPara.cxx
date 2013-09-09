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


//------------------------------------------------------------------------
//
// Global header
//
//------------------------------------------------------------------------

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
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <vcl/unohelp.hxx>
#include <editeng/editeng.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/unoipset.hxx>
#include <editeng/outliner.hxx>

//------------------------------------------------------------------------
//
// Project-local header
//
//------------------------------------------------------------------------

#include <com/sun/star/beans/PropertyState.hpp>

//!!!#include <svx/unoshape.hxx>
//!!!#include <svx/dialmgr.hxx>
//!!!#include "accessibility.hrc"

#include <editeng/unolingu.hxx>
#include <editeng/unopracc.hxx>
#include "editeng/AccessibleEditableTextPara.hxx"
#include "AccessibleHyperlink.hxx"

#include <svtools/colorcfg.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;


//------------------------------------------------------------------------
//
// AccessibleEditableTextPara implementation
//
//------------------------------------------------------------------------

namespace accessibility
{

    const SvxItemPropertySet* ImplGetSvxCharAndParaPropertiesSet()
    {
        // PropertyMap for character and paragraph properties
        static const SfxItemPropertyMapEntry aPropMap[] =
        {
            SVX_UNOEDIT_CHAR_PROPERTIES,
            SVX_UNOEDIT_PARA_PROPERTIES,
            SVX_UNOEDIT_NUMBERING_PROPERTIE,
            {MAP_CHAR_LEN("TextUserDefinedAttributes"),     EE_CHAR_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
            {MAP_CHAR_LEN("ParaUserDefinedAttributes"),     EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
            {0,0,0,0,0,0}
        };
        static SvxItemPropertySet aPropSet( aPropMap, EditEngine::GetGlobalItemPool() );
        return &aPropSet;
    }


    DBG_NAME( AccessibleEditableTextPara )

    // #i27138# - add parameter <_pParaManager>
    AccessibleEditableTextPara::AccessibleEditableTextPara(
                                const uno::Reference< XAccessible >& rParent,
                                const AccessibleParaManager* _pParaManager )
        : AccessibleTextParaInterfaceBase( m_aMutex ),
          mnParagraphIndex( 0 ),
          mnIndexInParent( 0 ),
          mpEditSource( NULL ),
          maEEOffset( 0, 0 ),
          mxParent( rParent ),
          // well, that's strictly (UNO) exception safe, though not
          // really robust. We rely on the fact that this member is
          // constructed last, and that the constructor body catches
          // exceptions, thus no chance for exceptions once the Id is
          // fetched. Nevertheless, normally should employ RAII here...
          mnNotifierClientId(::comphelper::AccessibleEventNotifier::registerClient()),
          // #i27138#
          mpParaManager( _pParaManager )
    {
#ifdef DBG_UTIL
        DBG_CTOR( AccessibleEditableTextPara, NULL );
        OSL_TRACE( "AccessibleEditableTextPara received ID: %d", mnNotifierClientId );
#endif

        try
        {
            // Create the state set.
            ::utl::AccessibleStateSetHelper* pStateSet  = new ::utl::AccessibleStateSetHelper ();
            mxStateSet = pStateSet;

            // these are always on
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
        DBG_DTOR( AccessibleEditableTextPara, NULL );

        // sign off from event notifier
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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return GetTextRange( 0, GetTextLen() );
    }

    ::com::sun::star::lang::Locale AccessibleEditableTextPara::implGetLocale()
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= SAL_MAX_INT32,
                   "AccessibleEditableTextPara::getLocale: paragraph index value overflow");

        // return locale of first character in the paragraph
        return LanguageTag(GetTextForwarder().GetLanguage( GetParagraphIndex(), 0 )).getLocale();
    }

    void AccessibleEditableTextPara::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        sal_uInt16 nStart, nEnd;

        if( GetSelection( nStart, nEnd ) )
        {
            nStartIndex = nStart;
            nEndIndex = nEnd;
        }
        else
        {
            // #102234# No exception, just set to 'invalid'
            nStartIndex = -1;
            nEndIndex = -1;
        }
    }

    void AccessibleEditableTextPara::implGetParagraphBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 /*nIndex*/ )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );
        DBG_WARNING( "AccessibleEditableTextPara::implGetParagraphBoundary: only a base implementation, ignoring the index" );

        rBoundary.startPos = 0;
        rBoundary.endPos = GetTextLen();
    }

    void AccessibleEditableTextPara::implGetLineBoundary( ::com::sun::star::i18n::Boundary& rBoundary, sal_Int32 nIndex )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SvxTextForwarder&   rCacheTF = GetTextForwarder();
        const sal_Int32     nParaIndex = GetParagraphIndex();

        DBG_ASSERT(nParaIndex >= 0 && nParaIndex <= SAL_MAX_INT32,
                   "AccessibleEditableTextPara::implGetLineBoundary: paragraph index value overflow");

        const sal_Int32 nTextLen = rCacheTF.GetTextLen( nParaIndex );

        CheckPosition(nIndex);

        rBoundary.startPos = rBoundary.endPos = -1;

        const sal_uInt16 nLineCount=rCacheTF.GetLineCount( nParaIndex );

        if( nIndex == nTextLen )
        {
            // #i17014# Special-casing one-behind-the-end character
            if( nLineCount <= 1 )
                rBoundary.startPos = 0;
            else
                rBoundary.startPos = nTextLen - rCacheTF.GetLineLen( nParaIndex,
                                                                     nLineCount-1 );

            rBoundary.endPos = nTextLen;
        }
        else
        {
            // normal line search
            sal_uInt16 nLine;
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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return mnNotifierClientId;
    }

    void AccessibleEditableTextPara::SetIndexInParent( sal_Int32 nIndex )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        mnIndexInParent = nIndex;
    }

    sal_Int32 AccessibleEditableTextPara::GetIndexInParent() const
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return mnIndexInParent;
    }

    void AccessibleEditableTextPara::SetParagraphIndex( sal_Int32 nIndex )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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
                catch (const uno::Exception&) // optional behaviour
                {
                }
                // index and therefore description changed
                FireEvent( AccessibleEventId::DESCRIPTION_CHANGED, uno::makeAny( getAccessibleDescription() ), aOldDesc );
                FireEvent( AccessibleEventId::NAME_CHANGED, uno::makeAny( getAccessibleName() ), aOldName );
            }
        }
        catch (const uno::Exception&) // optional behaviour
        {
        }
    }

    sal_Int32 AccessibleEditableTextPara::GetParagraphIndex() const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return mnParagraphIndex;
    }

    void AccessibleEditableTextPara::Dispose()
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        int nClientId( getNotifierClientId() );

        // #108212# drop all references before notifying dispose
        mxParent = NULL;
        mnNotifierClientId = -1;
        mpEditSource = NULL;

        // notify listeners
        if( nClientId != -1 )
        {
            try
            {
                uno::Reference < XAccessibleContext > xThis = getAccessibleContext();

                // #106234# Delegate to EventNotifier
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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        mpEditSource = pEditSource;

        WeakBullet::HardRefType aChild( maImageBullet.get() );
        if( aChild.is() )
            aChild->SetEditSource(pEditSource);

        if( !mpEditSource )
        {
            // going defunc
            UnSetState( AccessibleStateType::SHOWING );
            UnSetState( AccessibleStateType::VISIBLE );
            SetState( AccessibleStateType::INVALID );
            SetState( AccessibleStateType::DEFUNC );

            Dispose();
        }

        // #108900# Init last text content
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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        // check overflow
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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return MakeSelection( nEEIndex, nEEIndex+1 );
    }

    ESelection AccessibleEditableTextPara::MakeCursor( sal_Int32 nEEIndex )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return MakeSelection( nEEIndex, nEEIndex );
    }

    void AccessibleEditableTextPara::CheckIndex( sal_Int32 nIndex ) SAL_THROW((lang::IndexOutOfBoundsException, uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        if( nIndex < 0 || nIndex >= getCharacterCount() )
            throw lang::IndexOutOfBoundsException("AccessibleEditableTextPara: character index out of bounds",
                                                  uno::Reference< uno::XInterface >
                                                  ( static_cast< ::cppu::OWeakObject* > (this) ) ); // disambiguate hierarchy
    }

    void AccessibleEditableTextPara::CheckPosition( sal_Int32 nIndex ) SAL_THROW((lang::IndexOutOfBoundsException, uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        if( nIndex < 0 || nIndex > getCharacterCount() )
            throw lang::IndexOutOfBoundsException("AccessibleEditableTextPara: character position out of bounds",
                                                  uno::Reference< uno::XInterface >
                                                  ( static_cast< ::cppu::OWeakObject* > (this) ) ); // disambiguate hierarchy
    }

    void AccessibleEditableTextPara::CheckRange( sal_Int32 nStart, sal_Int32 nEnd ) SAL_THROW((lang::IndexOutOfBoundsException, uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        CheckPosition( nStart );
        CheckPosition( nEnd );
    }

    sal_Bool AccessibleEditableTextPara::GetSelection( sal_uInt16& nStartPos, sal_uInt16& nEndPos ) SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ESelection aSelection;
        sal_Int32 nPara = GetParagraphIndex();
        if( !GetEditViewForwarder().GetSelection( aSelection ) )
            return sal_False;

        if( aSelection.nStartPara < aSelection.nEndPara )
        {
            if( aSelection.nStartPara > nPara ||
                aSelection.nEndPara < nPara )
                return sal_False;

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
                return sal_False;

            if( nPara == aSelection.nStartPara )
                nStartPos = aSelection.nStartPos;
            else
                nStartPos = GetTextLen();

            if( nPara == aSelection.nEndPara )
                nEndPos = aSelection.nEndPos;
            else
                nEndPos = 0;
        }

        return sal_True;
    }

    OUString AccessibleEditableTextPara::GetTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return GetTextForwarder().GetText( MakeSelection(nStartIndex, nEndIndex) );
    }

    sal_uInt16 AccessibleEditableTextPara::GetTextLen() const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return GetTextForwarder().GetTextLen( static_cast< sal_uInt16 >( GetParagraphIndex() ) );
    }

    SvxEditSourceAdapter& AccessibleEditableTextPara::GetEditSource() const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        if( mpEditSource )
            return *mpEditSource;
        else
            throw uno::RuntimeException("No edit source, object is defunct",
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleEditableTextPara* > (this) ) ) ); // disambiguate hierarchy
    }

    SvxAccessibleTextAdapter& AccessibleEditableTextPara::GetTextForwarder() const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SvxEditSourceAdapter& rEditSource = GetEditSource();
        SvxAccessibleTextAdapter* pTextForwarder = rEditSource.GetTextForwarderAdapter();

        if( !pTextForwarder )
            throw uno::RuntimeException("Unable to fetch text forwarder, object is defunct",
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleEditableTextPara* > (this) ) ) ); // disambiguate hierarchy

        if( pTextForwarder->IsValid() )
            return *pTextForwarder;
        else
            throw uno::RuntimeException("Text forwarder is invalid, object is defunct",
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleEditableTextPara* > (this) ) ) ); // disambiguate hierarchy
    }

    SvxViewForwarder& AccessibleEditableTextPara::GetViewForwarder() const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SvxEditSource& rEditSource = GetEditSource();
        SvxViewForwarder* pViewForwarder = rEditSource.GetViewForwarder();

        if( !pViewForwarder )
        {
            throw uno::RuntimeException("Unable to fetch view forwarder, object is defunct",
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleEditableTextPara* > (this) ) ) ); // disambiguate hierarchy
        }

        if( pViewForwarder->IsValid() )
            return *pViewForwarder;
        else
            throw uno::RuntimeException("View forwarder is invalid, object is defunct",
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< ::cppu::OWeakObject* >
                                          ( const_cast< AccessibleEditableTextPara* > (this) )  ) );    // disambiguate hierarchy
    }

    SvxAccessibleTextEditViewAdapter& AccessibleEditableTextPara::GetEditViewForwarder( sal_Bool bCreate ) const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SvxEditSourceAdapter& rEditSource = GetEditSource();
        SvxAccessibleTextEditViewAdapter* pTextEditViewForwarder = rEditSource.GetEditViewForwarderAdapter( bCreate );

        if( !pTextEditViewForwarder )
        {
            if( bCreate )
                throw uno::RuntimeException("Unable to fetch view forwarder, object is defunct",
                                            uno::Reference< uno::XInterface >
                                            ( static_cast< ::cppu::OWeakObject* >
                                              ( const_cast< AccessibleEditableTextPara* > (this) ) ) ); // disambiguate hierarchy
            else
                throw uno::RuntimeException("No view forwarder, object not in edit mode",
                                            uno::Reference< uno::XInterface >
                                            ( static_cast< ::cppu::OWeakObject* >
                                              ( const_cast< AccessibleEditableTextPara* > (this) ) ) ); // disambiguate hierarchy
        }

        if( pTextEditViewForwarder->IsValid() )
            return *pTextEditViewForwarder;
        else
        {
            if( bCreate )
                throw uno::RuntimeException("View forwarder is invalid, object is defunct",
                                            uno::Reference< uno::XInterface >
                                            ( static_cast< ::cppu::OWeakObject* >
                                              ( const_cast< AccessibleEditableTextPara* > (this) )  ) );    // disambiguate hierarchy
            else
                throw uno::RuntimeException("View forwarder is invalid, object not in edit mode",
                                            uno::Reference< uno::XInterface >
                                            ( static_cast< ::cppu::OWeakObject* >
                                              ( const_cast< AccessibleEditableTextPara* > (this) )  ) );    // disambiguate hierarchy
        }
    }

    sal_Bool AccessibleEditableTextPara::HaveEditView() const
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SvxEditSource& rEditSource = GetEditSource();
        SvxEditViewForwarder* pViewForwarder = rEditSource.GetEditViewForwarder();

        if( !pViewForwarder )
            return sal_False;

        if( !pViewForwarder->IsValid() )
            return sal_False;

        return sal_True;
    }

    sal_Bool AccessibleEditableTextPara::HaveChildren()
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= SAL_MAX_INT32,
                   "AccessibleEditableTextPara::HaveChildren: paragraph index value overflow");

        return GetTextForwarder().HaveImageBullet( GetParagraphIndex() );
    }

    Rectangle AccessibleEditableTextPara::LogicToPixel( const Rectangle& rRect, const MapMode& rMapMode, SvxViewForwarder& rForwarder )
    {
        // convert to screen coordinates
        return Rectangle( rForwarder.LogicToPixel( rRect.TopLeft(), rMapMode ),
                          rForwarder.LogicToPixel( rRect.BottomRight(), rMapMode ) );
    }

    const Point& AccessibleEditableTextPara::GetEEOffset() const
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return maEEOffset;
    }

    void AccessibleEditableTextPara::SetEEOffset( const Point& rOffset )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        WeakBullet::HardRefType aChild( maImageBullet.get() );
        if( aChild.is() )
            aChild->SetEEOffset(rOffset);

        maEEOffset = rOffset;
    }

    void AccessibleEditableTextPara::FireEvent(const sal_Int16 nEventId, const uno::Any& rNewValue, const uno::Any& rOldValue) const
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        uno::Reference < XAccessibleContext > xThis( const_cast< AccessibleEditableTextPara* > (this)->getAccessibleContext() );

        AccessibleEventObject aEvent(xThis, nEventId, rNewValue, rOldValue);

        // #102261# Call global queue for focus events
        if( nEventId == AccessibleEventId::STATE_CHANGED )
            vcl::unohelper::NotifyAccessibleStateEventGlobally( aEvent );

        // #106234# Delegate to EventNotifier
        if( getNotifierClientId() != -1 )
            ::comphelper::AccessibleEventNotifier::addEvent( getNotifierClientId(),
                                                             aEvent );
    }

    void AccessibleEditableTextPara::GotPropertyEvent( const uno::Any& rNewValue, const sal_Int16 nEventId ) const
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        FireEvent( nEventId, rNewValue );
    }

    void AccessibleEditableTextPara::LostPropertyEvent( const uno::Any& rOldValue, const sal_Int16 nEventId ) const
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        FireEvent( nEventId, uno::Any(), rOldValue );
    }

    void AccessibleEditableTextPara::SetState( const sal_Int16 nStateId )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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

    sal_Bool AccessibleEditableTextPara::GetAttributeRun( sal_uInt16& nStartIndex, sal_uInt16& nEndIndex, sal_Int32 nIndex )
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        DBG_ASSERT(nIndex >= 0 && nIndex <= USHRT_MAX,
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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        uno::Any aRet;

        // must provide XAccesibleText by hand, since it comes publicly inherited by XAccessibleEditableText
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

    // XAccessible
    uno::Reference< XAccessibleContext > SAL_CALL AccessibleEditableTextPara::getAccessibleContext() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        // We implement the XAccessibleContext interface in the same object
        return uno::Reference< XAccessibleContext > ( this );
    }

    // XAccessibleContext
    sal_Int32 SAL_CALL AccessibleEditableTextPara::getAccessibleChildCount() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        return HaveChildren() ? 1 : 0;
    }

    uno::Reference< XAccessible > SAL_CALL AccessibleEditableTextPara::getAccessibleChild( sal_Int32 i ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        if( !HaveChildren() )
            throw lang::IndexOutOfBoundsException("No children available",
                                                  uno::Reference< uno::XInterface >
                                                  ( static_cast< ::cppu::OWeakObject* > (this) ) ); // static_cast: disambiguate hierarchy

        if( i != 0 )
            throw lang::IndexOutOfBoundsException("Invalid child index",
                                                  uno::Reference< uno::XInterface >
                                                  ( static_cast< ::cppu::OWeakObject* > (this) ) ); // static_cast: disambiguate hierarchy

        WeakBullet::HardRefType aChild( maImageBullet.get() );

        if( !aChild.is() )
        {
            // there is no hard reference available, create object then
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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

#ifdef DBG_UTIL
        if( !mxParent.is() )
            OSL_TRACE( "AccessibleEditableTextPara::getAccessibleParent: no frontend set, did somebody forgot to call AccessibleTextHelper::SetEventSource()?");
#endif

        return mxParent;
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getAccessibleIndexInParent() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return mnIndexInParent;
    }

    sal_Int16 SAL_CALL AccessibleEditableTextPara::getAccessibleRole() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return AccessibleRole::PARAGRAPH;
    }

    OUString SAL_CALL AccessibleEditableTextPara::getAccessibleDescription() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

//        SolarMutexGuard aGuard;

        return OUString();
    }

    OUString SAL_CALL AccessibleEditableTextPara::getAccessibleName() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

//        SolarMutexGuard aGuard;

        return OUString();
    }

    uno::Reference< XAccessibleRelationSet > SAL_CALL AccessibleEditableTextPara::getAccessibleRelationSet() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        // #i27138# - provide relations CONTENT_FLOWS_FROM
        // and CONTENT_FLOWS_TO
        if ( mpParaManager )
        {
            utl::AccessibleRelationSetHelper* pAccRelSetHelper =
                                        new utl::AccessibleRelationSetHelper();
            sal_Int32 nMyParaIndex( GetParagraphIndex() );
            // relation CONTENT_FLOWS_FROM
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

            // relation CONTENT_FLOWS_TO
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
            // no relations, therefore empty
            return uno::Reference< XAccessibleRelationSet >();
        }
    }

    uno::Reference< XAccessibleStateSet > SAL_CALL AccessibleEditableTextPara::getAccessibleStateSet() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        // Create a copy of the state set and return it.
        ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());

        if( !pStateSet )
            return uno::Reference<XAccessibleStateSet>();

        return uno::Reference<XAccessibleStateSet>( new ::utl::AccessibleStateSetHelper (*pStateSet) );
    }

    lang::Locale SAL_CALL AccessibleEditableTextPara::getLocale() throw (IllegalAccessibleComponentStateException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        return implGetLocale();
    }

    void SAL_CALL AccessibleEditableTextPara::addAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        if( getNotifierClientId() != -1 )
            ::comphelper::AccessibleEventNotifier::addEventListener( getNotifierClientId(), xListener );
    }

    void SAL_CALL AccessibleEditableTextPara::removeAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        if( getNotifierClientId() != -1 )
            ::comphelper::AccessibleEventNotifier::removeEventListener( getNotifierClientId(), xListener );
    }

    // XAccessibleComponent
    sal_Bool SAL_CALL AccessibleEditableTextPara::containsPoint( const awt::Point& aTmpPoint ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        if( HaveChildren() )
        {
            // #103862# No longer need to make given position relative
            Point aPoint( _aPoint.X, _aPoint.Y );

            // respect EditEngine offset to surrounding shape/cell
            aPoint -= GetEEOffset();

            // convert to EditEngine coordinate system
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

        // no children at all, or none at given position
        return uno::Reference< XAccessible >();
    }

    awt::Rectangle SAL_CALL AccessibleEditableTextPara::getBounds() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= SAL_MAX_INT32,
                   "AccessibleEditableTextPara::getBounds: index value overflow");

        SvxTextForwarder& rCacheTF = GetTextForwarder();
        Rectangle aRect = rCacheTF.GetParaBounds( GetParagraphIndex() );

        // convert to screen coordinates
        Rectangle aScreenRect = AccessibleEditableTextPara::LogicToPixel( aRect,
                                                                          rCacheTF.GetMapMode(),
                                                                          GetViewForwarder() );

        // offset from shape/cell
        Point aOffset = GetEEOffset();

        return awt::Rectangle( aScreenRect.Left() + aOffset.X(),
                               aScreenRect.Top() + aOffset.Y(),
                               aScreenRect.GetSize().Width(),
                               aScreenRect.GetSize().Height() );
    }

    awt::Point SAL_CALL AccessibleEditableTextPara::getLocation(  ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        awt::Rectangle aRect = getBounds();

        return awt::Point( aRect.X, aRect.Y );
    }

    awt::Point SAL_CALL AccessibleEditableTextPara::getLocationOnScreen(  ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        // relate us to parent
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
            // #i88070#
            // fallback to parent's <XAccessibleContext> instance
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
                                    ( static_cast< XAccessible* > (this) ) );   // disambiguate hierarchy
    }

    awt::Size SAL_CALL AccessibleEditableTextPara::getSize(  ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        awt::Rectangle aRect = getBounds();

        return awt::Size( aRect.Width, aRect.Height );
    }

    void SAL_CALL AccessibleEditableTextPara::grabFocus(  ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        // set cursor to this paragraph
        setSelection(0,0);
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getForeground(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        // #104444# Added to XAccessibleComponent interface
        svtools::ColorConfig aColorConfig;
        sal_uInt32 nColor = aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor;
        return static_cast<sal_Int32>(nColor);
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getBackground(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        // #104444# Added to XAccessibleComponent interface
        Color aColor( Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor() );

        // the background is transparent
        aColor.SetTransparency( 0xFF);

        return static_cast<sal_Int32>( aColor.GetColor() );
    }

    // XAccessibleText
    sal_Int32 SAL_CALL AccessibleEditableTextPara::getCaretPosition() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        if( !HaveEditView() )
            return -1;

        ESelection aSelection;
        if( GetEditViewForwarder().GetSelection( aSelection ) &&
            GetParagraphIndex() == aSelection.nEndPara )
        {
            // caret is always nEndPara,nEndPos
            return aSelection.nEndPos;
        }

        // not within this paragraph
        return -1;
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::setCaretPosition( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return setSelection(nIndex, nIndex);
    }

    sal_Unicode SAL_CALL AccessibleEditableTextPara::getCharacter( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getCharacter: index value overflow");

        return OCommonAccessibleText::getCharacter( nIndex );
    }

    uno::Sequence< beans::PropertyValue > SAL_CALL AccessibleEditableTextPara::getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< OUString >& rRequestedAttributes ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );
        SolarMutexGuard aGuard;

        CheckIndex(nIndex); // may throw IndexOutOfBoundsException

        // get default attribues...
        ::comphelper::SequenceAsHashMap aPropHashMap( getDefaultAttributes( rRequestedAttributes ) );

        // ... and override them with the direct attributes from the specific position
        uno::Sequence< beans::PropertyValue > aRunAttribs( getRunAttributes( nIndex, rRequestedAttributes ) );
        sal_Int32 nRunAttribs = aRunAttribs.getLength();
        const beans::PropertyValue *pRunAttrib = aRunAttribs.getConstArray();
        for (sal_Int32 k = 0;  k < nRunAttribs;  ++k)
        {
            const beans::PropertyValue &rRunAttrib = pRunAttrib[k];
            aPropHashMap[ rRunAttrib.Name ] = rRunAttrib.Value; //!! should not only be the value !!
        }

        // get resulting sequence
        uno::Sequence< beans::PropertyValue > aRes;
        aPropHashMap >> aRes;

        // since SequenceAsHashMap ignores property handles and property state
        // we have to restore the property state here (property handles are
        // of no use to the accessibility API).
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

        return aRes;
    }

    awt::Rectangle SAL_CALL AccessibleEditableTextPara::getCharacterBounds( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getCharacterBounds: index value overflow");

        // #108900# Have position semantics now for nIndex, as
        // one-past-the-end values are legal, too.
        CheckPosition( nIndex );

        SvxTextForwarder& rCacheTF = GetTextForwarder();
        Rectangle aRect = rCacheTF.GetCharBounds( static_cast< sal_uInt16 >( GetParagraphIndex() ), static_cast< sal_uInt16 >( nIndex ) );

        // convert to screen
        Rectangle aScreenRect = AccessibleEditableTextPara::LogicToPixel( aRect,
                                                                          rCacheTF.GetMapMode(),
                                                                          GetViewForwarder() );
        // #109864# offset from parent (paragraph), but in screen
        // coordinates. This makes sure the internal text offset in
        // the outline view forwarder gets cancelled out here
        awt::Rectangle aParaRect( getBounds() );
        aScreenRect.Move( -aParaRect.X, -aParaRect.Y );

        // offset from shape/cell
        Point aOffset = GetEEOffset();

        return awt::Rectangle( aScreenRect.Left() + aOffset.X(),
                               aScreenRect.Top() + aOffset.Y(),
                               aScreenRect.GetSize().Width(),
                               aScreenRect.GetSize().Height() );
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getCharacterCount() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getCharacterCount: index value overflow");

        return OCommonAccessibleText::getCharacterCount();
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getIndexAtPoint( const awt::Point& rPoint ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        sal_Int32 nPara;
        sal_uInt16 nIndex;

        // offset from surrounding cell/shape
        Point aOffset( GetEEOffset() );
        Point aPoint( rPoint.X - aOffset.X(), rPoint.Y - aOffset.Y() );

        // convert to logical coordinates
        SvxTextForwarder& rCacheTF = GetTextForwarder();
        Point aLogPoint( GetViewForwarder().PixelToLogic( aPoint, rCacheTF.GetMapMode() ) );

        // re-offset to parent (paragraph)
        Rectangle aParaRect = rCacheTF.GetParaBounds( GetParagraphIndex() );
        aLogPoint.Move( aParaRect.Left(), aParaRect.Top() );

        if( rCacheTF.GetIndexAtPoint( aLogPoint, nPara, nIndex ) &&
            GetParagraphIndex() == nPara )
        {
            // #102259# Double-check if we're _really_ on the given character
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
                // #103927# Don't throw for invalid nIndex values
                return -1;
            }
        }
        else
        {
            // not within our paragraph
            return -1;
        }
    }

    OUString SAL_CALL AccessibleEditableTextPara::getSelectedText() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getSelectedText: index value overflow");

        if( !HaveEditView() )
            return OUString();

        return OCommonAccessibleText::getSelectedText();
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getSelectionStart() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getSelectionStart: index value overflow");

        if( !HaveEditView() )
            return -1;

        return OCommonAccessibleText::getSelectionStart();
    }

    sal_Int32 SAL_CALL AccessibleEditableTextPara::getSelectionEnd() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getSelectionEnd: index value overflow");

        if( !HaveEditView() )
            return -1;

        return OCommonAccessibleText::getSelectionEnd();
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::setSelection: paragraph index value overflow");

        CheckRange(nStartIndex, nEndIndex);

        try
        {
            SvxEditViewForwarder& rCacheVF = GetEditViewForwarder( sal_True );
            return rCacheVF.SetSelection( MakeSelection(nStartIndex, nEndIndex) );
        }
        catch (const uno::RuntimeException&)
        {
            return sal_False;
        }
    }

    OUString SAL_CALL AccessibleEditableTextPara::getText() throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getText: paragraph index value overflow");

        return OCommonAccessibleText::getText();
    }

    OUString SAL_CALL AccessibleEditableTextPara::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getTextRange: paragraph index value overflow");

        return OCommonAccessibleText::getTextRange(nStartIndex, nEndIndex);
    }

    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleEditableTextPara::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getTextAtIndex: paragraph index value overflow");

        ::com::sun::star::accessibility::TextSegment aResult;
        aResult.SegmentStart = -1;
        aResult.SegmentEnd = -1;

        switch( aTextType )
        {
            // Not yet handled by OCommonAccessibleText. Missing
            // implGetAttributeRunBoundary() method there
            case AccessibleTextType::ATTRIBUTE_RUN:
            {
                const sal_Int32 nTextLen = GetTextForwarder().GetTextLen( GetParagraphIndex() );

                if( nIndex == nTextLen )
                {
                    // #i17014# Special-casing one-behind-the-end character
                    aResult.SegmentStart = aResult.SegmentEnd = nTextLen;
                }
                else
                {
                    sal_uInt16 nStartIndex, nEndIndex;

                    if( GetAttributeRun(nStartIndex, nEndIndex, nIndex) )
                    {
                        aResult.SegmentText = GetTextRange(nStartIndex, nEndIndex);
                        aResult.SegmentStart = nStartIndex;
                        aResult.SegmentEnd = nEndIndex;
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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getTextBeforeIndex: paragraph index value overflow");

        ::com::sun::star::accessibility::TextSegment aResult;
        aResult.SegmentStart = -1;
        aResult.SegmentEnd = -1;

        switch( aTextType )
        {
            // Not yet handled by OCommonAccessibleText. Missing
            // implGetAttributeRunBoundary() method there
            case AccessibleTextType::ATTRIBUTE_RUN:
            {
                const sal_Int32 nTextLen = GetTextForwarder().GetTextLen( GetParagraphIndex() );
                sal_uInt16 nStartIndex, nEndIndex;

                if( nIndex == nTextLen )
                {
                    // #i17014# Special-casing one-behind-the-end character
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
                        // already at the left border? If not, query
                        // one index further left
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

            default:
                aResult = OCommonAccessibleText::getTextBeforeIndex( nIndex, aTextType );
                break;
        } /* end of switch( aTextType ) */

        return aResult;
    }

    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleEditableTextPara::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                   "AccessibleEditableTextPara::getTextBehindIndex: paragraph index value overflow");

        ::com::sun::star::accessibility::TextSegment aResult;
        aResult.SegmentStart = -1;
        aResult.SegmentEnd = -1;

        switch( aTextType )
        {
            case AccessibleTextType::ATTRIBUTE_RUN:
            {
                sal_uInt16 nStartIndex, nEndIndex;

                if( GetAttributeRun(nStartIndex, nEndIndex, nIndex) )
                {
                    // already at the right border?
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

            default:
                aResult = OCommonAccessibleText::getTextBehindIndex( nIndex, aTextType );
                break;
        } /* end of switch( aTextType ) */

        return aResult;
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        try
        {
            SvxEditViewForwarder& rCacheVF = GetEditViewForwarder( sal_True );
            #if OSL_DEBUG_LEVEL > 0
            SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    // MUST be after GetEditViewForwarder(), see method docs
            (void)rCacheTF;
            #else
            GetTextForwarder();                                         // MUST be after GetEditViewForwarder(), see method docs
            #endif

            sal_Bool aRetVal;

            DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                       "AccessibleEditableTextPara::copyText: index value overflow");

            CheckRange(nStartIndex, nEndIndex);

            // save current selection
            ESelection aOldSelection;

            rCacheVF.GetSelection( aOldSelection );
            rCacheVF.SetSelection( MakeSelection(nStartIndex, nEndIndex) );
            aRetVal = rCacheVF.Copy();
            rCacheVF.SetSelection( aOldSelection ); // restore

            return aRetVal;
        }
        catch (const uno::RuntimeException&)
        {
            return sal_False;
        }
    }

    // XAccessibleEditableText
    sal_Bool SAL_CALL AccessibleEditableTextPara::cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        try
        {
            SvxEditViewForwarder& rCacheVF = GetEditViewForwarder( sal_True );
            SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    // MUST be after GetEditViewForwarder(), see method docs

            DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                       "AccessibleEditableTextPara::cutText: index value overflow");

            CheckRange(nStartIndex, nEndIndex);

            if( !rCacheTF.IsEditable( MakeSelection(nStartIndex, nEndIndex) ) )
                return sal_False; // non-editable area selected

            // don't save selection, might become invalid after cut!
            rCacheVF.SetSelection( MakeSelection(nStartIndex, nEndIndex) );

            return rCacheVF.Cut();
        }
        catch (const uno::RuntimeException&)
        {
            return sal_False;
        }
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::pasteText( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        try
        {
            SvxEditViewForwarder& rCacheVF = GetEditViewForwarder( sal_True );
            SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    // MUST be after GetEditViewForwarder(), see method docs

            DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                       "AccessibleEditableTextPara::pasteText: index value overflow");

            CheckPosition(nIndex);

            if( !rCacheTF.IsEditable( MakeSelection(nIndex) ) )
                return sal_False; // non-editable area selected

            // #104400# set empty selection (=> cursor) to given index
            rCacheVF.SetSelection( MakeCursor(nIndex) );

            return rCacheVF.Paste();
        }
        catch (const uno::RuntimeException&)
        {
            return sal_False;
        }
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        try
        {
            // #102710# Request edit view when doing changes
            // AccessibleEmptyEditSource relies on this behaviour
            GetEditViewForwarder( sal_True );
            SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    // MUST be after GetEditViewForwarder(), see method docs

            DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                       "AccessibleEditableTextPara::deleteText: index value overflow");

            CheckRange(nStartIndex, nEndIndex);

            if( !rCacheTF.IsEditable( MakeSelection(nStartIndex, nEndIndex) ) )
                return sal_False; // non-editable area selected

            sal_Bool bRet = rCacheTF.Delete( MakeSelection(nStartIndex, nEndIndex) );

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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        try
        {
            // #102710# Request edit view when doing changes
            // AccessibleEmptyEditSource relies on this behaviour
            GetEditViewForwarder( sal_True );
            SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    // MUST be after GetEditViewForwarder(), see method docs

            DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                       "AccessibleEditableTextPara::insertText: index value overflow");

            CheckPosition(nIndex);

            if( !rCacheTF.IsEditable( MakeSelection(nIndex) ) )
                return sal_False; // non-editable area selected

            // #104400# insert given text at empty selection (=> cursor)
            sal_Bool bRet = rCacheTF.InsertText( sText, MakeCursor(nIndex) );

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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        try
        {
            // #102710# Request edit view when doing changes
            // AccessibleEmptyEditSource relies on this behaviour
            GetEditViewForwarder( sal_True );
            SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    // MUST be after GetEditViewForwarder(), see method docs

            DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                       "AccessibleEditableTextPara::replaceText: index value overflow");

            CheckRange(nStartIndex, nEndIndex);

            if( !rCacheTF.IsEditable( MakeSelection(nStartIndex, nEndIndex) ) )
                return sal_False; // non-editable area selected

            // insert given text into given range => replace
            sal_Bool bRet = rCacheTF.InsertText( sReplacement, MakeSelection(nStartIndex, nEndIndex) );

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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        try
        {
            // #102710# Request edit view when doing changes
            // AccessibleEmptyEditSource relies on this behaviour
            GetEditViewForwarder( sal_True );
            SvxAccessibleTextAdapter& rCacheTF = GetTextForwarder();    // MUST be after GetEditViewForwarder(), see method docs
            sal_uInt16 nPara = static_cast< sal_uInt16 >( GetParagraphIndex() );

            DBG_ASSERT(GetParagraphIndex() >= 0 && GetParagraphIndex() <= USHRT_MAX,
                       "AccessibleEditableTextPara::setAttributes: index value overflow");

            CheckRange(nStartIndex, nEndIndex);

            if( !rCacheTF.IsEditable( MakeSelection(nStartIndex, nEndIndex) ) )
                return sal_False; // non-editable area selected

            // do the indices span the whole paragraph? Then use the outliner map
            // TODO: hold it as a member?
            SvxAccessibleTextPropertySet aPropSet( &GetEditSource(),
                                                   0 == nStartIndex &&
                                                   rCacheTF.GetTextLen(nPara) == nEndIndex ?
                                                   ImplGetSvxUnoOutlinerTextCursorSvxPropertySet() :
                                                   ImplGetSvxTextPortionSvxPropertySet() );

            aPropSet.SetSelection( MakeSelection(nStartIndex, nEndIndex) );

            // convert from PropertyValue to Any
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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        SolarMutexGuard aGuard;

        return replaceText(0, getCharacterCount(), sText);
    }

    // XAccessibleTextAttributes
    uno::Sequence< beans::PropertyValue > SAL_CALL AccessibleEditableTextPara::getDefaultAttributes(
            const uno::Sequence< OUString >& rRequestedAttributes )
        throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );
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

        // get XPropertySetInfo for paragraph attributes and
        // character attributes that span all the paragraphs text.
        SvxAccessibleTextPropertySet aPropSet( &GetEditSource(),
                ImplGetSvxCharAndParaPropertiesSet() );
        aPropSet.SetSelection( MakeSelection( 0, GetTextLen() ) );
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo = aPropSet.getPropertySetInfo();
        if (!xPropSetInfo.is())
            throw uno::RuntimeException("Cannot query XPropertySetInfo",
                        uno::Reference< uno::XInterface >
                        ( static_cast< XAccessible* > (this) ) );   // disambiguate hierarchy

        // build sequence of available properties to check
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

        // build resulting sequence
        uno::Sequence< beans::PropertyValue > aOutSequence( nLength );
        beans::PropertyValue* pOutSequence = aOutSequence.getArray();
        sal_Int32 nOutLen = 0;
        for (sal_Int32 i = 0;  i < nLength;  ++i)
        {
            // calling implementation functions:
            // _getPropertyState and _getPropertyValue (see below) to provide
            // the proper paragraph number when retrieving paragraph attributes
            PropertyState eState = aPropSet._getPropertyState( pProperties->Name, mnParagraphIndex );
            if ( eState == PropertyState_AMBIGUOUS_VALUE )
            {
                OSL_FAIL( "ambiguous property value encountered" );
            }

            //if (eState == PropertyState_DIRECT_VALUE)
            // per definition all paragraph properties and all character
            // properties spanning the whole paragraph should be returned
            // and declared as default value
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
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

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

        CheckIndex(nIndex);

        SvxAccessibleTextPropertySet aPropSet( &GetEditSource(),
                                               ImplGetSvxCharAndParaPropertiesSet() );
        aPropSet.SetSelection( MakeSelection( nIndex ) );
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo = aPropSet.getPropertySetInfo();
        if (!xPropSetInfo.is())
            throw uno::RuntimeException("Cannot query XPropertySetInfo",
                                        uno::Reference< uno::XInterface >
                                        ( static_cast< XAccessible* > (this) ) );   // disambiguate hierarchy

        // build sequence of available properties to check
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

        // build resulting sequence
        uno::Sequence< beans::PropertyValue > aOutSequence( nLength );
        beans::PropertyValue* pOutSequence = aOutSequence.getArray();
        sal_Int32 nOutLen = 0;
        for (sal_Int32 i = 0;  i < nLength;  ++i)
        {
            // calling 'regular' functions that will operate on the selection
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

    // XAccessibleHypertext
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

                    // Translate EE Index to accessible index
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

//        SvxAccessibleTextIndex aIndex;
//        aIndex.SetIndex(nPara, nCharIndex, rT);
//        const sal_uInt16 nEEIndex = aIndex.GetEEIndex();

        const sal_uInt16 nEEIndex = rT.CalcEditEngineIndex( nPara, nCharIndex );
        sal_Int32 nHLIndex = 0;
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

    // XAccessibleMultiLineText
    sal_Int32 SAL_CALL AccessibleEditableTextPara::getLineNumberAtIndex( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        sal_Int32 nRes = -1;
        sal_Int32 nPara = GetParagraphIndex();

        SvxTextForwarder &rCacheTF = GetTextForwarder();
        const bool bValidPara = 0 <= nPara && nPara < rCacheTF.GetParagraphCount();
        DBG_ASSERT( bValidPara, "getLineNumberAtIndex: current paragraph index out of range" );
        if (bValidPara)
        {
            // we explicitly allow for the index to point at the character right behind the text
            if (0 <= nIndex && nIndex <= rCacheTF.GetTextLen( nPara ))
                nRes = rCacheTF.GetLineNumberAtIndex( nPara, static_cast< sal_uInt16 >(nIndex) );
            else
                throw lang::IndexOutOfBoundsException();
        }
        return nRes;
    }

    // XAccessibleMultiLineText
    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleEditableTextPara::getTextAtLineNumber( sal_Int32 nLineNo ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::com::sun::star::accessibility::TextSegment aResult;
        sal_Int32 nPara = GetParagraphIndex();
        SvxTextForwarder &rCacheTF = GetTextForwarder();
        const bool bValidPara = 0 <= nPara && nPara < rCacheTF.GetParagraphCount();
        DBG_ASSERT( bValidPara, "getTextAtLineNumber: current paragraph index out of range" );
        if (bValidPara)
        {
            if (0 <= nLineNo && nLineNo < rCacheTF.GetLineCount( nPara ))
            {
                sal_uInt16 nStart = 0, nEnd = 0;
                rCacheTF.GetLineBoundaries( nStart, nEnd, nPara, static_cast< sal_uInt16 >(nLineNo) );
                if (nStart != 0xFFFF && nEnd != 0xFFFF)
                {
                    try
                    {
                        aResult.SegmentText     = getTextRange( nStart, nEnd );
                        aResult.SegmentStart    = nStart;
                        aResult.SegmentEnd      = nEnd;
                    }
                    catch (const lang::IndexOutOfBoundsException&)
                    {
                        // this is not the exception that should be raised in this function ...
                        DBG_ASSERT( 0, "unexpected exception" );
                    }
                }
            }
            else
                throw lang::IndexOutOfBoundsException();
        }
        return aResult;
    }

    // XAccessibleMultiLineText
    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleEditableTextPara::getTextAtLineWithCaret(  ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        ::com::sun::star::accessibility::TextSegment aResult;
        try
        {
            aResult = getTextAtLineNumber( getNumberOfLineWithCaret() );
        }
        catch (const lang::IndexOutOfBoundsException&)
        {
            // this one needs to be catched since this interface does not allow for it.
        }
        return aResult;
    }

    // XAccessibleMultiLineText
    sal_Int32 SAL_CALL AccessibleEditableTextPara::getNumberOfLineWithCaret(  ) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        sal_Int32 nRes = -1;
        try
        {
            nRes = getLineNumberAtIndex( getCaretPosition() );
        }
        catch (const lang::IndexOutOfBoundsException&)
        {
            // this one needs to be catched since this interface does not allow for it.
        }
        return nRes;
    }


    // XServiceInfo
    OUString SAL_CALL AccessibleEditableTextPara::getImplementationName (void) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        return OUString("AccessibleEditableTextPara");
    }

    sal_Bool SAL_CALL AccessibleEditableTextPara::supportsService (const OUString& sServiceName) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        //  Iterate over all supported service names and return true if on of them
        //  matches the given name.
        uno::Sequence< OUString> aSupportedServices (
            getSupportedServiceNames ());
        for (int i=0; i<aSupportedServices.getLength(); i++)
            if (sServiceName == aSupportedServices[i])
                return sal_True;
        return sal_False;
    }

    uno::Sequence< OUString> SAL_CALL AccessibleEditableTextPara::getSupportedServiceNames (void) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        const OUString sServiceName( getServiceName() );
        return uno::Sequence< OUString > (&sServiceName, 1);
    }

    // XServiceName
    OUString SAL_CALL AccessibleEditableTextPara::getServiceName (void) throw (uno::RuntimeException)
    {
        DBG_CHKTHIS( AccessibleEditableTextPara, NULL );

        // #105185# Using correct service now
        return OUString("com.sun.star.text.AccessibleParagraphView");
    }

}  // end of namespace accessibility

//------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
