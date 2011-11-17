#include "namedefdlg.hxx"

#include "document.hxx"


ScNameDefDlg::ScNameDefDlg( Window* pParent, ScDocument* pDoc ) :
    ModalDialog( pParent, ScResId( RID_SCDLG_NAMES_DEFINE) ),
    maBtnAdd( this, ScResId( BTN_ADD ) ),
    maBtnCancel( this, ScResId( BTN_CANCEL ) ),
    maFtInfo( this, ScResId( FT_INFO ) ),
    maFtName( this, ScResId( FT_NAME ) ),
    maFtRange( this, ScResId( FT_RANGE ) ),
    maFtScope( this, ScResId( FT_SCOPE ) ),
    maFlDiv( this, ScResId( FL_DIV ) ),
    maEdName( this, ScResId( ED_NAME ) ),
    maEdRange( this, ScResId( ED_RANGE ) ),
    maLbScope( this, ScResId( LB_SCOPE ) ),
    maGlobalNameStr( ResId::toString( ScResId( STR_GLOBAL_SCOPE ) ) )
{
    // Initialize scope list.
    maLbScope.InsertEntry(maGlobalNameStr);
    maLbScope.SelectEntryPos(0);
    SCTAB n = pDoc->GetTableCount();
    for (SCTAB i = 0; i < n; ++i)
    {
        rtl::OUString aTabName;
        pDoc->GetName(i, aTabName);
        maLbScope.InsertEntry(aTabName);
    }
    maBtnCancel.SetClickHdl(LINK( this, ScNameDefDlg, CancelBtnHdl));
}

void ScNameDefDlg::CancelPushed()
{
    Close();
}

IMPL_LINK( ScNameDefDlg, CancelBtnHdl, void*, EMPTYARG)
{
    CancelPushed();
    return 0;
}
