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
#include "precompiled_forms.hxx"
#include "richtextcontrol.hxx"
#include "frm_module.hxx"
#include "property.hrc"
#include "services.hxx"

#include "richtextmodel.hxx"
#include "richtextvclcontrol.hxx"
#include "clipboarddispatcher.hxx"
#include "parametrizedattributedispatcher.hxx"
#include "specialdispatchers.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/awt/PosSize.hpp>
/** === end UNO includes === **/

#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>

#include <svx/svxids.hrc>
#include <editeng/editview.hxx>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <sfx2/msgpool.hxx>

//--------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_ORichTextControl()
{
    static ::frm::OMultiInstanceAutoRegistration< ::frm::ORichTextControl > aAutoRegistration;
}

//.........................................................................
namespace frm
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::frame;

#define FORWARD_TO_PEER_1( unoInterface, method, param1 )   \
    Reference< unoInterface > xTypedPeer( getPeer(), UNO_QUERY );   \
    if ( xTypedPeer.is() )  \
    {   \
        xTypedPeer->method( param1 );  \
    }

#define FORWARD_TO_PEER_1_RET( returnType, unoInterface, method, param1 )   \
    returnType aReturn; \
    Reference< unoInterface > xTypedPeer( getPeer(), UNO_QUERY );   \
    if ( xTypedPeer.is() )  \
    {   \
        aReturn = xTypedPeer->method( param1 );  \
    }   \
    return aReturn;

#define FORWARD_TO_PEER_3( unoInterface, method, param1, param2, param3 )   \
    Reference< unoInterface > xTypedPeer( getPeer(), UNO_QUERY );   \
    if ( xTypedPeer.is() )  \
    {   \
        xTypedPeer->method( param1, param2, param3 );  \
    }

