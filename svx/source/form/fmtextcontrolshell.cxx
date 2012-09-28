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


#include "fmprop.hrc"
#include "svx/fmresids.hrc"
#include "fmtextcontroldialogs.hxx"
#include "fmtextcontrolfeature.hxx"
#include "fmtextcontrolshell.hxx"
#include "editeng/crsditem.hxx"
#include "svx/dialmgr.hxx"
#include "editeng/editeng.hxx"
#include "editeng/eeitem.hxx"
#include "svx/fmglob.hxx"
#include "editeng/scriptspaceitem.hxx"
#include "svx/svxids.hrc"
#include "editeng/udlnitem.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <comphelper/componentcontext.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxuno.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/itempool.hxx>
#include <svl/languageoptions.hxx>
#include <svtools/stringtransfer.hxx>
#include <svl/whiter.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/msgbox.hxx>
#include <vcl/outdev.hxx>
#include <osl/mutex.hxx>

#include <memory>

//........................................................................
namespace svx
{
//........................................................................

    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::form::runtime;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;

    //====================================================================
    typedef sal_uInt16 WhichId;

    //====================================================================
    static SfxSlotId pTextControlSlots[] =
    {
        SID_CLIPBOARD_FORMAT_ITEMS,
        SID_CUT,
        SID_COPY,
        SID_PASTE,
        SID_SELECTALL,
//        SID_ATTR_TABSTOP,           /* 2 */
        SID_ATTR_CHAR_FONT,
        SID_ATTR_CHAR_POSTURE,
        SID_ATTR_CHAR_WEIGHT,
        SID_ATTR_CHAR_SHADOWED,
        SID_ATTR_CHAR_WORDLINEMODE,
        SID_ATTR_CHAR_CONTOUR,
        SID_ATTR_CHAR_STRIKEOUT,
        SID_ATTR_CHAR_UNDERLINE,
        SID_ATTR_CHAR_FONTHEIGHT,
        SID_ATTR_CHAR_COLOR,
        SID_ATTR_CHAR_KERNING,
        SID_ATTR_CHAR_LANGUAGE,     /* 20 */
        SID_ATTR_CHAR_ESCAPEMENT,
        SID_ATTR_PARA_ADJUST,       /* 28 */
        SID_ATTR_PARA_ADJUST_LEFT,
        SID_ATTR_PARA_ADJUST_RIGHT,
        SID_ATTR_PARA_ADJUST_CENTER,
        SID_ATTR_PARA_ADJUST_BLOCK,
        SID_ATTR_PARA_LINESPACE,    /* 33 */
        SID_ATTR_PARA_LINESPACE_10,
        SID_ATTR_PARA_LINESPACE_15,
        SID_ATTR_PARA_LINESPACE_20,
        SID_ATTR_LRSPACE,           /* 48 */
        SID_ATTR_ULSPACE,           /* 49 */
        SID_ATTR_CHAR_AUTOKERN,
        SID_SET_SUPER_SCRIPT,
        SID_SET_SUB_SCRIPT,
        SID_CHAR_DLG,
        SID_PARA_DLG,
//        SID_TEXTDIRECTION_LEFT_TO_RIGHT, /* 907 */
//        SID_TEXTDIRECTION_TOP_TO_BOTTOM,
        SID_ATTR_CHAR_SCALEWIDTH,       /* 911 */
        SID_ATTR_CHAR_RELIEF,
        SID_ATTR_PARA_LEFT_TO_RIGHT,    /* 950 */
        SID_ATTR_PARA_RIGHT_TO_LEFT,
        SID_ATTR_CHAR_OVERLINE,
        0
    };

    // slots which we are not responsible for on the SfxShell level, but
    // need to handle during the "paragraph attributes" and/or "character
    // attributes" dialogs
    static SfxSlotId pDialogSlots[] =
    {
        SID_ATTR_TABSTOP,
        SID_ATTR_PARA_HANGPUNCTUATION,
        SID_ATTR_PARA_FORBIDDEN_RULES,
        SID_ATTR_PARA_SCRIPTSPACE,
        SID_ATTR_CHAR_LATIN_LANGUAGE,
        SID_ATTR_CHAR_CJK_LANGUAGE,
        SID_ATTR_CHAR_CTL_LANGUAGE,
        SID_ATTR_CHAR_LATIN_FONT,
        SID_ATTR_CHAR_CJK_FONT,
        SID_ATTR_CHAR_CTL_FONT,
        SID_ATTR_CHAR_LATIN_FONTHEIGHT,
        SID_ATTR_CHAR_CJK_FONTHEIGHT,
        SID_ATTR_CHAR_CTL_FONTHEIGHT,
        SID_ATTR_CHAR_LATIN_WEIGHT,
        SID_ATTR_CHAR_CJK_WEIGHT,
        SID_ATTR_CHAR_CTL_WEIGHT,
        SID_ATTR_CHAR_LATIN_POSTURE,
        SID_ATTR_CHAR_CJK_POSTURE,
        SID_ATTR_CHAR_CTL_POSTURE,
        SID_ATTR_CHAR_EMPHASISMARK,
        0
    };

    //====================================================================
    //= FmFocusListenerAdapter
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   XFocusListener
                                    >   FmFocusListenerAdapter_Base;
    class FmFocusListenerAdapter : public FmFocusListenerAdapter_Base
    {
    private:
        IFocusObserver*         m_pObserver;
        Reference< XWindow >    m_xWindow;

    public:
        FmFocusListenerAdapter( const Reference< XControl >& _rxControl, IFocusObserver* _pObserver );

        // clean up the instance
        void    dispose();

    protected:
        ~FmFocusListenerAdapter();

