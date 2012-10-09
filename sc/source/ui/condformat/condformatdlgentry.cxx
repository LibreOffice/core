#include "condformatdlg.hxx"
#include "condformatdlgentry.hxx"
#include "condformatdlg.hrc"
#include "conditio.hxx"

#include "document.hxx"

#include <vcl/vclevent.hxx>
#include <svl/style.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <vcl/msgbox.hxx>
#include "stlpool.hxx"
#include "tabvwsh.hxx"

#include "colorformat.hxx"

#include "globstr.hrc"

ScCondFrmtEntry::ScCondFrmtEntry(Window* pParent, ScDocument* pDoc, const ScAddress& rPos):
    Control(pParent, ScResId( RID_COND_ENTRY ) ),
    mbActive(false),
    maFtCondNr( this, ScResId( FT_COND_NR ) ),
    maFtCondition( this, ScResId( FT_CONDITION ) ),
    mnIndex(0),
    maStrCondition(ScResId( STR_CONDITION ).toString()),
    maLbType( this, ScResId( LB_TYPE ) ),
    mpDoc(pDoc),
    maPos(rPos)
{
    SetControlBackground(GetSettings().GetStyleSettings().GetDialogColor());

    /*
    */
    //disable entries for color formats

    maLbType.SetSelectHdl( LINK( pParent, ScCondFormatList, TypeListHdl ) );
    Init();
    maClickHdl = LINK( pParent, ScCondFormatList, EntrySelectHdl );
}

ScCondFrmtEntry::~ScCondFrmtEntry()
{
}

void ScCondFrmtEntry::Init()
{
    /*

    */
}

long ScCondFrmtEntry::Notify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
    {
        ImplCallEventListenersAndHandler( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, maClickHdl, this );
    }
    return Control::Notify(rNEvt);
}

void ScCondFrmtEntry::SetIndex(sal_Int32 nIndex)
{
    mnIndex = nIndex;
    rtl::OUStringBuffer aBuffer(maStrCondition);
    aBuffer.append(rtl::OUString::valueOf(nIndex));
    maFtCondNr.SetText(aBuffer.makeStringAndClear());
}

void ScCondFrmtEntry::SetHeight()
{
    long nPad = LogicToPixel(Size(42,2), MapMode(MAP_APPFONT)).getHeight();

    // Calculate maximum height we need from visible widgets
    sal_uInt16 nChildren = GetChildCount();

    long nMaxHeight = 0;
    for(sal_uInt16 i = 0; i < nChildren; i++)
    {
        Window *pChild  = GetChild(i);
        if(!pChild || !pChild->IsVisible())
            continue;
        Point aPos = pChild->GetPosPixel();
        Size aSize = pChild->GetSizePixel();
        nMaxHeight = std::max(aPos.Y() + aSize.Height(), nMaxHeight);
    }
    Size aSize = GetSizePixel();
    if(nMaxHeight > 0)
    {
        aSize.Height() = nMaxHeight + nPad;
        SetSizePixel(aSize);
    }
}


void ScCondFrmtEntry::Select()
{
    maFtCondition.SetText(rtl::OUString());
    maFtCondition.Hide();
    maLbType.Show();
    mbActive = true;
    SetHeight();
}

void ScCondFrmtEntry::Deselect()
{
    rtl::OUString maCondText("deselected");// = ScCondFormatHelper::GetExpression(CONDITION, maLbCondType.GetSelectEntryPos());
    maFtCondition.SetText(maCondText);
    maFtCondition.Show();
    maLbType.Hide();
    mbActive = false;
    SetHeight();
}

bool ScCondFrmtEntry::IsSelected() const
{
    return mbActive;
}

IMPL_LINK(ScCondFrmtEntry, EdModifyHdl, Edit*, pEdit)
{
    rtl::OUString aFormula = pEdit->GetText();
    ScCompiler aComp( mpDoc, maPos );
    aComp.SetGrammar( mpDoc->GetGrammar() );
    boost::scoped_ptr<ScTokenArray> mpCode(aComp.CompileString(aFormula));
    if(mpCode->GetCodeError())
    {
        pEdit->SetControlBackground(COL_LIGHTRED);
    }
    else
    {
        pEdit->SetControlBackground(GetSettings().GetStyleSettings().GetWindowColor());
    }
    return 0;
}

//
//condition

