/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Condition.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "precompiled_reportdesign.hxx"

#include "Condition.hxx"
#include "UITools.hxx"
#include "CondFormat.hxx"
#include "CondFormat.hrc"
#include "RptResId.hrc"
#include "ReportController.hxx"
#include "ModuleHelper.hxx"
#include "ColorChanger.hxx"
#include "RptResId.hrc"
#include "helpids.hrc"
#include "reportformula.hxx"

#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONMANAGER_HPP_
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XIMAGEMANAGER_HPP_
#include <com/sun/star/ui/XImageManager.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_IMAGETYPE_HPP_
#include <com/sun/star/ui/ImageType.hpp>
#endif

#define ITEMID_COLOR
#define ITEMID_BRUSH
#include <svx/tbcontrl.hxx>
#include <svx/svxids.hrc>
#include <svx/xtable.hxx>
#include <svx/tbxcolorupdate.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/imgdef.hxx>
#include <svtools/pathoptions.hxx>
#include <vcl/svapp.hxx>
#include <vcl/bmpacc.hxx>
#include <tools/diagnose_ex.h>
#include <rtl/ustrbuf.hxx>

namespace rptui
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

//========================================================================
// class SvxColorWindow_Impl --------------------------------------------------
//========================================================================
#ifndef WB_NO_DIRECTSELECT
#define WB_NO_DIRECTSELECT      ((WinBits)0x04000000)
#endif

#define PALETTE_X 10
#define PALETTE_Y 10
#define PALETTE_SIZE (PALETTE_X * PALETTE_Y)
class OColorPopup : public FloatingWindow
{
    DECL_LINK( SelectHdl, void * );
    Condition* m_pCondition;
    USHORT      m_nSlotId;
public:
    OColorPopup(Window* _pParent,Condition* _pCondition);
    ValueSet        m_aColorSet;

    virtual void KeyInput( const KeyEvent& rKEvt );
    virtual void Resize();

    void StartSelection();
    void SetSlotId(USHORT _nSlotId);
};
// -----------------------------------------------------------------------------
OColorPopup::OColorPopup(Window* _pParent,Condition* _pCondition)
:FloatingWindow(_pParent, WinBits( WB_BORDER | WB_STDFLOATWIN | WB_3DLOOK|WB_DIALOGCONTROL ))
,m_pCondition(_pCondition)
,m_nSlotId(0)
,m_aColorSet( this, WinBits( WB_ITEMBORDER | WB_NAMEFIELD | WB_3DLOOK | WB_NO_DIRECTSELECT) )
{
    m_aColorSet.SetHelpId( HID_RPT_POPUP_COLOR_CTRL );
    SetHelpId( HID_RPT_POPUP_COLOR );
    const Size aSize12( 13, 13 );
    ::std::auto_ptr<XColorTable> pColorTable(new XColorTable( SvtPathOptions().GetPalettePath() ));
    short i = 0;
    long nCount = pColorTable->Count();
    XColorEntry* pEntry = NULL;
    Color aColWhite( COL_WHITE );
    String aStrWhite( ModuleRes(STR_COLOR_WHITE) );

    if ( nCount > PALETTE_SIZE )
        // Show scrollbar if more than PALLETTE_SIZE colors are available
        m_aColorSet.SetStyle( m_aColorSet.GetStyle() | WB_VSCROLL );

    for ( i = 0; i < nCount; i++ )
    {
        pEntry = pColorTable->GetColor(i);
        m_aColorSet.InsertItem( i+1, pEntry->GetColor(), pEntry->GetName() );
    }

    while ( i < PALETTE_SIZE )
    {
        // fill empty elements if less then PALLETTE_SIZE colors are available
        m_aColorSet.InsertItem( i+1, aColWhite, aStrWhite );
        i++;
    }

    m_aColorSet.SetSelectHdl( LINK( this, OColorPopup, SelectHdl ) );
    m_aColorSet.SetColCount( PALETTE_X );
    m_aColorSet.SetLineCount( PALETTE_Y );
    Size aSize = m_aColorSet.CalcWindowSizePixel( aSize12 );
    aSize.Width()  += 4;
    aSize.Height() += 4;
    SetOutputSizePixel( aSize );
    m_aColorSet.Show();
}
// -----------------------------------------------------------------------------
void OColorPopup::KeyInput( const KeyEvent& rKEvt )
{
    m_aColorSet.KeyInput(rKEvt);
}

// -----------------------------------------------------------------------------
void OColorPopup::Resize()
{
    Size aSize = GetOutputSizePixel();
    aSize.Width()  -= 4;
    aSize.Height() -= 4;
    m_aColorSet.SetPosSizePixel( Point(2,2), aSize );
}

