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


#include <navtoolbar.hxx>
#include <frm_resource.hxx>
#include <featuredispatcher.hxx>
#include <strings.hrc>
#include <commandimageprovider.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/form/runtime/FormFeature.hpp>

#include <vcl/event.hxx>
#include <vcl/fixed.hxx>
#include <vcl/commandinfoprovider.hxx>

#include <sal/macros.h>
#include <osl/diagnose.h>
#include <tools/debug.hxx>

#define LID_RECORD_LABEL    1000
#define LID_RECORD_FILLER   1001


namespace frm
{


    using ::com::sun::star::uno::makeAny;
    namespace FormFeature = ::com::sun::star::form::runtime::FormFeature;


    namespace
    {
        bool isArtificialItem( sal_Int16 _nFeatureId )
        {
            return ( _nFeatureId == LID_RECORD_LABEL )
                || ( _nFeatureId == LID_RECORD_FILLER );
        }

        OUString getLabelString(const char* pResId)
        {
            OUString sLabel( " " );
            sLabel += FRM_RES_STRING(pResId);
            sLabel += " ";
            return sLabel;
        }

        OUString lcl_getCommandURL( const sal_Int16 _nFormFeature )
        {
            const sal_Char* pAsciiCommandName = nullptr;
            switch ( _nFormFeature )
            {
                case FormFeature::MoveAbsolute          : pAsciiCommandName = "AbsoluteRecord";     break;
                case FormFeature::TotalRecords          : pAsciiCommandName = "RecTotal";           break;
                case FormFeature::MoveToFirst           : pAsciiCommandName = "FirstRecord";        break;
                case FormFeature::MoveToPrevious        : pAsciiCommandName = "PrevRecord";         break;
                case FormFeature::MoveToNext            : pAsciiCommandName = "NextRecord";         break;
                case FormFeature::MoveToLast            : pAsciiCommandName = "LastRecord";         break;
                case FormFeature::SaveRecordChanges     : pAsciiCommandName = "RecSave";            break;
                case FormFeature::UndoRecordChanges     : pAsciiCommandName = "RecUndo";            break;
                case FormFeature::MoveToInsertRow       : pAsciiCommandName = "NewRecord";          break;
                case FormFeature::DeleteRecord          : pAsciiCommandName = "DeleteRecord";       break;
                case FormFeature::ReloadForm            : pAsciiCommandName = "Refresh";            break;
                case FormFeature::RefreshCurrentControl : pAsciiCommandName = "RefreshFormControl"; break;
                case FormFeature::SortAscending         : pAsciiCommandName = "Sortup";             break;
                case FormFeature::SortDescending        : pAsciiCommandName = "SortDown";           break;
                case FormFeature::InteractiveSort       : pAsciiCommandName = "OrderCrit";          break;
                case FormFeature::AutoFilter            : pAsciiCommandName = "AutoFilter";         break;
                case FormFeature::InteractiveFilter     : pAsciiCommandName = "FilterCrit";         break;
                case FormFeature::ToggleApplyFilter     : pAsciiCommandName = "FormFiltered";       break;
                case FormFeature::RemoveFilterAndSort   : pAsciiCommandName = "RemoveFilterSort";   break;
            }
            if ( pAsciiCommandName != nullptr )
                return ".uno:" + OUString::createFromAscii( pAsciiCommandName );

            OSL_FAIL( "lcl_getCommandURL: unknown FormFeature!" );
            return OUString();
        }
    }

    class ImplNavToolBar : public ToolBox
    {
    protected:
        const IFeatureDispatcher*   m_pDispatcher;

    public:
        explicit ImplNavToolBar( vcl::Window* _pParent )
            :ToolBox( _pParent, WB_3DLOOK )
            ,m_pDispatcher( nullptr )
        {
        }

        void setDispatcher( const IFeatureDispatcher* _pDispatcher )
        {
            m_pDispatcher = _pDispatcher;
        }

    protected:
        // ToolBox overridables
        virtual void        Select() override;

    };


