/*************************************************************************
 *
 *  $RCSfile: ftninfo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2001-02-23 12:45:01 $
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
#ifndef _FTNINFO_HXX
#define _FTNINFO_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#include "numrule.hxx"

class SwTxtFmtColl;
class SwPageDesc;

class SwEndNoteInfo : public SwClient
{
    SwDepend    aPageDescDep;
    SwDepend    aCharFmtDep, aAnchorCharFmtDep;
    String      sPrefix;
    String      sSuffix;
protected:
    BOOL      bEndNote;
public:
    SvxNumberType aFmt;
    USHORT    nFtnOffset;

    void        ChgPageDesc( SwPageDesc *pDesc );
    SwPageDesc *GetPageDesc( SwDoc &rDoc ) const;
    SwClient   *GetPageDescDep() const { return (SwClient*)&aPageDescDep; }

    void SetFtnTxtColl(SwTxtFmtColl& rColl);
    SwTxtFmtColl* GetFtnTxtColl() const { return  (SwTxtFmtColl*) GetRegisteredIn(); } // kann 0 sein

    SwCharFmt* GetCharFmt(SwDoc &rDoc) const;
    void SetCharFmt( SwCharFmt* );
    SwClient   *GetCharFmtDep() const { return (SwClient*)&aCharFmtDep; }

    SwCharFmt* GetAnchorCharFmt(SwDoc &rDoc) const;
    void SetAnchorCharFmt( SwCharFmt* );
    SwClient   *GetAnchorCharFmtDep() const { return (SwClient*)&aAnchorCharFmtDep; }

    virtual void Modify( SfxPoolItem* pOld, SfxPoolItem* pNew );

    SwEndNoteInfo & operator=(const SwEndNoteInfo&);
    BOOL operator==( const SwEndNoteInfo &rInf ) const;

    SwEndNoteInfo( SwTxtFmtColl *pTxtColl = 0);
    SwEndNoteInfo(const SwEndNoteInfo&);

    const String& GetPrefix() const         { return sPrefix; }
    const String& GetSuffix() const         { return sSuffix; }

    void SetPrefix(const String& rSet)      { sPrefix = rSet; }
    void SetSuffix(const String& rSet)      { sSuffix = rSet; }

    BOOL IsEndNoteInfo() const              { return bEndNote; }
};

enum SwFtnPos
{
    //Derzeit nur PAGE und CHAPTER. CHAPTER == Dokumentendenoten.
    FTNPOS_PAGE = 1,
    FTNPOS_CHAPTER = 8
};

enum SwFtnNum
{
    FTNNUM_PAGE, FTNNUM_CHAPTER, FTNNUM_DOC
};

class SwFtnInfo: public SwEndNoteInfo
{
public:
    String    aQuoVadis;
    String    aErgoSum;
    SwFtnPos  ePos;
    SwFtnNum  eNum;


    SwFtnInfo& operator=(const SwFtnInfo&);
    BOOL operator==( const SwFtnInfo &rInf ) const;

    SwFtnInfo(SwTxtFmtColl* pTxtColl = 0);
    SwFtnInfo(const SwFtnInfo&);
};


#endif
