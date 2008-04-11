/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: navtoolbar.cxx,v $
 * $Revision: 1.12 $
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
#include "navtoolbar.hxx"
#include "frm_resource.hxx"
#ifndef _FRM_RESOURCE_HRC_
#include "frm_resource.hrc"
#endif
#include <vcl/fixed.hxx>
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <sfx2/msgpool.hxx>
#include <sfx2/imgmgr.hxx>
#include "featuredispatcher.hxx"
#include <com/sun/star/uno/Any.hxx>

#include <memory>

#define LID_RECORD_LABEL    1
#define LID_RECORD_FILLER   2

//.........................................................................
namespace frm
{
//.........................................................................

    //=====================================================================
    //.....................................................................
    namespace
    {
        static bool isSfxSlot( sal_Int32 _nFeatureId )
        {
            // everything we use in this class is an SFX-slot - except the LID_* ids
            return ( _nFeatureId != LID_RECORD_LABEL )
                && ( _nFeatureId != LID_RECORD_FILLER );
        }

        static String getLabelString( USHORT _nResId )
        {
            String sLabel = String::CreateFromAscii( " " );
            sLabel += String( FRM_RES_STRING( _nResId ) );
            sLabel += String::CreateFromAscii( " " );
            return sLabel;
        }
        static bool lcl_isHighContrast( const Color& _rColor )
        {
            return _rColor.IsDark();
        }
    }

    //=====================================================================
    //= ImplNavToolBar
    //=====================================================================
    class ImplNavToolBar : public ToolBox
    {
    protected:
        const IFeatureDispatcher*   m_pDispatcher;

    public:
        ImplNavToolBar( Window* _pParent )
            :ToolBox( _pParent, WB_3DLOOK )
            ,m_pDispatcher( NULL )
        {
        }

        void setDispatcher( const IFeatureDispatcher* _pDispatcher )
        {
            m_pDispatcher = _pDispatcher;
        }

    protected:
        // ToolBox overridables
        virtual void        Select();

    };

    //---------------------------------------------------------------------
    void ImplNavToolBar::Select()
    {
        if ( m_pDispatcher )
        {
            if ( !m_pDispatcher->isEnabled( GetCurItemId() ) )
                // the toolbox is a little bit buggy: With TIB_REPEAT, it sometimes
                // happens that a select is reported, even though the respective
                // item has just been disabled.
                return;
            m_pDispatcher->dispatch( GetCurItemId() );
        }
    }

    //=====================================================================
    //= NavigationToolBar
    //=====================================================================
    DBG_NAME( NavigationToolBar )
    //---------------------------------------------------------------------
    NavigationToolBar::NavigationToolBar( Window* _pParent, WinBits _nStyle )
        :Window( _pParent, _nStyle )
        ,m_pDispatcher( NULL )
        ,m_eImageSize( eSmall )
        ,m_pToolbar( NULL )
    {
        DBG_CTOR( NavigationToolBar, NULL );
        implInit( );
    }