ScConditionFrmtEntry::ScConditionFrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScCondFormatEntry* pFormatEntry ):
    ScCondFrmtEntry( pParent, pDoc, rPos ),
    maLbCondType( this, ScResId( LB_CELLIS_TYPE ) ),
    maEdVal1( this, ScResId( ED_VAL1 ) ),
    maEdVal2( this, ScResId( ED_VAL2 ) ),
    maFtStyle( this, ScResId( FT_STYLE ) ),
    maLbStyle( this, ScResId( LB_STYLE ) ),
    maWdPreview( this, ScResId( WD_PREVIEW ) )
{

    FreeResource();
    maLbType.SelectEntryPos(1);

    Init();

    if(pFormatEntry)
    {
        rtl::OUString aStyleName = pFormatEntry->GetStyle();
        maLbStyle.SelectEntry(aStyleName);
        StyleSelectHdl(NULL);
        ScConditionMode eMode = pFormatEntry->GetOperation();
        maEdVal1.SetText(pFormatEntry->GetExpression(maPos, 0));
        switch(eMode)
        {
            case SC_COND_EQUAL:
                maLbCondType.SelectEntryPos(0);
                break;
            case SC_COND_LESS:
                maLbCondType.SelectEntryPos(1);
                break;
            case SC_COND_GREATER:
                maLbCondType.SelectEntryPos(2);
                break;
            case SC_COND_EQLESS:
                maLbCondType.SelectEntryPos(3);
                break;
            case SC_COND_EQGREATER:
                maLbCondType.SelectEntryPos(4);
                break;
            case SC_COND_NOTEQUAL:
                maLbCondType.SelectEntryPos(5);
                break;
            case SC_COND_BETWEEN:
                maEdVal2.SetText(pFormatEntry->GetExpression(maPos, 1));
                maLbCondType.SelectEntryPos(6);
                break;
            case SC_COND_NOTBETWEEN:
                maEdVal2.SetText(pFormatEntry->GetExpression(maPos, 1));
                maLbCondType.SelectEntryPos(7);
                break;
            case SC_COND_DUPLICATE:
                maLbCondType.SelectEntryPos(8);
                break;
            case SC_COND_NOTDUPLICATE:
                maLbCondType.SelectEntryPos(9);
                break;
            case SC_COND_DIRECT:
                //maLbType.SelectEntryPos(2);
                break;
            case SC_COND_NONE:
                break;
        }
    }
    else
    {
        maLbCondType.SelectEntryPos(0);
        maEdVal2.Hide();
        maLbStyle.SelectEntryPos(1);
    }
    maLbType.SelectEntryPos(1);
}

void ScConditionFrmtEntry::Init()
{
    maLbStyle.SetSeparatorPos(0);

    maEdVal1.SetStyle( maEdVal1.GetStyle() | WB_FORCECTRLBACKGROUND );
    maEdVal2.SetStyle( maEdVal2.GetStyle() | WB_FORCECTRLBACKGROUND );

    maEdVal1.SetModifyHdl( LINK( this, ScCondFrmtEntry, EdModifyHdl ) );
    maEdVal2.SetModifyHdl( LINK( this, ScCondFrmtEntry, EdModifyHdl ) );

    SfxStyleSheetIterator aStyleIter( mpDoc->GetStyleSheetPool(), SFX_STYLE_FAMILY_PARA );
    for ( SfxStyleSheetBase* pStyle = aStyleIter.First(); pStyle; pStyle = aStyleIter.Next() )
    {
        rtl::OUString aName = pStyle->GetName();
        maLbStyle.InsertEntry( aName );
    }
    maLbStyle.SetSelectHdl( LINK( this, ScConditionFrmtEntry, StyleSelectHdl ) );

    maLbCondType.SetSelectHdl( LINK( this, ScConditionFrmtEntry, ConditionTypeSelectHdl ) );
}

ScFormatEntry* ScConditionFrmtEntry::createConditionEntry() const
{
    ScConditionMode eMode;
    rtl::OUString aExpr2;
    switch(maLbCondType.GetSelectEntryPos())
    {
        case 0:
            eMode = SC_COND_EQUAL;
            break;
        case 1:
            eMode = SC_COND_LESS;
            break;
        case 2:
            eMode = SC_COND_GREATER;
            break;
        case 3:
            eMode = SC_COND_EQLESS;
            break;
        case 4:
            eMode = SC_COND_EQGREATER;
            break;
        case 5:
            eMode = SC_COND_NOTEQUAL;
            break;
        case 6:
            aExpr2 = maEdVal2.GetText();
            eMode = SC_COND_BETWEEN;
            if(aExpr2.isEmpty())
                return NULL;
            break;
        case 7:
            eMode = SC_COND_NOTBETWEEN;
            aExpr2 = maEdVal2.GetText();
            if(aExpr2.isEmpty())
                return NULL;
            break;
        case 8:
            eMode = SC_COND_DUPLICATE;
            break;
        case 9:
            eMode = SC_COND_NOTDUPLICATE;
            break;
        default:
            assert(false); // this cannot happen
            return NULL;
    }

    rtl::OUString aExpr1 = maEdVal1.GetText();

    ScFormatEntry* pEntry = new ScCondFormatEntry(eMode, aExpr1, aExpr2, mpDoc, maPos, maLbStyle.GetSelectEntry());

    return pEntry;
}

ScFormatEntry* ScConditionFrmtEntry::GetEntry() const
{
    return createConditionEntry();
}

void ScConditionFrmtEntry::SetActive()
{
    maLbCondType.Show();
    maEdVal1.Show();
    maEdVal2.Show();
    maFtStyle.Show();
    maLbStyle.Show();
    maWdPreview.Show();

    Select();
}

void ScConditionFrmtEntry::SetInactive()
{
    maLbCondType.Hide();
    maEdVal1.Hide();
    maEdVal2.Hide();
    maFtStyle.Hide();
    maLbStyle.Hide();
    maWdPreview.Hide();

    Deselect();
}

