/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SC_EXCRECDS_HXX
#define SC_EXCRECDS_HXX

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <svl/zforlist.hxx>
#include <tools/solar.h>
#include <vcl/vclenum.hxx>


#include <vector>
#include "olinetab.hxx"
#include "filter.hxx"
#include "rangelst.hxx"
#include "xerecord.hxx"
#include "xeroot.hxx"
#include "xeformula.hxx"
#include "xestring.hxx"
#include "root.hxx"
#include "excdefs.hxx"
#include <boost/shared_ptr.hpp>

//------------------------------------------------------------------ Forwards -

struct ScQueryEntry;

//----------------------------------------------------------- class ExcRecord -

class ExcRecord : public XclExpRecord
{
public:
    virtual void            Save( XclExpStream& rStrm );
    virtual void            SaveXml( XclExpXmlStream& rStrm );

    virtual sal_uInt16          GetNum() const = 0;
    virtual sal_Size        GetLen() const = 0;

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
    virtual sal_uInt16          GetNum() const;
    virtual sal_Size        GetLen() const;
};

//--------------------------------------------------------- class ExcDummyRec -

class ExcDummyRec : public ExcRecord
{
protected:
public:
    virtual void            Save( XclExpStream& rStrm );
    virtual sal_uInt16          GetNum() const;
    virtual const sal_uInt8*        GetData() const = 0;    // byte data must contain header and body
};


//------------------------------------------------------- class ExcBoolRecord -
// stores sal_Bool as 16bit val ( 0x0000 | 0x0001 )

class ExcBoolRecord : public ExcRecord
{
private:
    virtual void            SaveCont( XclExpStream& rStrm );

protected:
    sal_Bool                    bVal;

    inline                  ExcBoolRecord() : bVal( false ) {}

public:
    inline                  ExcBoolRecord( const sal_Bool bDefault ) : bVal( bDefault ) {}

    virtual sal_Size        GetLen( void ) const;
};


//--------------------------------------------------------- class ExcBof_Base -

class ExcBof_Base : public ExcRecord
{
private:
protected:
    sal_uInt16                  nDocType;
    sal_uInt16                  nVers;
    sal_uInt16                  nRupBuild;
    sal_uInt16                  nRupYear;
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

    virtual sal_uInt16          GetNum( void ) const;
    virtual sal_Size        GetLen( void ) const;
};


//------------------------------------------------------------- class ExcBofW -
// Header Record fuer WORKBOOKS

class ExcBofW : public ExcBof_Base
{
private:
    virtual void            SaveCont( XclExpStream& rStrm );
public:
                            ExcBofW( void );

    virtual sal_uInt16          GetNum( void ) const;
    virtual sal_Size        GetLen( void ) const;
};


//-------------------------------------------------------------- class ExcEof -

class ExcEof : public ExcRecord
{
private:
public:
    virtual sal_uInt16          GetNum( void ) const;
    virtual sal_Size        GetLen( void ) const;
};


//--------------------------------------------------------- class ExcDummy_00 -
// INTERFACEHDR to FNGROUPCOUNT (see excrecds.cxx)

class ExcDummy_00 : public ExcDummyRec
{
private:
    static const sal_uInt8      pMyData[];
    static const sal_Size   nMyLen;
public:
    virtual sal_Size        GetLen( void ) const;
    virtual const sal_uInt8*        GetData( void ) const;
};

// EXC_ID_WINDOWPROTECTION
class XclExpWindowProtection : public   XclExpBoolRecord
{
    public:
        XclExpWindowProtection(bool bValue);

    virtual void            SaveXml( XclExpXmlStream& rStrm );
};

// EXC_ID_PROTECT  Document Protection
class XclExpProtection : public XclExpBoolRecord
{
    public:
        XclExpProtection(bool bValue);
};

class XclExpSheetProtection : public XclExpProtection
{
    SCTAB                   mnTab;
    public:
        XclExpSheetProtection(bool bValue, SCTAB nTab);
    virtual void            SaveXml( XclExpXmlStream& rStrm );
};

class XclExpPassHash : public XclExpRecord
{
public:
    XclExpPassHash(const ::com::sun::star::uno::Sequence<sal_Int8>& aHash);
    virtual ~XclExpPassHash();

private:
    virtual void    WriteBody(XclExpStream& rStrm);

private:
    sal_uInt16  mnHash;
};


//-------------------------------------------------------- class ExcDummy_04x -
// PASSWORD to BOOKBOOL (see excrecds.cxx), no 1904

class ExcDummy_040 : public ExcDummyRec
{
private:
    static const sal_uInt8      pMyData[];
    static const sal_Size   nMyLen;
public:
    virtual sal_Size        GetLen( void ) const;
    virtual const sal_uInt8*        GetData( void ) const;
};



class ExcDummy_041 : public ExcDummyRec
{
private:
    static const sal_uInt8      pMyData[];
    static const sal_Size   nMyLen;
public:
    virtual sal_Size        GetLen( void ) const;
    virtual const sal_uInt8*        GetData( void ) const;
};


//------------------------------------------------------------- class Exc1904 -

class Exc1904 : public ExcBoolRecord
{
public:
                            Exc1904( ScDocument& rDoc );
    virtual sal_uInt16          GetNum( void ) const;