    protected:
        virtual void SAL_CALL focusGained( const FocusEvent& e ) throw (RuntimeException);
        virtual void SAL_CALL focusLost( const FocusEvent& e ) throw (RuntimeException);
        virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException);
    };

    //--------------------------------------------------------------------
    DBG_NAME( FmFocusListenerAdapter )
    //--------------------------------------------------------------------
    FmFocusListenerAdapter::FmFocusListenerAdapter( const Reference< XControl >& _rxControl, IFocusObserver* _pObserver )
        :m_pObserver( _pObserver )
        ,m_xWindow( _rxControl, UNO_QUERY )
    {
        DBG_CTOR( FmFocusListenerAdapter, NULL );

        DBG_ASSERT( m_xWindow.is(), "FmFocusListenerAdapter::FmFocusListenerAdapter: invalid control!" );
        osl_atomic_increment( &m_refCount );
        {
            try
            {
                if ( m_xWindow.is() )
                    m_xWindow->addFocusListener( this );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        osl_atomic_decrement( &m_refCount );
    }

    //--------------------------------------------------------------------
    FmFocusListenerAdapter::~FmFocusListenerAdapter()
    {
        acquire();
        dispose();

        DBG_DTOR( FmFocusListenerAdapter, NULL );
    }

    //--------------------------------------------------------------------
    void FmFocusListenerAdapter::dispose()
    {
        if ( m_xWindow.is() )
        {
            m_xWindow->removeFocusListener( this );
            m_xWindow.clear();
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL FmFocusListenerAdapter::focusGained( const FocusEvent& e ) throw (RuntimeException)
    {
        if ( m_pObserver )
            m_pObserver->focusGained( e );
    }

    //--------------------------------------------------------------------
    void SAL_CALL FmFocusListenerAdapter::focusLost( const FocusEvent& e ) throw (RuntimeException)
    {
        if ( m_pObserver )
            m_pObserver->focusLost( e );
    }

    //--------------------------------------------------------------------
    void SAL_CALL FmFocusListenerAdapter::disposing( const EventObject& Source ) throw (RuntimeException)
    {
        (void)Source;
        DBG_ASSERT( Source.Source == m_xWindow, "FmFocusListenerAdapter::disposing: where did this come from?" );
        m_xWindow.clear();
    }

    //====================================================================
    //= FmMouseListenerAdapter
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   XMouseListener
                                    >   FmMouseListenerAdapter_Base;
    class FmMouseListenerAdapter : public FmMouseListenerAdapter_Base
    {
    private:
        IContextRequestObserver*  m_pObserver;
        Reference< XWindow >    m_xWindow;

    public:
        FmMouseListenerAdapter( const Reference< XControl >& _rxControl, IContextRequestObserver* _pObserver );

        // clean up the instance
        void    dispose();

    protected:
        ~FmMouseListenerAdapter();

    protected:
        virtual void SAL_CALL mousePressed( const awt::MouseEvent& e ) throw (RuntimeException);
        virtual void SAL_CALL mouseReleased( const awt::MouseEvent& e ) throw (RuntimeException);
        virtual void SAL_CALL mouseEntered( const awt::MouseEvent& e ) throw (RuntimeException);
        virtual void SAL_CALL mouseExited( const awt::MouseEvent& e ) throw (RuntimeException);
        virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException);
    };

    //====================================================================
    //= FmMouseListenerAdapter
    //====================================================================
    //--------------------------------------------------------------------
    DBG_NAME( FmMouseListenerAdapter )
    //--------------------------------------------------------------------
    FmMouseListenerAdapter::FmMouseListenerAdapter( const Reference< XControl >& _rxControl, IContextRequestObserver* _pObserver )
        :m_pObserver( _pObserver )
        ,m_xWindow( _rxControl, UNO_QUERY )
    {
        DBG_CTOR( FmMouseListenerAdapter, NULL );

        DBG_ASSERT( m_xWindow.is(), "FmMouseListenerAdapter::FmMouseListenerAdapter: invalid control!" );
        osl_atomic_increment( &m_refCount );
        {
            try
            {
                if ( m_xWindow.is() )
                    m_xWindow->addMouseListener( this );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        osl_atomic_decrement( &m_refCount );
    }

    //--------------------------------------------------------------------
    FmMouseListenerAdapter::~FmMouseListenerAdapter()
    {
        acquire();
        dispose();

        DBG_DTOR( FmMouseListenerAdapter, NULL );
    }

    //--------------------------------------------------------------------
    void FmMouseListenerAdapter::dispose()
    {
        if ( m_xWindow.is() )
        {
            m_xWindow->removeMouseListener( this );
            m_xWindow.clear();
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL FmMouseListenerAdapter::mousePressed( const awt::MouseEvent& _rEvent ) throw (::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aGuard;
        // is this a request for a context menu?
        if ( _rEvent.PopupTrigger )
        {
            if ( m_pObserver )
                m_pObserver->contextMenuRequested( _rEvent );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL FmMouseListenerAdapter::mouseReleased( const awt::MouseEvent& /*e*/ ) throw (::com::sun::star::uno::RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void SAL_CALL FmMouseListenerAdapter::mouseEntered( const awt::MouseEvent& /*e*/ ) throw (::com::sun::star::uno::RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void SAL_CALL FmMouseListenerAdapter::mouseExited( const awt::MouseEvent& /*e*/ ) throw (::com::sun::star::uno::RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void SAL_CALL FmMouseListenerAdapter::disposing( const EventObject& Source ) throw (RuntimeException)
    {
        (void)Source;
        DBG_ASSERT( Source.Source == m_xWindow, "FmMouseListenerAdapter::disposing: where did this come from?" );
        m_xWindow.clear();
    }

    //====================================================================
    //= FmTextControlShell
    //====================================================================
    //------------------------------------------------------------------------
    namespace
    {
        //....................................................................
        void lcl_translateUnoStateToItem( SfxSlotId _nSlot, const Any& _rUnoState, SfxItemSet& _rSet )
        {
            WhichId nWhich = _rSet.GetPool()->GetWhich( _nSlot );
            if ( !_rUnoState.hasValue() )
            {
                if  ( ( _nSlot != SID_CUT )
                   && ( _nSlot != SID_COPY )
                   && ( _nSlot != SID_PASTE )
                    )
                {
                    _rSet.InvalidateItem( nWhich );
                }
            }
            else
            {
                switch ( _rUnoState.getValueType().getTypeClass() )
                {
                case TypeClass_BOOLEAN:
                {
                    sal_Bool bState = sal_False;
                    _rUnoState >>= bState;
                    if ( _nSlot == SID_ATTR_PARA_SCRIPTSPACE )
                        _rSet.Put( SvxScriptSpaceItem( bState, nWhich ) );
                    else
                        _rSet.Put( SfxBoolItem( nWhich, bState ) );
                }
                break;

                default:
                {
                    Sequence< PropertyValue > aComplexState;
                    if ( _rUnoState >>= aComplexState )
                    {
                        if ( !aComplexState.getLength() )
                            _rSet.InvalidateItem( nWhich );
                        else
                        {
                            SfxAllItemSet aAllItems( _rSet );
                            TransformParameters( _nSlot, aComplexState, aAllItems );
                            const SfxPoolItem* pTransformed = aAllItems.GetItem( nWhich );
                            OSL_ENSURE( pTransformed, "lcl_translateUnoStateToItem: non-empty parameter sequence leading to empty item?" );
                            if ( pTransformed )
                                _rSet.Put( *pTransformed );
                            else
                                _rSet.InvalidateItem( nWhich );
                        }
                    }
                    else
                    {
                        OSL_FAIL( "lcl_translateUnoStateToItem: invalid state!" );
                    }
                }
                }
            }
        }

        //....................................................................
        ::rtl::OUString lcl_getUnoSlotName( SfxApplication&, SfxSlotId _nSlotId )
        {
            ::rtl::OUString sSlotUnoName;

            SfxSlotPool& rSlotPool = SfxSlotPool::GetSlotPool( NULL );
            const SfxSlot* pSlot = rSlotPool.GetSlot( _nSlotId );

            const sal_Char* pAsciiUnoName = NULL;
            if ( pSlot )
            {
                pAsciiUnoName = pSlot->GetUnoName();
            }
            else
            {
                // some hard-coded slots, which do not have a UNO name at SFX level, but which
                // we nevertheless need to transport via UNO mechanisms, so we need a name
                switch ( _nSlotId )
                {
                case SID_ATTR_PARA_HANGPUNCTUATION: pAsciiUnoName = "AllowHangingPunctuation"; break;
                case SID_ATTR_PARA_FORBIDDEN_RULES: pAsciiUnoName = "ApplyForbiddenCharacterRules"; break;
                case SID_ATTR_PARA_SCRIPTSPACE: pAsciiUnoName = "UseScriptSpacing"; break;
                }
            }

            if ( pAsciiUnoName )
            {
                sSlotUnoName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:" ) );
                sSlotUnoName += ::rtl::OUString::createFromAscii( pAsciiUnoName );
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                ::rtl::OString sMessage( "lcl_getUnoSlotName: invalid slot id, or invalid slot, or no UNO name!\n" );
                sMessage += "(slot id: ";
                sMessage += ::rtl::OString::valueOf( (sal_Int32)_nSlotId );
                sMessage += ")";
                OSL_FAIL( sMessage.getStr() );
            }
#endif
            return sSlotUnoName;
        }

        //....................................................................
        bool lcl_determineReadOnly( const Reference< XControl >& _rxControl )
        {
            bool bIsReadOnlyModel = true;
            try
            {
                Reference< XPropertySet > xModelProps;
                if ( _rxControl.is() )
                    xModelProps = xModelProps.query( _rxControl->getModel() );
                Reference< XPropertySetInfo > xModelPropInfo;
                if ( xModelProps.is() )
                    xModelPropInfo = xModelProps->getPropertySetInfo();

                if ( !xModelPropInfo.is() || !xModelPropInfo->hasPropertyByName( FM_PROP_READONLY ) )
                    bIsReadOnlyModel = true;
                else
                {
                    sal_Bool bReadOnly = sal_True;
                    xModelProps->getPropertyValue( FM_PROP_READONLY ) >>= bReadOnly;
                    bIsReadOnlyModel = bReadOnly;
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return bIsReadOnlyModel;
        }

        //....................................................................
        static Window* lcl_getWindow( const Reference< XControl >& _rxControl )
        {
            Window* pWindow = NULL;
            try
            {
                Reference< XWindowPeer > xControlPeer;
                if ( _rxControl.is() )
                    xControlPeer = _rxControl->getPeer();
                if ( xControlPeer.is() )
                    pWindow = VCLUnoHelper::GetWindow( xControlPeer );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }

            return pWindow;
        }

        //....................................................................
        bool lcl_isRichText( const Reference< XControl >& _rxControl )
        {
            if ( !_rxControl.is() )
                return false;

            bool bIsRichText = false;
            try
            {
                Reference< XPropertySet > xModelProps( _rxControl->getModel(), UNO_QUERY );
                Reference< XPropertySetInfo > xPSI;
                if ( xModelProps.is() )
                    xPSI = xModelProps->getPropertySetInfo();
                ::rtl::OUString sRichTextPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "RichText" ) );
                if ( xPSI.is() && xPSI->hasPropertyByName( sRichTextPropertyName ) )
                {
                    OSL_VERIFY( xModelProps->getPropertyValue( sRichTextPropertyName ) >>= bIsRichText );
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return bIsRichText;
        }
    }

    //------------------------------------------------------------------------
    FmTextControlShell::FmTextControlShell( SfxViewFrame* _pFrame )
        :m_bActiveControl( false )
        ,m_bActiveControlIsReadOnly( true )
        ,m_bActiveControlIsRichText( false )
        ,m_pViewFrame( _pFrame )
        ,m_rBindings( _pFrame->GetBindings() )
        ,m_bNeedClipboardInvalidation( true )
    {
        m_aClipboardInvalidation.SetTimeoutHdl( LINK( this, FmTextControlShell, OnInvalidateClipboard ) );
        m_aClipboardInvalidation.SetTimeout( 200 );
    }

    //------------------------------------------------------------------------
    FmTextControlShell::~FmTextControlShell()
    {
        dispose();
    }

    //------------------------------------------------------------------------
    IMPL_LINK( FmTextControlShell, OnInvalidateClipboard, void*, /*_pNotInterestedIn*/ )
    {
        if ( m_bNeedClipboardInvalidation )
        {
            OSL_TRACE( "FmTextControlShell::ClipBoard: invalidating clipboard slots" );
            m_rBindings.Invalidate( SID_CUT );
            m_rBindings.Invalidate( SID_COPY );
            m_rBindings.Invalidate( SID_PASTE );
            m_bNeedClipboardInvalidation = false;
        }
        return 0L;
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::transferFeatureStatesToItemSet( ControlFeatures& _rDispatchers, SfxAllItemSet& _rSet, bool _bTranslateLatin )
    {
        SfxItemPool& rPool = *_rSet.GetPool();

        for (   ControlFeatures::const_iterator aFeature = _rDispatchers.begin();
                aFeature != _rDispatchers.end();
                ++aFeature
            )
        {
            SfxSlotId nSlotId( aFeature->first );
            #if OSL_DEBUG_LEVEL > 0
                ::rtl::OUString sUnoSlotName;
                if ( SFX_APP() )
                    sUnoSlotName = lcl_getUnoSlotName( *SFX_APP(), nSlotId );
                else
                    sUnoSlotName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "<unknown>" ) );
                ::rtl::OString sUnoSlotNameAscii( "\"" );
                sUnoSlotNameAscii += ::rtl::OString( sUnoSlotName.getStr(), sUnoSlotName.getLength(), RTL_TEXTENCODING_ASCII_US );
                sUnoSlotNameAscii += "\"";
            #endif

            if ( _bTranslateLatin )
            {
                // A rich text control offers a dispatcher for the "Font" slot/feature.
                // Sadly, the semantics of the dispatches is that the feature "Font" depends
                // on the current cursor position: If it's on latin text, it's the "latin font"
                // which is set up at the control. If it's on CJK text, it's the "CJK font", and
                // aequivalent for "CTL font".
                // The same holds for some other font related features/slots.
                // Thus, we have separate dispatches for "Latin Font", "Latin Font Size", etc,
                // which are only "virtual", in a sense that there exist no item with this id.
                // So when we encounter such a dispatcher for, say, "Latin Font", we need to
                // put an item into the set which has the "Font" id.

                switch ( nSlotId )
                {
                case SID_ATTR_CHAR_LATIN_FONT:      nSlotId = SID_ATTR_CHAR_FONT; break;
                case SID_ATTR_CHAR_LATIN_FONTHEIGHT:nSlotId = SID_ATTR_CHAR_FONTHEIGHT; break;
                case SID_ATTR_CHAR_LATIN_LANGUAGE:  nSlotId = SID_ATTR_CHAR_LANGUAGE; break;
                case SID_ATTR_CHAR_LATIN_POSTURE:   nSlotId = SID_ATTR_CHAR_POSTURE; break;
                case SID_ATTR_CHAR_LATIN_WEIGHT:    nSlotId = SID_ATTR_CHAR_WEIGHT; break;
                }
            }

            WhichId nWhich = rPool.GetWhich( nSlotId );
            bool bIsInPool = rPool.IsInRange( nWhich );
            if ( bIsInPool )
            {
                #if OSL_DEBUG_LEVEL > 0
                    bool bFeatureIsEnabled = aFeature->second->isFeatureEnabled();
                    ::rtl::OString sMessage( "FmTextControlShell::transferFeatureStatesToItemSet: found a feature state for " );
                    sMessage += sUnoSlotNameAscii;
                    if ( !bFeatureIsEnabled )
                        sMessage += " (disabled)";
                    OSL_TRACE( "%s", sMessage.getStr() );
                #endif

                lcl_translateUnoStateToItem( nSlotId, aFeature->second->getFeatureState(), _rSet );
            }
            #if OSL_DEBUG_LEVEL > 0
            else
            {
                ::rtl::OString sMessage( "FmTextControlShell::transferFeatureStatesToItemSet: found a feature state for " );
                sMessage += sUnoSlotNameAscii;
                sMessage += ", but could not translate it into an item!";
                OSL_TRACE( "%s", sMessage.getStr() );
            }
            #endif
        }
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::executeAttributeDialog( AttributeSet _eSet, SfxRequest& _rReq )
    {
        const SvxFontListItem* pFontList = PTR_CAST( SvxFontListItem, m_pViewFrame->GetObjectShell()->GetItem( SID_ATTR_CHAR_FONTLIST ) );
        DBG_ASSERT( pFontList, "FmTextControlShell::executeAttributeDialog: no font list item!" );
        if ( !pFontList )
            return;

        SfxItemPool* pPool = EditEngine::CreatePool();
        pPool->FreezeIdRanges();
        ::std::auto_ptr< SfxItemSet > pPureItems( new SfxItemSet( *pPool ) );

        // put the current states of the items into the set
        ::std::auto_ptr< SfxAllItemSet > pCurrentItems( new SfxAllItemSet( *pPureItems ) );
        transferFeatureStatesToItemSet( m_aControlFeatures, *pCurrentItems );

        // additional items, which we are not responsible for at the SfxShell level,
        // but which need to be forwarded to the dialog, anyway
        ControlFeatures aAdditionalFestures;
        fillFeatureDispatchers( m_xActiveControl, pDialogSlots, aAdditionalFestures );
        transferFeatureStatesToItemSet( aAdditionalFestures, *pCurrentItems, true );

        ::std::auto_ptr< SfxTabDialog > pDialog ( _eSet == eCharAttribs
                                                ? static_cast< SfxTabDialog* >( new TextControlCharAttribDialog( NULL, *pCurrentItems, *pFontList ) )
                                                : static_cast< SfxTabDialog* >( new TextControlParaAttribDialog( NULL, *pCurrentItems ) ) );
        if ( RET_OK == pDialog->Execute() )
        {
            const SfxItemSet& rModifiedItems = *pDialog->GetOutputItemSet();
            for ( WhichId nWhich = pPool->GetFirstWhich(); nWhich <= pPool->GetLastWhich(); ++nWhich )
            {
                if ( rModifiedItems.GetItemState( nWhich ) == SFX_ITEM_SET )
                {
                    SfxSlotId nSlotForItemSet = pPool->GetSlotId( nWhich );
                    const SfxPoolItem* pModifiedItem = rModifiedItems.GetItem( nWhich );


                    SfxSlotId nSlotForDispatcher = nSlotForItemSet;
                    switch ( nSlotForDispatcher )
                    {
                        case SID_ATTR_CHAR_FONT:      nSlotForDispatcher = SID_ATTR_CHAR_LATIN_FONT; break;
                        case SID_ATTR_CHAR_FONTHEIGHT:nSlotForDispatcher = SID_ATTR_CHAR_LATIN_FONTHEIGHT; break;
                        case SID_ATTR_CHAR_LANGUAGE:  nSlotForDispatcher = SID_ATTR_CHAR_LATIN_LANGUAGE; break;
                        case SID_ATTR_CHAR_POSTURE:   nSlotForDispatcher = SID_ATTR_CHAR_LATIN_POSTURE; break;
                        case SID_ATTR_CHAR_WEIGHT:    nSlotForDispatcher = SID_ATTR_CHAR_LATIN_WEIGHT; break;
                    }

                    // do we already have a dispatcher for this slot/feature?
                    ControlFeatures::const_iterator aFeaturePos = m_aControlFeatures.find( nSlotForDispatcher );
                    bool bFound = aFeaturePos != m_aControlFeatures.end( );

                    if ( !bFound )
                    {
                        aFeaturePos = aAdditionalFestures.find( nSlotForDispatcher );
                        bFound = aFeaturePos != aAdditionalFestures.end( );
                    }

                    if ( bFound )
                    {
                        Sequence< PropertyValue > aArgs;
                        // temporarily put the modified item into a "clean" set,
                        // and let TransformItems calc the respective UNO parameters
                        pPureItems->Put( *pModifiedItem );
                        TransformItems( nSlotForItemSet, *pPureItems, aArgs );
                        pPureItems->ClearItem( nWhich );

                        if  (   ( nSlotForItemSet == SID_ATTR_PARA_HANGPUNCTUATION )
                            ||  ( nSlotForItemSet == SID_ATTR_PARA_FORBIDDEN_RULES )
                            ||  ( nSlotForItemSet == SID_ATTR_PARA_SCRIPTSPACE )
                            )
                        {
                            // these are no UNO slots, they need special handling since TransformItems cannot
                            // handle them
                            DBG_ASSERT( aArgs.getLength() == 0, "FmTextControlShell::executeAttributeDialog: these are no UNO slots - are they?" );

                            const SfxBoolItem* pBoolItem = PTR_CAST( SfxBoolItem, pModifiedItem );
                            DBG_ASSERT( pBoolItem, "FmTextControlShell::executeAttributeDialog: no bool item?!" );
                            if ( pBoolItem )
                            {
                                aArgs.realloc( 1 );
                                aArgs[ 0 ].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Enable" ) );
                                aArgs[ 0 ].Value <<= (sal_Bool)pBoolItem->GetValue();
                            }
                        }

                        // dispatch this
                        aFeaturePos->second->dispatch( aArgs );
                    }
                #if OSL_DEBUG_LEVEL > 0
                    else
                    {
                        ::rtl::OString sError( "FmTextControShell::executeAttributeDialog: Could not handle the following item:" );
                        sError += "\n  SlotID: "; sError += ::rtl::OString::valueOf( (sal_Int32)nSlotForItemSet );
                        sError += "\n  WhichID: "; sError += ::rtl::OString::valueOf( (sal_Int32)nWhich );
                        sError += "\n  UNO name: ";

                        ::rtl::OUString sUnoSlotName = lcl_getUnoSlotName( *SFX_APP(), nSlotForItemSet );
                        if ( !sUnoSlotName.isEmpty() )
                            sError += ::rtl::OString( sUnoSlotName.getStr(), sUnoSlotName.getLength(), RTL_TEXTENCODING_ASCII_US );
                        else
                            sError += "unknown (no SfxSlot)";
                        OSL_FAIL( sError.getStr() );
                    }
                #endif
                }
            }
            _rReq.Done( rModifiedItems );
        }

        pDialog.reset();
        pCurrentItems.reset();
        pPureItems.reset();
        SfxItemPool::Free(pPool);
    }

    //------------------------------------------------------------------------
    bool FmTextControlShell::executeSelectAll( )
    {
        try
        {
            if ( m_xActiveTextComponent.is() )
            {
                sal_Int32 nTextLen = m_xActiveTextComponent->getText().getLength();
                m_xActiveTextComponent->setSelection( awt::Selection( 0, nTextLen ) );
                return true;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return false;   // not handled
    }

    //------------------------------------------------------------------------
    bool FmTextControlShell::executeClipboardSlot( SfxSlotId _nSlot )
    {
        try
        {
            if ( m_xActiveTextComponent.is() )
            {
                switch ( _nSlot )
                {
                case SID_COPY:
                case SID_CUT:
                {
                    ::rtl::OUString sSelectedText( m_xActiveTextComponent->getSelectedText() );
                    ::svt::OStringTransfer::CopyString( sSelectedText, lcl_getWindow( m_xActiveControl ) );
                    if ( SID_CUT == _nSlot )
                    {
                        awt::Selection aSelection( m_xActiveTextComponent->getSelection() );
                        m_xActiveTextComponent->insertText( aSelection, ::rtl::OUString() );
                    }
                }
                break;
                case SID_PASTE:
                {
                    ::rtl::OUString sClipboardContent;
                    OSL_VERIFY( ::svt::OStringTransfer::PasteString( sClipboardContent, lcl_getWindow( m_xActiveControl ) ) );
                    awt::Selection aSelection( m_xActiveTextComponent->getSelection() );
                    m_xActiveTextComponent->insertText( aSelection, sClipboardContent );
                }
                break;
                default:
                    OSL_FAIL( "FmTextControlShell::executeClipboardSlot: invalid slot!" );
                }
                return true;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return false;   // not handled
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::ExecuteTextAttribute( SfxRequest& _rReq )
    {
        SfxSlotId nSlot = _rReq.GetSlot();

        ControlFeatures::const_iterator aFeaturePos = m_aControlFeatures.find( nSlot );
        if ( aFeaturePos == m_aControlFeatures.end() )
        {
            // special slots
            switch ( nSlot )
            {
            case SID_CHAR_DLG:
                executeAttributeDialog( eCharAttribs, _rReq );
                break;

            case SID_PARA_DLG:
                executeAttributeDialog( eParaAttribs, _rReq );
                break;

            case SID_SELECTALL:
                executeSelectAll();
                break;

            case SID_CUT:
            case SID_COPY:
            case SID_PASTE:
                executeClipboardSlot( nSlot );
                break;

            default:
                DBG_ASSERT( aFeaturePos != m_aControlFeatures.end(), "FmTextControShell::ExecuteTextAttribute: I have no such dispatcher, and cannot handle it at all!" );
                return;
            }
        }
        else
        {
            // slots which are dispatched to the control

            switch ( nSlot )
            {
            case SID_ATTR_CHAR_STRIKEOUT:
            case SID_ATTR_CHAR_UNDERLINE:
            case SID_ATTR_CHAR_OVERLINE:
            {
                SfxItemSet aToggled( *_rReq.GetArgs() );

                lcl_translateUnoStateToItem( nSlot, aFeaturePos->second->getFeatureState(), aToggled );
                WhichId nWhich = aToggled.GetPool()->GetWhich( nSlot );
                const SfxPoolItem* pItem = aToggled.GetItem( nWhich );
                if ( ( SID_ATTR_CHAR_UNDERLINE == nSlot ) || ( SID_ATTR_CHAR_OVERLINE == nSlot ) )
                {
                    const SvxOverlineItem* pTextLine = PTR_CAST( SvxOverlineItem, pItem );
                    DBG_ASSERT( pTextLine, "FmTextControlShell::ExecuteTextAttribute: ooops - no underline/overline item!" );
                    if ( pTextLine )
                    {
                        FontUnderline eTL = pTextLine->GetLineStyle();
                        if ( SID_ATTR_CHAR_UNDERLINE == nSlot ) {
                            aToggled.Put( SvxUnderlineItem( eTL == UNDERLINE_SINGLE ? UNDERLINE_NONE : UNDERLINE_SINGLE, nWhich ) );
                        } else {
                            aToggled.Put( SvxOverlineItem( eTL == UNDERLINE_SINGLE ? UNDERLINE_NONE : UNDERLINE_SINGLE, nWhich ) );
                        }
                    }
                }
                else
                {
                    const SvxCrossedOutItem* pCrossedOut = PTR_CAST( SvxCrossedOutItem, pItem );
                    DBG_ASSERT( pCrossedOut, "FmTextControlShell::ExecuteTextAttribute: ooops - no CrossedOut item!" );
                    if ( pCrossedOut )
                    {
                        FontStrikeout eFS = pCrossedOut->GetStrikeout();
                        aToggled.Put( SvxCrossedOutItem( eFS == STRIKEOUT_SINGLE ? STRIKEOUT_NONE : STRIKEOUT_SINGLE, nWhich ) );
                    }
                }

                Sequence< PropertyValue > aArguments;
                TransformItems( nSlot, aToggled, aArguments );
                aFeaturePos->second->dispatch( aArguments );
            }
            break;

            case SID_ATTR_CHAR_FONTHEIGHT:
            case SID_ATTR_CHAR_FONT:
            case SID_ATTR_CHAR_POSTURE:
            case SID_ATTR_CHAR_WEIGHT:
            case SID_ATTR_CHAR_SHADOWED:
            case SID_ATTR_CHAR_CONTOUR:
            case SID_SET_SUPER_SCRIPT:
            case SID_SET_SUB_SCRIPT:
            {
                const SfxItemSet* pArgs = _rReq.GetArgs();
                Sequence< PropertyValue > aArgs;
                if ( pArgs )
                    TransformItems( nSlot, *pArgs, aArgs );
                aFeaturePos->second->dispatch( aArgs );
            }
            break;

            default:
                if ( aFeaturePos->second->isFeatureEnabled() )
                    aFeaturePos->second->dispatch();
                break;
            }
        }
        _rReq.Done();
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::GetTextAttributeState( SfxItemSet& _rSet )
    {
        SfxWhichIter aIter( _rSet );
        sal_uInt16 nSlot = aIter.FirstWhich();
        while ( nSlot )
        {
            if  (   ( nSlot == SID_ATTR_PARA_LEFT_TO_RIGHT )
                ||  ( nSlot == SID_ATTR_PARA_RIGHT_TO_LEFT )
                )
            {
                if ( !SvtLanguageOptions().IsCTLFontEnabled() )
                {
                    _rSet.DisableItem( nSlot );
                    nSlot = aIter.NextWhich();
                    continue;
                }
            }

            ControlFeatures::const_iterator aFeaturePos = m_aControlFeatures.find( nSlot );
            if ( aFeaturePos != m_aControlFeatures.end() )
            {
                if ( aFeaturePos->second->isFeatureEnabled() )
                    lcl_translateUnoStateToItem( nSlot, aFeaturePos->second->getFeatureState(), _rSet );
                else
                    _rSet.DisableItem( nSlot );
            }
            else
            {
                bool bDisable = false;

                bool bNeedWriteableControl = false;
                bool bNeedTextComponent = false;
                bool bNeedSelection = false;

                switch ( nSlot )
                {
                case SID_CHAR_DLG:
                case SID_PARA_DLG:
                    bDisable |= m_aControlFeatures.empty();
                    bNeedWriteableControl = true;
                    break;

                case SID_CUT:
                    bNeedSelection = true;
                    bNeedTextComponent = true;
                    bNeedWriteableControl = true;
                    OSL_TRACE( "FmTextControlShell::ClipBoard: need to invalidate again" );
                    m_bNeedClipboardInvalidation = true;
                    break;

                case SID_PASTE:
                {
                    Window* pActiveControlVCLWindow = lcl_getWindow( m_xActiveControl );
                    if ( pActiveControlVCLWindow )
                    {
                        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pActiveControlVCLWindow) );
                        bDisable |= !aDataHelper.HasFormat( SOT_FORMAT_STRING );
                    }
                    else
                        bDisable |= true;

                    bNeedTextComponent = true;
                    bNeedWriteableControl = true;
                }
                break;

                case SID_COPY:
                    bNeedTextComponent = true;
                    bNeedSelection = true;
                    break;

                case SID_SELECTALL:
                    bNeedTextComponent = true;
                    break;

                default:
                    // slot is unknown at all
                    bDisable |= true;
                    break;
                }
                OSL_POSTCOND( !bNeedSelection || bNeedTextComponent, "FmTextControlShell::GetTextAttributeState: bNeedSelection should imply bNeedTextComponent!" );

                if ( !bDisable && bNeedWriteableControl )
                    bDisable |= !IsActiveControl( ) || m_bActiveControlIsReadOnly;

                if ( !bDisable && bNeedTextComponent )
                    bDisable |= !m_xActiveTextComponent.is();

                if ( !bDisable && bNeedSelection )
                {
                    awt::Selection aSelection = m_xActiveTextComponent->getSelection();
                    bDisable |= aSelection.Min == aSelection.Max;
                }

                if ( bDisable )
                    _rSet.DisableItem( nSlot );
            }

            nSlot = aIter.NextWhich();
        }
    }

    //------------------------------------------------------------------------
    bool FmTextControlShell::IsActiveControl( bool _bCountRichTextOnly ) const
    {
        if ( _bCountRichTextOnly && !m_bActiveControlIsRichText )
            return false;

        return m_bActiveControl;
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::dispose()
    {
        if ( IsActiveControl() )
            controlDeactivated();
        if ( isControllerListening() )
            stopControllerListening();
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::designModeChanged( bool /*_bNewDesignMode*/ )
    {
        m_rBindings.Invalidate( pTextControlSlots );
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::formActivated( const Reference< XFormController >& _rxController )
    {
#if OSL_DEBUG_LEVEL > 0
        ::rtl::OString sTrace( "FmTextControlShell::formActivated: 0x" );
        sTrace += ::rtl::OString::valueOf( (sal_IntPtr)_rxController.get(), 16 );
        OSL_TRACE( "%s", sTrace.getStr() );
#endif

        DBG_ASSERT( _rxController.is(), "FmTextControlShell::formActivated: invalid controller!" );
        if ( !_rxController.is() )
            return;

        // sometimes, a form controller notifies activations, even if it's already activated
        if ( m_xActiveController == _rxController )
            return;

        try
        {
            startControllerListening( _rxController );
            controlActivated( _rxController->getCurrentControl() );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::formDeactivated( const Reference< XFormController >& _rxController )
    {
#if OSL_DEBUG_LEVEL > 0
        ::rtl::OString sTrace( "FmTextControlShell::formDeactivated: 0x" );
        sTrace += ::rtl::OString::valueOf( (sal_IntPtr)_rxController.get(), 16 );
        OSL_TRACE( "%s", sTrace.getStr() );
#endif
        (void)_rxController;

        if ( IsActiveControl() )
            controlDeactivated();
        if ( isControllerListening() )
            stopControllerListening();
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::startControllerListening( const Reference< XFormController >& _rxController )
    {
        OSL_PRECOND( _rxController.is(), "FmTextControlShell::startControllerListening: invalid controller!" );
        if ( !_rxController.is() )
            return;

        OSL_PRECOND( !isControllerListening(), "FmTextControlShell::startControllerListening: already listening!" );
        if ( isControllerListening() )
            stopControllerListening( );
        DBG_ASSERT( !isControllerListening(), "FmTextControlShell::startControllerListening: inconsistence!" );

        try
        {
            Sequence< Reference< XControl > > aControls( _rxController->getControls() );
            m_aControlObservers.resize( 0 );
            m_aControlObservers.reserve( aControls.getLength() );

            const Reference< XControl >* pControls = aControls.getConstArray();
            const Reference< XControl >* pControlsEnd = pControls + aControls.getLength();
            for ( ; pControls != pControlsEnd; ++pControls )
            {
                m_aControlObservers.push_back( FocusListenerAdapter( new FmFocusListenerAdapter( *pControls, this ) ) );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        m_xActiveController = _rxController;
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::stopControllerListening( )
    {
        OSL_PRECOND( isControllerListening(), "FmTextControlShell::stopControllerListening: inconsistence!" );

        // dispose all listeners associated with the controls of the active controller
        for (   FocusListenerAdapters::iterator aLoop = m_aControlObservers.begin();
                aLoop != m_aControlObservers.end();
                ++aLoop
            )
        {
            (*aLoop)->dispose();
        }

        FocusListenerAdapters aEmpty;
        m_aControlObservers.swap( aEmpty );

        m_xActiveController.clear();
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::implClearActiveControlRef()
    {
        // no more features for this control
        for (   ControlFeatures::iterator aLoop = m_aControlFeatures.begin();
                aLoop != m_aControlFeatures.end();
                ++aLoop
            )
        {
            aLoop->second->dispose();
        }

        ControlFeatures aEmpty;
        m_aControlFeatures.swap( aEmpty );

        if ( m_aContextMenuObserver.get() )
        {
            m_aContextMenuObserver->dispose();
            m_aContextMenuObserver = MouseListenerAdapter();
        }

        if ( m_xActiveTextComponent.is() )
        {
            OSL_TRACE( "FmTextControlShell::ClipBoard: stopping timer for clipboard invalidation" );
            m_aClipboardInvalidation.Stop();
        }
        // no more active control
        m_xActiveControl.clear();
        m_xActiveTextComponent.clear();
        m_bActiveControlIsReadOnly = true;
        m_bActiveControlIsRichText = false;
        m_bActiveControl = false;
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::controlDeactivated( )
    {
        DBG_ASSERT( IsActiveControl(), "FmTextControlShell::controlDeactivated: no active control!" );

        m_bActiveControl = false;

        m_rBindings.Invalidate( pTextControlSlots );
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::controlActivated( const Reference< XControl >& _rxControl )
    {
        // ensure that all knittings with the previously active control are lost
        if ( m_xActiveControl.is() )
            implClearActiveControlRef();
        DBG_ASSERT( m_aControlFeatures.empty(), "FmTextControlShell::controlActivated: should have no dispatchers when I'm here!" );

#if OSL_DEBUG_LEVEL > 0
        {
            Sequence< Reference< XControl > > aActiveControls;
            if ( m_xActiveController.is() )
                aActiveControls = m_xActiveController->getControls();

            bool bFoundThisControl = false;

            const Reference< XControl >* pControls = aActiveControls.getConstArray();
            const Reference< XControl >* pControlsEnd = pControls + aActiveControls.getLength();
            for ( ; ( pControls != pControlsEnd ) && !bFoundThisControl; ++pControls )
            {
                if ( *pControls == _rxControl )
                    bFoundThisControl = true;
            }
            DBG_ASSERT( bFoundThisControl, "FmTextControlShell::controlActivated: only controls which belong to the active controller can be activated!" );
        }
#endif
        // ask the control for dispatchers for our text-related slots
        fillFeatureDispatchers( _rxControl, pTextControlSlots, m_aControlFeatures );

        // remember this control
        m_xActiveControl = _rxControl;
        m_xActiveTextComponent = m_xActiveTextComponent.query( _rxControl );
        m_bActiveControlIsReadOnly = lcl_determineReadOnly( m_xActiveControl );
        m_bActiveControlIsRichText = lcl_isRichText( m_xActiveControl );

        // if we found a rich text control, we need context menu support
        if ( m_bActiveControlIsRichText )
        {
            DBG_ASSERT( NULL == m_aContextMenuObserver.get(), "FmTextControlShell::controlActivated: already have an observer!" );
            m_aContextMenuObserver = MouseListenerAdapter( new FmMouseListenerAdapter( _rxControl, this ) );
        }

        if ( m_xActiveTextComponent.is() )
        {
            OSL_TRACE( "FmTextControlShell::ClipBoard: starting timer for clipboard invalidation" );
            m_aClipboardInvalidation.Start();
        }

        m_bActiveControl = true;

        m_rBindings.Invalidate( pTextControlSlots );

        if ( m_pViewFrame )
            m_pViewFrame->UIFeatureChanged();

        // don't call the activation handler if we don't have any slots we can serve
        // The activation handler is used to put the shell on the top of the dispatcher stack,
        // so it's preferred when slots are distributed.
        // Note that this is a slight hack, to prevent that we grab slots from the SfxDispatcher
        // which should be served by other shells (e.g. Cut/Copy/Paste).
        // A real solution would be a forwarding-mechanism for slots: We should be on the top
        // if we're active, but if we cannot handle the slot, then we need to tell the dispatcher
        // to skip our shell, and pass the slot to the next one. However, this mechanism is not
        // not in place in SFX.
        // Another possibility would be to have dedicated shells for the slots which we might
        // or might not be able to serve. However, this could probably increase the number of
        // shells too much (In theory, nearly every slot could have an own shell then).
        //
        // #i51621# / 2005-08-19 / frank.schoenheit@sun.com
        bool bHaveAnyServeableSlots = m_xActiveTextComponent.is() || !m_aControlFeatures.empty();
        if ( m_aControlActivationHandler.IsSet() && bHaveAnyServeableSlots )
            m_aControlActivationHandler.Call( NULL );

        m_bNeedClipboardInvalidation = true;
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::fillFeatureDispatchers(  const Reference< XControl > _rxControl, SfxSlotId* _pZeroTerminatedSlots,
            ControlFeatures& _rDispatchers )
    {
        Reference< XDispatchProvider > xProvider( _rxControl, UNO_QUERY );
        SfxApplication* pApplication = SFX_APP();
        DBG_ASSERT( pApplication, "FmTextControlShell::fillFeatureDispatchers: no SfxApplication!" );
        if ( xProvider.is() && pApplication )
        {
            SfxSlotId* pSlots = _pZeroTerminatedSlots;
            while ( *pSlots )
            {
                FmTextControlFeature* pDispatcher = implGetFeatureDispatcher( xProvider, pApplication, *pSlots );
                if ( pDispatcher )
                    _rDispatchers.insert( ControlFeatures::value_type( *pSlots, ControlFeature( pDispatcher ) ) );

                ++pSlots;
            }
        }
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::impl_parseURL_nothrow( URL& _rURL )
    {
        try
        {
            if ( !m_xURLTransformer.is() )
            {
                m_xURLTransformer = util::URLTransformer::create( ::comphelper::getProcessComponentContext() );
            }
            if ( m_xURLTransformer.is() )
                m_xURLTransformer->parseStrict( _rURL );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------
    FmTextControlFeature* FmTextControlShell::implGetFeatureDispatcher( const Reference< XDispatchProvider >& _rxProvider, SfxApplication* _pApplication, SfxSlotId _nSlot )
    {
        OSL_PRECOND( _rxProvider.is() && _pApplication, "FmTextControlShell::implGetFeatureDispatcher: invalid arg(s)!" );
        URL aFeatureURL;
        aFeatureURL.Complete = lcl_getUnoSlotName( *_pApplication, _nSlot );
        impl_parseURL_nothrow( aFeatureURL );
        Reference< XDispatch > xDispatcher = _rxProvider->queryDispatch( aFeatureURL, ::rtl::OUString(), 0xFF );
        if ( xDispatcher.is() )
            return new FmTextControlFeature( xDispatcher, aFeatureURL, _nSlot, this );
        return NULL;
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::Invalidate( SfxSlotId _nSlot )
    {
        m_rBindings.Invalidate( _nSlot );
        // despite this method being called "Invalidate", we also update here - this gives more immediate
        // feedback in the UI
        m_rBindings.Update( _nSlot );
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::focusGained( const ::com::sun::star::awt::FocusEvent& _rEvent )
    {
        Reference< XControl > xControl( _rEvent.Source, UNO_QUERY );

#if OSL_DEBUG_LEVEL > 0
        ::rtl::OString sTrace( "FmTextControlShell::focusGained: 0x" );
        sTrace += ::rtl::OString::valueOf( (sal_IntPtr)xControl.get(), 16 );
        OSL_TRACE( "%s", sTrace.getStr() );
#endif

        DBG_ASSERT( xControl.is(), "FmTextControlShell::focusGained: suspicious focus event!" );
        if ( xControl.is() )
            controlActivated( xControl );
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::focusLost( const ::com::sun::star::awt::FocusEvent& _rEvent )
    {
        Reference< XControl > xControl( _rEvent.Source, UNO_QUERY );

#if OSL_DEBUG_LEVEL > 0
        ::rtl::OString sTrace( "FmTextControlShell::focusLost: 0x" );
        sTrace += ::rtl::OString::valueOf( (sal_IntPtr)xControl.get(), 16 );
        OSL_TRACE( "%s", sTrace.getStr() );
#endif

        m_bActiveControl = false;
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::ForgetActiveControl()
    {
        implClearActiveControlRef();
    }

    //------------------------------------------------------------------------
    void FmTextControlShell::contextMenuRequested( const awt::MouseEvent& /*_rEvent*/ )
    {
        m_rBindings.GetDispatcher()->ExecutePopup( SVX_RES( RID_FM_TEXTATTRIBUTE_MENU ) );
    }

//........................................................................
} // namespace svx
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