// -----------------------------------------------------------------------------
void OColorPopup::StartSelection()
{
    m_aColorSet.StartSelection();
}
// -----------------------------------------------------------------------------
void OColorPopup::SetSlotId(USHORT _nSlotId)
{
    m_nSlotId = _nSlotId;
    if ( SID_ATTR_CHAR_COLOR_BACKGROUND == _nSlotId || SID_BACKGROUND_COLOR == _nSlotId )
    {
        m_aColorSet.SetStyle( m_aColorSet.GetStyle() | WB_NONEFIELD );
        m_aColorSet.SetText( String(ModuleRes( STR_TRANSPARENT )) );
    } // if ( SID_ATTR_CHAR_COLOR_BACKGROUND == theSlotId || SID_BACKGROUND_COLOR == theSlotId )
}
// -----------------------------------------------------------------------------
IMPL_LINK( OColorPopup, SelectHdl, void *, EMPTYARG )
{
    USHORT nItemId = m_aColorSet.GetSelectItemId();
    Color aColor( nItemId == 0 ? Color( COL_TRANSPARENT ) : m_aColorSet.GetItemColor( nItemId ) );

    /*  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() calls.
        This instance may be deleted in the meantime (i.e. when a dialog is opened
        while in Dispatch()), accessing members will crash in this case. */
    m_aColorSet.SetNoSelection();

    if ( IsInPopupMode() )
        EndPopupMode();

    m_pCondition->ApplyCommand( m_nSlotId, aColor );
    return 0;
}

// =============================================================================
// = IExpressionFactory
// =============================================================================
class SAL_NO_VTABLE IExpressionFactory
{
public:
    virtual ::rtl::OUString     assembleExpression( const ::rtl::OUString& _rFieldDataSource, const ::rtl::OUString& _rLHS, const ::rtl::OUString& _rRHS ) const = 0;
    virtual bool                matchExpression( const ::rtl::OUString& _rExpression, const ::rtl::OUString& _rFieldDataSource, ::rtl::OUString& _out_rLHS, ::rtl::OUString& _out_rRHS ) const = 0;

    virtual ~IExpressionFactory() { }
};

// =============================================================================
// = ExpressionFactory
// =============================================================================
class ExpressionFactory : public IExpressionFactory
{
private:
    const ::rtl::OUString       m_sPattern;

public:
    ExpressionFactory( const sal_Char* _pAsciiPattern );

    // IExpressionFactory
    virtual ::rtl::OUString     assembleExpression( const ::rtl::OUString& _rFieldDataSource, const ::rtl::OUString& _rLHS, const ::rtl::OUString& _rRHS ) const;
    virtual bool                matchExpression( const ::rtl::OUString& _rExpression, const ::rtl::OUString& _rFieldDataSource, ::rtl::OUString& _out_rLHS, ::rtl::OUString& _out_rRHS ) const;
};

// -----------------------------------------------------------------------------
ExpressionFactory::ExpressionFactory( const sal_Char* _pAsciiPattern )
    :m_sPattern( ::rtl::OUString::createFromAscii( _pAsciiPattern ) )
{
}

// -----------------------------------------------------------------------------
::rtl::OUString ExpressionFactory::assembleExpression( const ::rtl::OUString& _rFieldDataSource, const ::rtl::OUString& _rLHS, const ::rtl::OUString& _rRHS ) const
{
    ::rtl::OUString sExpression( m_sPattern );

    sal_Int32 nPatternIndex = sExpression.indexOf( '$' );
    while ( nPatternIndex > -1 )
    {
        const ::rtl::OUString* pReplace = NULL;
        switch ( sExpression.getStr()[ nPatternIndex + 1 ] )
        {
        case '$': pReplace = &_rFieldDataSource; break;
        case '1': pReplace = &_rLHS; break;
        case '2': pReplace = &_rRHS; break;
        default: break;
        }

        if ( pReplace == NULL )
        {
            OSL_ENSURE( false, "ExpressionFactory::assembleExpression: illegal pattern!" );
            break;
        }

        sExpression = sExpression.replaceAt( nPatternIndex, 2, *pReplace );
        nPatternIndex = sExpression.indexOf( '$', nPatternIndex + pReplace->getLength() + 1 );
    }
    return sExpression;
}

