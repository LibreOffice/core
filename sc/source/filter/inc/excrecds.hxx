/*************************************************************************
 *
 *  $RCSfile: excrecds.hxx,v $
 *
 *  $Revision: 1.45 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-14 12:07:47 $
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

#ifndef _EXCRECDS_HXX
#define _EXCRECDS_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif


#include <vector>

#ifndef SC_OUTLINETAB_HXX
#include "olinetab.hxx"
#endif
#ifndef SC_FILTER_HXX
#include "filter.hxx"
#endif
#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif

#ifndef SC_XEROOT_HXX
#include "xeroot.hxx"
#endif
#ifndef SC_XEFORMULA_HXX
#include "xeformula.hxx"
#endif
#ifndef SC_XESTRING_HXX
#include "xestring.hxx"
#endif
#ifndef SC_XERECORD_HXX
#include "xerecord.hxx"
#endif

#ifndef _ROOT_HXX
#include "root.hxx"
#endif
#ifndef _FLTTOOLS_HXX
#include "flttools.hxx"
#endif
#ifndef _EXCDEFS_HXX
#include "excdefs.hxx"
#endif
#ifndef SC_CELL_HXX
#include "cell.hxx"
#endif

//------------------------------------------------------------------ Forwards -

class SvxBorderLine;

class SvStream;
class Font;
class List;
class ScPatternAttr;
class ScTokenArray;
class ScRangeData;
class ScDBData;
class ScEditCell;
class SfxItemSet;
class EditTextObject;
class ScPageHFItem;
class ScProgress;

class ExcTable;

//----------------------------------------------------------- class ExcRecord -

class ExcRecord : public XclExpRecord
{
public:
    virtual void            Save( XclExpStream& rStrm );

    virtual UINT16          GetNum() const = 0;
    virtual ULONG           GetLen() const = 0;

protected:
    virtual void            SaveCont( XclExpStream& rStrm );

private:
    /** Writes the body of the record. */
    virtual void            WriteBody( XclExpStream& rStrm );
};


//--------------------------------------------------------- class ExcEmptyRec -

class ExcEmptyRec : public ExcRecord
{
private:
protected:
public:
    virtual void            Save( XclExpStream& rStrm );
    virtual UINT16          GetNum() const;
    virtual ULONG           GetLen() const;
};


//------------------------------------------------------- class ExcRecordList -

class ExcRecordList : protected List, public ExcEmptyRec
{
private:
protected:
public:
    virtual                 ~ExcRecordList();

                            List::Count;

    inline ExcRecord*       First( void )               { return ( ExcRecord* ) List::First(); }
    inline ExcRecord*       Next( void )                { return ( ExcRecord* ) List::Next(); }

    inline void             Append( ExcRecord* pNew )   { if( pNew ) List::Insert( pNew, LIST_APPEND ); }
    inline const ExcRecord* Get( UINT32 nNum ) const    { return ( ExcRecord* ) List::GetObject( nNum ); }

    virtual void            Save( XclExpStream& rStrm );
};


//--------------------------------------------------------- class ExcDummyRec -

class ExcDummyRec : public ExcRecord
{
protected:
public:
    virtual void            Save( XclExpStream& rStrm );
    virtual UINT16          GetNum() const;
    virtual const BYTE*     GetData() const = 0;    // byte data must contain header and body
};


//------------------------------------------------------- class ExcBoolRecord -
// stores BOOL as 16bit val ( 0x0000 | 0x0001 )

class ExcBoolRecord : public ExcRecord
{
private:
    virtual void            SaveCont( XclExpStream& rStrm );

protected:
    BOOL                    bVal;

    inline                  ExcBoolRecord() : bVal( FALSE ) {}

public:
    inline                  ExcBoolRecord( const BOOL bDefault ) : bVal( bDefault ) {}
                            ExcBoolRecord( SfxItemSet*, USHORT nWhich, BOOL bDefault );

    virtual ULONG           GetLen( void ) const;
};


//--------------------------------------------------------- class ExcBof_Base -

class ExcBof_Base : public ExcRecord
{
private:
protected:
    UINT16                  nDocType;
    UINT16                  nVers;
    UINT16                  nRupBuild;
    UINT16                  nRupYear;
public:
                            ExcBof_Base( void );
};


//-------------------------------------------------------------- class ExcBof -
// Header Record fuer WORKSHEETS

class ExcBof : public ExcBof_Base
{
private:
    virtual void            SaveCont( XclExpStream& rStrm );
public:
                            ExcBof( void );

