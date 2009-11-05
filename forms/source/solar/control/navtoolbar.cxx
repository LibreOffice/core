/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: navtoolbar.cxx,v $
 * $Revision: 1.14 $
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
#include "featuredispatcher.hxx"
#include "frm_resource.hrc"

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/form/runtime/FormFeature.hpp>

#include <sfx2/imgmgr.hxx>
#include <svx/svxids.hrc>
#include <vcl/fixed.hxx>

#include <memory>

#define LID_RECORD_LABEL    1000
#define LID_RECORD_FILLER   1001

//.........................................................................
namespace frm
{
//.........................................................................

    using ::com::sun::star::uno::makeAny;
    namespace FormFeature = ::com::sun::star::form::runtime::FormFeature;

    //=====================================================================
    //.....................................................................
    namespace
    {
        static bool isArtificialItem( sal_Int16 _nFeatureId )
        {
            return ( _nFeatureId == LID_RECORD_LABEL )
                || ( _nFeatureId == LID_RECORD_FILLER );
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

        sal_Int32 lcl_getSlotId( const sal_Int16 _nFormFeature )
        {
            switch ( _nFormFeature )
            {
                case FormFeature::MoveAbsolute          : return SID_FM_RECORD_ABSOLUTE;
                case FormFeature::TotalRecords          : return SID_FM_RECORD_TOTAL;
                case FormFeature::MoveToFirst           : return SID_FM_RECORD_FIRST;
                case FormFeature::MoveToPrevious        : return SID_FM_RECORD_PREV;
                case FormFeature::MoveToNext            : return SID_FM_RECORD_NEXT;
                case FormFeature::MoveToLast            : return SID_FM_RECORD_LAST;
                case FormFeature::SaveRecordChanges     : return SID_FM_RECORD_SAVE;
                case FormFeature::UndoRecordChanges     : return SID_FM_RECORD_UNDO;
                case FormFeature::MoveToInsertRow       : return SID_FM_RECORD_NEW;
                case FormFeature::DeleteRecord          : return SID_FM_RECORD_DELETE;
                case FormFeature::ReloadForm            : return SID_FM_REFRESH;
                case FormFeature::RefreshCurrentControl : return SID_FM_REFRESH_FORM_CONTROL;
                case FormFeature::SortAscending         : return SID_FM_SORTUP;
                case FormFeature::SortDescending        : return SID_FM_SORTDOWN;
                case FormFeature::InteractiveSort       : return SID_FM_ORDERCRIT;
                case FormFeature::AutoFilter            : return SID_FM_AUTOFILTER;
                case FormFeature::InteractiveFilter     : return SID_FM_FILTERCRIT;
                case FormFeature::ToggleApplyFilter     : return SID_FM_FORM_FILTERED;
                case FormFeature::RemoveFilterAndSort   : return SID_FM_REMOVE_FILTER_SORT;
            }
            OSL_ENSURE( isArtificialItem( _nFormFeature ), "lcl_getSlotId: unknown FormFeature!" );
            return 0;
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

        RecordPositionInput* pPositionWindow = static_cast< RecordPositionInput* >( m_pToolbar->GetItemWindow( FormFeature::MoveAbsolute ) );
        OSL_ENSURE( pPositionWindow, "NavigationToolBar::setDispatcher: can't forward the dispatcher to the position window!" );
        if ( pPositionWindow )
            pPositionWindow->setDispatcher( _pDispatcher );

        updateFeatureStates( );
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

        if ( _nItemId == FormFeature::MoveAbsolute )
            m_pToolbar->EnableItem( LID_RECORD_LABEL, _bEnabled );

        if ( _nItemId == FormFeature::TotalRecords )
            m_pToolbar->EnableItem( LID_RECORD_FILLER, _bEnabled );
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::enableFeature( sal_Int16 _nFeatureId, bool _bEnabled )
    {
        DBG_ASSERT( m_pToolbar->GetItemPos( (USHORT)_nFeatureId ) != TOOLBOX_ITEM_NOTFOUND,
            "NavigationToolBar::enableFeature: invalid id!" );

        implEnableItem( (USHORT)_nFeatureId, _bEnabled );
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::checkFeature( sal_Int16 _nFeatureId, bool _bEnabled )
    {
        DBG_ASSERT( m_pToolbar->GetItemPos( (USHORT)_nFeatureId ) != TOOLBOX_ITEM_NOTFOUND,
            "NavigationToolBar::checkFeature: invalid id!" );

        m_pToolbar->CheckItem( (USHORT)_nFeatureId, _bEnabled );
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::setFeatureText( sal_Int16 _nFeatureId, const ::rtl::OUString& _rText )
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

        struct FeatureDescription
        {
            USHORT      nId;
            bool        bRepeat;
            bool        bItemWindow;
        } aSupportedFeatures[] =
        {
            { LID_RECORD_LABEL,                     false, true },
            { FormFeature::MoveAbsolute,            false, true },
            { LID_RECORD_FILLER,                    false, true },
            { FormFeature::TotalRecords,            false, true },
            { FormFeature::MoveToFirst,             true,  false },
            { FormFeature::MoveToPrevious,          true,  false },
            { FormFeature::MoveToNext,              true,  false },
            { FormFeature::MoveToLast,              true,  false },
            { FormFeature::MoveToInsertRow,         false, false },
            { 0, false, false },
            { FormFeature::SaveRecordChanges,       false, false },
            { FormFeature::UndoRecordChanges,       false, false },
            { FormFeature::DeleteRecord,            false, false },
            { FormFeature::ReloadForm,              false, false },
            { FormFeature::RefreshCurrentControl,   false, false },
            { 0, false, false },
            { FormFeature::SortAscending,           false, false },
            { FormFeature::SortDescending,          false, false },
            { FormFeature::InteractiveSort,         false, false },
            { FormFeature::AutoFilter,              false, false },
            { FormFeature::InteractiveFilter,       false, false },
            { FormFeature::ToggleApplyFilter,       false, false },
            { FormFeature::RemoveFilterAndSort,     false, false },
        };

        size_t nSupportedFeatures = sizeof( aSupportedFeatures ) / sizeof( aSupportedFeatures[0] );
        FeatureDescription* pSupportedFeatures = aSupportedFeatures;
        FeatureDescription* pSupportedFeaturesEnd = aSupportedFeatures + nSupportedFeatures;
        for ( ; pSupportedFeatures < pSupportedFeaturesEnd; ++pSupportedFeatures )
        {
            if ( pSupportedFeatures->nId )
            {   // it's _not_ a separator

                // the text(s) of the item
                String sItemText;
                String sItemHelpText;

                // TODO/CLEANUP: this code does nothing(!) nowadays
                //SfxSlotPool& rSlotPool = SfxSlotPool::GetSlotPool( NULL );
                //sItemText = rSlotPool.GetSlotName( pSupportedFeatures->nId, &sItemHelpText );

                // insert the entry
                m_pToolbar->InsertItem( pSupportedFeatures->nId, sItemText, pSupportedFeatures->bRepeat ? TIB_REPEAT : 0 );
                m_pToolbar->SetQuickHelpText( pSupportedFeatures->nId, sItemHelpText );
                if ( !isArtificialItem( pSupportedFeatures->nId ) )
                    m_pToolbar->SetHelpId( pSupportedFeatures->nId, lcl_getSlotId( pSupportedFeatures->nId ) );


                if ( pSupportedFeatures->bItemWindow )
                {
                    Window* pItemWindow = NULL;
                    if ( FormFeature::MoveAbsolute == pSupportedFeatures->nId )
                    {
                        pItemWindow = new RecordPositionInput( m_pToolbar );
                        static_cast< RecordPositionInput* >( pItemWindow )->setDispatcher( m_pDispatcher );
                    }
                    else if ( LID_RECORD_FILLER == pSupportedFeatures->nId )
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

                    switch ( pSupportedFeatures->nId )
                    {
                    case LID_RECORD_LABEL:
                        pItemWindow->SetText( getLabelString( RID_STR_LABEL_RECORD ) );
                        break;

                    case LID_RECORD_FILLER:
                        pItemWindow->SetText( getLabelString( RID_STR_LABEL_OF ) );
                        break;
                    }

                    m_pToolbar->SetItemWindow( pSupportedFeatures->nId, pItemWindow );
                }
            }
            else
            {   // a separator
                m_pToolbar->InsertSeparator( );
            }
        }

        forEachItemWindow( &NavigationToolBar::adjustItemWindowWidth, NULL );

        implSetImageSize( m_eImageSize, true );
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::implSetImageSize( ImageSize _eSize, bool _bForce )
    {
        if ( ( _eSize != m_eImageSize ) || _bForce )
        {
            m_eImageSize = _eSize;

            ::std::auto_ptr< SfxImageManager > pImageManager( new SfxImageManager( NULL ) );
            const bool bIsHighContrast = lcl_isHighContrast( GetBackground().GetColor() );

            const USHORT nCount = m_pToolbar->GetItemCount();
            for ( USHORT i=0; i<nCount; ++i )
            {
                USHORT nId = m_pToolbar->GetItemId( i );
                if ( TOOLBOXITEM_BUTTON == m_pToolbar->GetItemType( i ) )
                {
                    Image aItemImage;
                    if ( !isArtificialItem( nId ) )
                        aItemImage = pImageManager->GetImage( lcl_getSlotId( nId ), m_eImageSize == eLarge, bIsHighContrast );
                    m_pToolbar->SetItemImage( nId, aItemImage );
                }
            }

            // TODO: using the "official" command URLs belonging to a FormFeature, it should be possible to obtain
            // the images from some global UNO service, instead of using the SfxImageManager, and SFX slot IDs.
            // Also, those URLs allow (/me thinks) to obtain the command texts, which we could set at the toolbox,
            // so they would appear as tooltip.

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
                LID_RECORD_LABEL, FormFeature::MoveAbsolute, LID_RECORD_FILLER, FormFeature::TotalRecords, 0
            };
            pGroupIds = aPositionIds;
        }
        break;
        case eNavigation:
        {
            static const USHORT aNavigationIds[] = {
                FormFeature::MoveToFirst, FormFeature::MoveToPrevious, FormFeature::MoveToNext, FormFeature::MoveToLast, FormFeature::MoveToInsertRow, 0
            };
            pGroupIds = aNavigationIds;
        }
        break;
        case eRecordActions:
        {
            static const USHORT aActionIds[] = {
                FormFeature::SaveRecordChanges, FormFeature::UndoRecordChanges, FormFeature::DeleteRecord, FormFeature::ReloadForm, FormFeature::RefreshCurrentControl, 0
            };
            pGroupIds = aActionIds;
        }
        break;
        case eFilterSort:
        {
            static const USHORT aFilterSortIds[] = {
                FormFeature::SortAscending, FormFeature::SortDescending, FormFeature::InteractiveSort, FormFeature::AutoFilter, FormFeature::InteractiveFilter, FormFeature::ToggleApplyFilter, FormFeature::RemoveFilterAndSort, 0
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
        case eNavigation    : nIndicatorItem = FormFeature::MoveToFirst; break;
        case eRecordActions : nIndicatorItem = FormFeature::SaveRecordChanges;  break;
        case eFilterSort    : nIndicatorItem = FormFeature::SortAscending;       break;
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

            case STATE_CHANGE_MIRRORING:
            {
                BOOL bIsRTLEnabled( IsRTLEnabled() );
                m_pToolbar->EnableRTL( bIsRTLEnabled );
                forEachItemWindow( &NavigationToolBar::enableItemRTL, &bIsRTLEnabled );
                Resize();
            }
            break;
        }
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::Resize()
    {
        // resize/position the toolbox as a whole
        sal_Int32 nToolbarHeight = m_pToolbar->CalcWindowSizePixel().Height();

        sal_Int32 nMyHeight = GetOutputSizePixel().Height();
        m_pToolbar->SetPosSizePixel( Point( 0, ( nMyHeight - nToolbarHeight ) / 2 ),
                                     Size( GetSizePixel().Width(), nToolbarHeight ) );

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
#if 0
    //---------------------------------------------------------------------
    void NavigationToolBar::setItemWindowZoom( USHORT /* _nItemId */, Window* _pItemWindow, const void* /* _pParam */ ) const
    {
        _pItemWindow->SetZoom( GetZoom() );
        _pItemWindow->SetZoomedPointFont( IsControlFont() ? GetControlFont() : GetPointFont() );
    }
#endif
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

        case FormFeature::MoveAbsolute:
            sItemText = String::CreateFromAscii( "12345678" );
            break;

        case FormFeature::TotalRecords:
            sItemText = String::CreateFromAscii( "123456" );
            break;
        }

        Size aSize( _pItemWindow->GetTextWidth( sItemText ), /* _pItemWindow->GetSizePixel( ).Height() */ _pItemWindow->GetTextHeight() + 4 );
        aSize.Width() += 6;
        _pItemWindow->SetSizePixel( aSize );

        m_pToolbar->SetItemWindow( _nItemId, _pItemWindow );
    }

    //---------------------------------------------------------------------
    void NavigationToolBar::enableItemRTL( USHORT /*_nItemId*/, Window* _pItemWindow, const void* _pIsRTLEnabled ) const
    {
        _pItemWindow->EnableRTL( *static_cast< const BOOL* >( _pIsRTLEnabled ) );
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
                m_pDispatcher->dispatchWithArgument( FormFeature::MoveAbsolute, "Position", makeAny( (sal_Int32)nRecord ) );

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