// -----------------------------------------------------------------------------
bool ExpressionFactory::matchExpression( const ::rtl::OUString& _rExpression, const ::rtl::OUString& _rFieldDataSource, ::rtl::OUString& _out_rLHS, ::rtl::OUString& _out_rRHS ) const
{
    (void)_rExpression;
    (void)_rFieldDataSource;
    (void)_out_rLHS;
    (void)_out_rRHS;

    // if we had regular expression, the matching would be pretty easy ...
    // just replace $1 and $2 in the pattern with (.*), and then get them with \1 resp. \2.
    // Unfortunately, we don't have such a regexp engine ...

    // Okay, let's start with replacing all $$ in our pattern with the actual field data source
    ::rtl::OUString sMatchExpression( m_sPattern );
    const ::rtl::OUString sFieldDataPattern( RTL_CONSTASCII_USTRINGPARAM( "$$" ) );
    sal_Int32 nIndex( sMatchExpression.indexOf( sFieldDataPattern ) );
    while ( nIndex != -1 )
    {
        sMatchExpression = sMatchExpression.replaceAt( nIndex, sFieldDataPattern.getLength(), _rFieldDataSource );
        nIndex = sMatchExpression.indexOf( sFieldDataPattern, nIndex + _rFieldDataSource.getLength() );
    }

    const ::rtl::OUString sLHSPattern( RTL_CONSTASCII_USTRINGPARAM( "$1" ) );
    const ::rtl::OUString sRHSPattern( RTL_CONSTASCII_USTRINGPARAM( "$2" ) );
    sal_Int32 nLHSIndex( sMatchExpression.indexOf( sLHSPattern ) );
    sal_Int32 nRHSIndex( sMatchExpression.indexOf( sRHSPattern ) );

    // now we should have at most one occurance of $1 and $2, resp.
    OSL_ENSURE( sMatchExpression.indexOf( sLHSPattern, nLHSIndex + 1 ) == -1,
        "ExpressionFactory::matchExpression: unsupported pattern (more than one LHS occurance)!" );
    OSL_ENSURE( sMatchExpression.indexOf( sRHSPattern, nRHSIndex + 1 ) == -1,
        "ExpressionFactory::matchExpression: unsupported pattern (more than one RHS occurance)!" );
    // Also, an LHS must be present, and precede the RHS (if present)
    OSL_ENSURE( ( nLHSIndex != -1 ) && ( ( nLHSIndex < nRHSIndex ) || ( nRHSIndex == -1 ) ),
        "ExpressionFactory::matchExpression: no LHS, or an RHS preceeding the LHS - this is not supported!" );

    // up to the occurance of the LHS (which must exist, see above), the two expressions
    // must be identical
    if ( _rExpression.getLength() < nLHSIndex )
        return false;
    const ::rtl::OUString sExprPart1( _rExpression.copy( 0, nLHSIndex ) );
    const ::rtl::OUString sMatchExprPart1( sMatchExpression.copy( 0, nLHSIndex ) );
    if ( sExprPart1 != sMatchExprPart1 )
        // the left-most expression parts do not match
        return false;

    // after the occurance of the RHS (or the LHS, if there is no RHS), the two expressions
    // must be identical, too
    bool bHaveRHS( nRHSIndex != -1 );
    sal_Int32 nRightMostIndex( bHaveRHS ? nRHSIndex : nLHSIndex );
    const ::rtl::OUString sMatchExprPart3( sMatchExpression.copy( nRightMostIndex + 2 ) );
    if ( _rExpression.getLength() < sMatchExprPart3.getLength() )
        // the expression is not even long enough to hold the right-most part of the match expression
        return false;
    const ::rtl::OUString sExprPart3( _rExpression.copy( _rExpression.getLength() - sMatchExprPart3.getLength() ) );
    if ( sExprPart3 != sMatchExprPart3 )
        // the right-most expression parts do not match
        return false;

    // if we don't have an RHS, we're done
    if ( !bHaveRHS )
    {
        _out_rLHS = _rExpression.copy( sExprPart1.getLength(), _rExpression.getLength() - sExprPart1.getLength() - sExprPart3.getLength() );
        return true;
    }

    // strip the match expression by its right-most and left-most part, and by the placeholders $1 and $2
    sal_Int32 nMatchExprPart2Start( nLHSIndex + sLHSPattern.getLength() );
    ::rtl::OUString sMatchExprPart2 = sMatchExpression.copy(
        nMatchExprPart2Start,
        sMatchExpression.getLength() - nMatchExprPart2Start - sMatchExprPart3.getLength() - 2
    );
    // strip the expression by its left-most and right-most part
    const ::rtl::OUString sExpression( _rExpression.copy(
        sExprPart1.getLength(),
        _rExpression.getLength() - sExprPart1.getLength() - sExprPart3.getLength()
    ) );

    sal_Int32 nPart2Index = sExpression.indexOf( sMatchExprPart2 );
    if ( nPart2Index == -1 )
        // the "middle" part of the match expression does not exist in the expression at all
        return false;

    OSL_ENSURE( sExpression.indexOf( sMatchExprPart2, nPart2Index + 1 ) == -1,
        "ExpressionFactory::matchExpression: ambiguous matching!" );
        // if this fires, then we're lost: The middle part exists two times in the expression,
        // so we cannot reliably determine what's the LHS and what's the RHS.
        // Well, the whole mechanism is flawed, anyway:
        // Encoding the field content in the conditional expression will break as soon
        // as somebody
        // - assigns a Data Field to a control
        // - creates a conditional format expression for the control
        // - assigns another Data Field to the control
        // - opens the Conditional Format Dialog, again
        // Here, at the latest, you can see that we need another mechanism, anyway, which does not
        // rely on those strange expression building/matching

    _out_rLHS = sExpression.copy( 0, nPart2Index );
    _out_rRHS = sExpression.copy( nPart2Index + sMatchExprPart2.getLength() );

    return true;
}

