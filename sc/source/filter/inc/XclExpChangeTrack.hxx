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

#ifndef SC_XCLEXPCHANGETRACK_HXX
#define SC_XCLEXPCHANGETRACK_HXX

#include <stack>
#include <tools/datetime.hxx>
#include <rtl/uuid.h>
#include "bigrange.hxx"
#include "chgtrack.hxx"
#include "xelink.hxx"
#include "ftools.hxx"
#include "excrecds.hxx"

//___________________________________________________________________

class ScBaseCell;

//___________________________________________________________________
// XclExpUserBView - one UserBView record for each user

class XclExpUserBView : public ExcRecord
{
private:
    XclExpString                sUsername;
    sal_uInt8                   aGUID[ 16 ];

    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                XclExpUserBView( const String& rUsername, const sal_uInt8* pGUID );

    inline const sal_uInt8*     GetGUID() const { return aGUID; }

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;
};

//___________________________________________________________________
// XclExpUserBViewList - list of UserBView records

class XclExpUserBViewList : public ExcEmptyRec
{
private:
    std::vector<XclExpUserBView*> aViews;

public:

    typedef std::vector<XclExpUserBView*>::iterator iterator;
    typedef std::vector<XclExpUserBView*>::const_iterator const_iterator;

                                XclExpUserBViewList( const ScChangeTrack& rChangeTrack );
    virtual                     ~XclExpUserBViewList();

    inline iterator begin () { return aViews.begin(); }

    inline iterator end () { return aViews.end(); }

    inline const_iterator begin () const { return aViews.begin(); }

    inline const_iterator end () const { return aViews.end(); }

    virtual void                Save( XclExpStream& rStrm );
};

//___________________________________________________________________
// XclExpUsersViewBegin - begin of view block (one per sheet)

class XclExpUsersViewBegin : public ExcRecord
{
private:
    sal_uInt8                   aGUID[ 16 ];
    sal_uInt32                  nCurrTab;

    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                XclExpUsersViewBegin( const sal_uInt8* pGUID, sal_uInt32 nTab );
    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;
};

//___________________________________________________________________
// XclExpUsersViewEnd - end of view block (one per sheet)

class XclExpUsersViewEnd : public ExcRecord
{
private:
    virtual void                SaveCont( XclExpStream& rStrm );

public:
    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;
};

//___________________________________________________________________
// dummy record for "User Names" stream

class XclExpChTr0x0191 : public ExcRecord
{
private:
    virtual void                SaveCont( XclExpStream& rStrm );

public:
    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;
};

//___________________________________________________________________
// dummy record for "User Names" stream

class XclExpChTr0x0198 : public ExcRecord
{
private:
    virtual void                SaveCont( XclExpStream& rStrm );

public:
    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;
};

//___________________________________________________________________
// dummy record for "User Names" stream

class XclExpChTr0x0192 : public ExcRecord
{
private:
    virtual void                SaveCont( XclExpStream& rStrm );

public:
    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;
};

//___________________________________________________________________
// dummy record for "User Names" stream

class XclExpChTr0x0197 : public ExcRecord
{
private:
    virtual void                SaveCont( XclExpStream& rStrm );

public:
    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;
};

//___________________________________________________________________
// dummy record without content

class XclExpChTrEmpty : public ExcRecord
{
private:
    sal_uInt16                      nRecNum;

public:
    inline                      XclExpChTrEmpty( sal_uInt16 nNum ) : nRecNum( nNum ) {}
    virtual                     ~XclExpChTrEmpty();

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;
};

//___________________________________________________________________
// dummy record for "Revision Log" stream

class XclExpChTr0x0195 : public ExcRecord
{
private:
    virtual void                SaveCont( XclExpStream& rStrm );

public:
    virtual                     ~XclExpChTr0x0195();

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;
};

//___________________________________________________________________
// dummy record for "Revision Log" stream

class XclExpChTr0x0194 : public ExcRecord
{
private:
    XclExpString                sUsername;
    DateTime                    aDateTime;

    virtual void                SaveCont( XclExpStream& rStrm );

public:
    inline                      XclExpChTr0x0194( const ScChangeTrack& rChangeTrack );
    virtual                     ~XclExpChTr0x0194();

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;
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

