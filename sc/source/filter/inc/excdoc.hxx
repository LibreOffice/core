/*************************************************************************
 *
 *  $RCSfile: excdoc.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:12 $
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

#ifndef _EXCRECDS_HXX
#include "excrecds.hxx"
#endif
#ifndef _ROOT_HXX
#include "root.hxx"
#endif


#define EXC5_ANZROW                 (UINT16(16383))
#define XCL8_ROWMAX                 (UINT16(65535))


//------------------------------------------------------------------ Forwards -

class SvStream;
class ScBaseCell;
class ScHorizontalCellIterator;
class ScDocument;

class NameBuffer;

class ScProgress;

//class ScDPCollection;
//class ScDPObject;
//class ScRange;

//--------------------------------------------------- class ExcRecordListRefs -

#ifndef MAC
class ExcRecordListRefs : protected List
#else
class ExcRecordListRefs : public List
#endif
{
private:
protected:
public:
    virtual                     ~ExcRecordListRefs();
    inline void                 Append( ExcRecord* );
    inline ExcRecord*           First( void );
    inline ExcRecord*           Next( void );
};


inline void ExcRecordListRefs::Append( ExcRecord* pER )
{
    List::Insert( pER, CONTAINER_APPEND );
}


inline ExcRecord* ExcRecordListRefs::First( void )
{
    return ( ExcRecord* ) List::First();
}


inline ExcRecord* ExcRecordListRefs::Next( void )
{
    return ( ExcRecord* ) List::Next();
}


//--------------------------------------------------- class ExcRecordListInst -

class ExcRecordListInst : public ExcRecordListRefs
{
private:
protected:
public:
    virtual                     ~ExcRecordListInst();
};


//------------------------------------------------------------ class ExcTable -

class ExcTable : public ExcRoot
{
private:
    ExcRecordListInst           aRecList;
    UINT16                      nScTab;     // table number SC document
    UINT16                      nExcTab;    // table number Excel document
    UINT16                      nAktRow;    // fuer'n Iterator
    UINT16                      nAktCol;

    static ExcRowBlock*         pRowBlock;  // buffer for ROW recs

    void                        Clear();
    void                        NullTab( const String* pCodename = NULL );
    // pRec mit new anlegen und vergessen, delete macht ExcTable selber!
    inline void                 Add( ExcRecord *pRec );

    void                        AddRow( ExcRow* pRow );
    void                        AddUsedRow( ExcRow*& rpRow );   // Add() or delete
public:
                                ExcTable( RootData* pRD );
                                ExcTable( RootData* pRD, UINT16 nScTable );
                                ~ExcTable();

    void                        FillAsHeader( ExcRecordListRefs& rBundleSheetRecList );
    void                        FillAsTable( void );

    void                        Write( SvStream& );
};


inline void ExcTable::Add( ExcRecord* pRec )
{
    DBG_ASSERT( pRec, "-ExcTable::Add(): pRec ist NULL!" );

    aRecList.Append( pRec );
}


//--------------------------------------------------------- class ExcDocument -

class ExcDocument : private List, public ExcRoot
{
friend class ExcTable;

private:
    ExcRecordListRefs   aBundleSheetRecList;
    ExcTable            aHeader;

    ScProgress*         pPrgrsBar;

    static NameBuffer*  pTabNames;

    void                Clear( void );
    void                Add( UINT16 nScTab );
public:
                        ExcDocument( RootData* pRD/*, ExportFormatExcel*/ );
                        ~ExcDocument();
    void                ReadDoc( void );
    void                Write( SvStream& );
};




#endif

