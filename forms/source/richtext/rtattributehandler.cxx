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

#include "rtattributehandler.hxx"

#include <svx/svxids.hrc>
#include <editeng/eeitem.hxx>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <tools/mapunit.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/outdev.hxx>

#define ITEMID_ADJUST       EE_PARA_JUST
#include <editeng/adjitem.hxx>
#define ITEMID_WEIGHT       EE_CHAR_WEIGHT
#include <editeng/wghtitem.hxx>
#define ITEMID_ESCAPEMENT   EE_CHAR_ESCAPEMENT
#include <editeng/escpitem.hxx>
#define ITEMID_LINESPACING  EE_PARA_SBL
#include <editeng/lspcitem.hxx>
#define ITEMID_FONTHEIGHT   EE_CHAR_FONTHEIGHT
#include <editeng/fhgtitem.hxx>
#define ITEMID_FRAMEDIR     EE_PARA_WRITINGDIR
#include <editeng/frmdiritem.hxx>
#include <editeng/scripttypeitem.hxx>

//........................................................................
namespace frm
{
//........................................................................
    //====================================================================
    //= ReferenceBase
    //====================================================================
    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL ReferenceBase::acquire()
    {
        return osl_atomic_increment( &m_refCount );
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL ReferenceBase::release()
    {
        return osl_atomic_decrement( &m_refCount );
    }

    //--------------------------------------------------------------------
    ReferenceBase::~ReferenceBase()
    {
    }

    //====================================================================
    //= AttributeHandler
    //====================================================================
    //--------------------------------------------------------------------
    AttributeHandler::AttributeHandler( AttributeId _nAttributeId, WhichId _nWhichId )
        :m_nAttribute( _nAttributeId )
        ,m_nWhich    ( _nWhichId     )
    {
    }

    //--------------------------------------------------------------------
    AttributeHandler::~AttributeHandler()
    {
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL AttributeHandler::acquire()
    {
        return ReferenceBase::acquire();
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL AttributeHandler::release()
    {
        return ReferenceBase::release();
    }

    //--------------------------------------------------------------------
    AttributeId AttributeHandler::getAttributeId( ) const
    {
        return getAttribute();
    }

    //--------------------------------------------------------------------
    AttributeCheckState AttributeHandler::implGetCheckState( const SfxPoolItem& /*_rItem*/ ) const
    {
        OSL_FAIL( "AttributeHandler::implGetCheckState: not to be called!" );
        return eIndetermined;
    }

    //--------------------------------------------------------------------
    void AttributeHandler::putItemForScript( SfxItemSet& _rAttribs, const SfxPoolItem& _rItem, ScriptType _nForScriptType ) const
    {
        SvxScriptSetItem aSetItem( (WhichId)getAttributeId(), *_rAttribs.GetPool() );
        aSetItem.PutItemForScriptType( _nForScriptType, _rItem );
        _rAttribs.Put( aSetItem.GetItemSet(), sal_False );
    }

    //--------------------------------------------------------------------
    AttributeCheckState AttributeHandler::getCheckState( const SfxItemSet& _rAttribs ) const
    {
        AttributeCheckState eSimpleState( eIndetermined );
        const SfxPoolItem* pItem = _rAttribs.GetItem( getWhich() );
        if ( pItem )
            eSimpleState = implGetCheckState( *pItem );
        return eSimpleState;
    }

    //--------------------------------------------------------------------
    AttributeState AttributeHandler::getState( const SfxItemSet& _rAttribs ) const
    {
        AttributeState aState( eIndetermined );
        aState.eSimpleState = getCheckState( _rAttribs );
        return aState;
    }

    //====================================================================
    //= AttributeHandlerFactory
    //====================================================================
    //--------------------------------------------------------------------
    namespace
    {
        static WhichId lcl_implGetWhich( const SfxItemPool& _rPool, AttributeId _nAttributeId )
        {
            WhichId nWhich = 0;
            switch ( _nAttributeId )
            {
            case SID_ATTR_CHAR_LATIN_FONTHEIGHT:nWhich = EE_CHAR_FONTHEIGHT;break;
            case SID_ATTR_CHAR_LATIN_FONT:      nWhich = EE_CHAR_FONTINFO;  break;
            case SID_ATTR_CHAR_LATIN_LANGUAGE:  nWhich = EE_CHAR_LANGUAGE;  break;
            case SID_ATTR_CHAR_LATIN_POSTURE:   nWhich = EE_CHAR_ITALIC;    break;
            case SID_ATTR_CHAR_LATIN_WEIGHT:    nWhich = EE_CHAR_WEIGHT;    break;

            default:
                nWhich = _rPool.GetWhich( (SfxSlotId)_nAttributeId );
            }
            return nWhich;
        }
    }
    //--------------------------------------------------------------------
    ::rtl::Reference< IAttributeHandler > AttributeHandlerFactory::getHandlerFor( AttributeId _nAttributeId, const SfxItemPool& _rEditEnginePool )
    {
        ::rtl::Reference< IAttributeHandler > pReturn;
        switch ( _nAttributeId )
        {
        case SID_ATTR_PARA_ADJUST_LEFT  :
        case SID_ATTR_PARA_ADJUST_CENTER:
        case SID_ATTR_PARA_ADJUST_RIGHT :
        case SID_ATTR_PARA_ADJUST_BLOCK :
            pReturn = new ParaAlignmentHandler( _nAttributeId );
            break;

        case SID_ATTR_PARA_LINESPACE_10:
        case SID_ATTR_PARA_LINESPACE_15:
        case SID_ATTR_PARA_LINESPACE_20:
            pReturn = new LineSpacingHandler( _nAttributeId );
            break;

        case SID_SET_SUPER_SCRIPT:
        case SID_SET_SUB_SCRIPT:
            pReturn = new EscapementHandler( _nAttributeId );
            break;

        case SID_ATTR_CHAR_FONTHEIGHT:
        case SID_ATTR_CHAR_CTL_FONTHEIGHT:
        case SID_ATTR_CHAR_CJK_FONTHEIGHT:
        case SID_ATTR_CHAR_LATIN_FONTHEIGHT:
            pReturn = new FontSizeHandler( _nAttributeId, lcl_implGetWhich( _rEditEnginePool, _nAttributeId ) );
            break;

        case SID_ATTR_PARA_LEFT_TO_RIGHT:
        case SID_ATTR_PARA_RIGHT_TO_LEFT:
            pReturn = new ParagraphDirectionHandler( _nAttributeId );
            break;

        case SID_ATTR_PARA_HANGPUNCTUATION:
        case SID_ATTR_PARA_FORBIDDEN_RULES:
        case SID_ATTR_PARA_SCRIPTSPACE:
            pReturn = new BooleanHandler( _nAttributeId, lcl_implGetWhich( _rEditEnginePool, _nAttributeId ) );
            break;

        default:
            pReturn = new SlotHandler( (SfxSlotId)_nAttributeId, lcl_implGetWhich( _rEditEnginePool, _nAttributeId ) );
            break;

        }

        return pReturn;
    }

    //====================================================================
    //= ParaAlignmentHandler
    //====================================================================
    //--------------------------------------------------------------------
    ParaAlignmentHandler::ParaAlignmentHandler( AttributeId _nAttributeId )
        :AttributeHandler( _nAttributeId, EE_PARA_JUST )
        ,m_eAdjust( SVX_ADJUST_CENTER )
    {
        switch ( getAttribute() )
        {
            case SID_ATTR_PARA_ADJUST_LEFT  : m_eAdjust = SVX_ADJUST_LEFT;    break;
            case SID_ATTR_PARA_ADJUST_CENTER: m_eAdjust = SVX_ADJUST_CENTER;  break;
            case SID_ATTR_PARA_ADJUST_RIGHT : m_eAdjust = SVX_ADJUST_RIGHT;   break;
            case SID_ATTR_PARA_ADJUST_BLOCK : m_eAdjust = SVX_ADJUST_BLOCK;   break;
            default:
                OSL_FAIL( "ParaAlignmentHandler::ParaAlignmentHandler: invalid slot!" );
                break;
        }
    }

    //--------------------------------------------------------------------
    AttributeCheckState ParaAlignmentHandler::implGetCheckState( const SfxPoolItem& _rItem ) const
    {
        OSL_ENSURE( _rItem.ISA( SvxAdjustItem ), "ParaAlignmentHandler::implGetCheckState: invalid pool item!" );
        SvxAdjust eAdjust = static_cast< const SvxAdjustItem& >( _rItem ).GetAdjust();
        return ( eAdjust == m_eAdjust ) ? eChecked : eUnchecked;
    }

    //--------------------------------------------------------------------
    void ParaAlignmentHandler::executeAttribute( const SfxItemSet& /*_rCurrentAttribs*/, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType /*_nForScriptType*/ ) const
    {
        OSL_ENSURE( !_pAdditionalArg, "ParaAlignmentHandler::executeAttribute: this is a simple toggle attribute - no args possible!" );
        (void)_pAdditionalArg;
        _rNewAttribs.Put( SvxAdjustItem( m_eAdjust, getWhich() ) );
    }

    //====================================================================
    //= LineSpacingHandler
    //====================================================================
    //--------------------------------------------------------------------
    LineSpacingHandler::LineSpacingHandler( AttributeId _nAttributeId )
        :AttributeHandler( _nAttributeId, EE_PARA_SBL )
        ,m_nLineSpace( 100 )
    {
        switch ( getAttribute() )
        {
            case SID_ATTR_PARA_LINESPACE_10: m_nLineSpace = 100; break;
            case SID_ATTR_PARA_LINESPACE_15: m_nLineSpace = 150; break;
            case SID_ATTR_PARA_LINESPACE_20: m_nLineSpace = 200; break;
            default:
                OSL_FAIL( "LineSpacingHandler::LineSpacingHandler: invalid slot!" );
                break;
        }
    }

    //--------------------------------------------------------------------
    AttributeCheckState LineSpacingHandler::implGetCheckState( const SfxPoolItem& _rItem ) const
    {
        OSL_ENSURE( _rItem.ISA( SvxLineSpacingItem ), "LineSpacingHandler::implGetCheckState: invalid pool item!" );
        sal_uInt16 nLineSpace = static_cast< const SvxLineSpacingItem& >( _rItem ).GetPropLineSpace();
        return ( nLineSpace == m_nLineSpace ) ? eChecked : eUnchecked;
    }

    //--------------------------------------------------------------------
    void LineSpacingHandler::executeAttribute( const SfxItemSet& /*_rCurrentAttribs*/, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType /*_nForScriptType*/ ) const
    {
        OSL_ENSURE( !_pAdditionalArg, "LineSpacingHandler::executeAttribute: this is a simple toggle attribute - no args possible!" );
        (void)_pAdditionalArg;

        SvxLineSpacingItem aLineSpacing( m_nLineSpace, getWhich() );
        aLineSpacing.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
        if ( 100 == m_nLineSpace )
            aLineSpacing.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
        else
            aLineSpacing.SetPropLineSpace( (sal_uInt8)m_nLineSpace );

        _rNewAttribs.Put( aLineSpacing );
    }

    //====================================================================
    //= EscapementHandler
    //====================================================================
    //--------------------------------------------------------------------
    EscapementHandler::EscapementHandler( AttributeId _nAttributeId )
        :AttributeHandler( _nAttributeId, EE_CHAR_ESCAPEMENT )
        ,m_eEscapement( SVX_ESCAPEMENT_OFF )
    {
        switch ( getAttribute() )
        {
            case SID_SET_SUPER_SCRIPT   : m_eEscapement = SVX_ESCAPEMENT_SUPERSCRIPT; break;
            case SID_SET_SUB_SCRIPT     : m_eEscapement = SVX_ESCAPEMENT_SUBSCRIPT;   break;
            default:
                OSL_FAIL( "EscapementHandler::EscapementHandler: invalid slot!" );
                break;
        }
    }

    //--------------------------------------------------------------------
    AttributeCheckState EscapementHandler::implGetCheckState( const SfxPoolItem& _rItem ) const
    {
        OSL_ENSURE( _rItem.ISA( SvxEscapementItem ), "EscapementHandler::getState: invalid pool item!" );
        SvxEscapement eEscapement = static_cast< const SvxEscapementItem& >( _rItem ).GetEscapement();
        return ( eEscapement == m_eEscapement ) ? eChecked : eUnchecked;
    }

    //--------------------------------------------------------------------
    void EscapementHandler::executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType /*_nForScriptType*/ ) const
    {
        OSL_ENSURE( !_pAdditionalArg, "EscapementHandler::executeAttribute: this is a simple toggle attribute - no args possible!" );
            // well, in theory we could allow an SvxEscapementItem here, but this is not needed
        (void)_pAdditionalArg;

        bool bIsChecked = getCheckState( _rCurrentAttribs ) == eChecked;
        _rNewAttribs.Put( SvxEscapementItem( bIsChecked ? SVX_ESCAPEMENT_OFF : m_eEscapement, getWhich() ) );
    }

    //====================================================================
    //= SlotHandler
    //====================================================================
    //--------------------------------------------------------------------
    SlotHandler::SlotHandler( AttributeId _nAttributeId, WhichId _nWhichId )
        :AttributeHandler( _nAttributeId, _nWhichId )
        ,m_bScriptDependent( false )
    {
        m_bScriptDependent = ( SID_ATTR_CHAR_WEIGHT == _nAttributeId )
                         ||  ( SID_ATTR_CHAR_POSTURE == _nAttributeId )
                         ||  ( SID_ATTR_CHAR_FONT == _nAttributeId );
    }

    //--------------------------------------------------------------------
    AttributeState SlotHandler::getState( const SfxItemSet& _rAttribs ) const
    {
        AttributeState aState( eIndetermined );

        const SfxPoolItem* pItem = _rAttribs.GetItem( getWhich() );
        if ( pItem )
            aState.setItem( pItem->Clone() );

        return aState;
    }

    //--------------------------------------------------------------------
    void SlotHandler::executeAttribute( const SfxItemSet& /*_rCurrentAttribs*/, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const
    {
        if ( _pAdditionalArg )
        {
            SfxPoolItem* pCorrectWich = _pAdditionalArg->Clone();
            pCorrectWich->SetWhich( getWhich() );

            if ( m_bScriptDependent )
                putItemForScript( _rNewAttribs, *pCorrectWich, _nForScriptType );
            else
                _rNewAttribs.Put( *pCorrectWich );
            DELETEZ( pCorrectWich );
        }
        else
            OSL_FAIL( "SlotHandler::executeAttribute: need attributes to do something!" );
    }

    //====================================================================
    //= FontSizeHandler
    //====================================================================
    //--------------------------------------------------------------------
    FontSizeHandler::FontSizeHandler( AttributeId _nAttributeId, WhichId _nWhichId )
        :AttributeHandler( _nAttributeId, _nWhichId )
    {
        OSL_ENSURE( ( _nAttributeId == SID_ATTR_CHAR_FONTHEIGHT ) || ( _nAttributeId == SID_ATTR_CHAR_CTL_FONTHEIGHT )
            || ( _nAttributeId == SID_ATTR_CHAR_CJK_FONTHEIGHT ) || ( _nAttributeId == SID_ATTR_CHAR_LATIN_FONTHEIGHT ),
            "FontSizeHandler::FontSizeHandler: invalid attribute id!" );
    }

    //--------------------------------------------------------------------
    AttributeState FontSizeHandler::getState( const SfxItemSet& _rAttribs ) const
    {
        AttributeState aState( eIndetermined );

        const SfxPoolItem* pItem = _rAttribs.GetItem( getWhich() );
        const SvxFontHeightItem* pFontHeightItem = PTR_CAST( SvxFontHeightItem, pItem );
        OSL_ENSURE( pFontHeightItem || !pItem, "FontSizeHandler::getState: invalid item!" );
        if ( pFontHeightItem )
        {
            // by definition, the item should have the unit twip
            sal_uLong nHeight = pFontHeightItem->GetHeight();
            if ( _rAttribs.GetPool()->GetMetric( getWhich() ) != SFX_MAPUNIT_TWIP )
            {
                nHeight = OutputDevice::LogicToLogic(
                    Size( 0, nHeight ),
                    MapMode( (MapUnit)( _rAttribs.GetPool()->GetMetric( getWhich() ) ) ),
                    MapMode( MAP_TWIP )
                ).Height();
            }

            SvxFontHeightItem* pNewItem = new SvxFontHeightItem( nHeight, 100, getWhich() );
            pNewItem->SetProp( pFontHeightItem->GetProp(), pFontHeightItem->GetPropUnit() );
            aState.setItem( pNewItem );
        }

        return aState;
    }

    //--------------------------------------------------------------------
    void FontSizeHandler::executeAttribute( const SfxItemSet& /*_rCurrentAttribs*/, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const
    {
        const SvxFontHeightItem* pFontHeightItem = PTR_CAST( SvxFontHeightItem, _pAdditionalArg );
        OSL_ENSURE( pFontHeightItem, "FontSizeHandler::executeAttribute: need a FontHeightItem!" );

        if ( pFontHeightItem )
        {
            // corect measurement units
            SfxMapUnit eItemMapUnit = pFontHeightItem->GetPropUnit(); (void)eItemMapUnit;
            sal_uLong nHeight = pFontHeightItem->GetHeight();
            if ( _rNewAttribs.GetPool()->GetMetric( getWhich() ) != SFX_MAPUNIT_TWIP )
            {
                nHeight = OutputDevice::LogicToLogic(
                    Size( 0, nHeight ),
                    MapMode( (MapUnit)( SFX_MAPUNIT_TWIP ) ),
                    MapMode( (MapUnit)( _rNewAttribs.GetPool()->GetMetric( getWhich() ) ) )
                ).Height();
            }

            SvxFontHeightItem aNewItem( nHeight, 100, getWhich() );
            aNewItem.SetProp( pFontHeightItem->GetProp(), pFontHeightItem->GetPropUnit() );

            if ( ( getAttributeId() == SID_ATTR_CHAR_FONTHEIGHT ) && _nForScriptType )
                putItemForScript( _rNewAttribs, aNewItem, _nForScriptType );
            else
                _rNewAttribs.Put( aNewItem );
        }
    }

    //====================================================================
    //= ParagraphDirectionHandler
    //====================================================================
    //--------------------------------------------------------------------
    ParagraphDirectionHandler::ParagraphDirectionHandler( AttributeId _nAttributeId )
        :AttributeHandler( _nAttributeId, EE_PARA_WRITINGDIR )
        ,m_eParagraphDirection( FRMDIR_HORI_LEFT_TOP )
        ,m_eDefaultAdjustment( SVX_ADJUST_RIGHT )
        ,m_eOppositeDefaultAdjustment( SVX_ADJUST_LEFT )
    {
        switch ( getAttributeId() )
        {
            case SID_ATTR_PARA_LEFT_TO_RIGHT: m_eParagraphDirection = FRMDIR_HORI_LEFT_TOP; m_eDefaultAdjustment = SVX_ADJUST_LEFT; break;
            case SID_ATTR_PARA_RIGHT_TO_LEFT: m_eParagraphDirection = FRMDIR_HORI_RIGHT_TOP; m_eDefaultAdjustment = SVX_ADJUST_RIGHT; break;
            default:
                OSL_FAIL( "ParagraphDirectionHandler::ParagraphDirectionHandler: invalid attribute id!" );
        }

        if ( SVX_ADJUST_RIGHT == m_eDefaultAdjustment )
            m_eOppositeDefaultAdjustment = SVX_ADJUST_LEFT;
        else
            m_eOppositeDefaultAdjustment = SVX_ADJUST_RIGHT;
    }

    //--------------------------------------------------------------------
    AttributeCheckState ParagraphDirectionHandler::implGetCheckState( const SfxPoolItem& _rItem ) const
    {
        OSL_ENSURE( _rItem.ISA( SvxFrameDirectionItem ), "ParagraphDirectionHandler::implGetCheckState: invalid pool item!" );
        SvxFrameDirection eDirection = static_cast< SvxFrameDirection >( static_cast< const SvxFrameDirectionItem& >( _rItem ).GetValue() );
        return ( eDirection == m_eParagraphDirection ) ? eChecked : eUnchecked;
    }

    //--------------------------------------------------------------------
    void ParagraphDirectionHandler::executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* /*_pAdditionalArg*/, ScriptType /*_nForScriptType*/ ) const
    {
        _rNewAttribs.Put( SvxFrameDirectionItem( m_eParagraphDirection, getWhich() ) );

        // if the current adjustment of the was the default adjustment for the *previous* text direction,
        // then we toggle the adjustment, too
        SvxAdjust eCurrentAdjustment = SVX_ADJUST_LEFT;
        const SfxPoolItem* pCurrentAdjustment = NULL;
        if ( SFX_ITEM_ON == _rCurrentAttribs.GetItemState( EE_PARA_JUST, sal_True, &pCurrentAdjustment ) )
            eCurrentAdjustment = static_cast< const SvxAdjustItem* >( pCurrentAdjustment )->GetAdjust();

        if ( eCurrentAdjustment == m_eOppositeDefaultAdjustment )
            _rNewAttribs.Put( SvxAdjustItem( m_eDefaultAdjustment, EE_PARA_JUST ) );
    }

    //====================================================================
    //= BooleanHandler
    //====================================================================
    //--------------------------------------------------------------------
    BooleanHandler::BooleanHandler( AttributeId _nAttributeId, WhichId _nWhichId )
        :AttributeHandler( _nAttributeId, _nWhichId )
    {
    }

    //--------------------------------------------------------------------
    AttributeCheckState BooleanHandler::implGetCheckState( const SfxPoolItem& _rItem ) const
    {
        OSL_ENSURE( _rItem.ISA( SfxBoolItem ), "BooleanHandler::implGetCheckState: invalid item!" );
        if ( _rItem.ISA( SfxBoolItem ) )
            return static_cast< const SfxBoolItem& >( _rItem ).GetValue() ? eChecked : eUnchecked;

        return eIndetermined;
    }

    //--------------------------------------------------------------------
    void BooleanHandler::executeAttribute( const SfxItemSet& /*_rCurrentAttribs*/, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType /*_nForScriptType*/ ) const
    {
        OSL_ENSURE( _pAdditionalArg && _pAdditionalArg->ISA( SfxBoolItem ), "BooleanHandler::executeAttribute: invalid argument!" );
        if ( _pAdditionalArg )
        {
            SfxPoolItem* pCorrectWich = _pAdditionalArg->Clone();
            pCorrectWich->SetWhich( getWhich() );
            _rNewAttribs.Put( *pCorrectWich );
            DELETEZ( pCorrectWich );
        }
    }

//........................................................................
}   // namespace frm
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
