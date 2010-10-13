
#ifndef SC_DATAFDLG_HXX
#define SC_DATAFDLG_HXX


#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif


#include "global.hxx"

#include <tabvwsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

#define MAX_DATAFORM_COLS   256
#define MAX_DATAFORM_ROWS   32000
#define CTRL_HEIGHT         22
#define FIXED_WIDTH         60
#define EDIT_WIDTH          140
#define FIXED_LEFT          12
#define EDIT_LEFT           78
#define LINE_HEIGHT         30

//zhangyun
class ScDataFormDlg : public ModalDialog
{
private:

    PushButton      aBtnNew;
    PushButton      aBtnDelete;
    PushButton      aBtnRestore;
    PushButton      aBtnLast;
    PushButton      aBtnNext;
    PushButton      aBtnClose;
    ScrollBar       aSlider;
    FixedText       aFixedText;
    //FixedText       aFixedText1;
    //Edit            aEdit1;

    ScTabViewShell* pTabViewShell;
    ScDocument*     pDoc;
    sal_uInt16      aColLength;
    SCROW           aCurrentRow;
    SCCOL           nStartCol;
    SCCOL           nEndCol;
    SCROW           nStartRow;
    SCROW           nEndRow;
    SCTAB           nTab;
    BOOL            bNoSelection;

    FixedText** pFixedTexts;
    Edit** pEdits;

public:
    ScDataFormDlg( Window* pParent, ScTabViewShell* pTabViewShell);
    ~ScDataFormDlg();

    void FillCtrls(SCROW nCurrentRow);
private:

    void SetButtonState();

    // Handler:
    DECL_LINK( Impl_NewHdl,     PushButton*    );
    DECL_LINK( Impl_LastHdl,    PushButton*    );
    DECL_LINK( Impl_NextHdl,    PushButton*    );

    DECL_LINK( Impl_RestoreHdl, PushButton*    );
    DECL_LINK( Impl_DeleteHdl,  PushButton*    );
    DECL_LINK( Impl_CloseHdl,   PushButton*    );

    DECL_LINK( Impl_ScrollHdl,  ScrollBar*    );
    DECL_LINK( Impl_DataModifyHdl,  Edit*    );
};
#endif // SC_DATAFDLG_HXX
