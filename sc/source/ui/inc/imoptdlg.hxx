/*************************************************************************
 *
 *  $RCSfile: imoptdlg.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 08:03:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_IMOPTDLG_HXX
#define SC_IMOPTDLG_HXX

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif

#ifndef _SVX_TXENCBOX_HXX
#include <svx/txencbox.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

//===================================================================

class ScImportOptions;
class ScDelimiterTable;

class ScImportOptionsDlg : public ModalDialog
{
public:
                ScImportOptionsDlg( Window*                 pParent,
                                    BOOL                    bAscii = TRUE,
                                    const ScImportOptions*  pOptions = NULL,
                                    const String*           pStrTitle = NULL,
                                    BOOL                    bMultiByte = FALSE,
                                    BOOL                    bOnlyDbtoolsEncodings = FALSE,
                                    BOOL                    bImport = TRUE );

                ~ScImportOptionsDlg();

    void GetImportOptions( ScImportOptions& rOptions ) const;

private:
    FixedLine           aFlFieldOpt;
    FixedText           aFtFont;
    SvxTextEncodingBox  aLbFont;
    FixedText           aFtFieldSep;
    ComboBox            aEdFieldSep;
    FixedText           aFtTextSep;
    ComboBox            aEdTextSep;
    CheckBox            aCbFixed;
    OKButton            aBtnOk;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;

    ScDelimiterTable*   pFieldSepTab;
    ScDelimiterTable*   pTextSepTab;

private:
    USHORT GetCodeFromCombo( const ComboBox& rEd ) const;

    DECL_LINK( FixedWidthHdl, CheckBox* );
    DECL_LINK( DoubleClickHdl, ListBox* );
};

//------------------------------------------------------------------------

class ScImportOptions
{
public:
        ScImportOptions()
            : nFieldSepCode(0),nTextSepCode(0),eCharSet(RTL_TEXTENCODING_DONTKNOW),bFixedWidth(FALSE)
        {}
        ScImportOptions( const String& rStr );

        ScImportOptions( sal_Unicode nFieldSep, sal_Unicode nTextSep, const String& rStr )
            : nFieldSepCode(nFieldSep),nTextSepCode(nTextSep),aStrFont(rStr),bFixedWidth(FALSE)
        { eCharSet = ScGlobal::GetCharsetValue(aStrFont); }

        ScImportOptions( sal_Unicode nFieldSep, sal_Unicode nTextSep, rtl_TextEncoding nEnc )
            : nFieldSepCode(nFieldSep),nTextSepCode(nTextSep),bFixedWidth(FALSE)
        { SetTextEncoding( nEnc ); }

        ScImportOptions( const ScImportOptions& rCpy )
            : nFieldSepCode (rCpy.nFieldSepCode),
              nTextSepCode  (rCpy.nTextSepCode),
              aStrFont      (rCpy.aStrFont),
              eCharSet      (rCpy.eCharSet),
              bFixedWidth   (rCpy.bFixedWidth)
        {}

    ScImportOptions& operator=( const ScImportOptions& rCpy )
                        {
                            nFieldSepCode = rCpy.nFieldSepCode;
                            nTextSepCode  = rCpy.nTextSepCode;
                            aStrFont      = rCpy.aStrFont;
                            eCharSet      = rCpy.eCharSet;
                            bFixedWidth   = rCpy.bFixedWidth;
                            return *this;
                        }

    BOOL             operator==( const ScImportOptions& rCmp )
                        {
                            return
                                   nFieldSepCode == rCmp.nFieldSepCode
                                && nTextSepCode  == rCmp.nTextSepCode
                                && eCharSet      == rCmp.eCharSet
                                && aStrFont      == rCmp.aStrFont
                                && bFixedWidth   == rCmp.bFixedWidth;
                        }
    String  BuildString() const;

    void    SetTextEncoding( rtl_TextEncoding nEnc );

    sal_Unicode nFieldSepCode;
    sal_Unicode nTextSepCode;
    String      aStrFont;
    CharSet     eCharSet;
    BOOL        bFixedWidth;
};


#endif // SC_IMOPTDLG_HXX



