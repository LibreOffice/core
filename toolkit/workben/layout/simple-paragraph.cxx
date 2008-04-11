#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

#include <toolkit/awt/vclxwindow.hxx>

// include ---------------------------------------------------------------

#include <stdio.h>
#include <tools/shl.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/msgbox.hxx>

#include <stdio.h>

namespace SVX {
#include <svx/paraprev.hxx>        // Preview
};

#include "simple-paragraph.hxx"

#include <assert.h>





// StandardTabPage ----------------------------------------------------------

#define FT_LEFTINDENT            10
#define ED_LEFTINDENT            11
#define FT_FLINEINDENT            12
#define ED_FLINEINDENT            13
#define FT_RIGHTINDENT            14
#define ED_RIGHTINDENT            15
#define FL_INDENT                       16

#define FT_TOPDIST                      20
#define ED_TOPDIST                      21
#define FT_BOTTOMDIST            22
#define ED_BOTTOMDIST            23
#define FL_DIST                         24

#define BTN_LEFTALIGN            30
#define BTN_RIGHTALIGN            31
#define BTN_CENTERALIGN            32
#define BTN_JUSTIFYALIGN        33
#define FL_ALIGN                        34
#define FL_VERTALIGN                    35
#define LB_VERTALIGN                    36
#define FL_VERTEX                       37
#define CB_SNAP                         38
#define FT_VERTALIGN                    39


#define LB_LINEDIST                     40
#define FT_LINEDIST                     41
#define ED_LINEDISTPERCENT        42
#define ED_LINEDISTMETRIC        43
#define FL_LINEDIST                     44
#define WN_EXAMPLE                      46

#define CB_AUTO                         48
#define FT_LASTLINE                     49
#define LB_LASTLINE                     50
#define CB_EXPAND                       51
#define ST_LINEDIST_ABS                 52
#define ST_LEFTALIGN_ASIAN              53
#define ST_RIGHTALIGN_ASIAN             54

// ExtendedTabPage ----------------------------------------------------------

#define BTN_HYPHEN                      50
#define ED_HYPHENBEFORE            52
#define FT_HYPHENBEFORE            53
#define ED_HYPHENAFTER            55
#define FT_HYPHENAFTER            56
#define FL_HYPHEN                       57

#define BTN_PAGEBREAK            60
#define FT_BREAKTYPE            61
#define LB_BREAKTYPE            62
#define FT_BREAKPOSITION        63
#define LB_BREAKPOSITION        64
#define BTN_PAGECOLL            65
#define LB_PAGECOLL                     66
#define BTN_KEEPTOGETHER        67
#define BTN_WIDOWS                      68
#define ED_WIDOWS                       69
#define FT_WIDOWS                       70
#define BTN_ORPHANS                     71
#define ED_ORPHANS                      72
#define FT_ORPHANS                      73
#define FL_OPTIONS                      74
#define FT_PAGENUM                      75
#define ED_PAGENUM                      76
#define FL_BREAKS                        77

#define CB_KEEPTOGETHER                 80
#define FT_MAXHYPH                      81
#define ED_MAXHYPH                      83

#define CB_REGISTER                     84
#define FL_REGISTER                     85
#define FL_PROPERTIES                   90
#define FT_TEXTDIRECTION                91
#define LB_TEXTDIRECTION                92


//asian typography
#define FL_AS_OPTIONS                   1
#define CB_AS_HANG_PUNC                 2
#define CB_AS_ALLOW_WORD_BREAK          3
#define CB_AS_FORBIDDEN                 4
#define FL_AS_CHAR_DIST                 5
#define CB_AS_PUNCTUATION               6
#define CB_AS_SCRIPT_SPACE                7
#define CB_AS_ADJUST_NUMBERS            8








SvxSimpleParagraphDialog::SvxSimpleParagraphDialog( Window* pParent ) :
    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_STD_PARAGRAPH ), rAttr ),
    Dialog( pParent, "simple-paragraph.xml", "dialog" ),
    aLineSpacingList( this, "line-spacing-list" )
{
fprintf(stderr, "creating res mgr\n");
    pMgr = ResMgr::CreateResMgr("SOME_NAME");
fprintf(stderr, "getting parent\n");
    VCLXWindow *pCompParent = VCLXWindow::GetImplementation( GetPeer() );
    assert( pCompParent != NULL );
    assert( pCompParent->GetWindow() != NULL );
fprintf(stderr, "creating foreign vcl widget\n");
/*    pPrevWin = new SVX::SvxParaPrevWindow(
        pCompParent->GetWindow(),
        ResId(1234, *pMgr)
        );*/

fprintf(stderr, "done\n");
    FreeResource();
}

// -----------------------------------------------------------------------

SvxSimpleParagraphDialog::~SvxSimpleParagraphDialog()
{
}
