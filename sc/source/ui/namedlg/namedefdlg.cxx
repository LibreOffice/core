#include "namedefdlg.hxx"

#include <vcl/msgbox.hxx>
#include <sfx2/app.hxx>

#include "document.hxx"
#include "globalnames.hxx"
#include "rangenam.hxx"
#include "reffact.hxx"
#include "undorangename.hxx"

// defines -------------------------------------------------------------------

#define ABS_SREF          SCA_VALID \
    | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE
#define ABS_DREF          ABS_SREF \
    | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE | SCA_TAB2_ABSOLUTE
#define ABS_SREF3D      ABS_SREF | SCA_TAB_3D
#define ABS_DREF3D      ABS_DREF | SCA_TAB_3D

ScNameDefDlg::ScNameDefDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
        ScDocShell* pDocShell, std::map<rtl::OUString, ScRangeName*> aRangeMap,
        const ScAddress& aCursorPos, const bool bUndo ) :
    ScAnyRefDlg( pB, pCW, pParent, RID_SCDLG_NAMES_DEFINE ),
    maBtnAdd( this, ScResId( BTN_ADD ) ),
    maBtnCancel( this, ScResId( BTN_CANCEL ) ),
    maFtInfo( this, ScResId( FT_INFO ) ),
    maFtName( this, ScResId( FT_NAME ) ),
    maFtRange( this, ScResId( FT_RANGE ) ),
    maFtScope( this, ScResId( FT_SCOPE ) ),
    maFlDiv( this, ScResId( FL_DIV ) ),
    maEdName( this, ScResId( ED_NAME ) ),
    maEdRange( this, this, ScResId( ED_RANGE ) ),
    maRbRange( this, ScResId( RB_RANGE ), &maEdRange, this ),
    maLbScope( this, ScResId( LB_SCOPE ) ),
    maBtnRowHeader( this, ScResId( BTN_ROWHEADER ) ),
    maBtnColHeader( this, ScResId( BTN_COLHEADER ) ),
    maBtnPrintArea( this, ScResId( BTN_PRINTAREA ) ),
    maBtnCriteria( this, ScResId( BTN_CRITERIA ) ),
    mbUndo( bUndo ),
    mpDoc( pDocShell->GetDocument() ),
    mpDocShell ( pDocShell ),
    maCursorPos( aCursorPos ),
    maGlobalNameStr( ResId::toString( ScResId( STR_GLOBAL_SCOPE ) ) ),
    maRangeMap( aRangeMap )
{
    // Initialize scope list.
    maLbScope.InsertEntry(maGlobalNameStr);
    maLbScope.SelectEntryPos(0);
    SCTAB n = mpDoc->GetTableCount();
    for (SCTAB i = 0; i < n; ++i)
    {
        rtl::OUString aTabName;
        mpDoc->GetName(i, aTabName);
        maLbScope.InsertEntry(aTabName);
    }

    maBtnCancel.SetClickHdl( LINK( this, ScNameDefDlg, CancelBtnHdl));
    maBtnAdd.SetClickHdl( LINK( this, ScNameDefDlg, AddBtnHdl ));
    maEdName.SetModifyHdl( LINK( this, ScNameDefDlg, NameModifyHdl ));
    maEdRange.SetGetFocusHdl( LINK( this, ScNameDefDlg, AssignGetFocusHdl ) );

    maFtInfo.SetText(ResId::toString( ScResId( STR_DEFAULT_INFO ) ));

    maBtnAdd.Disable(); // empty name is invalid
}

void ScNameDefDlg::CancelPushed()
{
    Close();
}

bool ScNameDefDlg::IsNameValid()
{
    rtl::OUString aScope = maLbScope.GetSelectEntry();
    rtl::OUString aName = maEdName.GetText();

    ScRangeName* pRangeName = NULL;
    if(aScope == maGlobalNameStr)
    {
        pRangeName = maRangeMap.find(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(STR_GLOBAL_RANGE_NAME)))->second;
    }
    else
    {
        pRangeName = maRangeMap.find(aScope)->second;
    }

    if (!ScRangeData::IsNameValid( aName, mpDoc ))
    {
        maFtInfo.SetText(ResId::toString(ScResId( STR_ERR_NAME_INVALID )));
        maBtnAdd.Disable();
        return false;
    }
    else if (pRangeName->findByUpperName(ScGlobal::pCharClass->upper(aName)))
    {
        maFtInfo.SetText(ResId::toString(ScResId( STR_ERR_NAME_EXISTS )));
        maBtnAdd.Disable();
        return false;
    }
    maFtInfo.SetText(ResId::toString( ScResId( STR_DEFAULT_INFO ) ));
    maBtnAdd.Enable();
    return true;
}

