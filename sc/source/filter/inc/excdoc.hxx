/*************************************************************************
 *
 *  $RCSfile: excdoc.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-08 16:27:02 $
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

#ifndef _EXCDOC_HXX
#define _EXCDOC_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef SC_XEROOT_HXX
#include "xeroot.hxx"
#endif

#ifndef _EXCRECDS_HXX
#include "excrecds.hxx"
#endif
#ifndef _ROOT_HXX
#include "root.hxx"
#endif


#define XCL_COLMAX                  (UINT16(255))
#define EXC5_ANZROW                 (UINT16(16383))
#define XCL8_ROWMAX                 (UINT16(65535))


//------------------------------------------------------------------ Forwards -

class SvStream;
class ScBaseCell;
class ScHorizontalCellIterator;
class ScDocument;
class ScProgress;

class NameBuffer;

class XclExpStream;
class XclExpChangeTrack;


//--------------------------------------------------- class ExcRecordListRefs -

class ExcRecordListRefs : public List
{
private:
protected:
public:
    virtual                     ~ExcRecordListRefs();
    inline void                 Append( XclExpRecordBase* );
    inline XclExpRecordBase*    First();
    inline XclExpRecordBase*    Next();
                                List::Count;
};


inline void ExcRecordListRefs::Append( XclExpRecordBase* pER )
{
    List::Insert( pER, CONTAINER_APPEND );
}


inline XclExpRecordBase* ExcRecordListRefs::First()
{
    return ( XclExpRecordBase* ) List::First();
}


inline XclExpRecordBase* ExcRecordListRefs::Next()
{
    return ( XclExpRecordBase* ) List::Next();
}


//----------------------------------------------------------- class DefRowXFs -

class DefRowXFs
{
private:
    struct XclExpDefRowXFEntry
    {
        sal_uInt32                  mnXFId;
        sal_uInt16                  mnRow;
        inline explicit             XclExpDefRowXFEntry() : mnXFId( 0 ), mnRow( 0 ) {}
        inline explicit             XclExpDefRowXFEntry( sal_uInt32 nXFId, sal_uInt16 nRow ) :
                                        mnXFId( nXFId ), mnRow( nRow ) {}
    };

    typedef ::std::vector< XclExpDefRowXFEntry > XclExpDefRowXFVec;

    XclExpDefRowXFVec           maXFList;
    UINT32                      nLastList;
    UINT16                      nLastRow;

public:
                                DefRowXFs( void );

    inline void                 Append( sal_uInt16 nRow, sal_uInt32 nXFId );

    BOOL                        ChangeXF( sal_uInt16 nRow, sal_uInt32& rnXFId );
};

inline void DefRowXFs::Append( sal_uInt16 nRow, sal_uInt32 nXFId )
{
    maXFList.push_back( XclExpDefRowXFEntry( nXFId, nRow ) );
}


//------------------------------------------------------------ class ExcTable -

class ExcTable : public XclExpRecordBase, public ExcRoot
{
private:
    XclExpRecordList<>          aRecList;
    UINT16                      nScTab;     // table number SC document
    UINT16                      nExcTab;    // table number Excel document
    UINT16                      nAktRow;    // fuer'n Iterator
    UINT16                      nAktCol;

    static ExcRowBlock*         pRowBlock;  // buffer for ROW recs
    DefRowXFs*                  pDefRowXFs;

    void                        Clear();
    void                        NullTab( const String* pCodename = NULL );
    // pRec mit new anlegen und vergessen, delete macht ExcTable selber!
    inline void                 Add( XclExpRecordBase* pRec );

    void                        AddRow( ExcRow* pRow );
    void                        AddUsedRow( ExcRow*& rpRow );   // Add() or delete

public:
                                ExcTable( RootData* pRD );
                                ExcTable( RootData* pRD, UINT16 nScTable );
                                ~ExcTable();

    void                        FillAsHeader( ExcRecordListRefs& rBundleSheetRecList );
    void                        FillAsTable( void );

    void                        SetDefRowXF( sal_uInt16 nRowNum, sal_uInt32 nXFId );
    BOOL                        ModifyToDefaultRowXF( sal_uInt16 nRowNum, sal_uInt32& rnXFId );

    void                        Write( XclExpStream& );
};


inline void ExcTable::Add( XclExpRecordBase* pRec )
{
    DBG_ASSERT( pRec, "-ExcTable::Add(): pRec ist NULL!" );
    aRecList.Append( pRec );
}


//--------------------------------------------------------- class ExcDocument -

class ExcDocument : protected XclExpRoot
{
friend class ExcTable;

private:
    ExcRecordListRefs   aBundleSheetRecList;
    ExcTable            aHeader;

    XclExpRecordList< ExcTable > maTableList;

    ScProgress*         pPrgrsBar;

    static NameBuffer*  pTabNames;

    XclExpChangeTrack*  pExpChangeTrack;

public:
    explicit                    ExcDocument( const XclExpRoot& rRoot );
    virtual                     ~ExcDocument();

    void                ReadDoc( void );
    void                Write( SvStream& rSvStrm );
};




#endif

