/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imoptdlg.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:33:06 $
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

#ifndef INCLUDED_SCDLLAPI_H
#include "scdllapi.h"
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

//===================================================================

//CHINA001 class ScImportOptions;
//CHINA001 class ScDelimiterTable;

//CHINA001 class ScImportOptionsDlg : public ModalDialog
//CHINA001 {
//CHINA001 public:
//CHINA001 ScImportOptionsDlg( Window*              pParent,
//CHINA001 BOOL                    bAscii = TRUE,
//CHINA001 const ScImportOptions*   pOptions = NULL,
//CHINA001 const String*            pStrTitle = NULL,
//CHINA001 BOOL                    bMultiByte = FALSE,
//CHINA001 BOOL                    bOnlyDbtoolsEncodings = FALSE,
//CHINA001 BOOL                    bImport = TRUE );
//CHINA001
//CHINA001 ~ScImportOptionsDlg();
//CHINA001
//CHINA001 void GetImportOptions( ScImportOptions& rOptions ) const;
//CHINA001
//CHINA001 private:
//CHINA001 FixedLine           aFlFieldOpt;
//CHINA001 FixedText            aFtFont;
//CHINA001 SvxTextEncodingBox   aLbFont;
//CHINA001 FixedText            aFtFieldSep;
//CHINA001 ComboBox         aEdFieldSep;
//CHINA001 FixedText            aFtTextSep;
//CHINA001 ComboBox         aEdTextSep;
//CHINA001 CheckBox            aCbFixed;
//CHINA001 OKButton         aBtnOk;
//CHINA001 CancelButton     aBtnCancel;
//CHINA001 HelpButton           aBtnHelp;
//CHINA001
//CHINA001 ScDelimiterTable*    pFieldSepTab;
//CHINA001 ScDelimiterTable*    pTextSepTab;
//CHINA001
//CHINA001 private:
//CHINA001 USHORT GetCodeFromCombo( const ComboBox& rEd ) const;
//CHINA001
//CHINA001 DECL_LINK( FixedWidthHdl, CheckBox* );
//CHINA001 DECL_LINK( DoubleClickHdl, ListBox* );
//CHINA001 };

//------------------------------------------------------------------------

class SC_DLLPUBLIC ScImportOptions
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