IMPL_LINK_NOARG(ScConditionFrmtEntry, StyleSelectHdl)
{
    if(maLbStyle.GetSelectEntryPos() == 0)
    {
        // call new style dialog
        SfxUInt16Item aFamilyItem( SID_STYLE_FAMILY, SFX_STYLE_FAMILY_PARA );
        SfxStringItem aRefItem( SID_STYLE_REFERENCE, ScGlobal::GetRscString(STR_STYLENAME_STANDARD) );

        // unlock the dispatcher so SID_STYLE_NEW can be executed
        // (SetDispatcherLock would affect all Calc documents)
        ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
        SfxDispatcher* pDisp = pViewShell->GetDispatcher();
        sal_Bool bLocked = pDisp->IsLocked();
        if (bLocked)
            pDisp->Lock(false);

        // Execute the "new style" slot, complete with undo and all necessary updates.
        // The return value (SfxUInt16Item) is ignored, look for new styles instead.
        pDisp->Execute( SID_STYLE_NEW, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD | SFX_CALLMODE_MODAL,
                &aFamilyItem,
                &aRefItem,
                0L );

        if (bLocked)
            pDisp->Lock(sal_True);

        // Find the new style and add it into the style list boxes
        rtl::OUString aNewStyle;
        SfxStyleSheetIterator aStyleIter( mpDoc->GetStyleSheetPool(), SFX_STYLE_FAMILY_PARA );
        for ( SfxStyleSheetBase* pStyle = aStyleIter.First(); pStyle; pStyle = aStyleIter.Next() )
        {
            rtl::OUString aName = pStyle->GetName();
            if ( maLbStyle.GetEntryPos(aName) == LISTBOX_ENTRY_NOTFOUND )    // all lists contain the same entries
            {
                maLbStyle.InsertEntry(aName);
                maLbStyle.SelectEntry(aName);
            }
        }
    }

    rtl::OUString aStyleName = maLbStyle.GetSelectEntry();
    SfxStyleSheetBase* pStyleSheet = mpDoc->GetStyleSheetPool()->Find( aStyleName, SFX_STYLE_FAMILY_PARA );
    if(pStyleSheet)
    {
        const SfxItemSet& rSet = pStyleSheet->GetItemSet();
        maWdPreview.Init( rSet );
    }

    return 0;
}

// formula

ScFormulaFrmtEntry::ScFormulaFrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScCondFormatEntry* pFormat ):
    ScCondFrmtEntry( pParent, pDoc, rPos ),
    maFtStyle( this, ScResId( FT_STYLE ) ),
    maLbStyle( this, ScResId( LB_STYLE ) ),
    maWdPreview( this, ScResId( WD_PREVIEW ) ),
    maEdFormula( this, ScResId( ED_FORMULA ) )
{
    Init();

    FreeResource();
    maLbType.SelectEntryPos(2);

    if(pFormat)
    {
        maEdFormula.SetText(pFormat->GetExpression(rPos, 0, 0, pDoc->GetGrammar()));
        maLbStyle.SelectEntry(pFormat->GetStyle());
    }
    else
    {
        maLbStyle.SelectEntryPos(1);
    }
}

void ScFormulaFrmtEntry::Init()
{
    maLbStyle.SetSeparatorPos(0);

    SfxStyleSheetIterator aStyleIter( mpDoc->GetStyleSheetPool(), SFX_STYLE_FAMILY_PARA );
    for ( SfxStyleSheetBase* pStyle = aStyleIter.First(); pStyle; pStyle = aStyleIter.Next() )
    {
        rtl::OUString aName = pStyle->GetName();
        maLbStyle.InsertEntry( aName );
    }
    maLbStyle.SetSelectHdl( LINK( this, ScFormulaFrmtEntry, StyleSelectHdl ) );
}

IMPL_LINK_NOARG(ScFormulaFrmtEntry, StyleSelectHdl)
{
    if(maLbStyle.GetSelectEntryPos() == 0)
    {
        // call new style dialog
        SfxUInt16Item aFamilyItem( SID_STYLE_FAMILY, SFX_STYLE_FAMILY_PARA );
        SfxStringItem aRefItem( SID_STYLE_REFERENCE, ScGlobal::GetRscString(STR_STYLENAME_STANDARD) );

        // unlock the dispatcher so SID_STYLE_NEW can be executed
        // (SetDispatcherLock would affect all Calc documents)
        ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
        SfxDispatcher* pDisp = pViewShell->GetDispatcher();
        sal_Bool bLocked = pDisp->IsLocked();
        if (bLocked)
            pDisp->Lock(false);

        // Execute the "new style" slot, complete with undo and all necessary updates.
        // The return value (SfxUInt16Item) is ignored, look for new styles instead.
        pDisp->Execute( SID_STYLE_NEW, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD | SFX_CALLMODE_MODAL,
                &aFamilyItem,
                &aRefItem,
                0L );

        if (bLocked)
            pDisp->Lock(sal_True);

        // Find the new style and add it into the style list boxes
        rtl::OUString aNewStyle;
        SfxStyleSheetIterator aStyleIter( mpDoc->GetStyleSheetPool(), SFX_STYLE_FAMILY_PARA );
        for ( SfxStyleSheetBase* pStyle = aStyleIter.First(); pStyle; pStyle = aStyleIter.Next() )
        {
            rtl::OUString aName = pStyle->GetName();
            if ( maLbStyle.GetEntryPos(aName) == LISTBOX_ENTRY_NOTFOUND )    // all lists contain the same entries
            {
                maLbStyle.InsertEntry(aName);
                maLbStyle.SelectEntry(aName);
            }
        }
    }

    rtl::OUString aStyleName = maLbStyle.GetSelectEntry();
    SfxStyleSheetBase* pStyleSheet = mpDoc->GetStyleSheetPool()->Find( aStyleName, SFX_STYLE_FAMILY_PARA );
    if(pStyleSheet)
    {
        const SfxItemSet& rSet = pStyleSheet->GetItemSet();
        maWdPreview.Init( rSet );
    }

    return 0;
}

