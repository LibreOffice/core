/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ftninfo.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:49:12 $
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
#ifndef _FTNINFO_HXX
#define _FTNINFO_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif
//#ifndef _NUMRULE_HXX
//#include <numrule.hxx>
//#endif
#ifndef _CALBCK_HXX
#include <calbck.hxx>
#endif
#ifndef _SVX_NUMITEM_HXX
#include <svx/numitem.hxx>
#endif

class SwTxtFmtColl;
class SwPageDesc;
class SwCharFmt;
class SwDoc;

class SW_DLLPUBLIC SwEndNoteInfo : public SwClient
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

class SW_DLLPUBLIC SwFtnInfo: public SwEndNoteInfo
{
    using SwEndNoteInfo::operator ==;

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
