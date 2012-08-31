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
#ifndef _TOOLS_INETMSG_HXX
#define _TOOLS_INETMSG_HXX

#include "tools/toolsdllapi.h"
#include <rtl/string.hxx>
#include <rtl/textenc.h>
#include <rtl/ustring.hxx>
#include <tools/inetmime.hxx>
#include <tools/stream.hxx>

#include <vector>

class DateTime;

class INetMessageHeader
{
    OString m_aName;
    OString m_aValue;

public:
    INetMessageHeader()
    {}

    INetMessageHeader (
        const OString& rName, const OString& rValue)
        : m_aName (rName), m_aValue (rValue)
    {}

    INetMessageHeader (
        const INetMessageHeader& rHdr)
        : m_aName (rHdr.m_aName), m_aValue (rHdr.m_aValue)
    {}

    ~INetMessageHeader()
    {}

    INetMessageHeader& operator= (const INetMessageHeader& rHdr)
    {
        m_aName  = rHdr.m_aName;
        m_aValue = rHdr.m_aValue;
        return *this;
    }

    const OString& GetName() const { return m_aName; }
    const OString& GetValue() const { return m_aValue; }

    friend SvStream& operator<< (
        SvStream& rStrm, const INetMessageHeader& rHdr)
    {
        write_lenPrefixed_uInt8s_FromOString<sal_uInt16>(rStrm, rHdr.m_aName);
        write_lenPrefixed_uInt8s_FromOString<sal_uInt16>(rStrm, rHdr.m_aValue);
        return rStrm;
    }

    friend SvStream& operator>> (
        SvStream& rStrm, INetMessageHeader& rHdr)
    {
        rHdr.m_aName = read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rStrm);
        rHdr.m_aValue = read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rStrm);
        return rStrm;
    }
};

typedef ::std::vector< INetMessageHeader* > HeaderList_impl;

class INetMessage
{
    HeaderList_impl m_aHeaderList;

    sal_uIntPtr     m_nDocSize;
    OUString        m_aDocName;
    SvLockBytesRef  m_xDocLB;

    void ListCleanup_Impl();
    void ListCopy (const INetMessage& rMsg);

protected:
    OUString GetHeaderName_Impl (
        sal_uIntPtr nIndex, rtl_TextEncoding eEncoding) const
    {
        if ( nIndex < m_aHeaderList.size() ) {
            return OStringToOUString(m_aHeaderList[ nIndex ]->GetName(), eEncoding);
        } else {
            return OUString();
        }
    }

    OUString GetHeaderValue_Impl (
        sal_uIntPtr nIndex, INetMIME::HeaderFieldType eType) const
    {
        if ( nIndex < m_aHeaderList.size() ) {
            return INetMIME::decodeHeaderFieldBody(eType, m_aHeaderList[ nIndex ]->GetValue());
        } else {
            return OUString();
        }
    }

    void SetHeaderField_Impl (
        const INetMessageHeader &rHeader, sal_uIntPtr &rnIndex)
    {
        INetMessageHeader *p = new INetMessageHeader (rHeader);
        if (m_aHeaderList.size() <= rnIndex)
        {
            rnIndex = m_aHeaderList.size();
            m_aHeaderList.push_back( p );
        }
        else
        {
            delete m_aHeaderList[ rnIndex ];
            m_aHeaderList[ rnIndex ] = p;
        }
    }

    void SetHeaderField_Impl (
        INetMIME::HeaderFieldType  eType,
        const OString &rName,
        const OUString &rValue,
        sal_uIntPtr &rnIndex);

    virtual SvStream& operator<< (SvStream& rStrm) const;
    virtual SvStream& operator>> (SvStream& rStrm);

public:
    INetMessage() : m_nDocSize(0) {}
    virtual ~INetMessage();

    INetMessage (const INetMessage& rMsg)
        : m_nDocSize (rMsg.m_nDocSize),
          m_aDocName (rMsg.m_aDocName),
          m_xDocLB   (rMsg.m_xDocLB)
    {
        ListCopy (rMsg);
    }

