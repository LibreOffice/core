/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: excdoc.hxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:16:08 $
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

//------------------------------------------------------------------ Forwards -

class SvStream;
class ScBaseCell;
class ScHorizontalCellIterator;
class ScDocument;
class ScProgress;

class NameBuffer;

class XclExpChangeTrack;


//------------------------------------------------------------ class ExcTable -

class XclExpCellTable;

class ExcTable : public XclExpRecordBase, public XclExpRoot
{
private:
    typedef XclExpRecordList< ExcBundlesheetBase >  ExcBoundsheetList;
    typedef ScfRef< XclExpCellTable >               XclExpCellTableRef;

    XclExpRecordList<>          aRecList;
    XclExpCellTableRef          mxCellTable;

    SCTAB                       mnScTab;    // table number SC document
    UINT16                      nExcTab;    // table number Excel document
    UINT16                      nAktRow;    // fuer'n Iterator
    UINT16                      nAktCol;

    NameBuffer*                 pTabNames;

    // pRec mit new anlegen und vergessen, delete macht ExcTable selber!
    void                        Add( XclExpRecordBase* pRec );

public:
                                ExcTable( const XclExpRoot& rRoot );
                                ExcTable( const XclExpRoot& rRoot, SCTAB nScTab );
                                ~ExcTable();

    void                        FillAsHeader( ExcBoundsheetList& rBoundsheetList );
    void                        FillAsTable( size_t nCodeNameIdx );
    void                        FillAsEmptyTable( size_t nCodeNameIdx );

    void                        Write( XclExpStream& );
};


//--------------------------------------------------------- class ExcDocument -

class ExcDocument : protected XclExpRoot
{
friend class ExcTable;

private:
    typedef XclExpRecordList< ExcTable >            ExcTableList;
    typedef ExcTableList::RecordRefType             ExcTableRef;
    typedef XclExpRecordList< ExcBundlesheetBase >  ExcBoundsheetList;
    typedef ExcBoundsheetList::RecordRefType        ExcBoundsheetRef;

    ExcTable            aHeader;

    ExcTableList        maTableList;
    ExcBoundsheetList   maBoundsheetList;

    XclExpChangeTrack*  pExpChangeTrack;

public:
    explicit                    ExcDocument( const XclExpRoot& rRoot );
    virtual                     ~ExcDocument();

    void                ReadDoc( void );
    void                Write( SvStream& rSvStrm );
};




#endif

