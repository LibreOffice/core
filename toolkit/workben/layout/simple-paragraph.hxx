#ifndef _SVX_SIMPLE_PARAGRAPH_HXX
#define _SVX_SIMPLE_PARAGRAPH_HXX

#include <layout/layout.hxx>

#include <svx/paraprev.hxx>        // Preview

namespace SVX {
    class SvxParaPrevWindow;
};
class ResMgr;

class SvxSimpleParagraphDialog : public SfxTabPage, public layout::Dialog
{
private:
    ListBox aLineSpacingList;
    SVX::SvxParaPrevWindow *pPrevWin;
    ResMgr *pMgr;

public:
    SvxSimpleParagraphDialog( Window* pParent );
    ~SvxSimpleParagraphDialog();
};

#endif