ScFormatEntry* ScFormulaFrmtEntry::createFormulaEntry() const
{
    ScConditionMode eMode = SC_COND_DIRECT;
    rtl::OUString aFormula = maEdFormula.GetText();
    if(aFormula.isEmpty())
        return NULL;

    rtl::OUString aExpr2;
    ScFormatEntry* pEntry = new ScCondFormatEntry(eMode, aFormula, aExpr2, mpDoc, maPos, maLbStyle.GetSelectEntry());
    return pEntry;
}

ScFormatEntry* ScFormulaFrmtEntry::GetEntry() const
{
    return createFormulaEntry();
}

void ScFormulaFrmtEntry::SetActive()
{
    maWdPreview.Show();
    maFtStyle.Show();
    maLbStyle.Show();
    maEdFormula.Show();

    Select();
}

void ScFormulaFrmtEntry::SetInactive()
{
    maWdPreview.Hide();
    maFtStyle.Hide();
    maLbStyle.Hide();
    maEdFormula.Hide();

    Deselect();
}

//color scale

namespace {

void SetColorScaleEntryTypes( const ScColorScaleEntry& rEntry, ListBox& rLbType, Edit& rEdit, ColorListBox& rLbCol )
{
    switch(rEntry.GetType())
    {
        case COLORSCALE_MIN:
            rLbType.SelectEntryPos(0);
            break;
        case COLORSCALE_MAX:
            rLbType.SelectEntryPos(1);
            break;
        case COLORSCALE_PERCENTILE:
            rEdit.SetText(rtl::OUString::valueOf(rEntry.GetValue()));
            rLbType.SelectEntryPos(2);
            break;
        case COLORSCALE_PERCENT:
            rEdit.SetText(rtl::OUString::valueOf(rEntry.GetValue()));
            rLbType.SelectEntryPos(4);
            break;
        case COLORSCALE_FORMULA:
            rEdit.SetText(rEntry.GetFormula(formula::FormulaGrammar::GRAM_DEFAULT));
            rLbType.SelectEntryPos(5);
            break;
        case COLORSCALE_VALUE:
            rEdit.SetText(rtl::OUString::valueOf(rEntry.GetValue()));
            rLbType.SelectEntryPos(3);
            break;
        case COLORSCALE_AUTOMIN:
            rLbType.SelectEntryPos(6);
            break;
        case COLORSCALE_AUTOMAX:
            rLbType.SelectEntryPos(7);
            break;
    }
    rLbCol.SelectEntry(rEntry.GetColor());
}

void SetColorScaleEntry( ScColorScaleEntry* pEntry, const ListBox& rType, const Edit& rValue, ScDocument* pDoc, const ScAddress& rPos )
{
    sal_uInt32 nIndex = 0;
    double nVal = 0;
    SvNumberFormatter* pNumberFormatter = pDoc->GetFormatTable();
    pNumberFormatter->IsNumberFormat(rValue.GetText(), nIndex, nVal);
    switch(rType.GetSelectEntryPos())
    {
        case 0:
            pEntry->SetType(COLORSCALE_MIN);
            break;
        case 1:
            pEntry->SetType(COLORSCALE_MAX);
            break;
        case 2:
            pEntry->SetType(COLORSCALE_PERCENTILE);
            pEntry->SetValue(nVal);
            break;
        case 3:
            pEntry->SetType(COLORSCALE_VALUE);
            pEntry->SetValue(nVal);
            break;
        case 4:
            pEntry->SetType(COLORSCALE_PERCENT);
            pEntry->SetValue(nVal);
            break;
        case 5:
            pEntry->SetType(COLORSCALE_FORMULA);
            pEntry->SetFormula(rValue.GetText(), pDoc, rPos);
            break;
        case 6:
            pEntry->SetType(COLORSCALE_AUTOMIN);
            break;
        case 7:
            pEntry->SetType(COLORSCALE_AUTOMAX);
            break;
        default:
            break;
    }
}

ScColorScaleEntry* createColorScaleEntry( const ListBox& rType, const ColorListBox& rColor, const Edit& rValue, ScDocument* pDoc, const ScAddress& rPos )
{
    ScColorScaleEntry* pEntry = new ScColorScaleEntry();

    SetColorScaleEntry( pEntry, rType, rValue, pDoc, rPos );
    Color aColor = rColor.GetSelectEntryColor();
    pEntry->SetColor(aColor);
    return pEntry;
}

}

