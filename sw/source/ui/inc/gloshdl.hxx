/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gloshdl.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 15:35:06 $
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
#ifndef _GLOSHDL_HXX
#define _GLOSHDL_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

class SwWrtShell;
class SwTextBlocks;
class SvxMacro;
class SwGlossaries;
class SfxViewFrame;

// CLASS -----------------------------------------------------------------
class SW_DLLPUBLIC SwGlossaryHdl
{

    SwGlossaries&   rStatGlossaries;
    String          aCurGrp;
    SfxViewFrame*   pViewFrame;
    SwWrtShell*     pWrtShell;
    SwTextBlocks*   pCurGrp;

    SW_DLLPRIVATE void  _SetMacros(const String &rName,
                       const SvxMacro *pStart,
                       const SvxMacro *pEnd);

    SW_DLLPRIVATE BOOL  Expand( const String& rShortName,
                    SwGlossaries* pGlossaries,
                    SwTextBlocks *pGlossary );

public:
    BOOL    ConvertToNew(SwTextBlocks& rOld);
    void    GlossaryDlg();

    USHORT  GetGroupCnt() const;
    String  GetGroupName( USHORT, String* pTitle = 0 );
    BOOL    NewGroup(String & rGroupName, const String& rTitle);
    BOOL    DelGroup(const String &);
    BOOL    RenameGroup(const String & rOld, String& rNew, const String& rNewTitle);
    void    SetCurGroup(const String &aGrp, BOOL bApi = FALSE, BOOL bAlwaysCreateNew = FALSE);

    const String &GetCurGroup() const { return aCurGrp; }

    USHORT  GetGlossaryCnt();
    String  GetGlossaryName(USHORT);
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

    BOOL    ExpandGlossary();
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

            SwGlossaryHdl(SfxViewFrame* pViewFrame, SwWrtShell *);
            ~SwGlossaryHdl();
};

#endif // _GLOSHDL_HXX