// =============================================================================
// = Condition
// =============================================================================
// -----------------------------------------------------------------------------
Condition::Condition( Window* _pParent, IConditionalFormatAction& _rAction, ::rptui::OReportController& _rController )
    :Control(_pParent, ModuleRes(WIN_CONDITION))
    ,m_rController( _rController )
    ,m_rAction( _rAction )
    ,m_aHeader(this,            ModuleRes(FL_CONDITION_HEADER))
    ,m_aConditionType(this,       ModuleRes(LB_COND_TYPE))
    ,m_aOperationList( this,    ModuleRes(LB_OP))
    ,m_aCondLHS(this,           ModuleRes(ED_CONDITION_LHS))
    ,m_aOperandGlue(this,            ModuleRes(FT_AND))
    ,m_aCondRHS(this,           ModuleRes(ED_CONDITION_RHS))
    ,m_aActions(this,           ModuleRes(TB_FORMAT))
    ,m_aPreview(this,           ModuleRes(CRTL_FORMAT_PREVIEW))
    ,m_aMoveUp( this,           ModuleRes( BTN_MOVE_UP ) )
    ,m_aMoveDown( this,         ModuleRes( BTN_MOVE_DOWN ) )
    ,m_aAddCondition( this,     ModuleRes( BTN_ADD_CONDITION ) )
    ,m_aRemoveCondition( this,  ModuleRes( BTN_REMOVE_CONDITION ) )
    ,m_pColorFloat(NULL)
    ,m_pBtnUpdaterFontColor(NULL)
    ,m_pBtnUpdaterBackgroundColor(NULL)
    ,m_nCondIndex( 0 )
    ,m_nLastKnownWindowWidth( -1 )
    ,m_bInDestruction( false )
{
    m_aActions.SetStyle(m_aActions.GetStyle()|WB_LINESPACING);
    m_aCondLHS.GrabFocus();

    m_aConditionType.SetSelectHdl( LINK( this, Condition, OnTypeSelected ) );

    m_aOperationList.SetDropDownLineCount( 10 );
    m_aOperationList.SetSelectHdl( LINK( this, Condition, OnOperationSelected ) );

    m_aActions.SetSelectHdl(LINK(this, Condition, OnFormatAction));
    m_aActions.SetDropdownClickHdl( LINK( this, Condition, DropdownClick ) );
    setToolBox(&m_aActions);

    m_aMoveUp.SetClickHdl( LINK( this, Condition, OnConditionAction ) );
    m_aMoveDown.SetClickHdl( LINK( this, Condition, OnConditionAction ) );
    m_aAddCondition.SetClickHdl( LINK( this, Condition, OnConditionAction ) );
    m_aRemoveCondition.SetClickHdl( LINK( this, Condition, OnConditionAction ) );

    m_aMoveUp.SetStyle( m_aMoveUp.GetStyle() | WB_NOPOINTERFOCUS );
    m_aMoveDown.SetStyle( m_aMoveDown.GetStyle() | WB_NOPOINTERFOCUS );
    m_aAddCondition.SetStyle( m_aMoveUp.GetStyle() | WB_NOPOINTERFOCUS | WB_CENTER | WB_VCENTER );
    m_aRemoveCondition.SetStyle( m_aMoveDown.GetStyle() | WB_NOPOINTERFOCUS | WB_CENTER | WB_VCENTER );

    Font aFont( m_aAddCondition.GetFont() );
    aFont.SetWeight( WEIGHT_BOLD );
    m_aAddCondition.SetFont( aFont );
    m_aRemoveCondition.SetFont( aFont );

    m_aOperandGlue.SetStyle( m_aOperandGlue.GetStyle() | WB_VCENTER );

    m_aConditionType.SelectEntryPos( 0 );
    m_aOperationList.SelectEntryPos( 0 );

    // the toolbar got its site automatically, ensure that the preview is positioned
    // right of it
    Size aRelatedControls( LogicToPixel( Size( RELATED_CONTROLS, 0 ), MAP_APPFONT ) );
    Point aToolbarPos( m_aActions.GetPosPixel() );
    Size aToolbarSize( m_aActions.GetSizePixel() );
    m_aPreview.SetPosSizePixel( aToolbarPos.X() + aToolbarSize.Width() + 2 * aRelatedControls.Width(),
        0, 0, 0, WINDOW_POSSIZE_X );

    // ensure the toolbar is vertically centered, relative to the preview
    Size aPreviewSize( m_aPreview.GetSizePixel() );
    m_aActions.SetPosSizePixel( 0, aToolbarPos.Y() + ( aPreviewSize.Height() - aToolbarSize.Height() ) / 2, 0, 0, WINDOW_POSSIZE_Y );

    m_pBtnUpdaterBackgroundColor = new ::svx::ToolboxButtonColorUpdater(
                                            SID_BACKGROUND_COLOR, SID_BACKGROUND_COLOR, &m_aActions );
    m_pBtnUpdaterFontColor = new ::svx::ToolboxButtonColorUpdater(
                                            SID_ATTR_CHAR_COLOR2, SID_ATTR_CHAR_COLOR2, &m_aActions, TBX_UPDATER_MODE_CHAR_COLOR_NEW );


    m_aMoveUp.SetModeImage( ModuleRes( IMG_MOVE_UP_HC ), BMP_COLOR_HIGHCONTRAST );
    m_aMoveDown.SetModeImage( ModuleRes( IMG_MOVE_DOWN_HC ), BMP_COLOR_HIGHCONTRAST );

    FreeResource();
    Show();

    impl_layoutAll();

    m_aFieldExprFactories[ eBetween ]        = PExpressionFactory( new ExpressionFactory( "AND( ( $$ ) >= ( $1 ); ( $$ ) <= ( $2 ) )" ) );
    m_aFieldExprFactories[ eNotBetween ]     = PExpressionFactory( new ExpressionFactory( "NOT( AND( ( $$ ) >= ( $1 ); ( $$ ) <= ( $2 ) ) )" ) );
    m_aFieldExprFactories[ eEqualTo ]        = PExpressionFactory( new ExpressionFactory( "( $$ ) = ( $1 )" ) );
    m_aFieldExprFactories[ eNotEqualTo ]     = PExpressionFactory( new ExpressionFactory( "( $$ ) <> ( $1 )" ) );
    m_aFieldExprFactories[ eGreaterThan ]    = PExpressionFactory( new ExpressionFactory( "( $$ ) > ( $1 )" ) );
    m_aFieldExprFactories[ eLessThan ]       = PExpressionFactory( new ExpressionFactory( "( $$ ) < ( $1 )" ) );
    m_aFieldExprFactories[ eGreaterOrEqual ] = PExpressionFactory( new ExpressionFactory( "( $$ ) >= ( $1 )" ) );
    m_aFieldExprFactories[ eLessOrEqual ]    = PExpressionFactory( new ExpressionFactory( "( $$ ) <= ( $1 )" ) );
}