#define FORWARD_TO_PEER_3_RET( returnType, unoInterface, method, param1, param2, param3 )   \
    returnType aReturn; \
    Reference< unoInterface > xTypedPeer( getPeer(), UNO_QUERY );   \
    if ( xTypedPeer.is() )  \
    {   \
        aReturn = xTypedPeer->method( param1, param2, param3 );  \
    }   \
    return aReturn;

    //==================================================================
    // ORichTextControl
    //==================================================================
    DBG_NAME( ORichTextControl )
    //------------------------------------------------------------------
    ORichTextControl::ORichTextControl( const Reference< XMultiServiceFactory >& _rxORB )
        :m_xORB( _rxORB )
    {
        DBG_CTOR( ORichTextControl, NULL );
    }

    //------------------------------------------------------------------
    ORichTextControl::~ORichTextControl()
    {
        DBG_DTOR( ORichTextControl, NULL );
    }

    //------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( ORichTextControl, UnoEditControl, ORichTextControl_Base )

    //------------------------------------------------------------------
    Any SAL_CALL ORichTextControl::queryAggregation( const Type& _rType ) throw ( RuntimeException )
    {
        Any aReturn = UnoEditControl::queryAggregation( _rType );

        if ( !aReturn.hasValue() )
            aReturn = ORichTextControl_Base::queryInterface( _rType );

        return aReturn;
    }

    //------------------------------------------------------------------
    namespace
    {
        //..............................................................
        static void implAdjustTriStateFlag( const Reference< XPropertySet >& _rxProps, const ::rtl::OUString& _rPropertyName,
            WinBits& _rAllBits, WinBits _nPositiveFlag, WinBits nNegativeFlag )
        {
            sal_Bool bFlagValue = sal_False;
            if ( _rxProps->getPropertyValue( _rPropertyName ) >>= bFlagValue )
                _rAllBits |= ( bFlagValue ? _nPositiveFlag : nNegativeFlag );
        }

        //..............................................................
        static void implAdjustTwoStateFlag( const Any& _rValue, WinBits& _rAllBits, WinBits _nFlag, bool _bInvert = false )
        {
            sal_Bool bFlagValue = sal_False;
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

        //..............................................................
        static void implAdjustTwoStateFlag( const Reference< XPropertySet >& _rxProps, const ::rtl::OUString& _rPropertyName,
            WinBits& _rAllBits, WinBits _nFlag, bool _bInvert = false )
        {
            implAdjustTwoStateFlag( _rxProps->getPropertyValue( _rPropertyName ), _rAllBits, _nFlag, _bInvert );
        }

        //..............................................................
        static void adjustTwoStateWinBit( Window* _pWindow, const Any& _rValue, WinBits _nFlag, bool _bInvert = false )
        {
            WinBits nBits = _pWindow->GetStyle();
            implAdjustTwoStateFlag( _rValue, nBits, _nFlag, _bInvert );
            _pWindow->SetStyle( nBits );
        }

        //..............................................................
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

    //------------------------------------------------------------------
    void SAL_CALL ORichTextControl::createPeer( const Reference< XToolkit >& _rToolkit, const Reference< XWindowPeer >& _rParentPeer ) throw( RuntimeException )
    {
        sal_Bool bReallyActAsRichText = sal_False;
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
            OControl::initFormControlPeer( getPeer() );
            return;
        }

        SolarMutexGuard aGuard;

        if (!getPeer().is())
        {
            mbCreatingPeer = sal_True;

            // determine the VLC window for the parent
            Window* pParentWin = NULL;
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
                // by definition, the returned component is aquired once
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

            mbCreatingPeer = sal_False;

            OControl::initFormControlPeer( getPeer() );
        }
    }

    //------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ORichTextControl::getImplementationName()  throw( RuntimeException )
    {
        return getImplementationName_Static();
    }

    //------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ORichTextControl::getSupportedServiceNames()  throw( RuntimeException )
    {
        return getSupportedServiceNames_Static();
    }

    //------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ORichTextControl::getImplementationName_Static()
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.form.ORichTextControl" ) );
    }

    //------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ORichTextControl::getSupportedServiceNames_Static()
    {
        Sequence< ::rtl::OUString > aServices( 3 );
        aServices[ 0 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControl" ) );
        aServices[ 1 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlEdit" ) );
        aServices[ 2 ] = FRM_SUN_CONTROL_RICHTEXTCONTROL;
        return aServices;
    }

    //------------------------------------------------------------------
    Reference< XInterface > SAL_CALL ORichTextControl::Create( const Reference< XMultiServiceFactory >& _rxFactory )
    {
        return *( new ORichTextControl( _rxFactory ) );
    }

    //--------------------------------------------------------------------
    Reference< XDispatch > SAL_CALL ORichTextControl::queryDispatch( const ::com::sun::star::util::URL& _rURL, const ::rtl::OUString& _rTargetFrameName, sal_Int32 _nSearchFlags ) throw (RuntimeException)
    {
        FORWARD_TO_PEER_3_RET( Reference< XDispatch >, XDispatchProvider, queryDispatch, _rURL, _rTargetFrameName, _nSearchFlags );
    }

    //--------------------------------------------------------------------
    Sequence< Reference< XDispatch > > SAL_CALL ORichTextControl::queryDispatches( const Sequence< DispatchDescriptor >& _rRequests ) throw (RuntimeException)
    {
        FORWARD_TO_PEER_1_RET( Sequence< Reference< XDispatch > >, XDispatchProvider, queryDispatches, _rRequests );
    }

    //--------------------------------------------------------------------
    sal_Bool ORichTextControl::requiresNewPeer( const ::rtl::OUString& _rPropertyName ) const
    {
        return UnoControl::requiresNewPeer( _rPropertyName ) || _rPropertyName.equals( PROPERTY_RICH_TEXT );
    }

    //==================================================================
    // ORichTextPeer
    //==================================================================
    DBG_NAME( ORichTextPeer )
    //------------------------------------------------------------------
    ORichTextPeer* ORichTextPeer::Create( const Reference< XControlModel >& _rxModel, Window* _pParentWindow, WinBits _nStyle )
    {
        DBG_TESTSOLARMUTEX();

        // the EditEngine of the model
        RichTextEngine* pEngine = ORichTextModel::getEditEngine( _rxModel );
        OSL_ENSURE( pEngine, "ORichTextPeer::Create: could not obtaine the edit engine from the model!" );
        if ( !pEngine )
            return NULL;

        // the peer itself
        ORichTextPeer* pPeer = new ORichTextPeer;
        pPeer->acquire();   // by definition, the returned object is aquired once

        // the VCL control for the peer
        RichTextControl* pRichTextControl = new RichTextControl( pEngine, _pParentWindow, _nStyle, NULL, pPeer );

        // some knittings
        pRichTextControl->SetComponentInterface( pPeer );

        // outta here
        return pPeer;
    }

    //------------------------------------------------------------------
    ORichTextPeer::ORichTextPeer()
    {
        DBG_CTOR( ORichTextPeer, NULL );
    }

    //------------------------------------------------------------------
    ORichTextPeer::~ORichTextPeer()
    {
        DBG_DTOR( ORichTextPeer, NULL );
    }

    //------------------------------------------------------------------
    void ORichTextPeer::dispose( ) throw(RuntimeException)
    {
        {
            SolarMutexGuard aGuard;
            RichTextControl* pRichTextControl = static_cast< RichTextControl* >( GetWindow() );

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

    //--------------------------------------------------------------------
    void SAL_CALL ORichTextPeer::draw( sal_Int32 _nX, sal_Int32 _nY ) throw(::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        RichTextControl* pControl = static_cast< RichTextControl* >( GetWindow() );
        if ( !pControl )
            return;

        OutputDevice* pTargetDevice = VCLUnoHelper::GetOutputDevice( getGraphics() );
        OSL_ENSURE( pTargetDevice != NULL, "ORichTextPeer::draw: no graphics -> no drawing!" );
        if ( !pTargetDevice )
            return;

        ::Size aSize = pControl->GetSizePixel();
        const MapUnit eTargetUnit = pTargetDevice->GetMapMode().GetMapUnit();
        if ( eTargetUnit != MAP_PIXEL )
            aSize = pControl->PixelToLogic( aSize, eTargetUnit );

        ::Point aPos( _nX, _nY );
        // the XView::draw API talks about pixels, always ...
        if ( eTargetUnit != MAP_PIXEL )
            aPos = pTargetDevice->PixelToLogic( aPos );

        pControl->Draw( pTargetDevice, aPos, aSize, WINDOW_DRAW_NOCONTROLS );
    }

    //--------------------------------------------------------------------
    void SAL_CALL ORichTextPeer::setProperty( const ::rtl::OUString& _rPropertyName, const Any& _rValue ) throw (RuntimeException)
    {
        if ( !GetWindow() )
        {
            VCLXWindow::setProperty( _rPropertyName, _rValue );
            return;
        }

        if ( _rPropertyName.equals( PROPERTY_BACKGROUNDCOLOR ) )
        {
            RichTextControl* pControl = static_cast< RichTextControl* >( GetWindow() );
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
        else if (  _rPropertyName.equals( PROPERTY_HSCROLL ) )
        {
            adjustTwoStateWinBit( GetWindow(), _rValue, WB_HSCROLL );
        }
        else if ( _rPropertyName.equals( PROPERTY_VSCROLL ) )
        {
            adjustTwoStateWinBit( GetWindow(), _rValue, WB_VSCROLL );
        }
        else if ( _rPropertyName.equals( PROPERTY_HARDLINEBREAKS ) )
        {
            adjustTwoStateWinBit( GetWindow(), _rValue, WB_WORDBREAK, true );
        }
        else if ( _rPropertyName.equals( PROPERTY_READONLY ) )
        {
            RichTextControl* pControl = static_cast< RichTextControl* >( GetWindow() );
            sal_Bool bReadOnly( pControl->IsReadOnly() );
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
        else if ( _rPropertyName.equals( PROPERTY_HIDEINACTIVESELECTION ) )
        {
            RichTextControl* pRichTextControl = static_cast< RichTextControl* >( GetWindow() );
            sal_Bool bHide = pRichTextControl->GetHideInactiveSelection();
            OSL_VERIFY( _rValue >>= bHide );
            pRichTextControl->SetHideInactiveSelection( bHide );
        }
        else
            VCLXWindow::setProperty( _rPropertyName, _rValue );
    }

    //------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( ORichTextPeer, VCLXWindow, ORichTextPeer_Base )

    //------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( ORichTextPeer, VCLXWindow, ORichTextPeer_Base )

    //--------------------------------------------------------------------
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
            //
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

    //--------------------------------------------------------------------
    ORichTextPeer::SingleAttributeDispatcher ORichTextPeer::implCreateDispatcher( SfxSlotId _nSlotId, const ::com::sun::star::util::URL& _rURL )
    {
        RichTextControl* pRichTextControl = static_cast< RichTextControl* >( GetWindow() );
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
                else if (  ( _nSlotId == SID_ATTR_PARA_HANGPUNCTUATION )
                        || ( _nSlotId == SID_ATTR_PARA_FORBIDDEN_RULES )
                        || ( _nSlotId == SID_ATTR_PARA_SCRIPTSPACE )
                        )
                {
                    bNeedParametrizedDispatcher = false;
                }
                else
                {
                    SfxSlotPool& rSlotPool = SfxSlotPool::GetSlotPool( NULL );
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
                    ::rtl::OString sTrace( "ORichTextPeer::implCreateDispatcher: creating *parametrized* dispatcher for " );
                    sTrace += ::rtl::OString( _rURL.Complete.getStr(), _rURL.Complete.getLength(), RTL_TEXTENCODING_ASCII_US );
                    OSL_TRACE( sTrace.getStr() );
                #endif
                    pAttributeDispatcher = new OParametrizedAttributeDispatcher( pRichTextControl->getView(), _nSlotId, _rURL, pRichTextControl );
                }
                else
                {
                #if OSL_DEBUG_LEVEL > 0
                    ::rtl::OString sTrace( "ORichTextPeer::implCreateDispatcher: creating *normal* dispatcher for " );
                    sTrace += ::rtl::OString( _rURL.Complete.getStr(), _rURL.Complete.getLength(), RTL_TEXTENCODING_ASCII_US );
                    OSL_TRACE( sTrace.getStr() );
                #endif
                    pAttributeDispatcher = new OAttributeDispatcher( pRichTextControl->getView(), _nSlotId, _rURL, pRichTextControl );
                }
            }
        #if OSL_DEBUG_LEVEL > 0
            else
            {
                ::rtl::OString sTrace( "ORichTextPeer::implCreateDispatcher: not creating dispatcher (unsupported slot) for " );
                sTrace += ::rtl::OString( _rURL.Complete.getStr(), _rURL.Complete.getLength(), RTL_TEXTENCODING_ASCII_US );
                OSL_TRACE( sTrace.getStr() );
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

    //--------------------------------------------------------------------
    namespace
    {
        SfxSlotId lcl_getSlotFromUnoName( SfxSlotPool& _rSlotPool, const ::rtl::OUString& _rUnoSlotName )
        {
            const SfxSlot* pSlot = _rSlotPool.GetUnoSlot( _rUnoSlotName );
            if ( pSlot )
            {
                // okay, there's a slot with the given UNO name
                return lcl_translateConflictingSlot( pSlot->GetSlotId() );
            }

            // some hard-coded slots, which do not have a UNO name at SFX level, but which
            // we nevertheless need to transport via UNO mechanisms, so we need a name
            if ( _rUnoSlotName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "AllowHangingPunctuation" ) ) )
                return SID_ATTR_PARA_HANGPUNCTUATION;
            if ( _rUnoSlotName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ApplyForbiddenCharacterRules" ) ) )
                return SID_ATTR_PARA_FORBIDDEN_RULES;
            if ( _rUnoSlotName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "UseScriptSpacing" ) ) )
                return SID_ATTR_PARA_SCRIPTSPACE;

            OSL_ENSURE( pSlot, "lcl_getSlotFromUnoName: unknown UNO slot name!" );
            return 0;
        }
    }

    //--------------------------------------------------------------------
    Reference< XDispatch > SAL_CALL ORichTextPeer::queryDispatch( const ::com::sun::star::util::URL& _rURL, const ::rtl::OUString& /*_rTargetFrameName*/, sal_Int32 /*_nSearchFlags*/ ) throw (RuntimeException)
    {
        Reference< XDispatch > xReturn;
        if ( !GetWindow() )
        {
            OSL_ENSURE( sal_False, "ORichTextPeer::queryDispatch: already disposed?" );
            return xReturn;
        }

        // is it an UNO slot?
        ::rtl::OUString sUnoProtocolPrefix( RTL_CONSTASCII_USTRINGPARAM( ".uno:" ) );
        if ( 0 == _rURL.Complete.compareTo( sUnoProtocolPrefix, sUnoProtocolPrefix.getLength() ) )
        {
            ::rtl::OUString sUnoSlotName = _rURL.Complete.copy( sUnoProtocolPrefix.getLength() );
            SfxSlotId nSlotId = lcl_getSlotFromUnoName( SfxSlotPool::GetSlotPool( NULL ), sUnoSlotName );
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
                    xReturn = aDispatcherPos->second.getRef();
            }
        }

        return xReturn;
    }

    //--------------------------------------------------------------------
    Sequence< Reference< XDispatch > > SAL_CALL ORichTextPeer::queryDispatches( const Sequence< DispatchDescriptor >& _rRequests ) throw (RuntimeException)
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

    //--------------------------------------------------------------------
    void ORichTextPeer::onSelectionChanged( const ESelection& /*_rSelection*/ )
    {
        AttributeDispatchers::iterator aDispatcherPos = m_aDispatchers.find( SID_COPY );
        if ( aDispatcherPos != m_aDispatchers.end() )
            aDispatcherPos->second.get()->invalidate();

        aDispatcherPos = m_aDispatchers.find( SID_CUT );
        if ( aDispatcherPos != m_aDispatchers.end() )
            aDispatcherPos->second.get()->invalidate();
    }

//........................................................................
}   // namespace frm
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
