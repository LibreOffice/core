/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wordcountdialog.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:18:42 $
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
#ifndef SW_WORDCOUNTDIALOG_HXX
#define SW_WORDCOUNTDIALOG_HXX
#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
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

#endif
