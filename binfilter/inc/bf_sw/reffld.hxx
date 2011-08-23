/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _REFFLD_HXX
#define _REFFLD_HXX

#include <fldbas.hxx>
namespace binfilter {

class SfxPoolItem;
class SwDoc;
class SwTxtAttr;
class SwTxtNode;

enum REFERENCESUBTYPE
{
    REF_SETREFATTR = 0,
    REF_SEQUENCEFLD,
    REF_BOOKMARK,
    REF_OUTLINE,
    REF_FOOTNOTE,
    REF_ENDNOTE
};

enum REFERENCEMARK
{
    REF_BEGIN,
    REF_PAGE = REF_BEGIN,
    REF_CHAPTER,
    REF_CONTENT,
    REF_UPDOWN,
    REF_PAGE_PGDESC,
    REF_ONLYNUMBER,
    REF_ONLYCAPTION,
    REF_ONLYSEQNO,
    REF_END
};


/*--------------------------------------------------------------------
    Beschreibung: Referenz holen
 --------------------------------------------------------------------*/

class SwGetRefFieldType : public SwFieldType
{
    SwDoc* pDoc;
public:
    SwGetRefFieldType(SwDoc* pDoc );
     virtual SwFieldType*    Copy() const;

    SwDoc* 					GetDoc() const { return pDoc; }
    // ueberlagert, um alle Ref-Felder zu updaten
    virtual void Modify( SfxPoolItem *, SfxPoolItem * );

    void MergeWithOtherDoc( SwDoc& rDestDoc );

    static SwTxtNode* FindAnchor( SwDoc* pDoc, const String& rRefMark,
                                        USHORT nSubType, USHORT nSeqNo,
                                        USHORT* pStt, USHORT* pEnd = 0 );
};

/*--------------------------------------------------------------------
    Beschreibung: Referenzfeld
 --------------------------------------------------------------------*/

class SwGetRefField : public SwField
{
    String sSetRefName;
    String sTxt;
    USHORT nSubType;
    USHORT nSeqNo;
public:
    SwGetRefField( SwGetRefFieldType*, const String& rSetRef,
                    USHORT nSubType, USHORT nSeqNo, ULONG nFmt );

    virtual	String		GetCntnt(BOOL bName = FALSE) const;
    virtual String	 	Expand() const;
    virtual SwField* 	Copy() const;

    const String& 		GetSetRefName() const { return sSetRefName; }

    void 				UpdateField();
    void 				SetExpand( const String& rStr ) { sTxt = rStr; }

    // SubType erfragen/setzen
    virtual USHORT 		GetSubType() const;
    virtual void        SetSubType( USHORT n );

    // SequenceNo erfragen/setzen (nur fuer REF_SEQUENCEFLD interressant)
    USHORT 				GetSeqNo() const		{ return nSeqNo; }
    void  				SetSeqNo( USHORT n ) 	{ nSeqNo = n; }

    // Name der Referenz
    virtual const String& GetPar1() const;
    virtual void        SetPar1(const String& rStr);

    virtual String      GetPar2() const;
    virtual	BOOL       	QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMId ) const;
    virtual	BOOL		PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMId );

    void                ConvertProgrammaticToUIName();
};


} //namespace binfilter
#endif // _REFFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