// -----------------------------------------------------------------------------
Condition::~Condition()
{
    m_bInDestruction = true;

    delete m_pColorFloat;
    delete m_pBtnUpdaterFontColor;
    delete m_pBtnUpdaterBackgroundColor;
}
// -----------------------------------------------------------------------------
IMPL_LINK( Condition, DropdownClick, ToolBox*, /*pToolBar*/ )
{
    USHORT nId( m_aActions.GetCurItemId() );
    if ( !m_pColorFloat )
        m_pColorFloat = new OColorPopup(&m_aActions,this);

    USHORT nTextId = 0;
    switch(nId)
    {
        case SID_ATTR_CHAR_COLOR2:
            nTextId = STR_CHARCOLOR;
            break;
        case SID_BACKGROUND_COLOR:
            nTextId = STR_CHARBACKGROUND;
            break;
        default:
            break;
    } // switch(nId)
    if ( nTextId )
        m_pColorFloat->SetText(String(ModuleRes(nTextId)));
    m_pColorFloat->SetSlotId(nId);
    m_pColorFloat->SetPosPixel(m_aActions.GetItemPopupPosition(nId,m_pColorFloat->GetSizePixel()));
    m_pColorFloat->StartPopupMode(&m_aActions);
    m_pColorFloat->StartSelection();

    return 1;
}
//------------------------------------------------------------------
IMPL_LINK( Condition, OnFormatAction, ToolBox*, /*NOTINTERESTEDIN*/ )
{
    Color aCol(COL_AUTO);
    ApplyCommand(m_aActions.GetCurItemId(),aCol);
    return 0L;
}

//------------------------------------------------------------------
IMPL_LINK( Condition, OnConditionAction, Button*, _pClickedButton )
{
    if ( _pClickedButton == &m_aMoveUp )
        m_rAction.moveConditionUp( getConditionIndex() );
    else if ( _pClickedButton == &m_aMoveDown )
        m_rAction.moveConditionDown( getConditionIndex() );
    else if ( _pClickedButton == &m_aAddCondition )
        m_rAction.addCondition( getConditionIndex() );
    else if ( _pClickedButton == &m_aRemoveCondition )
        m_rAction.deleteCondition( getConditionIndex() );
    return 0L;
}