    void ImplNavToolBar::Select()
    {
        if ( m_pDispatcher )
        {
            if ( !m_pDispatcher->isEnabled( GetCurItemId() ) )
                // the toolbox is a little bit buggy: With ToolBoxItemBits::REPEAT, it sometimes
                // happens that a select is reported, even though the respective
                // item has just been disabled.
                return;
            m_pDispatcher->dispatch( GetCurItemId() );
        }
    }

    NavigationToolBar::NavigationToolBar( vcl::Window* _pParent, WinBits _nStyle,
                                          const PCommandImageProvider& _pImageProvider,
                                          const OUString & sModuleId )
        :Window( _pParent, _nStyle )
        ,m_pDispatcher( nullptr )
        ,m_pImageProvider( _pImageProvider )
        ,m_eImageSize( eSmall )
        ,m_pToolbar( nullptr )
        ,m_sModuleId( sModuleId )
    {
        implInit( );
    }


    NavigationToolBar::~NavigationToolBar( )
    {
        disposeOnce();
    }

    void NavigationToolBar::dispose()
    {
        for (auto & childWin : m_aChildWins)
            childWin.disposeAndClear();
        m_aChildWins.clear();
        m_pToolbar.disposeAndClear();
        vcl::Window::dispose();
    }


    void NavigationToolBar::setDispatcher( const IFeatureDispatcher* _pDispatcher )
    {
        m_pDispatcher = _pDispatcher;

        m_pToolbar->setDispatcher( _pDispatcher );

        RecordPositionInput* pPositionWindow = static_cast< RecordPositionInput* >( m_pToolbar->GetItemWindow( FormFeature::MoveAbsolute ) );
        OSL_ENSURE( pPositionWindow, "NavigationToolBar::setDispatcher: can't forward the dispatcher to the position window!" );
        if ( pPositionWindow )
            pPositionWindow->setDispatcher( _pDispatcher );

        // update feature states
        for ( ToolBox::ImplToolItems::size_type nPos = 0; nPos < m_pToolbar->GetItemCount(); ++nPos )
        {
            sal_uInt16 nItemId = m_pToolbar->GetItemId( nPos );

            if ( ( nItemId == LID_RECORD_LABEL ) || ( nItemId == LID_RECORD_FILLER ) )
                continue;

            // is this item enabled?
            bool bEnabled = m_pDispatcher && m_pDispatcher->isEnabled( nItemId );
            implEnableItem( nItemId, bEnabled );
        }
    }


    void NavigationToolBar::implEnableItem( sal_uInt16 _nItemId, bool _bEnabled )
    {
        m_pToolbar->EnableItem( _nItemId, _bEnabled );

        if ( _nItemId == FormFeature::MoveAbsolute )
            m_pToolbar->EnableItem( LID_RECORD_LABEL, _bEnabled );

        if ( _nItemId == FormFeature::TotalRecords )
            m_pToolbar->EnableItem( LID_RECORD_FILLER, _bEnabled );
    }


    void NavigationToolBar::enableFeature( sal_Int16 _nFeatureId, bool _bEnabled )
    {
        DBG_ASSERT( m_pToolbar->GetItemPos( static_cast<sal_uInt16>(_nFeatureId) ) != ToolBox::ITEM_NOTFOUND,
            "NavigationToolBar::enableFeature: invalid id!" );

        implEnableItem( static_cast<sal_uInt16>(_nFeatureId), _bEnabled );
    }


    void NavigationToolBar::checkFeature( sal_Int16 _nFeatureId, bool _bEnabled )
    {
        DBG_ASSERT( m_pToolbar->GetItemPos( static_cast<sal_uInt16>(_nFeatureId) ) != ToolBox::ITEM_NOTFOUND,
            "NavigationToolBar::checkFeature: invalid id!" );

        m_pToolbar->CheckItem( static_cast<sal_uInt16>(_nFeatureId), _bEnabled );
    }


