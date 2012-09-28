/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>

namespace desktop
{
    void displayCmdlineHelp( OUString const & unknown );
    void displayVersion();
#ifndef UNX
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
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