//------------------------------------------------------------------------------
void Condition::ApplyCommand( USHORT _nCommandId, const ::Color& _rColor)
{
    if ( _nCommandId == SID_ATTR_CHAR_COLOR2 )
        m_pBtnUpdaterFontColor->Update( _rColor );
    else if ( _nCommandId == SID_BACKGROUND_COLOR )
        m_pBtnUpdaterBackgroundColor->Update( _rColor );

    m_rAction.applyCommand( m_nCondIndex, _nCommandId, _rColor );
}
//------------------------------------------------------------------------------
ImageList Condition::getImageList(sal_Int16 _eBitmapSet,sal_Bool _bHiContast) const
{
    sal_Int16 nN = IMG_CONDFORMAT_DLG_SC;
    sal_Int16 nH = IMG_CONDFORMAT_DLG_SCH;
    if ( _eBitmapSet == SFX_SYMBOLS_SIZE_LARGE )
    {
        nN = IMG_CONDFORMAT_DLG_LC;
        nH = IMG_CONDFORMAT_DLG_LCH;
    }
    return ImageList(ModuleRes( _bHiContast ? nH : nN ));
}
//------------------------------------------------------------------
void Condition::resizeControls(const Size& _rDiff)
{
    // we use large images so we must change them
    if ( _rDiff.Width() || _rDiff.Height() )
    {
        Point aPos = LogicToPixel( Point( 2*RELATED_CONTROLS , 0), MAP_APPFONT );
        Invalidate();
    }
}
// -----------------------------------------------------------------------------
void Condition::Paint( const Rectangle& rRect )
{
    Control::Paint(rRect);

    // draw border
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    ColorChanger aColors( this, rStyleSettings.GetShadowColor(), rStyleSettings.GetDialogColor() );
    DrawRect( impl_getToolBarBorderRect() );
}
// -----------------------------------------------------------------------------
void Condition::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        // Check if we need to get new images for normal/high contrast mode
        checkImageList();
    }
    else if ( nType == STATE_CHANGE_TEXT )
    {
        // The physical toolbar changed its outlook and shows another logical toolbar!
        // We have to set the correct high contrast mode on the new tbx manager.
        //  pMgr->SetHiContrast( IsHiContrastMode() );
        checkImageList();
    }
}
// -----------------------------------------------------------------------------
void Condition::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ((( rDCEvt.GetType() == DATACHANGED_SETTINGS )   ||
        ( rDCEvt.GetType() == DATACHANGED_DISPLAY   ))  &&
        ( rDCEvt.GetFlags() & SETTINGS_STYLE        ))
    {
        // Check if we need to get new images for normal/high contrast mode
        checkImageList();
    }
}

// -----------------------------------------------------------------------------
void Condition::GetFocus()
{
    Control::GetFocus();
    if ( !m_bInDestruction )
        m_aCondLHS.GrabFocus();
}

// -----------------------------------------------------------------------------
void Condition::Resize()
{
    Control::Resize();
    impl_layoutAll();
}

// -----------------------------------------------------------------------------
Rectangle Condition::impl_getToolBarBorderRect() const
{
    const Point aToolbarPos( m_aActions.GetPosPixel() );
    const Size aToolbarSize( m_aActions.GetSizePixel() );
    const Size aRelatedControls = LogicToPixel( Size( RELATED_CONTROLS, RELATED_CONTROLS ), MAP_APPFONT );

    Rectangle aBorderRect( aToolbarPos, aToolbarSize );
    aBorderRect.Left() -= aRelatedControls.Width();
    aBorderRect.Top() -= aRelatedControls.Height();
    aBorderRect.Right() += aRelatedControls.Width();
    aBorderRect.Bottom() += aRelatedControls.Height();

    return aBorderRect;
}

// -----------------------------------------------------------------------------
void Condition::impl_layoutAll()
{
    // if our width changed, resize/-position some controls
    const Size aSize( GetOutputSizePixel() );
    if ( aSize.Width() == m_nLastKnownWindowWidth )
        return;

    m_nLastKnownWindowWidth = aSize.Width();

    const Size aRelatedControls( LogicToPixel( Size( RELATED_CONTROLS, RELATED_CONTROLS ), MAP_APPFONT ) );
    const Size aUnrelatedControls( LogicToPixel( Size( UNRELATED_CONTROLS, 0 ), MAP_APPFONT ) );
    const Point aRow1( LogicToPixel( Point( 0, ROW_1_POS ), MAP_APPFONT ) );
    const Point aRow3( LogicToPixel( Point( 0, ROW_3_POS ), MAP_APPFONT ) );

    // resize the header line
    m_aHeader.SetPosSizePixel( 0, 0, aSize.Width() - 2 * aRelatedControls.Width(), 0, WINDOW_POSSIZE_WIDTH );

    // position the up/down buttons
    const Size aButtonSize( LogicToPixel( Size( IMAGE_BUTTON_WIDTH, IMAGE_BUTTON_HEIGHT ), MAP_APPFONT ) );
    Point aButtonPos( aSize.Width() - aUnrelatedControls.Width() - aButtonSize.Width(), aRow1.Y() );
    m_aMoveUp.SetPosSizePixel( aButtonPos.X(), aButtonPos.Y(), aButtonSize.Width(), aButtonSize.Height() );
    aButtonPos.Move( 0, aButtonSize.Height() + aRelatedControls.Height() );
    m_aMoveDown.SetPosSizePixel( aButtonPos.X(), aButtonPos.Y(), aButtonSize.Width(), aButtonSize.Height() );

    // resize the preview
    const long nNewPreviewRight = aButtonPos.X() - aRelatedControls.Width();

    const Point aPreviewPos( m_aPreview.GetPosPixel() );
    OSL_ENSURE( aPreviewPos.X() < nNewPreviewRight, "Condition::impl_layoutAll: being *that* small should not be allowed!" );
    m_aPreview.SetPosSizePixel( 0, 0, nNewPreviewRight - aPreviewPos.X(), 0, WINDOW_POSSIZE_WIDTH );

    // position the add/remove buttons
    aButtonPos = Point( nNewPreviewRight - aButtonSize.Width(), aRow3.Y() );
    m_aRemoveCondition.SetPosSizePixel( aButtonPos.X(), aButtonPos.Y(), aButtonSize.Width(), aButtonSize.Height() );
    aButtonPos.Move( -( aButtonSize.Width() + aRelatedControls.Width() ), 0 );
    m_aAddCondition.SetPosSizePixel( aButtonPos.X(), aButtonPos.Y(), aButtonSize.Width(), aButtonSize.Height() );

    // layout the operands input controls
    impl_layoutOperands();
}

