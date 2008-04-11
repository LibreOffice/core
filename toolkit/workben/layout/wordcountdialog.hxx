/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: wordcountdialog.hxx,v $
 * $Revision: 1.3 $
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
#ifndef SW_WORDCOUNTDIALOG_HXX
#define SW_WORDCOUNTDIALOG_HXX
#include <sfx2/basedlgs.hxx>
#include <svtools/stdctrl.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <layout/layout.hxx>
#include <layout/layout-pre.hxx>
struct SwDocStat;
class SwWordCountDialog : public SfxModalDialog
{
    FixedLine       aCurrentFL;
    FixedText       aCurrentWordFT;
    FixedInfo       aCurrentWordFI;
    FixedText       aCurrentCharacterFT;
    FixedInfo       aCurrentCharacterFI;

    FixedLine       aDocFL;
    FixedText       aDocWordFT;
    FixedInfo       aDocWordFI;
    FixedText       aDocCharacterFT;
    FixedInfo       aDocCharacterFI;

    FixedLine       aBottomFL;

    OKButton        aOK;
    HelpButton      aHelp;

public:
    SwWordCountDialog(Window* pParent);
    ~SwWordCountDialog();

    void    SetValues(const SwDocStat& rCurrent, const SwDocStat& rDoc);
};

#include <layout/layout-post.hxx>

#endif
