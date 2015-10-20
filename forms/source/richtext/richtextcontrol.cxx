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

#include "richtextcontrol.hxx"
#include "property.hrc"
#include "services.hxx"

#include "richtextmodel.hxx"
#include "richtextvclcontrol.hxx"
#include "clipboarddispatcher.hxx"
#include "parametrizedattributedispatcher.hxx"
#include "specialdispatchers.hxx"

#include <com/sun/star/awt/PosSize.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>

#include <svx/svxids.hrc>
#include <editeng/editview.hxx>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <sfx2/msgpool.hxx>

namespace frm
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::frame;

    ORichTextControl::ORichTextControl()
        :UnoEditControl()
    {
    }


    ORichTextControl::~ORichTextControl()
    {
    }


    IMPLEMENT_FORWARD_XTYPEPROVIDER2( ORichTextControl, UnoEditControl, ORichTextControl_Base )


    Any SAL_CALL ORichTextControl::queryAggregation( const Type& _rType ) throw ( RuntimeException, std::exception )
    {
        Any aReturn = UnoEditControl::queryAggregation( _rType );

        if ( !aReturn.hasValue() )
            aReturn = ORichTextControl_Base::queryInterface( _rType );

        return aReturn;
    }


    namespace
    {

        static void implAdjustTriStateFlag( const Reference< XPropertySet >& _rxProps, const OUString& _rPropertyName,
            WinBits& _rAllBits, WinBits _nPositiveFlag, WinBits nNegativeFlag )
        {
            bool bFlagValue = false;
            if ( _rxProps->getPropertyValue( _rPropertyName ) >>= bFlagValue )
                _rAllBits |= ( bFlagValue ? _nPositiveFlag : nNegativeFlag );
        }


        static void implAdjustTwoStateFlag( const Any& _rValue, WinBits& _rAllBits, WinBits _nFlag, bool _bInvert = false )
        {
            bool bFlagValue = false;
            if ( _rValue >>= bFlagValue )
            {
                if ( _bInvert )
                    bFlagValue = !bFlagValue;
                if ( bFlagValue )
                    _rAllBits |= _nFlag;
                else
                    _rAllBits &= ~_nFlag;
            }
        }


        static void implAdjustTwoStateFlag( const Reference< XPropertySet >& _rxProps, const OUString& _rPropertyName,
            WinBits& _rAllBits, WinBits _nFlag, bool _bInvert = false )
        {
            implAdjustTwoStateFlag( _rxProps->getPropertyValue( _rPropertyName ), _rAllBits, _nFlag, _bInvert );
        }


        static void adjustTwoStateWinBit( vcl::Window* _pWindow, const Any& _rValue, WinBits _nFlag, bool _bInvert = false )
        {
            WinBits nBits = _pWindow->GetStyle();
            implAdjustTwoStateFlag( _rValue, nBits, _nFlag, _bInvert );
            _pWindow->SetStyle( nBits );
        }


        static WinBits getWinBits( const Reference< XControlModel >& _rxModel, WinBits nBaseBits = 0 )
        {
            WinBits nBits = nBaseBits;
            try
            {
                Reference< XPropertySet > xProps( _rxModel, UNO_QUERY );
                if ( xProps.is() )
                {
                    sal_Int16 nBorder = 0;
                    xProps->getPropertyValue( PROPERTY_BORDER ) >>= nBorder;
                    if ( nBorder )
                        nBits |= WB_BORDER;

                    implAdjustTriStateFlag( xProps, PROPERTY_TABSTOP,        nBits, WB_TABSTOP, WB_NOTABSTOP );
                    implAdjustTwoStateFlag( xProps, PROPERTY_HSCROLL,        nBits, WB_HSCROLL );
                    implAdjustTwoStateFlag( xProps, PROPERTY_VSCROLL,        nBits, WB_VSCROLL );
                    implAdjustTwoStateFlag( xProps, PROPERTY_HARDLINEBREAKS, nBits, WB_WORDBREAK, true );
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return nBits;
        }
    }


    void SAL_CALL ORichTextControl::createPeer( const Reference< XToolkit >& _rToolkit, const Reference< XWindowPeer >& _rParentPeer ) throw( RuntimeException, std::exception )
    {
        bool bReallyActAsRichText = false;
        try
        {
            Reference< XPropertySet > xModelProps( getModel(), UNO_QUERY_THROW );
            xModelProps->getPropertyValue( PROPERTY_RICH_TEXT ) >>= bReallyActAsRichText;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        if ( !bReallyActAsRichText )
        {
            UnoEditControl::createPeer( _rToolkit, _rParentPeer );
            return;
        }

        SolarMutexGuard aGuard;

        if (!getPeer().is())
        {
            mbCreatingPeer = true;

            // determine the VLC window for the parent
            vcl::Window* pParentWin = NULL;
            if ( _rParentPeer.is() )
            {
                VCLXWindow* pParentXWin = VCLXWindow::GetImplementation( _rParentPeer );
                if ( pParentXWin )
                    pParentWin = pParentXWin->GetWindow();
                DBG_ASSERT( pParentWin, "ORichTextControl::createPeer: could not obtain the VCL-level parent window!" );
            }

            // create the peer
            Reference< XControlModel > xModel( getModel() );
            ORichTextPeer* pPeer = ORichTextPeer::Create( xModel, pParentWin, getWinBits( xModel ) );
            DBG_ASSERT( pPeer, "ORichTextControl::createPeer: invalid peer returned!" );
            if ( pPeer )
            {
                // by definition, the returned component is acquired once
                pPeer->release();

                // announce the peer to the base class
                setPeer( pPeer );

                // initialize ourself (and thus the peer) with the model properties
                updateFromModel();

                Reference< XView >  xPeerView( getPeer(), UNO_QUERY );
                if ( xPeerView.is() )
                {
                    xPeerView->setZoom( maComponentInfos.nZoomX, maComponentInfos.nZoomY );
                    xPeerView->setGraphics( mxGraphics );
                }

                // a lot of initial settings from our component infos
                setPosSize( maComponentInfos.nX, maComponentInfos.nY, maComponentInfos.nWidth, maComponentInfos.nHeight, PosSize::POSSIZE );

                pPeer->setVisible   ( maComponentInfos.bVisible && !mbDesignMode );
                pPeer->setEnable    ( maComponentInfos.bEnable                   );
                pPeer->setDesignMode( mbDesignMode                               );

                peerCreated();
            }

            mbCreatingPeer = false;
        }
    }


    OUString SAL_CALL ORichTextControl::getImplementationName()  throw( RuntimeException, std::exception )
    {
        return getImplementationName_Static();
    }


    Sequence< OUString > SAL_CALL ORichTextControl::getSupportedServiceNames()  throw( RuntimeException, std::exception )
    {
        return getSupportedServiceNames_Static();
    }

    OUString SAL_CALL ORichTextControl::getImplementationName_Static()
    {
        return OUString( "com.sun.star.comp.form.ORichTextControl" );
    }

    Sequence< OUString > SAL_CALL ORichTextControl::getSupportedServiceNames_Static()
    {
        Sequence< OUString > aServices( 3 );
        aServices[ 0 ] = "com.sun.star.awt.UnoControl";
        aServices[ 1 ] = "com.sun.star.awt.UnoControlEdit";
        aServices[ 2 ] = FRM_SUN_CONTROL_RICHTEXTCONTROL;
        return aServices;
    }

    Reference< XDispatch > SAL_CALL ORichTextControl::queryDispatch( const css::util::URL& _rURL, const OUString& _rTargetFrameName, sal_Int32 _nSearchFlags ) throw (RuntimeException, std::exception)
    {
        Reference< XDispatch > aReturn;
        Reference< XDispatchProvider > xTypedPeer( getPeer(), UNO_QUERY );
        if ( xTypedPeer.is() )
        {
            aReturn = xTypedPeer->queryDispatch( _rURL, _rTargetFrameName, _nSearchFlags );
        }
        return aReturn;
    }

    Sequence< Reference< XDispatch > > SAL_CALL ORichTextControl::queryDispatches( const Sequence< DispatchDescriptor >& _rRequests ) throw (RuntimeException, std::exception)
    {
        Sequence< Reference< XDispatch > > aReturn;
        Reference< XDispatchProvider > xTypedPeer( getPeer(), UNO_QUERY );
        if ( xTypedPeer.is() )
        {
            aReturn = xTypedPeer->queryDispatches( _rRequests );
        }
        return aReturn;
    }

    bool ORichTextControl::requiresNewPeer( const OUString& _rPropertyName ) const
    {
        return UnoControl::requiresNewPeer( _rPropertyName ) || _rPropertyName == PROPERTY_RICH_TEXT;
    }

    // ORichTextPeer
    ORichTextPeer* ORichTextPeer::Create( const Reference< XControlModel >& _rxModel, vcl::Window* _pParentWindow, WinBits _nStyle )
    {
        DBG_TESTSOLARMUTEX();

        // the EditEngine of the model
        RichTextEngine* pEngine = ORichTextModel::getEditEngine( _rxModel );
        OSL_ENSURE( pEngine, "ORichTextPeer::Create: could not obtaine the edit engine from the model!" );
        if ( !pEngine )
            return NULL;

        // the peer itself
        ORichTextPeer* pPeer = new ORichTextPeer;
        pPeer->acquire();   // by definition, the returned object is acquired once

        // the VCL control for the peer
        VclPtrInstance<RichTextControl> pRichTextControl( pEngine, _pParentWindow, _nStyle, nullptr, pPeer );

        // some knittings
        pRichTextControl->SetComponentInterface( pPeer );

        // outta here
        return pPeer;
    }


    ORichTextPeer::ORichTextPeer()
    {
    }


    ORichTextPeer::~ORichTextPeer()
    {
    }


    void ORichTextPeer::dispose( ) throw(RuntimeException, std::exception)
    {
        {
            SolarMutexGuard aGuard;
            VclPtr< RichTextControl > pRichTextControl = GetAs< RichTextControl >();

            if ( pRichTextControl )
            {
                for (   AttributeDispatchers::iterator aDisposeLoop = m_aDispatchers.begin();
                        aDisposeLoop != m_aDispatchers.end();
                        ++aDisposeLoop
                    )
                {
                    pRichTextControl->disableAttributeNotification( aDisposeLoop->first );
                    aDisposeLoop->second->dispose();
                }
            }

            AttributeDispatchers aEmpty;
            m_aDispatchers.swap( aEmpty );
        }

        VCLXWindow::dispose();
    }


    void SAL_CALL ORichTextPeer::draw( sal_Int32 _nX, sal_Int32 _nY ) throw(css::uno::RuntimeException, std::exception)
    {
        SolarMutexGuard aGuard;

        VclPtr< RichTextControl > pControl = GetAs< RichTextControl >();
        if ( !pControl )
            return;

        OutputDevice* pTargetDevice = VCLUnoHelper::GetOutputDevice( getGraphics() );
        OSL_ENSURE( pTargetDevice != NULL, "ORichTextPeer::draw: no graphics -> no drawing!" );
        if ( !pTargetDevice )
            return;

        ::Size aSize = pControl->GetSizePixel();
        const MapUnit eTargetUnit = pTargetDevice->GetMapMode().GetMapUnit();
        if ( eTargetUnit != MAP_PIXEL )
            aSize = pTargetDevice->PixelToLogic( aSize );

        ::Point aPos( _nX, _nY );
        // the XView::draw API talks about pixels, always ...
        if ( eTargetUnit != MAP_PIXEL )
            aPos = pTargetDevice->PixelToLogic( aPos );

        pControl->Draw( pTargetDevice, aPos, aSize, DrawFlags::NoControls );
    }


    void SAL_CALL ORichTextPeer::setProperty( const OUString& _rPropertyName, const Any& _rValue )
        throw (RuntimeException,
               std::exception)
    {
        SolarMutexGuard g;

        if ( !GetWindow() )
        {
            VCLXWindow::setProperty( _rPropertyName, _rValue );
            return;
        }

        if ( _rPropertyName == PROPERTY_BACKGROUNDCOLOR )
        {
            VclPtr< RichTextControl > pControl = GetAs< RichTextControl >();
            if ( !_rValue.hasValue() )
            {
                pControl->SetBackgroundColor( );
            }
            else
            {
                sal_Int32 nColor = COL_TRANSPARENT;
                _rValue >>= nColor;
                pControl->SetBackgroundColor( Color( nColor ) );
            }
        }
        else if ( _rPropertyName == PROPERTY_HSCROLL )
        {
            adjustTwoStateWinBit( GetWindow(), _rValue, WB_HSCROLL );
        }
        else if ( _rPropertyName == PROPERTY_VSCROLL )
        {
            adjustTwoStateWinBit( GetWindow(), _rValue, WB_VSCROLL );
        }
        else if ( _rPropertyName == PROPERTY_HARDLINEBREAKS )
        {
            adjustTwoStateWinBit( GetWindow(), _rValue, WB_WORDBREAK, true );
        }
        else if ( _rPropertyName == PROPERTY_READONLY )
        {
            VclPtr< RichTextControl > pControl = GetAs< RichTextControl >();
            bool bReadOnly( pControl->IsReadOnly() );
            OSL_VERIFY( _rValue >>= bReadOnly );
            pControl->SetReadOnly( bReadOnly );

            // update the dispatchers
            for (   AttributeDispatchers::iterator aDispatcherLoop = m_aDispatchers.begin();
                    aDispatcherLoop != m_aDispatchers.end();
                    ++aDispatcherLoop
                )
            {
                aDispatcherLoop->second->invalidate();
            }
        }
        else if ( _rPropertyName == PROPERTY_HIDEINACTIVESELECTION )
        {
            VclPtr< RichTextControl > pRichTextControl = GetAs< RichTextControl >();
            bool bHide = pRichTextControl->GetHideInactiveSelection();
            OSL_VERIFY( _rValue >>= bHide );
            pRichTextControl->SetHideInactiveSelection( bHide );
        }
        else
            VCLXWindow::setProperty( _rPropertyName, _rValue );
    }


    IMPLEMENT_FORWARD_XINTERFACE2( ORichTextPeer, VCLXWindow, ORichTextPeer_Base )


    IMPLEMENT_FORWARD_XTYPEPROVIDER2( ORichTextPeer, VCLXWindow, ORichTextPeer_Base )


    namespace
    {
        static SfxSlotId lcl_translateConflictingSlot( SfxSlotId _nIDFromPool )
        {
            // HACK HACK HACK
            // unfortunately, some of our applications have some conflicting slots,
            // i.e. slots which have the same UNO name as an existing other (common)
            // slot.
            // For instance, both the slots SID_SET_SUPER_SCRIPT (from SVX) and FN_SET_SUPER_SCRIPT
            // (from SW) have the UNO name "SuperScript".
            // Now, if the controls lives in a text document, and asks the SfxSlotPool for
            // the id belonging to "SuperScript", it gets the FN_SET_SUPER_SCRIPT - which
            // is completely unknown to the EditEngine.
            // So, we need to translate such conflicting ids.

            // Note that the real solution would be to fix the applications to
            // *not* define conflicting slots. Alternatively, if SFX would provide a slot pool
            // which is *static* (i.e. independent on the active application), then we
            // would also never encounter such a conflict.
            SfxSlotId nReturn( _nIDFromPool );
            switch ( _nIDFromPool )
            {
            case 20411: /* FM_SET_SUPER_SCRIPT, originating in SW */
                nReturn = SID_SET_SUPER_SCRIPT;
                break;
            case 20412: /* FN_SET_SUB_SCRIPT, originating in SW */
                nReturn = SID_SET_SUB_SCRIPT;
                break;
            }
            return nReturn;
        }
    }


    ORichTextPeer::SingleAttributeDispatcher ORichTextPeer::implCreateDispatcher( SfxSlotId _nSlotId, const css::util::URL& _rURL )
    {
        VclPtr< RichTextControl > pRichTextControl = GetAs< RichTextControl >();
        OSL_PRECOND( pRichTextControl, "ORichTextPeer::implCreateDispatcher: invalid window!" );
        if ( !pRichTextControl )
            return SingleAttributeDispatcher( NULL );

        ORichTextFeatureDispatcher* pDispatcher = NULL;
        OAttributeDispatcher* pAttributeDispatcher = NULL;
        switch ( _nSlotId )
        {
        case SID_CUT:
            pDispatcher = new OClipboardDispatcher( pRichTextControl->getView(), OClipboardDispatcher::eCut );
            break;

        case SID_COPY:
            pDispatcher = new OClipboardDispatcher( pRichTextControl->getView(), OClipboardDispatcher::eCopy );
            break;

        case SID_PASTE:
            pDispatcher = new OPasteClipboardDispatcher( pRichTextControl->getView() );
            break;

        case SID_SELECTALL:
            pDispatcher = new OSelectAllDispatcher( pRichTextControl->getView(), _rURL );
            break;

        case SID_ATTR_PARA_LEFT_TO_RIGHT:
        case SID_ATTR_PARA_RIGHT_TO_LEFT:
            pAttributeDispatcher = new OParagraphDirectionDispatcher( pRichTextControl->getView(), _nSlotId, _rURL, pRichTextControl );
            break;

        case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
        case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
            pDispatcher = new OTextDirectionDispatcher( pRichTextControl->getView(), _rURL );
            break;

        case SID_ATTR_PARA_HANGPUNCTUATION:
        case SID_ATTR_PARA_FORBIDDEN_RULES:
        case SID_ATTR_PARA_SCRIPTSPACE:
            pAttributeDispatcher = new OAsianFontLayoutDispatcher( pRichTextControl->getView(), _nSlotId, _rURL, pRichTextControl );
            break;

        default:
        {
            // is it a supported slot?
            bool bSupportedSlot = false;
            if ( !bSupportedSlot )
            {
                const SfxItemPool& rPool = *pRichTextControl->getView().GetEmptyItemSet().GetPool();
                bSupportedSlot = rPool.IsInRange( rPool.GetWhich( _nSlotId ) );
            }
            if ( !bSupportedSlot )
                bSupportedSlot = RichTextControl::isMappableSlot( _nSlotId );

            if ( bSupportedSlot )
            {   // it's really a slot which is supported by the EditEngine

                bool bNeedParametrizedDispatcher = true;
                if  (  ( _nSlotId == SID_ATTR_CHAR_POSTURE )
                    || ( _nSlotId == SID_ATTR_CHAR_CJK_POSTURE )
                    || ( _nSlotId == SID_ATTR_CHAR_CTL_POSTURE )
                    || ( _nSlotId == SID_ATTR_CHAR_LATIN_POSTURE )
                    || ( _nSlotId == SID_ATTR_CHAR_WEIGHT )
                    || ( _nSlotId == SID_ATTR_CHAR_CJK_WEIGHT )
                    || ( _nSlotId == SID_ATTR_CHAR_CTL_WEIGHT )
                    || ( _nSlotId == SID_ATTR_CHAR_LATIN_WEIGHT )
                    || ( _nSlotId == SID_ATTR_CHAR_LANGUAGE )
                    || ( _nSlotId == SID_ATTR_CHAR_CJK_LANGUAGE )
                    || ( _nSlotId == SID_ATTR_CHAR_CTL_LANGUAGE )
                    || ( _nSlotId == SID_ATTR_CHAR_LATIN_LANGUAGE )
                    || ( _nSlotId == SID_ATTR_CHAR_CONTOUR )
                    || ( _nSlotId == SID_ATTR_CHAR_SHADOWED )
                    || ( _nSlotId == SID_ATTR_CHAR_WORDLINEMODE )
                    || ( _nSlotId == SID_ATTR_CHAR_COLOR )
                    || ( _nSlotId == SID_ATTR_CHAR_RELIEF )
                    || ( _nSlotId == SID_ATTR_CHAR_KERNING )
                    || ( _nSlotId == SID_ATTR_CHAR_AUTOKERN )
                    || ( _nSlotId == SID_ATTR_CHAR_SCALEWIDTH )
                    )
                {
                    bNeedParametrizedDispatcher = true;
                }
                else
                {
                    SfxSlotPool& rSlotPool = SfxSlotPool::GetSlotPool();
                    const SfxSlot* pSlot = rSlotPool.GetSlot( _nSlotId );
                    const SfxType* pType = pSlot ? pSlot->GetType() : NULL;
                    if ( pType )
                    {
                        bNeedParametrizedDispatcher = ( pType->nAttribs > 0 );
                    }
                }

                if ( bNeedParametrizedDispatcher )
                {
                #if OSL_DEBUG_LEVEL > 0
                    OString sTrace( "ORichTextPeer::implCreateDispatcher: creating *parametrized* dispatcher for " );
                    sTrace += OString( _rURL.Complete.getStr(), _rURL.Complete.getLength(), RTL_TEXTENCODING_ASCII_US );
                    OSL_TRACE( "%s", sTrace.getStr() );
                #endif
                    pAttributeDispatcher = new OParametrizedAttributeDispatcher( pRichTextControl->getView(), _nSlotId, _rURL, pRichTextControl );
                }
                else
                {
                #if OSL_DEBUG_LEVEL > 0
                    OString sTrace( "ORichTextPeer::implCreateDispatcher: creating *normal* dispatcher for " );
                    sTrace += OString( _rURL.Complete.getStr(), _rURL.Complete.getLength(), RTL_TEXTENCODING_ASCII_US );
                    OSL_TRACE( "%s", sTrace.getStr() );
                #endif
                    pAttributeDispatcher = new OAttributeDispatcher( pRichTextControl->getView(), _nSlotId, _rURL, pRichTextControl );
                }
            }
        #if OSL_DEBUG_LEVEL > 0
            else
            {
                OString sTrace( "ORichTextPeer::implCreateDispatcher: not creating dispatcher (unsupported slot) for " );
                sTrace += OString( _rURL.Complete.getStr(), _rURL.Complete.getLength(), RTL_TEXTENCODING_ASCII_US );
                OSL_TRACE( "%s", sTrace.getStr() );
            }
        #endif
        }
        break;
        }

        SingleAttributeDispatcher xDispatcher( pDispatcher );
        if ( pAttributeDispatcher )
        {
            xDispatcher = SingleAttributeDispatcher( pAttributeDispatcher );
            pRichTextControl->enableAttributeNotification( _nSlotId, pAttributeDispatcher );
        }

        return xDispatcher;
    }


    namespace
    {
        SfxSlotId lcl_getSlotFromUnoName( SfxSlotPool& _rSlotPool, const OUString& _rUnoSlotName )
        {
            const SfxSlot* pSlot = _rSlotPool.GetUnoSlot( _rUnoSlotName );
            if ( pSlot )
            {
                // okay, there's a slot with the given UNO name
                return lcl_translateConflictingSlot( pSlot->GetSlotId() );
            }

            // some hard-coded slots, which do not have a UNO name at SFX level, but which
            // we nevertheless need to transport via UNO mechanisms, so we need a name
            if ( _rUnoSlotName == "AllowHangingPunctuation" )
                return SID_ATTR_PARA_HANGPUNCTUATION;
            if ( _rUnoSlotName == "ApplyForbiddenCharacterRules" )
                return SID_ATTR_PARA_FORBIDDEN_RULES;
            if ( _rUnoSlotName == "UseScriptSpacing" )
                return SID_ATTR_PARA_SCRIPTSPACE;

            OSL_ENSURE( pSlot, "lcl_getSlotFromUnoName: unknown UNO slot name!" );
            return 0;
        }
    }


    Reference< XDispatch > SAL_CALL ORichTextPeer::queryDispatch( const css::util::URL& _rURL, const OUString& /*_rTargetFrameName*/, sal_Int32 /*_nSearchFlags*/ ) throw (RuntimeException, std::exception)
    {
        Reference< XDispatch > xReturn;
        if ( !GetWindow() )
        {
            OSL_FAIL( "ORichTextPeer::queryDispatch: already disposed?" );
            return xReturn;
        }

        // is it an UNO slot?
        OUString sUnoProtocolPrefix( ".uno:" );
        if ( _rURL.Complete.startsWith( sUnoProtocolPrefix ) )
        {
            OUString sUnoSlotName = _rURL.Complete.copy( sUnoProtocolPrefix.getLength() );
            SfxSlotId nSlotId = lcl_getSlotFromUnoName( SfxSlotPool::GetSlotPool(), sUnoSlotName );
            if ( nSlotId > 0 )
            {
                // do we already have a dispatcher for this?
                AttributeDispatchers::const_iterator aDispatcherPos = m_aDispatchers.find( nSlotId );
                if ( aDispatcherPos == m_aDispatchers.end() )
                {
                    SingleAttributeDispatcher pDispatcher = implCreateDispatcher( nSlotId, _rURL );
                    if ( pDispatcher.is() )
                    {
                        aDispatcherPos = m_aDispatchers.insert( AttributeDispatchers::value_type( nSlotId, pDispatcher ) ).first;
                    }
                }

                if ( aDispatcherPos != m_aDispatchers.end() )
                    xReturn = aDispatcherPos->second.get();
            }
        }

        return xReturn;
    }


    Sequence< Reference< XDispatch > > SAL_CALL ORichTextPeer::queryDispatches( const Sequence< DispatchDescriptor >& _rRequests ) throw (RuntimeException, std::exception)
    {
        Sequence< Reference< XDispatch > >  aReturn( _rRequests.getLength() );
        Reference< XDispatch >*             pReturn = aReturn.getArray();

        const DispatchDescriptor* pRequest = _rRequests.getConstArray();
        const DispatchDescriptor* pRequestEnd = pRequest + _rRequests.getLength();
        for ( ; pRequest != pRequestEnd; ++pRequest, ++pReturn )
        {
            *pReturn = queryDispatch( pRequest->FeatureURL, pRequest->FrameName, pRequest->SearchFlags );
        }
        return aReturn;
    }


    void ORichTextPeer::onSelectionChanged( const ESelection& /*_rSelection*/ )
    {
        AttributeDispatchers::iterator aDispatcherPos = m_aDispatchers.find( SID_COPY );
        if ( aDispatcherPos != m_aDispatchers.end() )
            aDispatcherPos->second.get()->invalidate();

        aDispatcherPos = m_aDispatchers.find( SID_CUT );
        if ( aDispatcherPos != m_aDispatchers.end() )
            aDispatcherPos->second.get()->invalidate();
    }


}   // namespace frm

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_form_ORichTextControl_get_implementation(css::uno::XComponentContext*,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::ORichTextControl());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