// -----------------------------------------------------------------------------
IMPL_LINK( Condition, OnTypeSelected, ListBox*, /*_pNotInterestedIn*/ )
{
    impl_layoutOperands();
    return 0L;
}

// -----------------------------------------------------------------------------
IMPL_LINK( Condition, OnOperationSelected, ListBox*, /*_pNotInterestedIn*/ )
{
    impl_layoutOperands();
    return 0L;
}

// -----------------------------------------------------------------------------
void Condition::impl_layoutOperands()
{
    const ConditionType eType( impl_getCurrentConditionType() );
    const ComparisonOperation eOperation( impl_getCurrentComparisonOperation() );

    const bool bIsExpression = ( eType == eExpression );
    const bool bHaveRHS =
            (   ( eType == eFieldValueComparison )
            &&  (   ( eOperation == eBetween )
                ||  ( eOperation == eNotBetween )
                )
            );

    const Size aRelatedControls( LogicToPixel( Size( RELATED_CONTROLS, 0 ), MAP_APPFONT ) );
    const Rectangle aPreviewRect( m_aPreview.GetPosPixel(), m_aPreview.GetSizePixel() );

    // the "condition type" list box
    Rectangle aCondTypeRect( m_aConditionType.GetPosPixel(), m_aConditionType.GetSizePixel() );
    Point aOpListPos( aCondTypeRect.Right() + aRelatedControls.Width(), aCondTypeRect.Top() );
    Size aOpListSize( LogicToPixel( Size( COND_OP_WIDTH, 60 ), MAP_APPFONT ) );
    m_aOperationList.SetPosSizePixel( aOpListPos.X(), aOpListPos.Y(),
        aOpListSize.Width(), aOpListSize.Height() );
    m_aOperationList.Show( !bIsExpression );

    // the LHS input field
    Point aLHSPos( aOpListPos.X() + aOpListSize.Width() + aRelatedControls.Width(), aOpListPos.Y() );
    if ( bIsExpression )
        aLHSPos.X() = aOpListPos.X();
    Size aLHSSize( LogicToPixel( Size( EDIT_WIDTH, EDIT_HEIGHT ), MAP_APPFONT ) );
    if ( !bHaveRHS )
        aLHSSize.Width() = aPreviewRect.Right() - aLHSPos.X();
    m_aCondLHS.SetPosSizePixel( aLHSPos.X(), aLHSPos.Y(), aLHSSize.Width(), aLHSSize.Height() );

    if ( bHaveRHS )
    {
        // the "and" text being the glue between LHS and RHS
        Point aOpGluePos( aLHSPos.X() + aLHSSize.Width() + aRelatedControls.Width(), aLHSPos.Y() );
        Size aOpGlueSize( m_aOperandGlue.GetTextWidth( m_aOperandGlue.GetText() ) + aRelatedControls.Width(), aLHSSize.Height() );
        m_aOperandGlue.SetPosSizePixel( aOpGluePos.X(), aOpGluePos.Y(), aOpGlueSize.Width(), aOpGlueSize.Height() );

        // the RHS input field
        Point aRHSPos( aOpGluePos.X() + aOpGlueSize.Width() + aRelatedControls.Width(), aOpGluePos.Y() );
        Size aRHSSize( aPreviewRect.Right() - aRHSPos.X(), aLHSSize.Height() );
        m_aCondRHS.SetPosSizePixel( aRHSPos.X(), aRHSPos.Y(), aRHSSize.Width(), aRHSSize.Height() );
    }

    m_aOperandGlue.Show( bHaveRHS );
    m_aCondRHS.Show( bHaveRHS );
}

// -----------------------------------------------------------------------------
void Condition::impl_setCondition( const ::rtl::OUString& _rConditionFormula )
{
    // determine the condition's type and comparison operation
    ConditionType eType( eFieldValueComparison );
    ComparisonOperation eOperation( eBetween );

    // LHS and RHS, matched below
    ::rtl::OUString sLHS, sRHS;

    if ( _rConditionFormula.getLength() )
    {
        // the unprefixed expression which forms the condition
        ReportFormula aFormula( _rConditionFormula );
        OSL_ENSURE( aFormula.getType() == ReportFormula::Expression, "Condition::setCondition: illegal formula!" );
        ::rtl::OUString sExpression;
        if ( aFormula.getType() == ReportFormula::Expression )
            sExpression = aFormula.getExpression();
        // as fallback, if the below matching does not succeed, assume
        // the whole expression is the LHS
        sLHS = sExpression;

        // the data field (or expression) to which our control is bound
        const ReportFormula aFieldContentFormula( m_rAction.getDataField() );
        const ::rtl::OUString sUnprefixedFieldContent( aFieldContentFormula.getBracketedFieldOrExpression() );

        // check whether one of the Field Value Expression Factories recognizes the expression
        for (   ExpressionFactories::const_iterator fac = m_aFieldExprFactories.begin();
                fac != m_aFieldExprFactories.end();
                ++fac
            )
        {
            if ( fac->second->matchExpression( sExpression, sUnprefixedFieldContent, sLHS, sRHS ) )
            {
                eType = eFieldValueComparison;
                eOperation = fac->first;
                break;
            }
        }
    }

    // update UI
    m_aConditionType.SelectEntryPos( (USHORT)eType );
    m_aOperationList.SelectEntryPos( (USHORT)eOperation );
    m_aCondLHS.SetText( sLHS );
    m_aCondRHS.SetText( sRHS );

    // re-layout
    impl_layoutOperands();
}

