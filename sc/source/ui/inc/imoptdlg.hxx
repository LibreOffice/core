/*************************************************************************
 *
 *  $RCSfile: imoptdlg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:59 $
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

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
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
                                    BOOL                    bAsciiImport = TRUE,
                                    const ScImportOptions*  pOptions = NULL,
                                    const String*           pStrTitle = NULL );

                ~ScImportOptionsDlg();

    void GetImportOptions( ScImportOptions& rOptions ) const;

private:
    FixedText           aFtFieldSep;
    ComboBox            aEdFieldSep;
    FixedText           aFtTextSep;
    ComboBox            aEdTextSep;
    FixedText           aFtFont;
    ListBox             aLbFont;
    GroupBox            aGbFieldOpt;
    OKButton            aBtnOk;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;

    ScDelimiterTable*   pFieldSepTab;
    ScDelimiterTable*   pTextSepTab;
    String              aCharKeyList;

private:
    USHORT GetCodeFromCombo( const ComboBox& rEd ) const;
};

//------------------------------------------------------------------------

class ScImportOptions
{
public:
        ScImportOptions()
            : nFieldSepCode(0),nTextSepCode(0),eCharSet(RTL_TEXTENCODING_DONTKNOW)
        {}
        ScImportOptions( const String& rStr );

        ScImportOptions( USHORT nFieldSep, USHORT nTextSep, const String& rStr )
            : nFieldSepCode(nFieldSep),nTextSepCode(nTextSep),aStrFont(rStr),eCharSet(RTL_TEXTENCODING_DONTKNOW)
        { eCharSet = GetCharsetValue(aStrFont); }

        ScImportOptions( const ScImportOptions& rCpy )
            : nFieldSepCode (rCpy.nFieldSepCode),
              nTextSepCode  (rCpy.nTextSepCode),
              aStrFont      (rCpy.aStrFont),
              eCharSet      (rCpy.eCharSet)
        {}

    ScImportOptions& operator=( const ScImportOptions& rCpy )
                        {
                            nFieldSepCode = rCpy.nFieldSepCode;
                            nTextSepCode  = rCpy.nTextSepCode;
                            aStrFont      = rCpy.aStrFont;
                            eCharSet      = rCpy.eCharSet;
                            return *this;
                        }

    BOOL             operator==( const ScImportOptions& rCmp )
                        {
                            return
                                   nFieldSepCode == rCmp.nFieldSepCode
                                && nTextSepCode  == rCmp.nTextSepCode
                                && eCharSet      == rCmp.eCharSet
                                && aStrFont      == rCmp.aStrFont;
                        }
    String  BuildString() const;
    String  BuildParaString( const String& rTyp, const String& rDsn ) const;

    USHORT  nFieldSepCode;
    USHORT  nTextSepCode;
    String  aStrFont;
    CharSet eCharSet;
};


#endif // SC_IMOPTDLG_HXX