    virtual void                SaveCont( XclExpStream& rStrm );

public:
    inline                      XclExpChTrHeader() : nCount( 0 ) {}
    virtual                     ~XclExpChTrHeader();

    inline void                 SetGUID( const sal_uInt8* pGUID )   { memcpy( aGUID, pGUID, 16 ); }
    inline void                 SetCount( sal_uInt32 nNew )         { nCount = nNew; }

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;

    virtual void                SaveXml( XclExpXmlStream& rStrm );
};

//___________________________________________________________________
// XclExpChTrInfo - header of action group of a user

class XclExpChTrInfo : public ExcRecord
{
private:
    XclExpString                sUsername;
    sal_Int32                   mnLogNumber;
    DateTime                    aDateTime;
    sal_uInt8                   aGUID[ 16 ];

    virtual void                SaveCont( XclExpStream& rStrm );

public:
    inline                      XclExpChTrInfo(
                                    const String& rUsername,
                                    const DateTime& rDateTime,
                                    const sal_uInt8* pGUID,
                                    sal_Int32 nLogNumber );
    virtual                     ~XclExpChTrInfo();

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;

    virtual void                SaveXml( XclExpXmlStream& rStrm );
};

inline XclExpChTrInfo::XclExpChTrInfo( const String& rUsername, const DateTime& rDateTime, const sal_uInt8* pGUID, sal_Int32 nLogNumber ) :
    sUsername( rUsername ),
    mnLogNumber( nLogNumber ),
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
                                    { return static_cast< sal_uInt16 >( (pLast - pBuffer) + 1 ); }
    inline void                 GetBufferCopy( sal_uInt16* pDest ) const
                                    { memcpy( pDest, pBuffer, sizeof(sal_uInt16) * GetBufferCount() ); }
};

//___________________________________________________________________
// XclExpChTrTabId - tab id record

class XclExpChTrTabId : public ExcRecord
{
private:
    sal_uInt16*                 pBuffer;
    sal_uInt16                  nTabCount;
    bool                        mbInRevisionHeaders;

    inline void                 Clear() { if( pBuffer ) delete[] pBuffer; pBuffer = NULL; }

    virtual void                SaveCont( XclExpStream& rStrm );

public:
    inline                      XclExpChTrTabId( sal_uInt16 nCount ) :
                                    pBuffer( NULL ), nTabCount( nCount ), mbInRevisionHeaders( false ) {}
                                XclExpChTrTabId( const XclExpChTrTabIdBuffer& rBuffer, bool bInRevisionHeaders = false );
    virtual                     ~XclExpChTrTabId();

    void                        Copy( const XclExpChTrTabIdBuffer& rBuffer );

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;

    virtual void                SaveXml( XclExpXmlStream& rStrm );
};

//___________________________________________________________________
// XclExpChTrAction - base class for action records

class XclExpChTrAction : public ExcRecord
{
private:
    String                      sUsername;
    DateTime                    aDateTime;
    sal_uInt32                  nIndex;         // action number
    XclExpChTrAction*           pAddAction;     // additional record for this action
    sal_Bool                    bAccepted;

protected:
    const XclExpTabInfo&        rTabInfo;       // for table num export (sc num -> xcl num)
    const XclExpChTrTabIdBuffer& rIdBuffer;     // for table num export (xcl num -> tab id)
    sal_uInt32                  nLength;        // this is not the record size
    sal_uInt16                  nOpCode;        // EXC_CHTR_OP_***
    sal_Bool                    bForceInfo;

                                XclExpChTrAction( const XclExpChTrAction& rCopy );

    void                        SetAddAction( XclExpChTrAction* pAction );
    void                        AddDependentContents(
                                    const ScChangeAction& rAction,
                                    const XclExpRoot& rRoot,
                                    ScChangeTrack& rChangeTrack );

    inline void                 Write2DAddress( XclExpStream& rStrm, const ScAddress& rAddress ) const;
    inline void                 Write2DRange( XclExpStream& rStrm, const ScRange& rRange ) const;
    inline sal_uInt16           GetTabId( SCTAB nTabId ) const;
    inline void                 WriteTabId( XclExpStream& rStrm, SCTAB nTabId ) const;

