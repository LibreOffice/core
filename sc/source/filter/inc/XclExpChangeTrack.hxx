/*************************************************************************
 *
 *  $RCSfile: XclExpChangeTrack.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dr $ $Date: 2000-12-18 16:31:35 $
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

#ifndef _SC_XCLEXPCHANGETRACK_HXX
#define _SC_XCLEXPCHANGETRACK_HXX

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#ifndef SC_BIGRANGE_HXX
#include "bigrange.hxx"
#endif
#ifndef SC_CHGTRACK_HXX
#include "chgtrack.hxx"
#endif

#ifndef _EXCDOC_HXX
#include "excdoc.hxx"
#endif
#ifndef _EXCRECDS_HXX
#include "excrecds.hxx"
#endif
#ifndef _XCL97EXP_HXX
#include "xcl97exp.hxx"
#endif

//___________________________________________________________________

class ExcUPN;
class ScBaseCell;

//___________________________________________________________________
// XclExpUserBView - one UserBView record for each user

class XclExpUserBView : public ExcRecord
{
private:
    XclRawUnicodeString         sUsername;
    sal_uInt8                   aGUID[ 16 ];

protected:
    virtual void                SaveCont( SvStream& rStrm );

public:
                                XclExpUserBView( const String& rUsername, const sal_uInt8* pGUID );

    inline const sal_uInt8*     GetGUID() const { return aGUID; }

    virtual sal_uInt16          GetNum() const;
    virtual sal_uInt16          GetLen() const;
};

//___________________________________________________________________
// XclExpUserBViewList - list of UserBView records

class XclExpUserBViewList : public ExcEmptyRec, private List
{
private:
    inline XclExpUserBView*     _First()    { return (XclExpUserBView*) List::First(); }
    inline XclExpUserBView*     _Next()     { return (XclExpUserBView*) List::Next(); }

public:
                                XclExpUserBViewList( const ScChangeTrack& rChangeTrack );
    virtual                     ~XclExpUserBViewList();

    inline const XclExpUserBView* First()   { return (const XclExpUserBView*) List::First(); }
    inline const XclExpUserBView* Next()    { return (const XclExpUserBView*) List::Next(); }

    virtual void                Save( SvStream& rStrm );
};

//___________________________________________________________________
// XclExpUsersViewBegin - begin of view block (one per sheet)

class XclExpUsersViewBegin : public ExcRecord
{
private:
    sal_uInt8                   aGUID[ 16 ];
    sal_uInt32                  nCurrTab;

protected:
    virtual void                SaveCont( SvStream& rStrm );

public:
                                XclExpUsersViewBegin( const sal_uInt8* pGUID, sal_uInt32 nTab );
    virtual sal_uInt16          GetNum() const;
    virtual sal_uInt16          GetLen() const;
};

//___________________________________________________________________
// XclExpUsersViewEnd - end of view block (one per sheet)

class XclExpUsersViewEnd : public ExcRecord
{
protected:
    virtual void                SaveCont( SvStream& rStrm );

public:
    virtual sal_uInt16          GetNum() const;
    virtual sal_uInt16          GetLen() const;
};

//___________________________________________________________________
// dummy record for "User Names" stream

class XclExpChTr0x0191 : public ExcRecord
{
protected:
    virtual void                SaveCont( SvStream& rStrm );

public:
    virtual sal_uInt16          GetNum() const;
    virtual sal_uInt16          GetLen() const;
};

//___________________________________________________________________
// dummy record for "User Names" stream

class XclExpChTr0x0198 : public ExcRecord
{
protected:
    virtual void                SaveCont( SvStream& rStrm );

public:
    virtual sal_uInt16          GetNum() const;
    virtual sal_uInt16          GetLen() const;
};

//___________________________________________________________________
// dummy record for "User Names" stream

class XclExpChTr0x0192 : public ExcRecord
{
protected:
    virtual void                SaveCont( SvStream& rStrm );

public:
    virtual sal_uInt16          GetNum() const;
    virtual sal_uInt16          GetLen() const;
};

//___________________________________________________________________
// dummy record for "User Names" stream

class XclExpChTr0x0197 : public ExcRecord
{
protected:
    virtual void                SaveCont( SvStream& rStrm );

public:
    virtual sal_uInt16          GetNum() const;
    virtual sal_uInt16          GetLen() const;
};

//___________________________________________________________________
// dummy record without content

class XclExpChTrEmpty : public ExcRecord
{
private:
    sal_uInt16                  nRecNum;

public:
    inline                      XclExpChTrEmpty( sal_uInt16 nNum ) : nRecNum( nNum ) {}
    virtual                     ~XclExpChTrEmpty();

    virtual sal_uInt16          GetNum() const;
    virtual sal_uInt16          GetLen() const;
};

//___________________________________________________________________
// dummy record for "Revision Log" stream

class XclExpChTr0x0195 : public ExcRecord
{
protected:
    virtual void                SaveCont( SvStream& rStrm );

public:
    virtual                     ~XclExpChTr0x0195();

    virtual sal_uInt16          GetNum() const;
    virtual sal_uInt16          GetLen() const;
};

//___________________________________________________________________
// dummy record for "Revision Log" stream

class XclExpChTr0x0194 : public ExcRecord
{
private:
    XclRawUnicodeString         sUsername;
    DateTime                    aDateTime;

protected:
    virtual void                SaveCont( SvStream& rStrm );

public:
    inline                      XclExpChTr0x0194( const ScChangeTrack& rChangeTrack );
    virtual                     ~XclExpChTr0x0194();

    virtual sal_uInt16          GetNum() const;
    virtual sal_uInt16          GetLen() const;
};

inline XclExpChTr0x0194::XclExpChTr0x0194( const ScChangeTrack& rChangeTrack ) :
    sUsername( rChangeTrack.GetUser() ),
    aDateTime( rChangeTrack.GetFixDateTime() )
{
}

//___________________________________________________________________
// XclExpChTrHeader - header record, includes action count

class XclExpChTrHeader : public ExcRecord
{
private:
    sal_uInt8                   aGUID[ 16 ];
    sal_uInt32                  nCount;

protected:
    virtual void                SaveCont( SvStream& rStrm );

public:
    inline                      XclExpChTrHeader() : nCount( 0 ) {}
    virtual                     ~XclExpChTrHeader();

    inline void                 SetGUID( const sal_uInt8* pGUID )   { memcpy( aGUID, pGUID, 16 ); }
    inline void                 SetCount( sal_uInt32 nNew )         { nCount = nNew; }

    virtual sal_uInt16          GetNum() const;
    virtual sal_uInt16          GetLen() const;
};

//___________________________________________________________________
// XclExpChTrInfo - header of action group of a user

class XclExpChTrInfo : public ExcRecord
{
private:
    XclRawUnicodeString         sUsername;
    DateTime                    aDateTime;
    sal_uInt8                   aGUID[ 16 ];

protected:
    virtual void                SaveCont( SvStream& rStrm );

public:
    inline                      XclExpChTrInfo(
                                    const String& rUsername,
                                    const DateTime& rDateTime,
                                    const sal_uInt8* pGUID );
    virtual                     ~XclExpChTrInfo();

    virtual sal_uInt16          GetNum() const;
    virtual sal_uInt16          GetLen() const;
};

inline XclExpChTrInfo::XclExpChTrInfo( const String& rUsername, const DateTime& rDateTime, const sal_uInt8* pGUID ) :
    sUsername( rUsername ),
    aDateTime( rDateTime )
{
    memcpy( aGUID, pGUID, 16 );
}

//___________________________________________________________________
// XclExpChTrTabIdBuffer - buffer for tab id's

class XclExpChTrTabIdBuffer
{
private:
    sal_uInt16*                 pBuffer;
    sal_uInt16*                 pLast;
    sal_uInt16                  nBufSize;
    sal_uInt16                  nLastId;

public:
                                XclExpChTrTabIdBuffer( sal_uInt16 nCount );
                                XclExpChTrTabIdBuffer( const XclExpChTrTabIdBuffer& rCopy );
                                ~XclExpChTrTabIdBuffer();

    void                        InitFill( sal_uInt16 nIndex );
    void                        InitFillup();

    sal_uInt16                  GetId( sal_uInt16 nIndex ) const;
    void                        Remove();

    inline sal_uInt16           GetBufferCount() const
                                    { return (pLast - pBuffer) + 1; }
    inline void                 GetBufferCopy( sal_uInt16* pDest ) const
                                    { memcpy( pDest, pBuffer, sizeof(sal_uInt16) * GetBufferCount() ); }
};

//___________________________________________________________________
// XclExpChTrTabIdBufferList

class XclExpChTrTabIdBufferList : private List
{
private:
    inline XclExpChTrTabIdBuffer* First()   { return (XclExpChTrTabIdBuffer*) List::First(); }
    inline XclExpChTrTabIdBuffer* Next()    { return (XclExpChTrTabIdBuffer*) List::Next(); }

public:
    virtual                     ~XclExpChTrTabIdBufferList();

    inline void                 Append( XclExpChTrTabIdBuffer* pNew )
                                    { List::Insert( pNew, LIST_APPEND ); }
};

//___________________________________________________________________
// XclExpChTrTabId - tab id record

class XclExpChTrTabId : public ExcRecord
{
private:
    sal_uInt16*                 pBuffer;
    sal_uInt16                  nTabCount;

    inline void                 Clear() { if( pBuffer ) delete[] pBuffer; pBuffer = NULL; }

protected:
    virtual void                SaveCont( SvStream& rStrm );

public:
    inline                      XclExpChTrTabId( sal_uInt16 nCount ) :
                                    pBuffer( NULL ), nTabCount( nCount ) {}
                                XclExpChTrTabId( const XclExpChTrTabIdBuffer& rBuffer );
    virtual                     ~XclExpChTrTabId();

    void                        Copy( const XclExpChTrTabIdBuffer& rBuffer );

    virtual sal_uInt16          GetNum() const;
    virtual sal_uInt16          GetLen() const;
};

//___________________________________________________________________
// XclExpChTrAction - base class for action records

class XclExpChTrAction : public ExcRecord
{
private:
    String                      sUsername;
    DateTime                    aDateTime;
    sal_uInt32                  nIndex;         // action number
    ExcRecord*                  pAddRecord;     // additional record for this action
    sal_Bool                    bAccepted;

protected:
    const ExcETabNumBuffer&     rTabBuffer;     // for table num export (sc num -> xcl num)
    const XclExpChTrTabIdBuffer& rIdBuffer;     // for table num export (xcl num -> tab id)
    sal_uInt32                  nLength;        // this is not the record size
    sal_uInt16                  nOpCode;        // EXC_CHTR_OP_***
    sal_Bool                    bForceInfo;

    inline                      XclExpChTrAction( const XclExpChTrAction& rCopy );

    void                        SetAddRecord( ExcRecord* pRecord );

    inline void                 Write2DAddress( SvStream& rStrm, const ScAddress& rAddress ) const;
    inline void                 Write2DRange( SvStream& rStrm, const ScRange& rRange ) const;
    inline void                 WriteTabId( SvStream& rStrm, sal_uInt16 nTabId ) const;

                                // save header data, call SaveActionData()
    virtual void                SaveCont( SvStream& rStrm );
    inline sal_uInt16           GetHeaderByteCount() const  { return 12; }

                                // overload to save action data without header, called by SaveCont()
    virtual void                SaveActionData( SvStream& rStrm ) const = 0;
                                // overload to get action size without header, called by GetLen()
    virtual sal_uInt16          GetActionByteCount() const = 0;

public:
                                XclExpChTrAction(
                                    const ScChangeAction& rAction,
                                    const RootData& rRootData,
                                    const XclExpChTrTabIdBuffer& rTabIdBuffer,
                                    sal_uInt16 nNewOpCode = EXC_CHTR_OP_UNKNOWN );
    virtual                     ~XclExpChTrAction();

    inline const String&        GetUsername() const             { return sUsername; }
    inline const DateTime&      GetDateTime() const             { return aDateTime; }
    inline const XclExpChTrTabIdBuffer& GetTabIdBuffer() const  { return rIdBuffer; }
    inline sal_Bool             ForceInfoRecord() const         { return bForceInfo; }

                                // set own index & return new index
                                // could be overloaded to use more indexes per action
    virtual void                SetIndex( sal_uInt32& rIndex );

    virtual void                Save( SvStream& rStrm );
    virtual sal_uInt16          GetLen() const;
};

inline XclExpChTrAction::XclExpChTrAction( const XclExpChTrAction& rCopy ) :
    sUsername( rCopy.sUsername ),
    aDateTime( rCopy.aDateTime ),
    nIndex( 0 ),
    rTabBuffer( rCopy.rTabBuffer ),
    pAddRecord( NULL ),
    rIdBuffer( rCopy.rIdBuffer ),
    nLength( rCopy.nLength ),
    nOpCode( rCopy.nOpCode )
{
}

inline void XclExpChTrAction::Write2DAddress( SvStream& rStrm, const ScAddress& rAddress ) const
{
    rStrm   << (sal_uInt16) rAddress.Row()
            << (sal_uInt16) rAddress.Col();
}

inline void XclExpChTrAction::Write2DRange( SvStream& rStrm, const ScRange& rRange ) const
{
    rStrm   << (sal_uInt16) rRange.aStart.Row()
            << (sal_uInt16) rRange.aEnd.Row()
            << (sal_uInt16) rRange.aStart.Col()
            << (sal_uInt16) rRange.aEnd.Col();
}

inline void XclExpChTrAction::WriteTabId( SvStream& rStrm, sal_uInt16 nTab ) const
{
    rStrm << rIdBuffer.GetId( rTabBuffer.GetExcTable( nTab ) );
}

//___________________________________________________________________
// XclExpChTrData - cell content itself

struct XclExpChTrData
{
    XclUnicodeString*           pString;
    ExcUPN*                     pUPN;
    UINT16List*                 pRefList;
    double                      fValue;
    sal_Int32                   nRKValue;
    sal_uInt16                  nType;
    sal_uInt16                  nSize;

    inline                      XclExpChTrData();
    inline                      ~XclExpChTrData() { Clear(); }
    void                        Clear();

    void                        WriteFormula(
                                    XclContinue& rCont,
                                    const RootData& rRootData,
                                    const XclExpChTrTabIdBuffer& rTabIdBuffer );
    void                        Write(
                                    SvStream& rStrm,
                                    const RootData& rRootData,
                                    const XclExpChTrTabIdBuffer& rTabIdBuffer,
                                    sal_uInt16& rRecOffset );
};

inline XclExpChTrData::XclExpChTrData() :
    pString( NULL ),
    pUPN( NULL ),
    pRefList( NULL ),
    fValue( 0.0 ),
    nRKValue( 0 ),
    nType( EXC_CHTR_TYPE_EMPTY ),
    nSize( 0 )
{
}

//___________________________________________________________________
// XclExpChTrCellContent - changed cell content

class XclExpChTrCellContent : public XclExpChTrAction, private ExcRoot
{
private:
    XclExpChTrData*             pOldData;
    XclExpChTrData*             pNewData;
    sal_uInt16                  nOldLength;     // this is not the record size

    void                        MakeEmptyChTrData( XclExpChTrData*& rpData );

protected:
    ScAddress                   aPosition;

    void                        GetCellData(
                                    const ScBaseCell* pScCell,
                                    XclExpChTrData*& rpData,
                                    sal_uInt32& rXclLength1,
                                    sal_uInt16& rXclLength2 );

    virtual void                SaveActionData( SvStream& rStrm ) const;

public:
                                XclExpChTrCellContent(
                                    const ScChangeActionContent& rAction,
                                    RootData& rRootData,
                                    const XclExpChTrTabIdBuffer& rTabIdBuffer );
    virtual                     ~XclExpChTrCellContent();

    virtual sal_uInt16          GetNum() const;
    virtual sal_uInt16          GetActionByteCount() const;
};

//___________________________________________________________________
// XclExpChTrInsert - insert/delete columns/rows

class XclExpChTrInsert : public XclExpChTrAction
{
protected:
    ScRange                     aRange;

                                XclExpChTrInsert( const XclExpChTrInsert& rCopy ) :
                                    XclExpChTrAction( rCopy ), aRange( rCopy.aRange ) {}

    virtual void                SaveActionData( SvStream& rStrm ) const;

public:
                                XclExpChTrInsert(
                                    const ScChangeAction& rAction,
                                    const RootData& rRootData,
                                    const XclExpChTrTabIdBuffer& rTabIdBuffer );
    virtual                     ~XclExpChTrInsert();

    virtual sal_uInt16          GetNum() const;
    virtual sal_uInt16          GetActionByteCount() const;
};

//___________________________________________________________________
// XclExpChTrInsertTab - insert table

class XclExpChTrInsertTab : public XclExpChTrAction, private ExcRoot
{
private:
    sal_uInt16                  nTab;

protected:
    virtual void                SaveActionData( SvStream& rStrm ) const;

public:
                                XclExpChTrInsertTab(
                                    const ScChangeAction& rAction,
                                    RootData& rRootData,
                                    const XclExpChTrTabIdBuffer& rTabIdBuffer );
    virtual                     ~XclExpChTrInsertTab();

    virtual sal_uInt16          GetNum() const;
    virtual sal_uInt16          GetActionByteCount() const;
};

//___________________________________________________________________
// XclExpChTrMoveRange - move cell range

class XclExpChTrMoveRange : public XclExpChTrAction
{
protected:
    ScRange                     aSourceRange;
    ScRange                     aDestRange;

    virtual void                SaveActionData( SvStream& rStrm ) const;

public:
                                XclExpChTrMoveRange(
                                    const ScChangeActionMove& rAction,
                                    const RootData& rRootData,
                                    const XclExpChTrTabIdBuffer& rTabIdBuffer );
    virtual                     ~XclExpChTrMoveRange();

    virtual void                Save( SvStream& rStrm );
    virtual sal_uInt16          GetNum() const;
    virtual sal_uInt16          GetActionByteCount() const;
};

//___________________________________________________________________
// XclExpChTr0x019A - additional data for delete action

class XclExpChTr0x014A : public XclExpChTrInsert
{
protected:
    virtual void                SaveActionData( SvStream& rStrm ) const;

public:
                                XclExpChTr0x014A( const XclExpChTrInsert& rAction );
    virtual                     ~XclExpChTr0x014A();

    virtual sal_uInt16          GetNum() const;
    virtual sal_uInt16          GetActionByteCount() const;
};

//___________________________________________________________________
// XclExpChTrActionStack - temporary action stack

class XclExpChTrActionStack : private Stack
{
public:
    virtual                     ~XclExpChTrActionStack();

    void                        Push( XclExpChTrAction* pNewRec );
    inline XclExpChTrAction*    Pop()   { return (XclExpChTrAction*) Stack::Pop(); }
};

//___________________________________________________________________
// XclExpChTrRecordList - list of "Revision Log" stream records

class XclExpChTrRecordList : private List
{
private:
    inline ExcRecord*           First() { return (ExcRecord*) List::First(); }
    inline ExcRecord*           Next()  { return (ExcRecord*) List::Next(); }

public:
    virtual                     ~XclExpChTrRecordList();

                                List::Count;
    void                        Append( ExcRecord* pNewRec );
    void                        Save( SvStream& rStrm );
};

//___________________________________________________________________
// XclExpChangeTrack - exports the "Revision Log" stream

class XclExpChangeTrack : private ExcRoot
{
private:
    XclExpChTrRecordList        aRecList;
    XclExpChTrActionStack       aActionStack;
    XclExpChTrTabIdBufferList   aTabIdBufferList;
    XclExpChTrTabIdBuffer*      pTabIdBuffer;

    ScDocument*                 pTempDoc;           // empty document
    ScChangeTrack*              pTempChangeTrack;   // copy of <pOrigChangeTrack>

    sal_uInt32                  nNewAction;         // action number, 1-based
    XclExpChTrHeader*           pHeader;            // header record for last GUID
    sal_uInt8                   aGUID[ 16 ];        // GUID for action info records
    sal_Bool                    bValidGUID;

    sal_Bool                    CreateTempChangeTrack();
    void                        PushActionRecord( const ScChangeAction& rAction );

    sal_Bool                    WriteUserNamesStream();

public:
                                XclExpChangeTrack( RootData* pRootData );
                                ~XclExpChangeTrack();

    void                        Write();
};

//___________________________________________________________________

#endif // _SC_XCLEXPCHANGETRACK_HXX

