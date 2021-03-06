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

#pragma once

#include <memory>
#include <stack>
#include <tools/datetime.hxx>
#include <chgtrack.hxx>
#include <document.hxx>
#include "xelink.hxx"
#include "xestring.hxx"
#include "excrecds.hxx"
#include "xlformula.hxx"
#include "xeformula.hxx"

class ExcXmlRecord : public ExcRecord
{
public:
    virtual std::size_t GetLen() const override;
    virtual sal_uInt16  GetNum() const override;
    virtual void        Save( XclExpStream& rStrm ) override;
};

// XclExpUserBView - one UserBView record for each user

class XclExpUserBView : public ExcRecord
{
private:
    XclExpString                sUsername;
    sal_uInt8                   aGUID[ 16 ];

    virtual void                SaveCont( XclExpStream& rStrm ) override;

public:
                                XclExpUserBView( const OUString& rUsername, const sal_uInt8* pGUID );

    const sal_uInt8*     GetGUID() const { return aGUID; }

    virtual sal_uInt16              GetNum() const override;
    virtual std::size_t         GetLen() const override;
};

// XclExpUserBViewList - list of UserBView records

class XclExpUserBViewList : public ExcEmptyRec
{
private:
    std::vector<XclExpUserBView> aViews;

public:

    typedef std::vector<XclExpUserBView>::const_iterator const_iterator;

                                XclExpUserBViewList( const ScChangeTrack& rChangeTrack );
    virtual                     ~XclExpUserBViewList() override;

    const_iterator cbegin () { return aViews.cbegin(); }
    const_iterator cend () { return aViews.cend(); }

    virtual void                Save( XclExpStream& rStrm ) override;
};

// XclExpUsersViewBegin - begin of view block (one per sheet)

class XclExpUsersViewBegin : public ExcRecord
{
private:
    sal_uInt8                   aGUID[ 16 ];
    sal_uInt32                  nCurrTab;

    virtual void                SaveCont( XclExpStream& rStrm ) override;

public:
                                XclExpUsersViewBegin( const sal_uInt8* pGUID, sal_uInt32 nTab );
    virtual sal_uInt16              GetNum() const override;
    virtual std::size_t         GetLen() const override;
};

// XclExpUsersViewEnd - end of view block (one per sheet)

class XclExpUsersViewEnd : public ExcRecord
{
private:
    virtual void                SaveCont( XclExpStream& rStrm ) override;

public:
    virtual sal_uInt16              GetNum() const override;
    virtual std::size_t         GetLen() const override;
};

// dummy record for "User Names" stream

class XclExpChTr0x0191 : public ExcRecord
{
private:
    virtual void                SaveCont( XclExpStream& rStrm ) override;

public:
    virtual sal_uInt16              GetNum() const override;
    virtual std::size_t         GetLen() const override;
};

// dummy record for "User Names" stream

class XclExpChTr0x0198 : public ExcRecord
{
private:
    virtual void                SaveCont( XclExpStream& rStrm ) override;

public:
    virtual sal_uInt16              GetNum() const override;
    virtual std::size_t         GetLen() const override;
};

// dummy record for "User Names" stream

class XclExpChTr0x0192 : public ExcRecord
{
private:
    virtual void                SaveCont( XclExpStream& rStrm ) override;

public:
    virtual sal_uInt16              GetNum() const override;
    virtual std::size_t         GetLen() const override;
};

// dummy record for "User Names" stream

class XclExpChTr0x0197 : public ExcRecord
{
private:
    virtual void                SaveCont( XclExpStream& rStrm ) override;

public:
    virtual sal_uInt16              GetNum() const override;
    virtual std::size_t         GetLen() const override;
};

// dummy record without content

class XclExpChTrEmpty : public ExcRecord
{
private:
    sal_uInt16                      nRecNum;

public:
    XclExpChTrEmpty( sal_uInt16 nNum ) : nRecNum( nNum ) {}
    virtual                     ~XclExpChTrEmpty() override;

    virtual sal_uInt16              GetNum() const override;
    virtual std::size_t         GetLen() const override;
};

// dummy record for "Revision Log" stream

class XclExpChTr0x0195 : public ExcRecord
{
private:
    virtual void                SaveCont( XclExpStream& rStrm ) override;

public:
    virtual                     ~XclExpChTr0x0195() override;

    virtual sal_uInt16              GetNum() const override;
    virtual std::size_t         GetLen() const override;
};

// dummy record for "Revision Log" stream

class XclExpChTr0x0194 : public ExcRecord
{
private:
    XclExpString                sUsername;
    DateTime                    aDateTime;

    virtual void                SaveCont( XclExpStream& rStrm ) override;

public:
    inline                      XclExpChTr0x0194( const ScChangeTrack& rChangeTrack );
    virtual                     ~XclExpChTr0x0194() override;