ScColorScale2FrmtEntry::ScColorScale2FrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScColorScaleFormat* pFormat ):
    ScCondFrmtEntry( pParent, pDoc, rPos ),
    maLbColorFormat( this, ScResId( LB_COLOR_FORMAT ) ),
    maLbColScale2( this, ScResId( LB_COL_SCALE2 ) ),
    maLbEntryTypeMin( this, ScResId( LB_TYPE_COL_SCALE ) ),
    maLbEntryTypeMax( this, ScResId( LB_TYPE_COL_SCALE ) ),
    maEdMin( this, ScResId( ED_COL_SCALE ) ),
    maEdMax( this, ScResId( ED_COL_SCALE ) ),
    maLbColMin( this, ScResId( LB_COL) ),
    maLbColMax( this, ScResId( LB_COL) )
{
    maLbType.SelectEntryPos(0);
    maLbColorFormat.SelectEntryPos(0);
    if(pFormat)
    {
        ScColorScaleFormat::const_iterator itr = pFormat->begin();
        SetColorScaleEntryTypes(*itr, maLbEntryTypeMin, maEdMin, maLbColMin);
        ++itr;
        SetColorScaleEntryTypes(*itr, maLbEntryTypeMax, maEdMax, maLbColMax);
    }
    else
    {
        maLbEntryTypeMin.SelectEntryPos(0);
        maLbEntryTypeMax.SelectEntryPos(1);
    }
    FreeResource();

    maLbColorFormat.SetSelectHdl( LINK( pParent, ScCondFormatList, ColFormatTypeHdl ) );

    Init();
}

void ScColorScale2FrmtEntry::Init()
{
    maLbEntryTypeMin.SetSelectHdl( LINK( this, ScColorScale2FrmtEntry, EntryTypeHdl ) );
    maLbEntryTypeMax.SetSelectHdl( LINK( this, ScColorScale2FrmtEntry, EntryTypeHdl ) );

    SfxObjectShell*     pDocSh      = SfxObjectShell::Current();
    const SfxPoolItem*  pItem       = NULL;
    XColorListRef       pColorTable;

    DBG_ASSERT( pDocSh, "DocShell not found!" );

    /*
    Point aPointLb = maLbEntryTypeMiddle.GetPosPixel();
    Point aPointEd = maEdMiddle.GetPosPixel();
    Point aPointCol = maLbColMiddle.GetPosPixel();
    const long nMovePos = maLbEntryTypeMiddle.GetSizePixel().Width() * 1.2;
    aPointLb.X() += nMovePos;
    aPointEd.X() += nMovePos;
    aPointCol.X() += nMovePos;
    maLbEntryTypeMiddle.SetPosPixel(aPointLb);
    maEdMiddle.SetPosPixel(aPointEd);
    maLbColMiddle.SetPosPixel(aPointCol);
    aPointLb.X() += nMovePos;
    aPointEd.X() += nMovePos;
    aPointCol.X() += nMovePos;
    maLbEntryTypeMax.SetPosPixel(aPointLb);
    maEdMax.SetPosPixel(aPointEd);
    maLbColMax.SetPosPixel(aPointCol);
    */

    if ( pDocSh )
    {
        pItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if ( pItem != NULL )
            pColorTable = ( (SvxColorListItem*)pItem )->GetColorList();
    }
    if ( pColorTable.is() )
    {
        // filling the line color box
        maLbColMin.SetUpdateMode( false );
        maLbColMax.SetUpdateMode( false );

        for ( long i = 0; i < pColorTable->Count(); ++i )
        {
            XColorEntry* pEntry = pColorTable->GetColor(i);
            maLbColMin.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
            maLbColMax.InsertEntry( pEntry->GetColor(), pEntry->GetName() );

            if(pEntry->GetColor() == Color(COL_LIGHTRED))
                maLbColMin.SelectEntryPos(i);
            if(pEntry->GetColor() == Color(COL_LIGHTBLUE))
                maLbColMax.SelectEntryPos(i);
        }
        maLbColMin.SetUpdateMode( sal_True );
        maLbColMax.SetUpdateMode( sal_True );
    }

    EntryTypeHdl(&maLbEntryTypeMin);
    EntryTypeHdl(&maLbEntryTypeMax);
}

ScFormatEntry* ScColorScale2FrmtEntry::createColorscaleEntry() const
{
    ScColorScaleFormat* pColorScale = new ScColorScaleFormat(mpDoc);
    pColorScale->AddEntry(createColorScaleEntry(maLbEntryTypeMin, maLbColMin, maEdMin, mpDoc, maPos));
    pColorScale->AddEntry(createColorScaleEntry(maLbEntryTypeMax, maLbColMax, maEdMax, mpDoc, maPos));
    return pColorScale;
}

ScFormatEntry* ScColorScale2FrmtEntry::GetEntry() const
{
    return createColorscaleEntry();
}

void ScColorScale2FrmtEntry::SetActive()
{
    maLbColScale2.Show();

    maLbEntryTypeMin.Show();
    maLbEntryTypeMax.Show();

    maEdMin.Show();
    maEdMax.Show();

    maLbColMin.Show();
    maLbColMax.Show();

    Select();
}

void ScColorScale2FrmtEntry::SetInactive()
{
    maLbColScale2.Hide();

    maLbEntryTypeMin.Hide();
    maLbEntryTypeMax.Hide();

    maEdMin.Hide();
    maEdMax.Hide();

    maLbColMin.Hide();
    maLbColMax.Hide();

    Deselect();
}

IMPL_LINK( ScColorScale2FrmtEntry, EntryTypeHdl, ListBox*, pBox )
{
    bool bEnableEdit = true;
    sal_Int32 nPos = pBox->GetSelectEntryPos();
    if(nPos == 0 || nPos == 1)
    {
        bEnableEdit = false;
    }

    Edit* pEd = NULL;
    if(pBox == &maLbEntryTypeMin)
        pEd = &maEdMin;
    else if(pBox == &maLbEntryTypeMax)
        pEd = &maEdMax;

    if(bEnableEdit)
        pEd->Enable();
    else
        pEd->Disable();

    return 0;
}