// -----------------------------------------------------------------------------
void Condition::setCondition( const uno::Reference< report::XFormatCondition >& _rxCondition )
{
    OSL_PRECOND( _rxCondition.is(), "Condition::setCondition: empty condition object!" );
    if ( !_rxCondition.is() )
        return;

    ::rtl::OUString sConditionFormula;
    try
    {
        if ( _rxCondition.is() )
            sConditionFormula =  _rxCondition->getFormula();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    impl_setCondition( sConditionFormula );
    updateToolbar( _rxCondition.get() );
}

// -----------------------------------------------------------------------------
void Condition::updateToolbar(const uno::Reference< report::XReportControlFormat >& _xReportControlFormat)
{
    OSL_ENSURE(_xReportControlFormat.is(),"XReportControlFormat is NULL!");
    if ( _xReportControlFormat.is() )
    {
        USHORT nItemCount = m_aActions.GetItemCount();
        for (USHORT j = 0; j< nItemCount; ++j)
        {
            USHORT nItemId = m_aActions.GetItemId(j);
            m_aActions.CheckItem( nItemId, m_rController.isFormatCommandEnabled( nItemId, _xReportControlFormat ) );
        }

        try
        {
            Font aBaseFont( Application::GetDefaultDevice()->GetSettings().GetStyleSettings().GetAppFont() );
            SvxFont aFont( VCLUnoHelper::CreateFont( _xReportControlFormat->getFontDescriptor(), aBaseFont ) );
            aFont.SetHeight( OutputDevice::LogicToLogic( Size( 0, (sal_Int32)aFont.GetHeight() ), MAP_POINT, MAP_TWIP ).Height());
            aFont.SetEmphasisMark( static_cast< FontEmphasisMark >( _xReportControlFormat->getControlTextEmphasis() ) );
            aFont.SetRelief( static_cast< FontRelief >( _xReportControlFormat->getCharRelief() ) );
            aFont.SetColor( _xReportControlFormat->getCharColor() );
            m_aPreview.SetFont( aFont );
            m_aPreview.SetBackColor( _xReportControlFormat->getControlBackground() );
            m_aPreview.SetTextLineColor( Color( _xReportControlFormat->getCharUnderlineColor() ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}
// -----------------------------------------------------------------------------
void Condition::fillFormatCondition(const uno::Reference< report::XFormatCondition >& _xCondition)
{
    const ConditionType eType( impl_getCurrentConditionType() );
    const ComparisonOperation eOperation( impl_getCurrentComparisonOperation() );

    const ::rtl::OUString sLHS( m_aCondLHS.GetText() );
    const ::rtl::OUString sRHS( m_aCondRHS.GetText() );

    ::rtl::OUString sUndecoratedFormula( sLHS );

    if ( eType == eFieldValueComparison )
    {
        ReportFormula aFieldContentFormula( m_rAction.getDataField() );
        ::rtl::OUString sUnprefixedFieldContent( aFieldContentFormula.getBracketedFieldOrExpression() );

        PExpressionFactory pFactory( m_aFieldExprFactories[ eOperation ] );
        sUndecoratedFormula = pFactory->assembleExpression( sUnprefixedFieldContent, sLHS, sRHS );
    }

    ReportFormula aFormula( ReportFormula::Expression, sUndecoratedFormula );
    _xCondition->setFormula( aFormula.getCompleteFormula() );
}
// -----------------------------------------------------------------------------
void Condition::setConditionIndex( size_t _nCondIndex, size_t _nCondCount )
{
    m_nCondIndex = _nCondIndex;
    String sHeader( ModuleRes( STR_NUMBERED_CONDITION ) );
    sHeader.SearchAndReplaceAscii( "$number$", String::CreateFromInt32( _nCondIndex + 1 ) );
    m_aHeader.SetText( sHeader );

    m_aMoveUp.Enable( _nCondIndex > 0 );
    OSL_PRECOND( _nCondCount > 0, "Condition::setConditionIndex: having no conditions at all is nonsense!" );
    m_aMoveDown.Enable( _nCondIndex < _nCondCount - 1 );
}

// -----------------------------------------------------------------------------
bool Condition::isEmpty() const
{
    return m_aCondLHS.GetText().Len() == 0;
}

// =============================================================================
} // rptui
// =============================================================================