    virtual void            SaveXml( XclExpXmlStream& rStrm );
private:
    sal_Bool                    bDateCompatibility;
};


//------------------------------------------------------ class ExcBundlesheet -

class ExcBundlesheetBase : public ExcRecord
{
protected:
    sal_Size                nStrPos;
    sal_Size                nOwnPos;    // Position NACH # und Len
    sal_uInt16                  nGrbit;
    SCTAB                   nTab;

                            ExcBundlesheetBase();

public:
                            ExcBundlesheetBase( RootData& rRootData, SCTAB nTab );

    inline void             SetStreamPos( sal_Size nNewStrPos ) { nStrPos = nNewStrPos; }
    void                    UpdateStreamPos( XclExpStream& rStrm );

    virtual sal_uInt16          GetNum() const;
};



class ExcBundlesheet : public ExcBundlesheetBase
{
private:
    OString            aName;

    virtual void            SaveCont( XclExpStream& rStrm );

public:
                            ExcBundlesheet( RootData& rRootData, SCTAB nTab );
    virtual sal_Size        GetLen() const;
};

//--------------------------------------------------------- class ExcDummy_02 -
// sheet dummies: CALCMODE to SETUP

class ExcDummy_02a : public ExcDummyRec
{
private:
    static const sal_uInt8      pMyData[];
    static const sal_Size   nMyLen;
public:
    virtual sal_Size        GetLen( void ) const;
    virtual const sal_uInt8*        GetData( void ) const;
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


// XclExpWsbool ===============================================================

class XclExpWsbool : public XclExpUInt16Record
{
public:
    explicit                    XclExpWsbool( bool bFitToPages, SCTAB nScTab = -1, XclExpFilterManager* pManager = NULL );

    virtual void                SaveXml( XclExpXmlStream& rStrm );
private:
    SCTAB                       mnScTab;
    XclExpFilterManager*        mpManager;
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
    sal_uInt8               nType;
    sal_uInt8               nOper;
    double                  fVal;
    XclExpString*           pText;

protected:
public:
                            ExcFilterCondition();
                            ~ExcFilterCondition();

    inline bool             IsEmpty() const     { return (nType == EXC_AFTYPE_NOTUSED); }
    inline bool             HasEqual() const    { return (nOper == EXC_AFOPER_EQUAL); }
    sal_uLong               GetTextBytes() const;

    void                    SetCondition( sal_uInt8 nTp, sal_uInt8 nOp, double fV, OUString* pT );

    void                    Save( XclExpStream& rStrm );
    void                    SaveXml( XclExpXmlStream& rStrm );
    void                    SaveText( XclExpStream& rStrm );
};

// ----------------------------------------------------------------------------

class XclExpAutofilter : public XclExpRecord, protected XclExpRoot
{
private:
    enum FilterType { FilterCondition, MultiValue };
    FilterType              meType;
    sal_uInt16              nCol;
    sal_uInt16              nFlags;
    ExcFilterCondition      aCond[ 2 ];
    std::vector<OUString> maMultiValues;

    bool                    AddCondition( ScQueryConnect eConn, sal_uInt8 nType,
                                sal_uInt8 nOp, double fVal, OUString* pText,
                                bool bSimple = false );

    virtual void            WriteBody( XclExpStream& rStrm );

public:
                            XclExpAutofilter( const XclExpRoot& rRoot, sal_uInt16 nC );

    inline sal_uInt16       GetCol() const          { return nCol; }
    inline bool             HasTop10() const        { return ::get_flag( nFlags, EXC_AFFLAG_TOP10 ); }

    bool                    HasCondition() const;
    bool                    AddEntry( const ScQueryEntry& rEntry );
    bool                    AddMultiValueEntry( const ScQueryEntry& rEntry );

    virtual void            SaveXml( XclExpXmlStream& rStrm );
};

// ----------------------------------------------------------------------------

class ExcAutoFilterRecs : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            ExcAutoFilterRecs( const XclExpRoot& rRoot, SCTAB nTab );
    virtual             ~ExcAutoFilterRecs();

    void                AddObjRecs();

    virtual void        Save( XclExpStream& rStrm );
    virtual void        SaveXml( XclExpXmlStream& rStrm );

    bool                HasFilterMode() const;

private:
    XclExpAutofilter*   GetByCol( SCCOL nCol ); // always 0-based
    bool                IsFiltered( SCCOL nCol );

private:
    typedef XclExpRecordList< XclExpAutofilter >    XclExpAutofilterList;
    typedef XclExpAutofilterList::RecordRefType     XclExpAutofilterRef;

    XclExpAutofilterList maFilterList;
    XclExpFiltermode*   pFilterMode;
    XclExpAutofilterinfo* pFilterInfo;
    ScRange                 maRef;
    bool mbAutoFilter;
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

    /** Returns whether or not FilterMode is present */
    bool                HasFilterMode( SCTAB nScTab );

private:
    using               XclExpRoot::CreateRecord;

    typedef boost::shared_ptr< ExcAutoFilterRecs >  XclExpTabFilterRef;
    typedef ::std::map< SCTAB, XclExpTabFilterRef > XclExpTabFilterMap;

    XclExpTabFilterMap  maFilterMap;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