ScColorScale3FrmtEntry::ScColorScale3FrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScColorScaleFormat* pFormat ):
    ScCondFrmtEntry( pParent, pDoc, rPos ),
    maLbColorFormat( this, ScResId( LB_COLOR_FORMAT ) ),
    maLbColScale2( this, ScResId( LB_COL_SCALE2 ) ),
    maLbColScale3( this, ScResId( LB_COL_SCALE3 ) ),
    maLbEntryTypeMin( this, ScResId( LB_TYPE_COL_SCALE ) ),
    maLbEntryTypeMiddle( this, ScResId( LB_TYPE_COL_SCALE ) ),
    maLbEntryTypeMax( this, ScResId( LB_TYPE_COL_SCALE ) ),
    maEdMin( this, ScResId( ED_COL_SCALE ) ),
    maEdMiddle( this, ScResId( ED_COL_SCALE ) ),
    maEdMax( this, ScResId( ED_COL_SCALE ) ),
    maLbColMin( this, ScResId( LB_COL) ),
    maLbColMiddle( this, ScResId( LB_COL) ),
    maLbColMax( this, ScResId( LB_COL) )
{
    maLbType.SelectEntryPos(0);
    if(pFormat)
    {
        if(pFormat->size() == 2)
            maLbColorFormat.SelectEntryPos(0);
        else
            maLbColorFormat.SelectEntryPos(1);
        ScColorScaleFormat::const_iterator itr = pFormat->begin();
        SetColorScaleEntryTypes(*itr, maLbEntryTypeMin, maEdMin, maLbColMin);
        if(pFormat->size() == 3)
        {
            ++itr;
            SetColorScaleEntryTypes(*itr, maLbEntryTypeMiddle, maEdMiddle, maLbColMiddle);
        }
        ++itr;
        SetColorScaleEntryTypes(*itr, maLbEntryTypeMax, maEdMax, maLbColMax);
    }
    else
    {
        maLbColorFormat.SelectEntryPos(1);
        maLbEntryTypeMin.SelectEntryPos(0);
        maLbEntryTypeMiddle.SelectEntryPos(2);
        maLbEntryTypeMax.SelectEntryPos(1);
        maEdMiddle.SetText(rtl::OUString::valueOf(static_cast<sal_Int32>(50)));
    }
    FreeResource();

    maLbColorFormat.SetSelectHdl( LINK( pParent, ScCondFormatList, ColFormatTypeHdl ) );

    Init();
}

void ScColorScale3FrmtEntry::Init()
{
    maLbEntryTypeMin.SetSelectHdl( LINK( this, ScColorScale3FrmtEntry, EntryTypeHdl ) );
    maLbEntryTypeMax.SetSelectHdl( LINK( this, ScColorScale3FrmtEntry, EntryTypeHdl ) );
    maLbEntryTypeMiddle.SetSelectHdl( LINK( this, ScColorScale3FrmtEntry, EntryTypeHdl ) );

    SfxObjectShell*     pDocSh      = SfxObjectShell::Current();
    const SfxPoolItem*  pItem       = NULL;
    XColorListRef       pColorTable;

    DBG_ASSERT( pDocSh, "DocShell not found!" );

    Point aPointLb = maLbEntryTypeMiddle.GetPosPixel();
    Point aPointEd = maEdMiddle.GetPosPixel();
    Point aPointCol = maLbColMiddle.GetPosPixel();
    const long nMovePos = maLbEntryTypeMiddle.GetSizePixel().Width() * 1.2;
    aPointLb.X() += nMovePos;
    aPointEd.X() += nMovePos;
    aPointCol.X() += nMovePos;
    maLbEntryTypeMiddle.SetPosPixel(aPointLb);
    maEdMiddle.SetPosPixel(aPointEd);
    maLbColMiddle.SetPosPixel(aPointCol);
    aPointLb.X() += nMovePos;
    aPointEd.X() += nMovePos;
    aPointCol.X() += nMovePos;
    maLbEntryTypeMax.SetPosPixel(aPointLb);
    maEdMax.SetPosPixel(aPointEd);
    maLbColMax.SetPosPixel(aPointCol);

    if ( pDocSh )
    {
        pItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if ( pItem != NULL )
            pColorTable = ( (SvxColorListItem*)pItem )->GetColorList();
    }
    if ( pColorTable.is() )
    {
        // filling the line color box
        maLbColMin.SetUpdateMode( false );
        maLbColMiddle.SetUpdateMode( false );
        maLbColMax.SetUpdateMode( false );

        for ( long i = 0; i < pColorTable->Count(); ++i )
        {
            XColorEntry* pEntry = pColorTable->GetColor(i);
            maLbColMin.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
            maLbColMiddle.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
            maLbColMax.InsertEntry( pEntry->GetColor(), pEntry->GetName() );

            if(pEntry->GetColor() == Color(COL_LIGHTRED))
                maLbColMin.SelectEntryPos(i);
            if(pEntry->GetColor() == Color(COL_GREEN))
                maLbColMiddle.SelectEntryPos(i);
            if(pEntry->GetColor() == Color(COL_LIGHTBLUE))
                maLbColMax.SelectEntryPos(i);
        }
        maLbColMin.SetUpdateMode( sal_True );
        maLbColMiddle.SetUpdateMode( sal_True );
        maLbColMax.SetUpdateMode( sal_True );
    }

    EntryTypeHdl(&maLbEntryTypeMin);
    EntryTypeHdl(&maLbEntryTypeMiddle);
    EntryTypeHdl(&maLbEntryTypeMax);
}

