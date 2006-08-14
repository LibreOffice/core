/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ascfldlg.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 17:38:18 $
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
#ifndef _ASCFLDLG_HXX
#define _ASCFLDLG_HXX
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _SVX_TXENCBOX_HXX
#include <svx/txencbox.hxx>
#endif

#ifndef _SVX_LANGBOX_HXX
#include <svx/langbox.hxx>
#endif

class SwAsciiOptions;
class SvStream;
class SwDocShell;

class SwAsciiFilterDlg : public SfxModalDialog
{
    FixedLine           aFL;
    FixedText           aCharSetFT;
    SvxTextEncodingBox  aCharSetLB;
    FixedText           aFontFT;
    ListBox             aFontLB;
    FixedText           aLanguageFT;
    SvxLanguageBox      aLanguageLB;
    FixedText           aCRLF_FT;
    RadioButton         aCRLF_RB, aCR_RB, aLF_RB;
    OKButton            aOkPB;
    CancelButton        aCancelPB;
    HelpButton          aHelpPB;
    String              sSystemCharSet;
    BOOL                bSaveLineStatus;

    DECL_LINK( CharSetSelHdl, SvxTextEncodingBox* );
    DECL_LINK( LineEndHdl, RadioButton* );
    void SetCRLF( LineEnd eEnd );
    LineEnd GetCRLF() const;

public:
    // CTOR:    for import - pStream is the inputstream
    //          for export - pStream must be 0
    SwAsciiFilterDlg( Window* pParent, SwDocShell& rDocSh,
                        SvStream* pStream );
    virtual ~SwAsciiFilterDlg();

    void FillOptions( SwAsciiOptions& rOptions );
};



#endif

