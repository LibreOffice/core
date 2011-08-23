/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _FTNINFO_HXX
#define _FTNINFO_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#include "numrule.hxx"
namespace binfilter {

class SwTxtFmtColl;
class SwPageDesc;

class SwEndNoteInfo : public SwClient
{
    SwDepend  	aPageDescDep;
    SwDepend 	aCharFmtDep, aAnchorCharFmtDep;
    String 		sPrefix;
    String 		sSuffix;
protected:
    BOOL 	  bEndNote;
public:
    SvxNumberType aFmt;
    USHORT 	  nFtnOffset;

    void 		ChgPageDesc( SwPageDesc *pDesc );
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

    const String& GetPrefix() const 		{ return sPrefix; }
    const String& GetSuffix() const 		{ return sSuffix; }

    void SetPrefix(const String& rSet)		{ sPrefix = rSet; }
    void SetSuffix(const String& rSet)		{ sSuffix = rSet; }

    BOOL IsEndNoteInfo() const 				{ return bEndNote; }
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
    String	  aErgoSum;
    SwFtnPos  ePos;
    SwFtnNum  eNum;


    SwFtnInfo& operator=(const SwFtnInfo&);
    BOOL operator==( const SwFtnInfo &rInf ) const;

    SwFtnInfo(SwTxtFmtColl* pTxtColl = 0);
    SwFtnInfo(const SwFtnInfo&);
};


} //namespace binfilter
#endif