ScFormatEntry* ScColorScale3FrmtEntry::createColorscaleEntry() const
{
    ScColorScaleFormat* pColorScale = new ScColorScaleFormat(mpDoc);
    pColorScale->AddEntry(createColorScaleEntry(maLbEntryTypeMin, maLbColMin, maEdMin, mpDoc, maPos));
    if(maLbColorFormat.GetSelectEntryPos() == 1)
        pColorScale->AddEntry(createColorScaleEntry(maLbEntryTypeMiddle, maLbColMiddle, maEdMiddle, mpDoc, maPos));
    pColorScale->AddEntry(createColorScaleEntry(maLbEntryTypeMax, maLbColMax, maEdMax, mpDoc, maPos));
    return pColorScale;
}

ScFormatEntry* ScColorScale3FrmtEntry::GetEntry() const
{
    return createColorscaleEntry();
}

void ScColorScale3FrmtEntry::SetActive()
{
    maLbColScale2.Show();
    maLbColScale3.Show();

    maLbEntryTypeMin.Show();
    maLbEntryTypeMiddle.Show();
    maLbEntryTypeMax.Show();

    maEdMin.Show();
    maEdMiddle.Show();
    maEdMax.Show();

    maLbColMin.Show();
    maLbColMiddle.Show();
    maLbColMax.Show();

    Select();
}

void ScColorScale3FrmtEntry::SetInactive()
{
    maLbColScale2.Hide();
    maLbColScale3.Hide();

    maLbEntryTypeMin.Hide();
    maLbEntryTypeMiddle.Hide();
    maLbEntryTypeMax.Hide();

    maEdMin.Hide();
    maEdMiddle.Hide();
    maEdMax.Hide();

    maLbColMin.Hide();
    maLbColMiddle.Hide();
    maLbColMax.Hide();

    Deselect();
}

IMPL_LINK( ScColorScale3FrmtEntry, EntryTypeHdl, ListBox*, pBox )
{
    bool bEnableEdit = true;
    sal_Int32 nPos = pBox->GetSelectEntryPos();
    if(nPos == 0 || nPos == 1)
    {
        bEnableEdit = false;
    }

    Edit* pEd = NULL;
    if(pBox == &maLbEntryTypeMin)
        pEd = &maEdMin;
    else if(pBox == &maLbEntryTypeMiddle)
        pEd = &maEdMiddle;
    else if(pBox == &maLbEntryTypeMax)
        pEd = &maEdMax;

    if(bEnableEdit)
        pEd->Enable();
    else
        pEd->Disable();

    return 0;
}

IMPL_LINK_NOARG( ScConditionFrmtEntry, ConditionTypeSelectHdl )
{
    if(maLbCondType.GetSelectEntryPos() == 6 || maLbCondType.GetSelectEntryPos() == 7)
    {
        maEdVal2.Show();
    }
    else
    {
        maEdVal2.Hide();
    }

    return 0;
}

//databar

namespace {

void SetDataBarEntryTypes( const ScColorScaleEntry& rEntry, ListBox& rLbType, Edit& rEdit )
{
    switch(rEntry.GetType())
    {
        case COLORSCALE_MIN:
            rLbType.SelectEntryPos(0);
            break;
        case COLORSCALE_MAX:
            rLbType.SelectEntryPos(1);
            break;
        case COLORSCALE_PERCENTILE:
            rEdit.SetText(rtl::OUString::valueOf(rEntry.GetValue()));
            rLbType.SelectEntryPos(2);
            break;
        case COLORSCALE_PERCENT:
            rEdit.SetText(rtl::OUString::valueOf(rEntry.GetValue()));
            rLbType.SelectEntryPos(4);
            break;
        case COLORSCALE_FORMULA:
            rEdit.SetText(rEntry.GetFormula(formula::FormulaGrammar::GRAM_DEFAULT));
            rLbType.SelectEntryPos(5);
            break;
        case COLORSCALE_VALUE:
            rEdit.SetText(rtl::OUString::valueOf(rEntry.GetValue()));
            rLbType.SelectEntryPos(3);
            break;
        case COLORSCALE_AUTOMIN:
            rLbType.SelectEntryPos(6);
            break;
        case COLORSCALE_AUTOMAX:
            rLbType.SelectEntryPos(7);
            break;
    }
}

}