void ScNameDefDlg::AddPushed()
{
    rtl::OUString aScope = maLbScope.GetSelectEntry();
    rtl::OUString aName = maEdName.GetText();
    rtl::OUString aExpression = maEdRange.GetText();

    if (!aName.getLength())
    {
        return;
    }
    if (!aScope.getLength())
    {
        return;
    }

    ScRangeName* pRangeName = NULL;
    if(aScope == maGlobalNameStr)
    {
        pRangeName = maRangeMap.find(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(STR_GLOBAL_RANGE_NAME)))->second;
    }
    else
    {
        pRangeName = maRangeMap.find(aScope)->second;
    }
    if (!pRangeName)
        return;

    if (!IsNameValid()) //should not happen, but make sure we don't break anything
        return;
    else
    {
        if ( mpDoc )
        {
            ScRangeData*    pNewEntry   = NULL;
            RangeType       nType       = RT_NAME;

            pNewEntry = new ScRangeData( mpDoc,
                    aName,
                    aExpression,
                    maCursorPos,
                    nType );
            if (pNewEntry)
            {
                nType = nType
                    | (maBtnRowHeader .IsChecked() ? RT_ROWHEADER  : RangeType(0))
                    | (maBtnColHeader .IsChecked() ? RT_COLHEADER  : RangeType(0))
                    | (maBtnPrintArea .IsChecked() ? RT_PRINTAREA  : RangeType(0))
                    | (maBtnCriteria  .IsChecked() ? RT_CRITERIA   : RangeType(0));
                pNewEntry->AddType(nType);
            }

            // aExpression valid?
            if ( 0 == pNewEntry->GetErrCode() )
            {
                if ( !pRangeName->insert( pNewEntry ) )
                    pNewEntry = NULL;

                if (mbUndo)
                {
                    // this means we called directly through the menu

                    SCTAB nTab;
                    // if no table with that name is found, assume global range name
                    if (!mpDoc->GetTable(aScope, nTab))
                        nTab = -1;

                    mpDocShell->GetUndoManager()->AddUndoAction(
                            new ScUndoAddRangeData( mpDocShell, pNewEntry, nTab) );

                    // set table stream invalid, otherwise RangeName won't be saved if no other
                    // call invalidates the stream
                    if (nTab != -1)
                        mpDoc->SetStreamValid(nTab, false);
                }
                SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );
                Close();
            }
            else
            {
                delete pNewEntry;
                ErrorBox(this,WinBits(WB_OK|WB_DEF_OK),ResId::toString(ScResId(STR_INVALIDSYMBOL))).Execute();
                Selection aCurSel = Selection( 0, SELECTION_MAX );
                maEdRange.GrabFocus();
                maEdRange.SetSelection( aCurSel );
            }
        }
    }
}

sal_Bool ScNameDefDlg::IsRefInputMode() const
{
    return maEdRange.IsEnabled();
}

void ScNameDefDlg::RefInputDone( sal_Bool bForced)
{
    ScAnyRefDlg::RefInputDone(bForced);
    EdModifyHdl(&maEdRange);
}

void ScNameDefDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( maEdRange.IsEnabled() )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(&maEdRange);
        String aRefStr;
        rRef.Format( aRefStr, ABS_DREF3D, pDocP,
                ScAddress::Details(pDocP->GetAddressConvention(), 0, 0) );
        maEdRange.SetRefString( aRefStr );
    }
}

sal_Bool ScNameDefDlg::Close()
{
    return DoClose( ScNameDefDlgWrapper::GetChildWindowId() );
}

void ScNameDefDlg::SetActive()
{
    maEdRange.GrabFocus();
    RefInputDone();
}

IMPL_LINK( ScNameDefDlg, CancelBtnHdl, void*, EMPTYARG)
{
    CancelPushed();
    return 0;
}

IMPL_LINK( ScNameDefDlg, AddBtnHdl, void*, EMPTYARG)
{
    AddPushed();
    return 0;
};

IMPL_LINK( ScNameDefDlg, NameModifyHdl, void*, EMPTYARG)
{
    IsNameValid();
    return 0;
}

IMPL_LINK( ScNameDefDlg, EdModifyHdl, void *, EMPTYARG )
{
    IsNameValid();
    return 0;
}

IMPL_LINK( ScNameDefDlg, AssignGetFocusHdl, void *, EMPTYARG )
{
    EdModifyHdl( &maEdRange );
    return 0;
}
