#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

namespace desktop
{
    void displayCmdlineHelp( void );

    class CmdlineHelpDialog : public ModalDialog
    {
    public:
        CmdlineHelpDialog ( void );

        FixedText   m_ftHead;
        FixedText   m_ftLeft;
        FixedText   m_ftRight;
        FixedText   m_ftBottom;
        OKButton    m_btOk;
    };



}