    virtual sal_uInt16              GetNum() const override;
    virtual std::size_t         GetLen() const override;
};

inline XclExpChTr0x0194::XclExpChTr0x0194( const ScChangeTrack& rChangeTrack ) :
    sUsername( rChangeTrack.GetUser() ),
    aDateTime( rChangeTrack.GetFixDateTime() )
{
}

// XclExpChTrHeader - header record, includes action count

class XclExpChTrHeader : public ExcRecord
{
private:
    sal_uInt8                   aGUID[ 16 ];
    sal_uInt32                  nCount;

    virtual void                SaveCont( XclExpStream& rStrm ) override;

public:
    XclExpChTrHeader() : nCount( 0 ) {}
    virtual                     ~XclExpChTrHeader() override;

    void                 SetGUID( const sal_uInt8* pGUID )   { memcpy( aGUID, pGUID, 16 ); }
    void                 SetCount( sal_uInt32 nNew )         { nCount = nNew; }

    virtual sal_uInt16              GetNum() const override;
    virtual std::size_t         GetLen() const override;

    virtual void                SaveXml( XclExpXmlStream& rStrm ) override;
};

class XclExpXmlChTrHeaders : public ExcXmlRecord
{
    sal_uInt8 maGUID[16];
public:
    void SetGUID( const sal_uInt8* pGUID );

    virtual void SaveXml( XclExpXmlStream& rStrm ) override;
};

class XclExpChTrTabIdBuffer;
class XclExpChTrAction;

class XclExpXmlChTrHeader : public ExcXmlRecord
{
    OUString maUserName;
    DateTime maDateTime;
    sal_uInt8 maGUID[16];
    sal_Int32 mnLogNumber;
    sal_uInt32 mnMinAction;
    sal_uInt32 mnMaxAction;

    std::vector<sal_uInt16> maTabBuffer;
    std::vector<std::unique_ptr<XclExpChTrAction>> maActions;

public:
    XclExpXmlChTrHeader(
        const OUString& rUserName, const DateTime& rDateTime, const sal_uInt8* pGUID,
        sal_Int32 nLogNumber, const XclExpChTrTabIdBuffer& rBuf );

    virtual void SaveXml( XclExpXmlStream& rStrm ) override;

    void AppendAction( std::unique_ptr<XclExpChTrAction> pAction );
};

// XclExpChTrInfo - header of action group of a user

class XclExpChTrInfo : public ExcRecord
{
private:
    XclExpString                sUsername;
    DateTime                    aDateTime;
    sal_uInt8                   aGUID[ 16 ];

    virtual void                SaveCont( XclExpStream& rStrm ) override;

public:
    XclExpChTrInfo( const OUString& rUsername, const DateTime& rDateTime,
                    const sal_uInt8* pGUID );

    virtual                     ~XclExpChTrInfo() override;

    virtual sal_uInt16              GetNum() const override;
    virtual std::size_t         GetLen() const override;
};

// XclExpChTrTabIdBuffer - buffer for tab id's

class XclExpChTrTabIdBuffer
{
private:
    std::unique_ptr<sal_uInt16[]>
                                pBuffer;
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

    sal_uInt16           GetBufferCount() const
                                    { return static_cast< sal_uInt16 >( (pLast - pBuffer.get()) + 1 ); }
    void                 GetBufferCopy( sal_uInt16* pDest ) const
                                    { memcpy( pDest, pBuffer.get(), sizeof(sal_uInt16) * GetBufferCount() ); }
};

// XclExpChTrTabId - tab id record

class XclExpChTrTabId : public ExcRecord
{
private:
    std::unique_ptr<sal_uInt16[]> pBuffer;
    sal_uInt16                  nTabCount;

    void                 Clear() { pBuffer.reset(); }

    virtual void                SaveCont( XclExpStream& rStrm ) override;

public:
    XclExpChTrTabId( sal_uInt16 nCount ) : nTabCount( nCount ) {}
                                XclExpChTrTabId( const XclExpChTrTabIdBuffer& rBuffer );
    virtual                     ~XclExpChTrTabId() override;

    void                        Copy( const XclExpChTrTabIdBuffer& rBuffer );

    virtual sal_uInt16              GetNum() const override;
    virtual std::size_t         GetLen() const override;
};

// XclExpChTrAction - base class for action records

class XclExpChTrAction : public ExcRecord
{
private:
    OUString                    sUsername;
    DateTime                    aDateTime;
    sal_uInt32                  nIndex;         // action number
    std::unique_ptr<XclExpChTrAction>
                                pAddAction;     // additional record for this action
    bool                        bAccepted;

protected:
    const XclExpTabInfo&        rTabInfo;       // for table num export (sc num -> xcl num)
    const XclExpChTrTabIdBuffer& rIdBuffer;     // for table num export (xcl num -> tab id)
    sal_uInt32                  nLength;        // this is not the record size
    sal_uInt16                  nOpCode;        // EXC_CHTR_OP_***
    bool                        bForceInfo;