    INetMessage& operator= (const INetMessage& rMsg)
    {
        m_nDocSize = rMsg.m_nDocSize;
        m_aDocName = rMsg.m_aDocName;
        m_xDocLB   = rMsg.m_xDocLB;
        ListCopy (rMsg);
        return *this;
    }

    sal_uIntPtr GetHeaderCount() const { return m_aHeaderList.size(); }

    OUString GetHeaderName (sal_uIntPtr nIndex) const
    {
        return GetHeaderName_Impl (nIndex, RTL_TEXTENCODING_ASCII_US);
    }

    OUString GetHeaderValue (sal_uIntPtr nIndex) const
    {
        return GetHeaderValue_Impl (nIndex, INetMIME::HEADER_FIELD_TEXT);
    }

    INetMessageHeader GetHeaderField (sal_uIntPtr nIndex) const
    {
        if ( nIndex < m_aHeaderList.size() ) {
            return INetMessageHeader( *m_aHeaderList[ nIndex ] );
        } else {
            return INetMessageHeader();
        }
    }

    virtual sal_uIntPtr SetHeaderField (
        const INetMessageHeader &rField,
        sal_uIntPtr nIndex = ((sal_uIntPtr)-1)
    );

    sal_uIntPtr GetDocumentSize() const { return m_nDocSize; }
    void  SetDocumentSize (sal_uIntPtr nSize) { m_nDocSize = nSize; }

    const OUString& GetDocumentName() const { return m_aDocName; }
    void  SetDocumentName (const OUString& rName) { m_aDocName = rName; }

    SvLockBytes* GetDocumentLB() const { return m_xDocLB; }
    void         SetDocumentLB (SvLockBytes *pDocLB) { m_xDocLB = pDocLB; }

    friend SvStream& operator<< (
        SvStream& rStrm, const INetMessage& rMsg)
    {
        return rMsg.operator<< (rStrm);
    }

    friend SvStream& operator>> (
        SvStream& rStrm, INetMessage& rMsg)
    {
        return rMsg.operator>> (rStrm);
    }
};

#define INETMSG_RFC822_BCC                 0
#define INETMSG_RFC822_CC                  1
#define INETMSG_RFC822_COMMENTS            2
#define INETMSG_RFC822_DATE                3
#define INETMSG_RFC822_FROM                4
#define INETMSG_RFC822_IN_REPLY_TO         5
#define INETMSG_RFC822_KEYWORDS            6
#define INETMSG_RFC822_MESSAGE_ID          7
#define INETMSG_RFC822_REFERENCES          8
#define INETMSG_RFC822_REPLY_TO            9
#define INETMSG_RFC822_RETURN_PATH        10
#define INETMSG_RFC822_SENDER             11
#define INETMSG_RFC822_SUBJECT            12
#define INETMSG_RFC822_TO                 13
#define INETMSG_RFC822_X_MAILER           14
#define INETMSG_RFC822_RETURN_RECEIPT_TO  15
#define INETMSG_RFC822_NUMHDR             16

class TOOLS_DLLPUBLIC INetRFC822Message : public INetMessage
{
    sal_uIntPtr m_nIndex[INETMSG_RFC822_NUMHDR];

protected:
    virtual SvStream& operator<< (SvStream& rStrm) const;
    virtual SvStream& operator>> (SvStream& rStrm);

public:
    INetRFC822Message();
    INetRFC822Message (const INetRFC822Message& rMsg);
    virtual ~INetRFC822Message();

    INetRFC822Message& operator= (const INetRFC822Message& rMsg);

    static bool ParseDateField (
        const OUString& rDateField, DateTime& rDateTime);

    using INetMessage::SetHeaderField;
    virtual sal_uIntPtr SetHeaderField (
        const INetMessageHeader &rHeader,
        sal_uIntPtr nIndex = ((sal_uIntPtr)-1)
    );

    // Header fields.

    OUString GetBCC() const
    {
        return GetHeaderValue_Impl (
            m_nIndex[INETMSG_RFC822_BCC],
            INetMIME::HEADER_FIELD_ADDRESS);
    }

    OUString GetCC() const
    {
        return GetHeaderValue_Impl (
            m_nIndex[INETMSG_RFC822_CC],
            INetMIME::HEADER_FIELD_ADDRESS);
    }

