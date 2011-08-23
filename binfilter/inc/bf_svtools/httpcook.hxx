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

#ifndef SVTOOLS_HTTPCOOK_HXX
#define SVTOOLS_HTTPCOOK_HXX

#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _SFXPOOLITEM_HXX
#include <bf_svtools/poolitem.hxx>
#endif

namespace binfilter
{

/*=======================================================================
 *
 *=====================================================================*/
#define CNTHTTP_COOKIE_FLAG_SECURE            0x01

#define CNTHTTP_COOKIE_POLICY_INTERACTIVE     0x00
#define CNTHTTP_COOKIE_POLICY_ACCEPTED        0x01
#define CNTHTTP_COOKIE_POLICY_BANNED          0x02

#define CNTHTTP_COOKIE_DOMAIN_POLICY          0x10

#define CNTHTTP_COOKIE_DOMAIN_ACCEPTED \
    (CNTHTTP_COOKIE_DOMAIN_POLICY | CNTHTTP_COOKIE_POLICY_ACCEPTED)
#define CNTHTTP_COOKIE_DOMAIN_BANNED \
    (CNTHTTP_COOKIE_DOMAIN_POLICY | CNTHTTP_COOKIE_POLICY_BANNED)

/*=======================================================================
 *
 * CntHTTPCookie.
 *
 *=====================================================================*/
struct CntHTTPCookie
{
    String   m_aName;
    String   m_aValue;
    String   m_aDomain;
    String   m_aPath;
    DateTime m_aExpires;
    USHORT   m_nFlags;
    USHORT   m_nPolicy;

    CntHTTPCookie (void)
        : m_aExpires (Date(0), Time(0)),
          m_nFlags   (0),
          m_nPolicy  (CNTHTTP_COOKIE_POLICY_INTERACTIVE)
    {}

    BOOL replaces (const CntHTTPCookie& rOther) const
    {
        return ((m_aDomain == rOther.m_aDomain) &&
                (m_aPath   == rOther.m_aPath  ) &&
                (m_aName   == rOther.m_aName  )    );
    }

    BOOL operator== (const CntHTTPCookie& rOther) const
    {
        return ((m_aName    == rOther.m_aName   ) &&
                (m_aValue   == rOther.m_aValue  ) &&
                (m_aDomain  == rOther.m_aDomain ) &&
                (m_aPath    == rOther.m_aPath   ) &&
                (m_aExpires == rOther.m_aExpires) &&
                (m_nFlags   == rOther.m_nFlags  ) &&
                (m_nPolicy  == rOther.m_nPolicy )    );
    }

    void write (SvStream& rStrm) const
    {
        SfxPoolItem::writeUnicodeString(rStrm, m_aName);
        SfxPoolItem::writeUnicodeString(rStrm, m_aValue);
        SfxPoolItem::writeUnicodeString(rStrm, m_aDomain);
        SfxPoolItem::writeUnicodeString(rStrm, m_aPath);

        rStrm << m_aExpires.GetDate();
        rStrm << m_aExpires.GetTime();

        rStrm << m_nFlags;
        rStrm << m_nPolicy;
    }

    void read (SvStream& rStrm, bool bUnicode)
    {
        SfxPoolItem::readUnicodeString(rStrm, m_aName, bUnicode);
        SfxPoolItem::readUnicodeString(rStrm, m_aValue, bUnicode);
        SfxPoolItem::readUnicodeString(rStrm, m_aDomain, bUnicode);
        SfxPoolItem::readUnicodeString(rStrm, m_aPath, bUnicode);

        sal_uInt32 nValue = 0;
        rStrm >> nValue;
        m_aExpires.SetDate (nValue);
        rStrm >> nValue;
        m_aExpires.SetTime (nValue);

        rStrm >> m_nFlags;
        rStrm >> m_nPolicy;
    }
};

/*=======================================================================
 *
 * CntHTTPCookieRequest.
 *
 *=====================================================================*/
enum CntHTTPCookieRequestType
{
    CNTHTTP_COOKIE_REQUEST_RECV = 0,
    CNTHTTP_COOKIE_REQUEST_SEND
};

struct CntHTTPCookieRequest
{
    const String& 			 m_rURL;
    List&         			 m_rCookieList;
    CntHTTPCookieRequestType m_eType;
    USHORT					 m_nRet;

    CntHTTPCookieRequest (
        const String& rURL,
        List& rCookieList,
        CntHTTPCookieRequestType eType)
        : m_rURL (rURL),
          m_rCookieList (rCookieList),
          m_eType(eType),
          m_nRet (CNTHTTP_COOKIE_POLICY_BANNED) {}
};

}

#endif // SVTOOLS_HTTPCOOK_HXX