    //---------------------------------------------------------------------
    NavigationToolBar::~NavigationToolBar( )
    {
        for (   ::std::vector< Window* >::iterator loopChildWins = m_aChildWins.begin();
                loopChildWins != m_aChildWins.end();
                ++loopChildWins
            )
        {
            delete *loopChildWins;
        }
        delete m_pToolbar;
        DBG_DTOR( NavigationToolBar, NULL );
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::setDispatcher( const IFeatureDispatcher* _pDispatcher )
    {
        m_pDispatcher = _pDispatcher;

        m_pToolbar->setDispatcher( _pDispatcher );

        RecordPositionInput* pPositionWindow = static_cast< RecordPositionInput* >( m_pToolbar->GetItemWindow( SID_FM_RECORD_ABSOLUTE ) );
        OSL_ENSURE( pPositionWindow, "NavigationToolBar::setDispatcher: can't forward the dispatcher to the position window!" );
        if ( pPositionWindow )
            pPositionWindow->setDispatcher( _pDispatcher );

        updateFeatureStates( );
    }

    //---------------------------------------------------------------------
    const IFeatureDispatcher* NavigationToolBar::getDispatcher( )
    {
        return m_pDispatcher;
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::updateFeatureStates( )
    {
        for ( USHORT nPos = 0; nPos < m_pToolbar->GetItemCount(); ++nPos )
        {
            USHORT nItemId = m_pToolbar->GetItemId( nPos );

            if ( ( nItemId == LID_RECORD_LABEL ) || ( nItemId == LID_RECORD_FILLER ) )
                continue;

            // is this item enabled?
            bool bEnabled = m_pDispatcher ? m_pDispatcher->isEnabled( nItemId ) : false;
            implEnableItem( nItemId, bEnabled );
        }
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::implEnableItem( USHORT _nItemId, bool _bEnabled )
    {
        m_pToolbar->EnableItem( _nItemId, _bEnabled );

        if ( _nItemId == SID_FM_RECORD_ABSOLUTE )
            m_pToolbar->EnableItem( LID_RECORD_LABEL, _bEnabled );

        if ( _nItemId == SID_FM_RECORD_TOTAL )
            m_pToolbar->EnableItem( LID_RECORD_FILLER, _bEnabled );
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::enableFeature( sal_Int32 _nFeatureId, bool _bEnabled )
    {
        DBG_ASSERT( m_pToolbar->GetItemPos( (USHORT)_nFeatureId ) != TOOLBOX_ITEM_NOTFOUND,
            "NavigationToolBar::enableFeature: invalid id!" );

        implEnableItem( (USHORT)_nFeatureId, _bEnabled );
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::checkFeature( sal_Int32 _nFeatureId, bool _bEnabled )
    {
        DBG_ASSERT( m_pToolbar->GetItemPos( (USHORT)_nFeatureId ) != TOOLBOX_ITEM_NOTFOUND,
            "NavigationToolBar::checkFeature: invalid id!" );

        m_pToolbar->CheckItem( (USHORT)_nFeatureId, _bEnabled );
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::setFeatureText( sal_Int32 _nFeatureId, const ::rtl::OUString& _rText )
    {
        DBG_ASSERT( m_pToolbar->GetItemPos( (USHORT)_nFeatureId ) != TOOLBOX_ITEM_NOTFOUND,
            "NavigationToolBar::checkFeature: invalid id!" );

        Window* pItemWindow = m_pToolbar->GetItemWindow( (USHORT)_nFeatureId );
        if ( pItemWindow )
            pItemWindow->SetText( _rText );
        else
            m_pToolbar->SetItemText( (USHORT)_nFeatureId, _rText );
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::implInit( )
    {
        m_pToolbar = new ImplNavToolBar( this );
        m_pToolbar->SetOutStyle( TOOLBOX_STYLE_FLAT );
        m_pToolbar->Show();

        // need the SfxApplication for retrieving informations about our
        // items. We could duplicate all the information here in our lib
        // (such as the item text and the image), but why should we?

        struct SlotDescription
        {
            USHORT      nId;
            bool        bRepeat;
            bool        bItemWindow;
        } aSupportedSlots[] =
        {
            { LID_RECORD_LABEL,             false, true },
            { SID_FM_RECORD_ABSOLUTE,       false, true },
            { LID_RECORD_FILLER,            false, true },
            { SID_FM_RECORD_TOTAL,          false, true },

            { SID_FM_RECORD_FIRST,          true,  false },
            { SID_FM_RECORD_PREV,           true,  false },
            { SID_FM_RECORD_NEXT,           true,  false },
            { SID_FM_RECORD_LAST,           true,  false },
            { 0, false, false },
            { SID_FM_RECORD_SAVE,           false, false },
            { SID_FM_RECORD_UNDO,           false, false },
            { SID_FM_RECORD_NEW,            false, false },
            { SID_FM_RECORD_DELETE,         false, false },
            { SID_FM_REFRESH,               false, false },
            { 0, false, false },
            { SID_FM_SORTUP,                false, false },
            { SID_FM_SORTDOWN,              false, false },
            { SID_FM_ORDERCRIT,             false, false },
            { SID_FM_AUTOFILTER,            false, false },
            { SID_FM_FILTERCRIT,            false, false },
            { SID_FM_FORM_FILTERED,         false, false },
            { SID_FM_REMOVE_FILTER_SORT,    false, false },
        };

        size_t nSupportedSlots = sizeof( aSupportedSlots ) / sizeof( aSupportedSlots[0] );
        SlotDescription* pSupportedSlots = aSupportedSlots;
        SlotDescription* pSupportedSlotsEnd = aSupportedSlots + nSupportedSlots;
        for ( ; pSupportedSlots < pSupportedSlotsEnd; ++pSupportedSlots )
        {
            if ( pSupportedSlots->nId )
            {   // it's _not_ a separator

                // the text(s) of the item
                String sItemText;
                String sItemHelpText;

                // TODO/CLEANUP: this code does nothing(!) nowadays
                //SfxSlotPool& rSlotPool = SfxSlotPool::GetSlotPool( NULL );
                //sItemText = rSlotPool.GetSlotName( pSupportedSlots->nId, &sItemHelpText );

                // insert the entry
                m_pToolbar->InsertItem( pSupportedSlots->nId, sItemText, pSupportedSlots->bRepeat ? TIB_REPEAT : 0 );
                m_pToolbar->SetQuickHelpText( pSupportedSlots->nId, sItemHelpText );
                if ( isSfxSlot( pSupportedSlots->nId ) )
                    m_pToolbar->SetHelpId( pSupportedSlots->nId, pSupportedSlots->nId );


                if ( pSupportedSlots->bItemWindow )
                {
                    Window* pItemWindow = NULL;
                    if ( SID_FM_RECORD_ABSOLUTE == pSupportedSlots->nId )
                    {
                        pItemWindow = new RecordPositionInput( m_pToolbar );
                        static_cast< RecordPositionInput* >( pItemWindow )->setDispatcher( m_pDispatcher );
                    }
                    else if ( LID_RECORD_FILLER == pSupportedSlots->nId )
                    {
                        pItemWindow = new FixedText( m_pToolbar, WB_CENTER | WB_VCENTER );
                        pItemWindow->SetBackground(Wallpaper(Color(COL_TRANSPARENT)));
                    }
                    else
                    {
                        pItemWindow = new FixedText( m_pToolbar, WB_VCENTER );
                        pItemWindow->SetBackground();
                        pItemWindow->SetPaintTransparent(TRUE);
                    }
                    m_aChildWins.push_back( pItemWindow );

                    switch ( pSupportedSlots->nId )
                    {
                    case LID_RECORD_LABEL:
                        pItemWindow->SetText( getLabelString( RID_STR_LABEL_RECORD ) );
                        break;

                    case LID_RECORD_FILLER:
                        pItemWindow->SetText( getLabelString( RID_STR_LABEL_OF ) );
                        break;
                    }

                    m_pToolbar->SetItemWindow( pSupportedSlots->nId, pItemWindow );
                }
            }
            else
            {   // a separator
                m_pToolbar->InsertSeparator( );
            }
        }

        forEachItemWindow( &NavigationToolBar::adjustItemWindowWidth, NULL );

        // the image of the item
        ::std::auto_ptr< SfxImageManager > pImageManager( new SfxImageManager( NULL ) );
        pImageManager->SetImagesForceSize( *m_pToolbar, FALSE, m_eImageSize == eLarge );
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::implSetImageSize( ImageSize _eSize, bool _bForce )
    {
        if ( ( _eSize != m_eImageSize ) || _bForce )
        {
            m_eImageSize = _eSize;
            ::std::auto_ptr< SfxImageManager > pImageManager( new SfxImageManager( NULL ) );
            pImageManager->SetImagesForceSize( *m_pToolbar, lcl_isHighContrast( GetBackground().GetColor() ), m_eImageSize == eLarge );

            // parts of our layout is dependent on the size of our icons
            Resize();
        }
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::SetImageSize( ImageSize _eSize )
    {
        implSetImageSize( _eSize );
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::ShowFunctionGroup( FunctionGroup _eGroup, bool _bShow )
    {
        const USHORT* pGroupIds = NULL;

        switch ( _eGroup )
        {
        case ePosition:
        {
            static const USHORT aPositionIds[] = {
                LID_RECORD_LABEL, SID_FM_RECORD_ABSOLUTE, LID_RECORD_FILLER, SID_FM_RECORD_TOTAL, 0
            };
            pGroupIds = aPositionIds;
        }
        break;
        case eNavigation:
        {
            static const USHORT aNavigationIds[] = {
                SID_FM_RECORD_FIRST, SID_FM_RECORD_PREV, SID_FM_RECORD_NEXT, SID_FM_RECORD_LAST, 0
            };
            pGroupIds = aNavigationIds;
        }
        break;
        case eRecordActions:
        {
            static const USHORT aActionIds[] = {
                SID_FM_RECORD_SAVE, SID_FM_RECORD_UNDO, SID_FM_RECORD_NEW, SID_FM_RECORD_DELETE, SID_FM_REFRESH, 0
            };
            pGroupIds = aActionIds;
        }
        break;
        case eFilterSort:
        {
            static const USHORT aFilterSortIds[] = {
                SID_FM_SORTUP, SID_FM_SORTDOWN, SID_FM_ORDERCRIT, SID_FM_AUTOFILTER, SID_FM_FILTERCRIT, SID_FM_FORM_FILTERED, SID_FM_REMOVE_FILTER_SORT, 0
            };
            pGroupIds = aFilterSortIds;
        }
        break;
        default:
            OSL_ENSURE( sal_False, "NavigationToolBar::ShowFunctionGroup: invalid group id!" );
        }

        if ( pGroupIds )
            while ( *pGroupIds )
                m_pToolbar->ShowItem( *pGroupIds++, _bShow );
    }

    //---------------------------------------------------------------------
    bool NavigationToolBar::IsFunctionGroupVisible( FunctionGroup _eGroup )
    {
        USHORT nIndicatorItem = 0;
        switch ( _eGroup )
        {
        case ePosition      : nIndicatorItem = LID_RECORD_LABEL;    break;
        case eNavigation    : nIndicatorItem = SID_FM_RECORD_FIRST; break;
        case eRecordActions : nIndicatorItem = SID_FM_RECORD_SAVE;  break;
        case eFilterSort    : nIndicatorItem = SID_FM_SORTUP;       break;
        default:
            OSL_ENSURE( sal_False, "NavigationToolBar::IsFunctionGroupVisible: invalid group id!" );
        }

        return m_pToolbar->IsItemVisible( nIndicatorItem );
    }

    //------------------------------------------------------------------------------
    void NavigationToolBar::StateChanged( StateChangedType nType )
    {
        Window::StateChanged( nType );

        switch ( nType )
        {
            case STATE_CHANGE_ZOOM:
//                m_pToolbar->SetZoom( GetZoom() );
//                forEachItemWindow( setItemWindowZoom, NULL );
                // the ToolBox class is not zoomable at the moment, so
                // we better have no zoom at all instead of only half a zoom ...
                break;

            case STATE_CHANGE_CONTROLFONT:
                forEachItemWindow( &NavigationToolBar::setItemControlFont, NULL );
                forEachItemWindow( &NavigationToolBar::adjustItemWindowWidth, NULL );
                break;

            case STATE_CHANGE_CONTROLFOREGROUND:
                forEachItemWindow( &NavigationToolBar::setItemControlForeground, NULL );
                break;
        }
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::Resize()
    {
        // resize/position the toolbox as a whole
        sal_Int32 nToolbarHeight = m_pToolbar->CalcWindowSizePixel().Height();

        sal_Int32 nMyHeight = GetOutputSizePixel().Height();
        m_pToolbar->SetPosPixel( Point( 0, ( nMyHeight - nToolbarHeight ) / 2 ) );
        m_pToolbar->SetSizePixel( Size( GetSizePixel().Width(), nToolbarHeight ) );

        Window::Resize();
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::SetControlBackground()
    {
        Window::SetControlBackground();
        m_pToolbar->SetControlBackground();
        forEachItemWindow( &NavigationToolBar::setItemBackground, NULL );

        // the contrast of the background color may have changed, so force
        // the images to be rebuild (high contrast requires a possibly different
        // image set)
        implSetImageSize( m_eImageSize, true );
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::SetControlBackground( const Color& _rColor )
    {
        Window::SetControlBackground( _rColor );
        m_pToolbar->SetControlBackground( _rColor );
        forEachItemWindow( &NavigationToolBar::setItemBackground, &_rColor );

        // the contrast of the background color may have changed, so force
        // the images to be rebuild (high contrast requires a possibly different
        // image set)
        implSetImageSize( m_eImageSize, true );
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::SetTextLineColor( )
    {
        Window::SetTextLineColor( );
        m_pToolbar->SetTextLineColor( );
        forEachItemWindow( &NavigationToolBar::setTextLineColor, NULL );
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::SetTextLineColor( const Color& _rColor )
    {
        Window::SetTextLineColor( _rColor );
        m_pToolbar->SetTextLineColor( _rColor );
        forEachItemWindow( &NavigationToolBar::setTextLineColor, &_rColor );
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::forEachItemWindow( ItemWindowHandler _handler, const void* _pParam )
    {
        for ( USHORT item = 0; item < m_pToolbar->GetItemCount(); ++item )
        {
            USHORT nItemId = m_pToolbar->GetItemId( item );
            Window* pItemWindow = m_pToolbar->GetItemWindow( nItemId );
            if ( pItemWindow )
                (this->*_handler)( nItemId, pItemWindow, _pParam );
        }
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::setItemBackground( USHORT /* _nItemId */, Window* _pItemWindow, const void* _pColor ) const
    {
        if ( _pColor )
            _pItemWindow->SetControlBackground( *static_cast< const Color* >( _pColor ) );
        else
            _pItemWindow->SetControlBackground();
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::setTextLineColor( USHORT /* _nItemId */, Window* _pItemWindow, const void* _pColor ) const
    {
        if ( _pColor )
            _pItemWindow->SetTextLineColor( *static_cast< const Color* >( _pColor ) );
        else
            _pItemWindow->SetTextLineColor();
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::setItemWindowZoom( USHORT /* _nItemId */, Window* _pItemWindow, const void* /* _pParam */ ) const
    {
        _pItemWindow->SetZoom( GetZoom() );
        _pItemWindow->SetZoomedPointFont( IsControlFont() ? GetControlFont() : GetPointFont() );
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::setItemControlFont( USHORT /* _nItemId */, Window* _pItemWindow, const void* /* _pParam */ ) const
    {
        if ( IsControlFont() )
            _pItemWindow->SetControlFont( GetControlFont() );
        else
            _pItemWindow->SetControlFont( );
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::setItemControlForeground( USHORT /* _nItemId */, Window* _pItemWindow, const void* /* _pParam */ ) const
    {
        if ( IsControlForeground() )
            _pItemWindow->SetControlForeground( GetControlForeground() );
        else
            _pItemWindow->SetControlForeground( );
        _pItemWindow->SetTextColor( GetTextColor() );
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::adjustItemWindowWidth( USHORT _nItemId, Window* _pItemWindow, const void* /* _pParam */ ) const
    {
        String sItemText;
        switch ( _nItemId )
        {
        case LID_RECORD_LABEL:
            sItemText = getLabelString( RID_STR_LABEL_RECORD );
            break;

        case LID_RECORD_FILLER:
            sItemText = getLabelString( RID_STR_LABEL_OF );
            break;

        case SID_FM_RECORD_ABSOLUTE:
            sItemText = String::CreateFromAscii( "12345678" );
            break;

        case SID_FM_RECORD_TOTAL:
            sItemText = String::CreateFromAscii( "123456" );
            break;
        }

        Size aSize( _pItemWindow->GetTextWidth( sItemText ), /* _pItemWindow->GetSizePixel( ).Height() */ _pItemWindow->GetTextHeight() + 4 );
        aSize.Width() += 6;
        _pItemWindow->SetSizePixel( aSize );

        m_pToolbar->SetItemWindow( _nItemId, _pItemWindow );
    }

    //=====================================================================
    //= RecordPositionInput
    //=====================================================================
    //---------------------------------------------------------------------
    RecordPositionInput::RecordPositionInput( Window* _pParent )
        :NumericField( _pParent, WB_BORDER | WB_VCENTER )
        ,m_pDispatcher( NULL )
    {
        SetMin( 1 );
        SetFirst( 1 );
        SetSpinSize( 1 );
        SetDecimalDigits( 0 );
        SetStrictFormat( TRUE );
        SetBorderStyle( WINDOW_BORDER_MONO );
    }

    //---------------------------------------------------------------------
    RecordPositionInput::~RecordPositionInput()
    {
    }

    //---------------------------------------------------------------------
    void RecordPositionInput::setDispatcher( const IFeatureDispatcher* _pDispatcher )
    {
        m_pDispatcher = _pDispatcher;
    }

    //---------------------------------------------------------------------
    void RecordPositionInput::FirePosition( sal_Bool _bForce )
    {
        if ( _bForce || ( GetText() != GetSavedValue() ) )
        {
            sal_Int64 nRecord = GetValue();
            if ( nRecord < GetMin() || nRecord > GetMax() )
                return;

            if ( m_pDispatcher )
                m_pDispatcher->dispatchWithArgument( SID_FM_RECORD_ABSOLUTE, "Position", ::com::sun::star::uno::makeAny( (sal_Int32)nRecord ) );

            SaveValue();
        }
    }

    //---------------------------------------------------------------------
    void RecordPositionInput::LoseFocus()
    {
        FirePosition( sal_False );
    }

    //---------------------------------------------------------------------
    void RecordPositionInput::KeyInput( const KeyEvent& rKeyEvent )
    {
        if( rKeyEvent.GetKeyCode() == KEY_RETURN && GetText().Len() )
            FirePosition( sal_True );
        else
            NumericField::KeyInput( rKeyEvent );
    }


//.........................................................................
}   // namespace frm
//.........................................................................