    virtual UINT16          GetNum( void ) const;
    virtual ULONG           GetLen( void ) const;
};


//------------------------------------------------------------- class ExcBofW -
// Header Record fuer WORKBOOKS

class ExcBofW : public ExcBof_Base
{
private:
    virtual void            SaveCont( XclExpStream& rStrm );
public:
                            ExcBofW( void );

    virtual UINT16          GetNum( void ) const;
    virtual ULONG           GetLen( void ) const;
};


//-------------------------------------------------------------- class ExcEof -

class ExcEof : public ExcRecord
{
private:
public:
    virtual UINT16          GetNum( void ) const;
    virtual ULONG           GetLen( void ) const;
};


//----------------------------------------------------- class ExcFngroupcount -

class ExcFngroupcount : public ExcRecord
{
private:
    virtual void            SaveCont( XclExpStream& rStrm );
public:
    virtual UINT16          GetNum( void ) const;
    virtual ULONG           GetLen( void ) const;
};


//--------------------------------------------------------- class ExcDummy_00 -
// INTERFACEHDR to FNGROUPCOUNT (see excrecds.cxx)

class ExcDummy_00 : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const ULONG      nMyLen;
public:
    virtual ULONG           GetLen( void ) const;
    virtual const BYTE*     GetData( void ) const;
};

// EXC_ID_WINDOWPROTECTION
class XclExpWindowProtection : public   XclExpBoolRecord
{
    public:
        XclExpWindowProtection(bool bValue);
};

// EXC_ID_PROTECT  Document Protection
class XclExpDocProtection : public  XclExpBoolRecord
{
    public:
        XclExpDocProtection(bool bValue);
};


//-------------------------------------------------------- class ExcDummy_04x -
// PASSWORD to BOOKBOOL (see excrecds.cxx), no 1904

class ExcDummy_040 : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const ULONG      nMyLen;
public:
    virtual ULONG           GetLen( void ) const;
    virtual const BYTE*     GetData( void ) const;
};



class ExcDummy_041 : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const ULONG      nMyLen;
public:
    virtual ULONG           GetLen( void ) const;
    virtual const BYTE*     GetData( void ) const;
};


//------------------------------------------------------------- class Exc1904 -

class Exc1904 : public ExcBoolRecord
{
public:
                            Exc1904( ScDocument& rDoc );
    virtual UINT16          GetNum( void ) const;
};


//------------------------------------------------------ class ExcBundlesheet -

class ExcBundlesheetBase : public ExcRecord
{
protected:
    ULONG                   nStrPos;
    ULONG                   nOwnPos;    // Position NACH # und Len
    UINT16                  nGrbit;

                            ExcBundlesheetBase();

public:
                            ExcBundlesheetBase( RootData& rRootData, SCTAB nTab );

    inline void             SetStreamPos( ULONG nNewStrPos ) { nStrPos = nNewStrPos; }
    void                    UpdateStreamPos( XclExpStream& rStrm );

    virtual UINT16          GetNum() const;
};



class ExcBundlesheet : public ExcBundlesheetBase
{
private:
    ByteString              aName;

    virtual void            SaveCont( XclExpStream& rStrm );

public:
                            ExcBundlesheet( RootData& rRootData, SCTAB nTab );
    virtual ULONG           GetLen() const;
};

//--------------------------------------------------------- class ExcDummy_02 -
// sheet dummies: CALCMODE to SETUP

class ExcDummy_02a : public ExcDummyRec
{
private:
    static const BYTE       pMyData[];
    static const ULONG      nMyLen;
public:
    virtual ULONG           GetLen( void ) const;
    virtual const BYTE*     GetData( void ) const;
};


// ----------------------------------------------------------------------------

/** This record contains the Windows country IDs for the UI and document language. */
class XclExpCountry : public XclExpRecord
{
public:
    explicit                    XclExpCountry( const XclExpRoot& rRoot );

private:
    sal_uInt16                  mnUICountry;        /// The UI country ID.
    sal_uInt16                  mnDocCountry;       /// The document country ID.

    /** Writes the body of the COUNTRY record. */
    virtual void                WriteBody( XclExpStream& rStrm );
};


//---------------------------------------------------------- class ExcWindow2 -

class XclExpWindow2 : public XclExpRecord
{
public:
    explicit            XclExpWindow2( const XclExpRoot& rRoot, SCTAB nScTab );

private:
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    sal_uInt16          mnFlags;
};


//-------------------------------------------------------- class ExcSelection -

class ExcSelection : public ExcRecord
{
private:
    UINT16                  nCol;
    UINT16                  nRow;
    UINT8                   nPane;

