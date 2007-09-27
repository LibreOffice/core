/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: reffld.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:09:10 $
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
#ifndef _REFFLD_HXX
#define _REFFLD_HXX

#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif

class SfxPoolItem;
class SwDoc;
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

    SwDoc*                  GetDoc() const { return pDoc; }
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

    virtual String      GetCntnt(BOOL bName = FALSE) const;
    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    const String&       GetSetRefName() const { return sSetRefName; }

    void                UpdateField();
    void                SetExpand( const String& rStr ) { sTxt = rStr; }

    // SubType erfragen/setzen
    virtual USHORT      GetSubType() const;
    virtual void        SetSubType( USHORT n );

    // SequenceNo erfragen/setzen (nur fuer REF_SEQUENCEFLD interressant)
    USHORT              GetSeqNo() const        { return nSeqNo; }
    void                SetSeqNo( USHORT n )    { nSeqNo = n; }

    // Name der Referenz
    virtual const String& GetPar1() const;
    virtual void        SetPar1(const String& rStr);

    virtual String      GetPar2() const;
    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, USHORT nWhichId ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, USHORT nWhichId );

    void                ConvertProgrammaticToUIName();

    virtual String GetDescription() const;
};


#endif // _REFFLD_HXX