ScDataBarFrmtEntry::ScDataBarFrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScDataBarFormat* pFormat ):
    ScCondFrmtEntry( pParent, pDoc, rPos ),
    maLbColorFormat( this, ScResId( LB_COLOR_FORMAT ) ),
    maLbDataBarMinType( this, ScResId( LB_TYPE_COL_SCALE ) ),
    maLbDataBarMaxType( this, ScResId( LB_TYPE_COL_SCALE ) ),
    maEdDataBarMin( this, ScResId( ED_COL_SCALE ) ),
    maEdDataBarMax( this, ScResId( ED_COL_SCALE ) ),
    maBtOptions( this, ScResId( BTN_OPTIONS ) )
{
    maLbColorFormat.SelectEntryPos(2);
    maLbType.SelectEntryPos(0);
    if(pFormat)
    {
        mpDataBarData.reset(new ScDataBarFormatData(*pFormat->GetDataBarData()));
        SetDataBarEntryTypes(*mpDataBarData->mpLowerLimit, maLbDataBarMinType, maEdDataBarMin);
        SetDataBarEntryTypes(*mpDataBarData->mpUpperLimit, maLbDataBarMaxType, maEdDataBarMax);
        DataBarTypeSelectHdl(NULL);
    }
    else
    {
        maLbDataBarMinType.SelectEntryPos(0);
        maLbDataBarMaxType.SelectEntryPos(1);
    }
    Init();

    maLbColorFormat.SetSelectHdl( LINK( pParent, ScCondFormatList, ColFormatTypeHdl ) );

    FreeResource();
}

ScFormatEntry* ScDataBarFrmtEntry::GetEntry() const
{
    return createDatabarEntry();
}

void ScDataBarFrmtEntry::Init()
{
    maLbDataBarMinType.SetSelectHdl( LINK( this, ScDataBarFrmtEntry, DataBarTypeSelectHdl ) );
    maLbDataBarMaxType.SetSelectHdl( LINK( this, ScDataBarFrmtEntry, DataBarTypeSelectHdl ) );
    Point aPointEdDataBar = maEdDataBarMin.GetPosPixel();
    Point aPointLbDataBar = maLbDataBarMaxType.GetPosPixel();
    long nMovePos = 10;
    aPointEdDataBar.X() += 2*nMovePos;
    aPointLbDataBar.X() += 2*nMovePos;
    maEdDataBarMax.SetPosPixel(aPointEdDataBar);
    maLbDataBarMaxType.SetPosPixel(aPointLbDataBar);

    maBtOptions.SetClickHdl( LINK( this, ScDataBarFrmtEntry, OptionBtnHdl ) );

    mpDataBarData.reset(new ScDataBarFormatData());
    mpDataBarData->mpUpperLimit.reset(new ScColorScaleEntry());
    mpDataBarData->mpLowerLimit.reset(new ScColorScaleEntry());
    mpDataBarData->mpLowerLimit->SetType(COLORSCALE_MIN);
    mpDataBarData->mpUpperLimit->SetType(COLORSCALE_MAX);
    mpDataBarData->maPositiveColor = COL_LIGHTBLUE;
}

ScFormatEntry* ScDataBarFrmtEntry::createDatabarEntry() const
{
    SetColorScaleEntry(mpDataBarData->mpLowerLimit.get(), maLbDataBarMinType, maEdDataBarMin, mpDoc, maPos);
    SetColorScaleEntry(mpDataBarData->mpUpperLimit.get(), maLbDataBarMaxType, maEdDataBarMax, mpDoc, maPos);
    ScDataBarFormat* pDataBar = new ScDataBarFormat(mpDoc);
    pDataBar->SetDataBarData(new ScDataBarFormatData(*mpDataBarData.get()));
    return pDataBar;
}

void ScDataBarFrmtEntry::SetActive()
{
    maLbDataBarMinType.Show();
    maLbDataBarMaxType.Show();
    maEdDataBarMin.Show();
    maEdDataBarMax.Show();
    maBtOptions.Show();

    Select();
}

void ScDataBarFrmtEntry::SetInactive()
{
    maLbDataBarMinType.Hide();
    maLbDataBarMaxType.Hide();
    maEdDataBarMin.Hide();
    maEdDataBarMax.Hide();
    maBtOptions.Hide();

    Deselect();
}

IMPL_LINK_NOARG( ScDataBarFrmtEntry, DataBarTypeSelectHdl )
{
    sal_Int32 nSelectPos = maLbDataBarMinType.GetSelectEntryPos();
    if(nSelectPos == 0 || nSelectPos == 1)
        maEdDataBarMin.Disable();
    else
        maEdDataBarMin.Enable();

    nSelectPos = maLbDataBarMaxType.GetSelectEntryPos();
    if(nSelectPos == 0 || nSelectPos == 1)
        maEdDataBarMax.Disable();
    else
        maEdDataBarMax.Enable();

    return 0;
}

IMPL_LINK_NOARG( ScDataBarFrmtEntry, OptionBtnHdl )
{
    SetColorScaleEntry(mpDataBarData->mpLowerLimit.get(), maLbDataBarMinType, maEdDataBarMin, mpDoc, maPos);
    SetColorScaleEntry(mpDataBarData->mpUpperLimit.get(), maLbDataBarMaxType, maEdDataBarMax, mpDoc, maPos);
    ScDataBarSettingsDlg* pDlg = new ScDataBarSettingsDlg(this, *mpDataBarData, mpDoc);
    if( pDlg->Execute() == RET_OK)
    {
        mpDataBarData.reset(pDlg->GetData());
        SetDataBarEntryTypes(*mpDataBarData->mpLowerLimit, maLbDataBarMinType, maEdDataBarMin);
        SetDataBarEntryTypes(*mpDataBarData->mpUpperLimit, maLbDataBarMaxType, maEdDataBarMax);
        DataBarTypeSelectHdl(NULL);
    }
    return 0;
}