                                XclExpChTrAction( const XclExpChTrAction& rCopy );

    void                        SetAddAction( XclExpChTrAction* pAction );
    void                        AddDependentContents(
                                    const ScChangeAction& rAction,
                                    const XclExpRoot& rRoot,
                                    const ScChangeTrack& rChangeTrack );

    static inline void          Write2DAddress( XclExpStream& rStrm, const ScAddress& rAddress );
    static inline void          Write2DRange( XclExpStream& rStrm, const ScRange& rRange );
    inline sal_uInt16           GetTabId( SCTAB nTabId ) const;
    inline void                 WriteTabId( XclExpStream& rStrm, SCTAB nTabId ) const;

                                // save header data, call SaveActionData()
    virtual void                SaveCont( XclExpStream& rStrm ) override;
    static std::size_t   GetHeaderByteCount() { return 12; }

                                // override to save action data without header, called by SaveCont()
    virtual void                SaveActionData( XclExpStream& rStrm ) const = 0;
                                // override to get action size without header, called by GetLen()
    virtual std::size_t         GetActionByteCount() const = 0;

                                // do something before writing the record
    virtual void                PrepareSaveAction( XclExpStream& rStrm ) const;
                                // do something after writing the record
    virtual void                CompleteSaveAction( XclExpStream& rStrm ) const;

    bool                 GetAccepted() const { return bAccepted; }

public:
                                XclExpChTrAction(
                                    const ScChangeAction& rAction,
                                    const XclExpRoot& rRoot,
                                    const XclExpChTrTabIdBuffer& rTabIdBuffer,
                                    sal_uInt16 nNewOpCode = EXC_CHTR_OP_UNKNOWN );
    virtual                     ~XclExpChTrAction() override;

    const OUString&      GetUsername() const             { return sUsername; }
    const DateTime&      GetDateTime() const             { return aDateTime; }
    const XclExpChTrTabIdBuffer& GetTabIdBuffer() const  { return rIdBuffer; }
    bool                 ForceInfoRecord() const         { return bForceInfo; }

                                // set own index & return new index
                                // could override to use more indexes per action
    void                        SetIndex( sal_uInt32& rIndex );

    virtual void                Save( XclExpStream& rStrm ) override;
    virtual std::size_t         GetLen() const override;

    XclExpChTrAction*    GetAddAction() { return pAddAction.get(); }
    sal_uInt32           GetActionNumber() const { return nIndex; }
};

inline void XclExpChTrAction::Write2DAddress( XclExpStream& rStrm, const ScAddress& rAddress )
{
    rStrm   << static_cast<sal_uInt16>(rAddress.Row())
            << static_cast<sal_uInt16>(rAddress.Col());
}

inline void XclExpChTrAction::Write2DRange( XclExpStream& rStrm, const ScRange& rRange )
{
    rStrm   << static_cast<sal_uInt16>(rRange.aStart.Row())
            << static_cast<sal_uInt16>(rRange.aEnd.Row())
            << static_cast<sal_uInt16>(rRange.aStart.Col())
            << static_cast<sal_uInt16>(rRange.aEnd.Col());
}

inline sal_uInt16 XclExpChTrAction::GetTabId( SCTAB nTab ) const
{
    return rIdBuffer.GetId( rTabInfo.GetXclTab( nTab ) );
}

inline void XclExpChTrAction::WriteTabId( XclExpStream& rStrm, SCTAB nTab ) const
{
    rStrm << GetTabId( nTab );
}

// XclExpChTrData - cell content itself

struct XclExpChTrData
{
    std::unique_ptr<XclExpString> pString;
    XclExpStringRef             mpFormattedString;
    const ScFormulaCell*        mpFormulaCell;
    XclTokenArrayRef            mxTokArr;
    XclExpRefLog                maRefLog;
    double                      fValue;
    sal_Int32                   nRKValue;
    sal_uInt16                  nType;
    std::size_t                 nSize;

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

// XclExpChTrCellContent - changed cell content

class XclExpChTrCellContent final : public XclExpChTrAction, protected XclExpRoot
{
    std::unique_ptr<XclExpChTrData> pOldData;
    std::unique_ptr<XclExpChTrData> pNewData;
    sal_uInt16                  nOldLength;     // this is not the record size
    ScAddress                   aPosition;

    static void                 MakeEmptyChTrData( std::unique_ptr<XclExpChTrData>& rpData );

    void GetCellData(
        const XclExpRoot& rRoot, const ScCellValue& rScCell, std::unique_ptr<XclExpChTrData>& rpData,
        sal_uInt32& rXclLength1, sal_uInt16& rXclLength2 );

