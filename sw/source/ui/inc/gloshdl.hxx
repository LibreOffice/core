/*************************************************************************
 *
 *  $RCSfile: gloshdl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:40 $
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
#ifndef _GLOSHDL_HXX
#define _GLOSHDL_HXX


#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
class SwWrtShell;
class SwTextBlocks;
class SvxMacro;
class SwGlossaries;
class SfxViewFrame;

// CLASS -----------------------------------------------------------------
class SwGlossaryHdl
{

    SwGlossaries&   rStatGlossaries;
    String          aCurGrp;
    SfxViewFrame*   pViewFrame;
    SwWrtShell*     pWrtShell;
    SwTextBlocks*   pCurGrp;

    void    _SetMacros(const String &rName,
                       const SvxMacro *pStart,
                       const SvxMacro *pEnd);

    BOOL    Expand( const String& rShortName,
                    SwGlossaries* pGlossaries,
                    SwTextBlocks *pGlossary,
                    BOOL bApi = FALSE );
public:
    void    GlossaryDlg();

    USHORT  GetGroupCnt() const;
    String  GetGroupName( USHORT, String* pTitle = 0 );
    BOOL    NewGroup(String & rGroupName, const String& rTitle);
    BOOL    DelGroup(const String &);
    BOOL    RenameGroup(const String & rOld, String& rNew, const String& rNewTitle);
    void    SetCurGroup(const String &aGrp, BOOL bApi = FALSE, BOOL bAlwaysCreateNew = FALSE);

    const String &GetCurGroup() const { return aCurGrp; }

    USHORT  GetGlossaryCnt();
    USHORT  GetGlossaryCnt(const String& rGrpName); //fuer Basic
    String  GetGlossaryName(USHORT);
    String  GetGlossaryName(USHORT, const String& rGrpName); //fuer Basic
    String  GetGlossaryShortName(const String &rName);
    String  GetGlossaryShortName(USHORT);

    BOOL    Rename( const String& rOldShortName,  const String& rNewShortName,
                        const String& rNewName);
    BOOL    CopyOrMove( const String& rSourceGroupName,  String& rSourceShortName,
                        const String& rDestGroupName, const String& rLongName, BOOL bMove );
    BOOL    HasShortName(const String &rShortName) const;
    //wird NewGlossary vom Basic gerufen, dann soll dir zuvor eingestellte
    //Gruppe ggf. neu erstellt werden
    BOOL    NewGlossary(const String &rName, const String &rShortName,
                BOOL bApiCall = FALSE, BOOL bNoAttr = FALSE );
    BOOL    DelGlossary(const String&);
    BOOL    CopyToClipboard(SwWrtShell& rSh, const String& rShortName);

    BOOL    ExpandGlossary(BOOL bUseStandard = TRUE, BOOL bApi = FALSE);
    BOOL    ExpandGlossary( const String& rShortName, BOOL bApi = FALSE );
    BOOL    InsertGlossary(const String &rName);

    void    SetMacros(const String& rName,
                      const SvxMacro* pStart,
                      const SvxMacro* pEnd,
                      SwTextBlocks *pGlossary = 0 );
    void    GetMacros(const String& rShortName,
                      SvxMacro& rStart,
                      SvxMacro& rEnd,
                      SwTextBlocks* pGlossary = 0 );

    BOOL    IsReadOnly( const String* = 0 ) const;
    BOOL    IsOld() const;

    BOOL    FindGroupName(String & rGroup); // Gruppe ohne Pfadindex finden

    BOOL    ImportGlossaries( const String& rName );

    String  GetValidShortCut( const String& rLong,
                                 BOOL bCheckInBlock = FALSE ) const;

            SwGlossaryHdl(SfxViewFrame* pViewFrame, SwWrtShell *);
            ~SwGlossaryHdl();
};

#endif // _GLOSHDL_HXX