    OUString GetComments() const
    {
        return GetHeaderValue_Impl (
            m_nIndex[INETMSG_RFC822_COMMENTS],
            INetMIME::HEADER_FIELD_TEXT);
    }

    OUString GetDate() const
    {
        return GetHeaderValue_Impl (
            m_nIndex[INETMSG_RFC822_DATE],
            INetMIME::HEADER_FIELD_STRUCTURED);
    }

    OUString GetFrom() const
    {
        return GetHeaderValue_Impl (
            m_nIndex[INETMSG_RFC822_FROM],
            INetMIME::HEADER_FIELD_ADDRESS);
    }

    OUString GetInReplyTo() const
    {
        return GetHeaderValue_Impl (
            m_nIndex[INETMSG_RFC822_IN_REPLY_TO],
            INetMIME::HEADER_FIELD_ADDRESS); // ??? MESSAGE_ID ???
    }

    OUString GetKeywords() const
    {
        return GetHeaderValue_Impl (
            m_nIndex[INETMSG_RFC822_KEYWORDS],
            INetMIME::HEADER_FIELD_PHRASE);
    }

    OUString GetMessageID() const
    {
        return GetHeaderValue_Impl (
            m_nIndex[INETMSG_RFC822_MESSAGE_ID],
            INetMIME::HEADER_FIELD_MESSAGE_ID);
    }

    OUString GetReferences() const
    {
        return GetHeaderValue_Impl (
            m_nIndex[INETMSG_RFC822_REFERENCES],
            INetMIME::HEADER_FIELD_ADDRESS);
    }

    OUString GetReplyTo() const
    {
        return GetHeaderValue_Impl (
            m_nIndex[INETMSG_RFC822_REPLY_TO],
            INetMIME::HEADER_FIELD_ADDRESS);
    }

    OUString GetReturnPath() const
    {
        return GetHeaderValue_Impl (
            m_nIndex[INETMSG_RFC822_RETURN_PATH],
            INetMIME::HEADER_FIELD_ADDRESS);
    }

    OUString GetReturnReceiptTo() const
    {
        return GetHeaderValue_Impl (
            m_nIndex[INETMSG_RFC822_RETURN_RECEIPT_TO],
            INetMIME::HEADER_FIELD_ADDRESS);
    }

    OUString GetSender() const
    {
        return GetHeaderValue_Impl (
            m_nIndex[INETMSG_RFC822_SENDER],
            INetMIME::HEADER_FIELD_ADDRESS);
    }

    OUString GetSubject() const
    {
        return GetHeaderValue_Impl (
            m_nIndex[INETMSG_RFC822_SUBJECT],
            INetMIME::HEADER_FIELD_TEXT);
    }

    OUString GetTo() const
    {
        return GetHeaderValue_Impl (
            m_nIndex[INETMSG_RFC822_TO],
            INetMIME::HEADER_FIELD_TEXT);
    }

    // Stream operators.

    friend SvStream& operator<< (
        SvStream& rStrm, const INetRFC822Message& rMsg)
    {
        return rMsg.operator<< (rStrm);
    }

    friend SvStream& operator>> (
        SvStream& rStrm, INetRFC822Message& rMsg)
    {
        return rMsg.operator>> (rStrm);
    }
};

#define INETMSG_MIME_VERSION                    0
#define INETMSG_MIME_CONTENT_DESCRIPTION        1
#define INETMSG_MIME_CONTENT_DISPOSITION        2
#define INETMSG_MIME_CONTENT_ID                 3
#define INETMSG_MIME_CONTENT_TYPE               4
#define INETMSG_MIME_CONTENT_TRANSFER_ENCODING  5
#define INETMSG_MIME_NUMHDR                     6

enum INetMessageContainerType
{
    INETMSG_MESSAGE_RFC822,
    INETMSG_MULTIPART_MIXED,
    INETMSG_MULTIPART_ALTERNATIVE,
    INETMSG_MULTIPART_DIGEST,
    INETMSG_MULTIPART_PARALLEL,
    INETMSG_MULTIPART_RELATED,
    INETMSG_MULTIPART_FORM_DATA
};