    virtual void                SaveActionData( XclExpStream& rStrm ) const override;

public:
                                XclExpChTrCellContent(
                                    const ScChangeActionContent& rAction,
                                    const XclExpRoot& rRoot,
                                    const XclExpChTrTabIdBuffer& rTabIdBuffer );
    virtual                     ~XclExpChTrCellContent() override;

    virtual sal_uInt16              GetNum() const override;
    virtual std::size_t         GetActionByteCount() const override;

    virtual void                SaveXml( XclExpXmlStream& rStrm ) override;
};

// XclExpChTrInsert - insert/delete columns/rows

class XclExpChTrInsert : public XclExpChTrAction
{
    bool mbEndOfList;

protected:
    ScRange                     aRange;

    XclExpChTrInsert( const XclExpChTrInsert& rCopy );

    virtual void                SaveActionData( XclExpStream& rStrm ) const override;
    virtual void                PrepareSaveAction( XclExpStream& rStrm ) const override;
    virtual void                CompleteSaveAction( XclExpStream& rStrm ) const override;

public:
                                XclExpChTrInsert(
                                    const ScChangeAction& rAction,
                                    const XclExpRoot& rRoot,
                                    const XclExpChTrTabIdBuffer& rTabIdBuffer,
                                    const ScChangeTrack& rChangeTrack );
    virtual                     ~XclExpChTrInsert() override;

    virtual sal_uInt16              GetNum() const override;
    virtual std::size_t         GetActionByteCount() const override;

    virtual void                SaveXml( XclExpXmlStream& rStrm ) override;
};

// XclExpChTrInsertTab - insert table

class XclExpChTrInsertTab : public XclExpChTrAction, protected XclExpRoot
{
private:
    SCTAB                   nTab;

protected:
    virtual void                SaveActionData( XclExpStream& rStrm ) const override;

public:
                                XclExpChTrInsertTab(
                                    const ScChangeAction& rAction,
                                    const XclExpRoot& rRoot,
                                    const XclExpChTrTabIdBuffer& rTabIdBuffer );
    virtual                     ~XclExpChTrInsertTab() override;

    virtual sal_uInt16              GetNum() const override;
    virtual std::size_t         GetActionByteCount() const override;

    virtual void                SaveXml( XclExpXmlStream& rStrm ) override;
};

// XclExpChTrMoveRange - move cell range

class XclExpChTrMoveRange final : public XclExpChTrAction
{
    ScRange                     aSourceRange;
    ScRange                     aDestRange;

    virtual void                SaveActionData( XclExpStream& rStrm ) const override;
    virtual void                PrepareSaveAction( XclExpStream& rStrm ) const override;
    virtual void                CompleteSaveAction( XclExpStream& rStrm ) const override;

public:
                                XclExpChTrMoveRange(
                                    const ScChangeActionMove& rAction,
                                    const XclExpRoot& rRoot,
                                    const XclExpChTrTabIdBuffer& rTabIdBuffer,
                                    const ScChangeTrack& rChangeTrack );
    virtual                     ~XclExpChTrMoveRange() override;

    virtual sal_uInt16              GetNum() const override;
    virtual std::size_t         GetActionByteCount() const override;

    virtual void                SaveXml( XclExpXmlStream& rStrm ) override;
};

// XclExpChTr0x019A - additional data for delete action

class XclExpChTr0x014A : public XclExpChTrInsert
{
protected:
    virtual void                SaveActionData( XclExpStream& rStrm ) const override;

public:
                                XclExpChTr0x014A( const XclExpChTrInsert& rAction );
    virtual                     ~XclExpChTr0x014A() override;

    virtual sal_uInt16              GetNum() const override;
    virtual std::size_t         GetActionByteCount() const override;

    virtual void                SaveXml( XclExpXmlStream& rStrm ) override;
};

// XclExpChangeTrack - exports the "Revision Log" stream

class XclExpChangeTrack : protected XclExpRoot
{
    typedef std::vector<std::unique_ptr<ExcRecord>> RecListType;
    RecListType maRecList;           // list of "Revision Log" stream records
    std::stack<XclExpChTrAction*> aActionStack;
    XclExpChTrTabIdBuffer*        pTabIdBuffer;
    std::vector<std::unique_ptr<XclExpChTrTabIdBuffer>>
                                  maBuffers;

    ScDocumentUniquePtr         xTempDoc;           // empty document

    ScChangeTrack*              CreateTempChangeTrack();
    void                        PushActionRecord( const ScChangeAction& rAction );

    bool                        WriteUserNamesStream();

public:
                                XclExpChangeTrack( const XclExpRoot& rRoot );
                                virtual ~XclExpChangeTrack() override;

    void                        Write();
    void                        WriteXml( XclExpXmlStream& rStrm );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