    void NavigationToolBar::setFeatureText( sal_Int16 _nFeatureId, const OUString& _rText )
    {
        DBG_ASSERT( m_pToolbar->GetItemPos( static_cast<sal_uInt16>(_nFeatureId) ) != ToolBox::ITEM_NOTFOUND,
            "NavigationToolBar::checkFeature: invalid id!" );

        vcl::Window* pItemWindow = m_pToolbar->GetItemWindow( static_cast<sal_uInt16>(_nFeatureId) );
        if ( pItemWindow )
            pItemWindow->SetText( _rText );
        else
            m_pToolbar->SetItemText( static_cast<sal_uInt16>(_nFeatureId), _rText );
    }


    void NavigationToolBar::implInit( )
    {
        m_pToolbar = VclPtr<ImplNavToolBar>::Create( this );
        m_pToolbar->SetOutStyle( TOOLBOX_STYLE_FLAT );
        m_pToolbar->Show();

        // need the SfxApplication for retrieving information about our
        // items. We could duplicate all the information here in our lib
        // (such as the item text and the image), but why should we?

        static struct FeatureDescription
        {
            sal_uInt16      nId;
            bool        bRepeat;
            bool        bItemWindow;
        } const aSupportedFeatures[] =
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

        FeatureDescription const * pSupportedFeatures = aSupportedFeatures;
        FeatureDescription const * pSupportedFeaturesEnd = aSupportedFeatures + SAL_N_ELEMENTS( aSupportedFeatures );
        for ( ; pSupportedFeatures < pSupportedFeaturesEnd; ++pSupportedFeatures )
        {
            if ( pSupportedFeatures->nId )
            {   // it's _not_ a separator

                // insert the entry
                m_pToolbar->InsertItem( pSupportedFeatures->nId, OUString(), pSupportedFeatures->bRepeat ? ToolBoxItemBits::REPEAT : ToolBoxItemBits::NONE );
                m_pToolbar->SetQuickHelpText( pSupportedFeatures->nId, OUString() );  // TODO

                if ( !isArtificialItem( pSupportedFeatures->nId ) )
                {
                    OUString sCommandURL( lcl_getCommandURL( pSupportedFeatures->nId ) );
                    m_pToolbar->SetItemCommand( pSupportedFeatures->nId, sCommandURL );
                    m_pToolbar->SetQuickHelpText( pSupportedFeatures->nId,
                            vcl::CommandInfoProvider::GetLabelForCommand(sCommandURL, m_sModuleId) );
                }

                if ( pSupportedFeatures->bItemWindow )
                {
                    vcl::Window* pItemWindow = nullptr;
                    if ( FormFeature::MoveAbsolute == pSupportedFeatures->nId )
                    {
                        pItemWindow = VclPtr<RecordPositionInput>::Create( m_pToolbar );
                        static_cast< RecordPositionInput* >( pItemWindow )->setDispatcher( m_pDispatcher );
                    }
                    else if ( LID_RECORD_FILLER == pSupportedFeatures->nId )
                    {
                        pItemWindow = VclPtr<FixedText>::Create( m_pToolbar, WB_CENTER | WB_VCENTER );
                        pItemWindow->SetBackground(Wallpaper(COL_TRANSPARENT));
                    }
                    else
                    {
                        pItemWindow = VclPtr<FixedText>::Create( m_pToolbar, WB_VCENTER );
                        pItemWindow->SetBackground();
                        pItemWindow->SetPaintTransparent(true);
                    }
                    m_aChildWins.emplace_back(pItemWindow );

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

        forEachItemWindow( &NavigationToolBar::adjustItemWindowWidth );

        implUpdateImages();
    }


    void NavigationToolBar::implUpdateImages()
    {
        OSL_ENSURE( m_pImageProvider, "NavigationToolBar::implUpdateImages: no image provider => no images!" );
        if ( !m_pImageProvider )
            return;

        const ToolBox::ImplToolItems::size_type nItemCount = m_pToolbar->GetItemCount();

        // collect the FormFeatures in the toolbar
        std::vector<sal_Int16> aFormFeatures;
        aFormFeatures.reserve( nItemCount );

        for ( ToolBox::ImplToolItems::size_type i=0; i<nItemCount; ++i )
        {
            sal_uInt16 nId = m_pToolbar->GetItemId( i );
            if ( ( ToolBoxItemType::BUTTON == m_pToolbar->GetItemType( i ) ) && !isArtificialItem( nId ) )
                aFormFeatures.push_back( nId );
        }

        // translate them into command URLs
        css::uno::Sequence< OUString > aCommandURLs( aFormFeatures.size() );
        size_t i = 0;
        for (auto const& formFeature : aFormFeatures)
        {
            aCommandURLs[i++] = lcl_getCommandURL(formFeature);
        }

        // retrieve the images for the command URLs
        CommandImages aCommandImages = m_pImageProvider->getCommandImages( aCommandURLs, m_eImageSize == eLarge );

        // and set them at the toolbar
        CommandImages::const_iterator commandImage = aCommandImages.begin();
        for (auto const& formFeature : aFormFeatures)
        {
            m_pToolbar->SetItemImage( formFeature, *commandImage );
            ++commandImage;
        }

        // parts of our layout is dependent on the size of our icons
        Resize();
    }


    void NavigationToolBar::implSetImageSize( ImageSize _eSize )
    {
        if ( _eSize != m_eImageSize )
        {
            m_eImageSize = _eSize;
            implUpdateImages();
        }
    }


    void NavigationToolBar::SetImageSize( ImageSize _eSize )
    {
        implSetImageSize( _eSize );
    }


    void NavigationToolBar::ShowFunctionGroup( FunctionGroup _eGroup, bool _bShow )
    {
        const sal_uInt16* pGroupIds = nullptr;

        switch ( _eGroup )
        {
        case ePosition:
        {
            static const sal_uInt16 aPositionIds[] = {
                LID_RECORD_LABEL, FormFeature::MoveAbsolute, LID_RECORD_FILLER, FormFeature::TotalRecords, 0
            };
            pGroupIds = aPositionIds;
        }
        break;
        case eNavigation:
        {
            static const sal_uInt16 aNavigationIds[] = {
                FormFeature::MoveToFirst, FormFeature::MoveToPrevious, FormFeature::MoveToNext, FormFeature::MoveToLast, FormFeature::MoveToInsertRow, 0
            };
            pGroupIds = aNavigationIds;
        }
        break;
        case eRecordActions:
        {
            static const sal_uInt16 aActionIds[] = {
                FormFeature::SaveRecordChanges, FormFeature::UndoRecordChanges, FormFeature::DeleteRecord, FormFeature::ReloadForm, FormFeature::RefreshCurrentControl, 0
            };
            pGroupIds = aActionIds;
        }
        break;
        case eFilterSort:
        {
            static const sal_uInt16 aFilterSortIds[] = {
                FormFeature::SortAscending, FormFeature::SortDescending, FormFeature::InteractiveSort, FormFeature::AutoFilter, FormFeature::InteractiveFilter, FormFeature::ToggleApplyFilter, FormFeature::RemoveFilterAndSort, 0
            };
            pGroupIds = aFilterSortIds;
        }
        break;
        default:
            OSL_FAIL( "NavigationToolBar::ShowFunctionGroup: invalid group id!" );
        }

        if ( pGroupIds )
            while ( *pGroupIds )
                m_pToolbar->ShowItem( *pGroupIds++, _bShow );
    }


    bool NavigationToolBar::IsFunctionGroupVisible( FunctionGroup _eGroup )
    {
        sal_uInt16 nIndicatorItem = 0;
        switch ( _eGroup )
        {
        case ePosition      : nIndicatorItem = LID_RECORD_LABEL;    break;
        case eNavigation    : nIndicatorItem = FormFeature::MoveToFirst; break;
        case eRecordActions : nIndicatorItem = FormFeature::SaveRecordChanges;  break;
        case eFilterSort    : nIndicatorItem = FormFeature::SortAscending;       break;
        default:
            OSL_FAIL( "NavigationToolBar::IsFunctionGroupVisible: invalid group id!" );
        }

        return m_pToolbar->IsItemVisible( nIndicatorItem );
    }


    void NavigationToolBar::StateChanged( StateChangedType nType )
    {
        Window::StateChanged( nType );

        switch ( nType )
        {
            case StateChangedType::Zoom:
//                m_pToolbar->SetZoom( GetZoom() );
//                forEachItemWindow( setItemWindowZoom, NULL );
                // the ToolBox class is not zoomable at the moment, so
                // we better have no zoom at all instead of only half a zoom ...
                break;

            case StateChangedType::ControlFont:
                forEachItemWindow( &NavigationToolBar::setItemControlFont );
                forEachItemWindow( &NavigationToolBar::adjustItemWindowWidth );
                break;

            case StateChangedType::ControlForeground:
                forEachItemWindow( &NavigationToolBar::setItemControlForeground );
                break;

            case StateChangedType::Mirroring:
            {
                sal_Bool bIsRTLEnabled( IsRTLEnabled() );
                m_pToolbar->EnableRTL( bIsRTLEnabled );
                forEachItemWindow( &NavigationToolBar::enableItemRTL, &bIsRTLEnabled );
                Resize();
            }
            break;
            default:;
        }
    }


    void NavigationToolBar::Resize()
    {
        // resize/position the toolbox as a whole
        sal_Int32 nToolbarHeight = m_pToolbar->CalcWindowSizePixel().Height();

        sal_Int32 nMyHeight = GetOutputSizePixel().Height();
        m_pToolbar->SetPosSizePixel( Point( 0, ( nMyHeight - nToolbarHeight ) / 2 ),
                                     Size( GetSizePixel().Width(), nToolbarHeight ) );

        Window::Resize();
    }


    void NavigationToolBar::SetControlBackground()
    {
        Window::SetControlBackground();
        m_pToolbar->SetControlBackground();
        forEachItemWindow( &NavigationToolBar::setItemBackground, nullptr );

        implUpdateImages();
    }


    void NavigationToolBar::SetControlBackground( const Color& _rColor )
    {
        Window::SetControlBackground( _rColor );
        m_pToolbar->SetControlBackground( _rColor );
        forEachItemWindow( &NavigationToolBar::setItemBackground, &_rColor );

        implUpdateImages();
    }


    void NavigationToolBar::SetTextLineColor( )
    {
        Window::SetTextLineColor( );
        m_pToolbar->SetTextLineColor( );
        forEachItemWindow( &NavigationToolBar::setTextLineColor, nullptr );
    }


    void NavigationToolBar::SetTextLineColor( const Color& _rColor )
    {
        Window::SetTextLineColor( _rColor );
        m_pToolbar->SetTextLineColor( _rColor );
        forEachItemWindow( &NavigationToolBar::setTextLineColor, &_rColor );
    }


    void NavigationToolBar::forEachItemWindow( ItemWindowHandler _handler )
    {
        for ( ToolBox::ImplToolItems::size_type item = 0; item < m_pToolbar->GetItemCount(); ++item )
        {
            sal_uInt16 nItemId = m_pToolbar->GetItemId( item );
            vcl::Window* pItemWindow = m_pToolbar->GetItemWindow( nItemId );
            if ( pItemWindow )
                (this->*_handler)( nItemId, pItemWindow );
        }
    }

    void NavigationToolBar::forEachItemWindow( ItemWindowHandler2 _handler, const void* _pParam )
    {
        for ( ToolBox::ImplToolItems::size_type item = 0; item < m_pToolbar->GetItemCount(); ++item )
        {
            sal_uInt16 nItemId = m_pToolbar->GetItemId( item );
            vcl::Window* pItemWindow = m_pToolbar->GetItemWindow( nItemId );
            if ( pItemWindow )
                (*_handler)( nItemId, pItemWindow, _pParam );
        }
    }

    void NavigationToolBar::setItemBackground( sal_uInt16 /* _nItemId */, vcl::Window* _pItemWindow, const void* _pColor )
    {
        if ( _pColor )
            _pItemWindow->SetControlBackground( *static_cast< const Color* >( _pColor ) );
        else
            _pItemWindow->SetControlBackground();
    }


    void NavigationToolBar::setTextLineColor( sal_uInt16 /* _nItemId */, vcl::Window* _pItemWindow, const void* _pColor )
    {
        if ( _pColor )
            _pItemWindow->SetTextLineColor( *static_cast< const Color* >( _pColor ) );
        else
            _pItemWindow->SetTextLineColor();
    }
#if 0

    void NavigationToolBar::setItemWindowZoom( sal_uInt16 /* _nItemId */, vcl::Window* _pItemWindow, const void* /* _pParam */ ) const
    {
        _pItemWindow->SetZoom( GetZoom() );
        _pItemWindow->SetZoomedPointFont( IsControlFont() ? GetControlFont() : GetPointFont() );
    }
#endif

    void NavigationToolBar::setItemControlFont( sal_uInt16 /* _nItemId */, vcl::Window* _pItemWindow ) const
    {
        if ( IsControlFont() )
            _pItemWindow->SetControlFont( GetControlFont() );
        else
            _pItemWindow->SetControlFont( );
    }


    void NavigationToolBar::setItemControlForeground( sal_uInt16 /* _nItemId */, vcl::Window* _pItemWindow ) const
    {
        if ( IsControlForeground() )
            _pItemWindow->SetControlForeground( GetControlForeground() );
        else
            _pItemWindow->SetControlForeground( );
        _pItemWindow->SetTextColor( GetTextColor() );
    }


    void NavigationToolBar::adjustItemWindowWidth( sal_uInt16 _nItemId, vcl::Window* _pItemWindow ) const
    {
        OUString sItemText;
        switch ( _nItemId )
        {
        case LID_RECORD_LABEL:
            sItemText = getLabelString( RID_STR_LABEL_RECORD );
            break;

        case LID_RECORD_FILLER:
            sItemText = getLabelString( RID_STR_LABEL_OF );
            break;

        case FormFeature::MoveAbsolute:
            sItemText = "12345678";
            break;

        case FormFeature::TotalRecords:
            sItemText = "123456";
            break;
        }

        Size aSize( _pItemWindow->GetTextWidth( sItemText ), /* _pItemWindow->GetSizePixel( ).Height() */ _pItemWindow->GetTextHeight() + 4 );
        aSize.AdjustWidth(6 );
        _pItemWindow->SetSizePixel( aSize );

        m_pToolbar->SetItemWindow( _nItemId, _pItemWindow );
    }


    void NavigationToolBar::enableItemRTL( sal_uInt16 /*_nItemId*/, vcl::Window* _pItemWindow, const void* _pIsRTLEnabled )
    {
        _pItemWindow->EnableRTL( *static_cast< const sal_Bool* >( _pIsRTLEnabled ) );
    }

    RecordPositionInput::RecordPositionInput( vcl::Window* _pParent )
        :NumericField( _pParent, WB_BORDER | WB_VCENTER )
        ,m_pDispatcher( nullptr )
    {
        SetMin( 1 );
        SetFirst( 1 );
        SetSpinSize( 1 );
        SetDecimalDigits( 0 );
        SetStrictFormat( true );
        SetBorderStyle( WindowBorderStyle::MONO );
    }


    void RecordPositionInput::setDispatcher( const IFeatureDispatcher* _pDispatcher )
    {
        m_pDispatcher = _pDispatcher;
    }


    void RecordPositionInput::FirePosition( bool _bForce )
    {
        if ( _bForce || IsValueChangedFromSaved() )
        {
            sal_Int64 nRecord = GetValue();
            if ( nRecord < GetMin() || nRecord > GetMax() )
                return;

            if ( m_pDispatcher )
                m_pDispatcher->dispatchWithArgument( FormFeature::MoveAbsolute, "Position", makeAny( static_cast<sal_Int32>(nRecord) ) );

            SaveValue();
        }
    }


    void RecordPositionInput::LoseFocus()
    {
        FirePosition( false );
    }


    void RecordPositionInput::KeyInput( const KeyEvent& rKeyEvent )
    {
        if( rKeyEvent.GetKeyCode() == KEY_RETURN && !GetText().isEmpty() )
            FirePosition( true );
        else
            NumericField::KeyInput( rKeyEvent );
    }


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