class INetMIMEMessage;
typedef ::std::vector< INetMIMEMessage* > INetMIMEMessgeList_impl;

class TOOLS_DLLPUBLIC INetMIMEMessage : public INetRFC822Message
{
    sal_uIntPtr             m_nIndex[INETMSG_MIME_NUMHDR];
    INetMIMEMessage*        pParent;
    INetMIMEMessgeList_impl aChildren;
    OString                 m_aBoundary;
    bool                    bHeaderParsed;

    friend class INetMIMEMessageStream;

    const OString& GetMultipartBoundary() const { return m_aBoundary; }
    void SetMultipartBoundary (const OString& rBnd) { m_aBoundary = rBnd; }

    void CleanupImp();
    void CopyImp    (const INetMIMEMessage& rMsg);
    void SetHeaderParsed() { bHeaderParsed = sal_True; }

protected:
    virtual SvStream& operator<< (SvStream& rStrm) const;
    virtual SvStream& operator>> (SvStream& rStrm);

public:
    INetMIMEMessage();
    INetMIMEMessage (const INetMIMEMessage& rMsg);
    virtual ~INetMIMEMessage();

    INetMIMEMessage& operator= (const INetMIMEMessage& rMsg);

    bool HeaderParsed() const { return bHeaderParsed; }

    virtual INetMIMEMessage* CreateMessage (
        const INetMIMEMessage& rMsg) const;

    using INetRFC822Message::SetHeaderField;
    virtual sal_uIntPtr SetHeaderField (
        const INetMessageHeader &rHeader,
        sal_uIntPtr nIndex = ((sal_uIntPtr)-1)
    );

    // Header fields.

    void     SetMIMEVersion (const OUString& rVersion);
    OUString GetMIMEVersion() const
    {
        return GetHeaderValue (m_nIndex[INETMSG_MIME_VERSION]);
    }

    OUString GetContentDescription() const
    {
        return GetHeaderValue (m_nIndex[INETMSG_MIME_CONTENT_DESCRIPTION]);
    }

    void     SetContentDisposition (const OUString& rDisposition);
    OUString GetContentDisposition() const
    {
        return GetHeaderValue (m_nIndex[INETMSG_MIME_CONTENT_DISPOSITION]);
    }

    OUString GetContentID() const
    {
        return GetHeaderValue (m_nIndex[INETMSG_MIME_CONTENT_ID]);
    }

    void     SetContentType (const OUString& rType);
    OUString GetContentType() const
    {
        return GetHeaderValue (m_nIndex[INETMSG_MIME_CONTENT_TYPE]);
    }

    void     SetContentTransferEncoding (const OUString& rEncoding);
    OUString GetContentTransferEncoding() const
    {
        return GetHeaderValue (m_nIndex[INETMSG_MIME_CONTENT_TRANSFER_ENCODING]);
    }

    OUString GetDefaultContentType ();

    // Message container methods.

    bool IsContainer() const
    {
        return (IsMessage() || IsMultipart());
    }
    bool IsMessage() const
    {
        OUString aType (GetContentType());
        return aType.matchIgnoreAsciiCase("message/");
    }
    bool IsMultipart() const
    {
        OUString aType (GetContentType());
        return aType.matchIgnoreAsciiCase("multipart/");
    }

    INetMIMEMessage* GetChild (sal_uIntPtr nIndex) const
    {
        return ( nIndex < aChildren.size() ) ? aChildren[ nIndex ] : NULL;
    }
    INetMIMEMessage* GetParent() const { return pParent; }

    bool EnableAttachChild (
        INetMessageContainerType eType = INETMSG_MULTIPART_MIXED);
    bool AttachChild (
        INetMIMEMessage& rChildMsg, bool bOwner = true );

    // Stream operators.

    friend SvStream& operator<< (
        SvStream& rStrm, const INetMIMEMessage& rMsg)
    {
        return rMsg.operator<< (rStrm);
    }

    friend SvStream& operator>> (
        SvStream& rStrm, INetMIMEMessage& rMsg)
    {
        return rMsg.operator>> (rStrm);
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