                                // save header data, call SaveActionData()
    virtual void                SaveCont( XclExpStream& rStrm );
    inline sal_Size             GetHeaderByteCount() const  { return 12; }

                                // overload to save action data without header, called by SaveCont()
    virtual void                SaveActionData( XclExpStream& rStrm ) const = 0;
                                // overload to get action size without header, called by GetLen()
    virtual sal_Size            GetActionByteCount() const = 0;

                                // do something before writing the record
    virtual void                PrepareSaveAction( XclExpStream& rStrm ) const;
                                // do something after writing the record
    virtual void                CompleteSaveAction( XclExpStream& rStrm ) const;

    inline sal_uInt32           GetActionNumber() const { return nIndex; }
    inline sal_Bool             GetAccepted() const { return bAccepted; }

public:
                                XclExpChTrAction(
                                    const ScChangeAction& rAction,
                                    const XclExpRoot& rRoot,
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

    virtual void                Save( XclExpStream& rStrm );
    virtual sal_Size            GetLen() const;

    inline XclExpChTrAction*    GetAddAction() { return pAddAction; }
};

inline void XclExpChTrAction::Write2DAddress( XclExpStream& rStrm, const ScAddress& rAddress ) const
{
    rStrm   << (sal_uInt16) rAddress.Row()
            << (sal_uInt16) rAddress.Col();
}

inline void XclExpChTrAction::Write2DRange( XclExpStream& rStrm, const ScRange& rRange ) const
{
    rStrm   << (sal_uInt16) rRange.aStart.Row()
            << (sal_uInt16) rRange.aEnd.Row()
            << (sal_uInt16) rRange.aStart.Col()
            << (sal_uInt16) rRange.aEnd.Col();
}

inline sal_uInt16 XclExpChTrAction::GetTabId( SCTAB nTab ) const
{
    return rIdBuffer.GetId( rTabInfo.GetXclTab( nTab ) );
}

inline void XclExpChTrAction::WriteTabId( XclExpStream& rStrm, SCTAB nTab ) const
{
    rStrm << GetTabId( nTab );
}

//___________________________________________________________________
// XclExpChTrData - cell content itself

struct XclExpChTrData
{
    XclExpString*               pString;
    XclExpStringRef             mpFormattedString;
    const ScFormulaCell*        mpFormulaCell;
    XclTokenArrayRef            mxTokArr;
    XclExpRefLog                maRefLog;
    double                      fValue;
    sal_Int32                   nRKValue;
    sal_uInt16                  nType;
    sal_Size                    nSize;

                                XclExpChTrData();
                                ~XclExpChTrData();
    void                        Clear();

    void                        WriteFormula(
                                    XclExpStream& rStrm,
                                    const XclExpChTrTabIdBuffer& rTabIdBuffer );
    void                        Write(
                                    XclExpStream& rStrm,
                                    const XclExpChTrTabIdBuffer& rTabIdBuffer );
};

//___________________________________________________________________
// XclExpChTrCellContent - changed cell content

class XclExpChTrCellContent : public XclExpChTrAction, protected XclExpRoot
{
private:
    XclExpChTrData*             pOldData;
    XclExpChTrData*             pNewData;
    sal_uInt16                  nOldLength;     // this is not the record size

    void                        MakeEmptyChTrData( XclExpChTrData*& rpData );

protected:
    ScAddress                   aPosition;

    void                        GetCellData(
                                    const XclExpRoot& rRoot,
                                    const ScBaseCell* pScCell,
                                    XclExpChTrData*& rpData,
                                    sal_uInt32& rXclLength1,
                                    sal_uInt16& rXclLength2 );

    virtual void                SaveActionData( XclExpStream& rStrm ) const;

public:
                                XclExpChTrCellContent(
                                    const ScChangeActionContent& rAction,
                                    const XclExpRoot& rRoot,
                                    const XclExpChTrTabIdBuffer& rTabIdBuffer );
    virtual                     ~XclExpChTrCellContent();

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetActionByteCount() const;

