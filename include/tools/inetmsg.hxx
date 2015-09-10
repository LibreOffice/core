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
#ifndef INCLUDED_TOOLS_INETMSG_HXX
#define INCLUDED_TOOLS_INETMSG_HXX

#include <tools/toolsdllapi.h>
#include <rtl/string.hxx>
#include <rtl/textenc.h>
#include <rtl/ustring.hxx>
#include <tools/inetmime.hxx>
#include <tools/stream.hxx>

#include <vector>
#include <map>

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

    friend SvStream& WriteINetMessageHeader(
        SvStream& rStrm, const INetMessageHeader& rHdr)
    {
        write_uInt16_lenPrefixed_uInt8s_FromOString(rStrm, rHdr.m_aName);
        write_uInt16_lenPrefixed_uInt8s_FromOString(rStrm, rHdr.m_aValue);
        return rStrm;
    }

    friend SvStream& ReadINetMessageHeader (
        SvStream& rStrm, INetMessageHeader& rHdr)
    {
        rHdr.m_aName = read_uInt16_lenPrefixed_uInt8s_ToOString(rStrm);
        rHdr.m_aValue = read_uInt16_lenPrefixed_uInt8s_ToOString(rStrm);
        return rStrm;
    }
};

/**
  RFC822 fields
*/
enum class InetMessageField
{
    BCC                =  0,
    CC                 =  1,
    COMMENTS           =  2,
    DATE               =  3,
    FROM               =  4,
    IN_REPLY_TO        =  5,
    KEYWORDS           =  6,
    MESSAGE_ID         =  7,
    REFERENCES         =  8,
    REPLY_TO           =  9,
    RETURN_PATH        = 10,
    SENDER             = 11,
    SUBJECT            = 12,
    TO                 = 13,
    X_MAILER           = 14,
    RETURN_RECEIPT_TO  = 15,
    NUMHDR             = 16,
};

enum class InetMessageMime
{
    VERSION                    = 0,
    CONTENT_DESCRIPTION        = 1,
    CONTENT_DISPOSITION        = 2,
    CONTENT_ID                 = 3,
    CONTENT_TYPE               = 4,
    CONTENT_TRANSFER_ENCODING  = 5,
    NUMHDR                     = 6,
};

class TOOLS_DLLPUBLIC INetMIMEMessage
{
    ::std::vector< INetMessageHeader* >
                    m_aHeaderList;

    sal_uIntPtr     m_nDocSize;
    OUString        m_aDocName;
    SvLockBytesRef  m_xDocLB;

    void ListCleanup_Impl();
    void ListCopy (const INetMIMEMessage& rMsg);

    ::std::map<InetMessageField, sal_uIntPtr> m_nRFC822Index;

    ::std::map<InetMessageMime, sal_uIntPtr>  m_nMIMEIndex;
    INetMIMEMessage*                          pParent;
    ::std::vector< INetMIMEMessage* >         aChildren;
    OString                 m_aBoundary;
    bool                    bHeaderParsed;

    friend class INetMIMEMessageStream;

    const OString& GetMultipartBoundary() const { return m_aBoundary; }
    void SetMultipartBoundary (const OString& rBnd) { m_aBoundary = rBnd; }

    void CleanupImp();
    void CopyImp    (const INetMIMEMessage& rMsg);
    void SetHeaderParsed() { bHeaderParsed = true; }

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

    sal_uIntPtr SetRFC822HeaderField (
        const INetMessageHeader &rHeader, sal_uIntPtr nNewIndex);

public:
    INetMIMEMessage();
    INetMIMEMessage (const INetMIMEMessage& rMsg);
    ~INetMIMEMessage();

    INetMIMEMessage& operator= (const INetMIMEMessage& rMsg);

    sal_uIntPtr GetHeaderCount() const { return m_aHeaderList.size(); }

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

    sal_uIntPtr SetHeaderField (
        const INetMessageHeader &rField,
        sal_uIntPtr nIndex = ((sal_uIntPtr)-1)
    );

    sal_uIntPtr GetDocumentSize() const { return m_nDocSize; }
    void  SetDocumentSize (sal_uIntPtr nSize) { m_nDocSize = nSize; }

    SvLockBytes* GetDocumentLB() const { return m_xDocLB; }
    void         SetDocumentLB (SvLockBytes *pDocLB) { m_xDocLB = pDocLB; }

    static bool ParseDateField (
        const OUString& rDateField, DateTime& rDateTime);

    static INetMIMEMessage* CreateMessage (
        const INetMIMEMessage& rMsg);

    void     SetMIMEVersion (const OUString& rVersion);
    void     SetContentDisposition (const OUString& rDisposition);
    void     SetContentType (const OUString& rType);
    OUString GetContentType() const
    {
        return GetHeaderValue (m_nMIMEIndex.at(InetMessageMime::CONTENT_TYPE));
    }

    void     SetContentTransferEncoding (const OUString& rEncoding);
    OUString GetContentTransferEncoding() const
    {
        return GetHeaderValue (m_nMIMEIndex.at(InetMessageMime::CONTENT_TRANSFER_ENCODING));
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

    bool EnableAttachMultipartFormDataChild();
    bool AttachChild (
        INetMIMEMessage& rChildMsg, bool bOwner = true );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
