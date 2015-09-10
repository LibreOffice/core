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

    INetMessageHeader& operator= (const INetMessageHeader& rHdr)
    {
        m_aName  = rHdr.m_aName;
        m_aValue = rHdr.m_aValue;
        return *this;
    }

    const OString& GetName() const { return m_aName; }
    const OString& GetValue() const { return m_aValue; }
};

enum class InetMessageMime
{
    VERSION                    = 0,
    CONTENT_DISPOSITION        = 1,
    CONTENT_TYPE               = 2,
    CONTENT_TRANSFER_ENCODING  = 3,
    NUMHDR                     = 4,
};

class TOOLS_DLLPUBLIC INetMIMEMessage
{
    ::std::vector< INetMessageHeader* >
                    m_aHeaderList;

    SvLockBytesRef  m_xDocLB;

    ::std::map<InetMessageMime, sal_uIntPtr>  m_nMIMEIndex;
    INetMIMEMessage*                          pParent;
    ::std::vector< INetMIMEMessage* >         aChildren;
    OString                 m_aBoundary;

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

    bool IsMessage() const
    {
        OUString aType (GetContentType());
        return aType.matchIgnoreAsciiCase("message/");
    }

    INetMIMEMessage (const INetMIMEMessage& rMsg) = delete;
    INetMIMEMessage& operator= (const INetMIMEMessage& rMsg) = delete;

public:
    INetMIMEMessage();
    ~INetMIMEMessage();

    sal_uIntPtr GetHeaderCount() const { return m_aHeaderList.size(); }

    INetMessageHeader GetHeaderField (sal_uIntPtr nIndex) const
    {
        if ( nIndex < m_aHeaderList.size() ) {
            return INetMessageHeader( *m_aHeaderList[ nIndex ] );
        } else {
            return INetMessageHeader();
        }
    }

    SvLockBytes* GetDocumentLB() const { return m_xDocLB; }
    void         SetDocumentLB (SvLockBytes *pDocLB) { m_xDocLB = pDocLB; }

    static bool ParseDateField (
        const OUString& rDateField, DateTime& rDateTime);

    void     SetMIMEVersion (const OUString& rVersion);
    void     SetContentDisposition (const OUString& rDisposition);
    void     SetContentType (const OUString& rType);
    OUString GetContentType() const
    {
        return GetHeaderValue_Impl(
            m_nMIMEIndex.at(InetMessageMime::CONTENT_TYPE),
            INetMIME::HEADER_FIELD_TEXT);
    }

    void     SetContentTransferEncoding (const OUString& rEncoding);

    OUString GetDefaultContentType ();

    // Message container methods.

    bool IsContainer() const
    {
        return (IsMessage() || IsMultipart());
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

    const OString& GetMultipartBoundary() const { return m_aBoundary; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