    virtual void                SaveXml( XclExpXmlStream& rStrm );
};

//___________________________________________________________________
// XclExpChTrInsert - insert/delete columns/rows

class XclExpChTrInsert : public XclExpChTrAction
{
protected:
    ScRange                     aRange;

                                XclExpChTrInsert( const XclExpChTrInsert& rCopy ) :
                                    XclExpChTrAction( rCopy ), aRange( rCopy.aRange ) {}

    virtual void                SaveActionData( XclExpStream& rStrm ) const;
    virtual void                PrepareSaveAction( XclExpStream& rStrm ) const;
    virtual void                CompleteSaveAction( XclExpStream& rStrm ) const;

public:
                                XclExpChTrInsert(
                                    const ScChangeAction& rAction,
                                    const XclExpRoot& rRoot,
                                    const XclExpChTrTabIdBuffer& rTabIdBuffer,
                                    ScChangeTrack& rChangeTrack );
    virtual                     ~XclExpChTrInsert();

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetActionByteCount() const;

    virtual void                SaveXml( XclExpXmlStream& rStrm );
};

//___________________________________________________________________
// XclExpChTrInsertTab - insert table

class XclExpChTrInsertTab : public XclExpChTrAction, protected XclExpRoot
{
private:
    SCTAB                   nTab;

protected:
    virtual void                SaveActionData( XclExpStream& rStrm ) const;

public:
                                XclExpChTrInsertTab(
                                    const ScChangeAction& rAction,
                                    const XclExpRoot& rRoot,
                                    const XclExpChTrTabIdBuffer& rTabIdBuffer );
    virtual                     ~XclExpChTrInsertTab();

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetActionByteCount() const;

    virtual void                SaveXml( XclExpXmlStream& rStrm );
};

//___________________________________________________________________
// XclExpChTrMoveRange - move cell range

class XclExpChTrMoveRange : public XclExpChTrAction
{
protected:
    ScRange                     aSourceRange;
    ScRange                     aDestRange;

    virtual void                SaveActionData( XclExpStream& rStrm ) const;
    virtual void                PrepareSaveAction( XclExpStream& rStrm ) const;
    virtual void                CompleteSaveAction( XclExpStream& rStrm ) const;

public:
                                XclExpChTrMoveRange(
                                    const ScChangeActionMove& rAction,
                                    const XclExpRoot& rRoot,
                                    const XclExpChTrTabIdBuffer& rTabIdBuffer,
                                    ScChangeTrack& rChangeTrack );
    virtual                     ~XclExpChTrMoveRange();

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetActionByteCount() const;

    virtual void                SaveXml( XclExpXmlStream& rStrm );
};

//___________________________________________________________________
// XclExpChTr0x019A - additional data for delete action

class XclExpChTr0x014A : public XclExpChTrInsert
{
protected:
    virtual void                SaveActionData( XclExpStream& rStrm ) const;

public:
                                XclExpChTr0x014A( const XclExpChTrInsert& rAction );
    virtual                     ~XclExpChTr0x014A();

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetActionByteCount() const;

    virtual void                SaveXml( XclExpXmlStream& rStrm );
};

//___________________________________________________________________
// XclExpChangeTrack - exports the "Revision Log" stream

class XclExpChangeTrack : protected XclExpRoot
{
private:
    std::vector<ExcRecord*>       aRecList;           // list of "Revision Log" stream records
    std::stack<XclExpChTrAction*> aActionStack;
    XclExpChTrTabIdBuffer*        pTabIdBuffer;
    std::vector<XclExpChTrTabIdBuffer*> maBuffers;

    ScDocument*                 pTempDoc;           // empty document

    XclExpChTrHeader*           pHeader;            // header record for last GUID
    sal_uInt8                   aGUID[ 16 ];        // GUID for action info records
    sal_Bool                    bValidGUID;

    ScChangeTrack*              CreateTempChangeTrack();
    void                        PushActionRecord( const ScChangeAction& rAction );

    sal_Bool                    WriteUserNamesStream();

public:
                                XclExpChangeTrack( const XclExpRoot& rRoot );
                                ~XclExpChangeTrack();

    void                        Write();
    void                        WriteXml( XclExpXmlStream& rStrm );
};

//___________________________________________________________________

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