    virtual void            SaveCont( XclExpStream& rStrm );

public:
    inline                  ExcSelection( UINT16 _nCol, UINT16 _nRow, UINT8 _nPane ) :
                                nCol( _nCol ), nRow( _nRow ), nPane( _nPane ) {}

    virtual UINT16          GetNum( void ) const;
    virtual ULONG           GetLen( void ) const;
};


// XclExpWsbool ===============================================================

class XclExpWsbool : public XclExpUInt16Record
{
public:
    explicit                    XclExpWsbool( bool bFitToPages );
};


// ============================================================================

class XclExpFiltermode : public XclExpEmptyRecord
{
public:
    explicit            XclExpFiltermode();
};

// ----------------------------------------------------------------------------

class XclExpAutofilterinfo : public XclExpUInt16Record
{
public:
    explicit            XclExpAutofilterinfo( const ScAddress& rStartPos, SCCOL nScCol );

    inline const ScAddress GetStartPos() const { return maStartPos; }
    inline SCCOL        GetColCount() const { return static_cast< SCCOL >( GetValue() ); }

private:
    ScAddress           maStartPos;
};

// ----------------------------------------------------------------------------

class ExcFilterCondition
{
private:
    UINT8                   nType;
    UINT8                   nOper;
    double                  fVal;
    XclExpString*           pText;

protected:
public:
                            ExcFilterCondition();
                            ~ExcFilterCondition();

    inline BOOL             IsEmpty() const     { return (nType == EXC_AFTYPE_NOTUSED); }
    inline BOOL             HasEqual() const    { return (nOper == EXC_AFOPER_EQUAL); }
    ULONG                   GetTextBytes() const;

    void                    SetCondition( UINT8 nTp, UINT8 nOp, double fV, String* pT );

    void                    Save( XclExpStream& rStrm );
    void                    SaveText( XclExpStream& rStrm );
};

// ----------------------------------------------------------------------------

class XclExpAutofilter : public XclExpRecord, protected XclExpRoot
{
private:
    UINT16                  nCol;
    UINT16                  nFlags;
    ExcFilterCondition      aCond[ 2 ];

    BOOL                    AddCondition( ScQueryConnect eConn, UINT8 nType,
                                UINT8 nOp, double fVal, String* pText,
                                BOOL bSimple = FALSE );

    virtual void            WriteBody( XclExpStream& rStrm );

protected:
public:
                            XclExpAutofilter( const XclExpRoot& rRoot, UINT16 nC );

    inline UINT16           GetCol() const          { return nCol; }
    inline BOOL             HasCondition() const    { return !aCond[ 0 ].IsEmpty(); }
    inline BOOL             HasTop10() const        { return TRUEBOOL( nFlags & EXC_AFFLAG_TOP10 ); }

    BOOL                    AddEntry( const ScQueryEntry& rEntry );
};

// ----------------------------------------------------------------------------

class ExcAutoFilterRecs : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            ExcAutoFilterRecs( const XclExpRoot& rRoot, SCTAB nTab );
    virtual             ~ExcAutoFilterRecs();

    void                AddObjRecs();

    virtual void        Save( XclExpStream& rStrm );

private:
    XclExpAutofilter*   GetByCol( SCCOL nCol ); // always 0-based
    BOOL                IsFiltered( SCCOL nCol );

private:
    typedef XclExpRecordList< XclExpAutofilter >    XclExpAutofilterList;
    typedef XclExpAutofilterList::RecordRefType     XclExpAutofilterRef;

    XclExpAutofilterList maFilterList;
    XclExpFiltermode*   pFilterMode;
    XclExpAutofilterinfo* pFilterInfo;
};

// ----------------------------------------------------------------------------

/** Sheet filter manager. Contains auto filters or advanced filters from all sheets. */
class XclExpFilterManager : protected XclExpRoot
{
public:
    explicit            XclExpFilterManager( const XclExpRoot& rRoot );

    /** Creates the filter records for the specified sheet.
        @descr  Creates and inserts related built-in NAME records. Therefore this
            function is called from the name buffer itself. */
    void                InitTabFilter( SCTAB nScTab );

    /** Returns a record object containing all filter records for the specified sheet. */
    XclExpRecordRef     CreateRecord( SCTAB nScTab );

private:
    typedef ScfRef< ExcAutoFilterRecs >             XclExpTabFilterRef;
    typedef ::std::map< SCTAB, XclExpTabFilterRef > XclExpTabFilterMap;

    XclExpTabFilterMap  maFilterMap;
};


#endif

