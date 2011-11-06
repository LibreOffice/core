/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SVTOOLS_HTTPCOOK_HXX
#define SVTOOLS_HTTPCOOK_HXX

#include <tools/datetime.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <svl/poolitem.hxx>

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
    sal_uInt16   m_nFlags;
    sal_uInt16   m_nPolicy;

    CntHTTPCookie (void)
        : m_aExpires (Date(0), Time(0)),
          m_nFlags   (0),
          m_nPolicy  (CNTHTTP_COOKIE_POLICY_INTERACTIVE)
    {}

    sal_Bool replaces (const CntHTTPCookie& rOther) const
    {
        return ((m_aDomain == rOther.m_aDomain) &&
                (m_aPath   == rOther.m_aPath  ) &&
                (m_aName   == rOther.m_aName  )    );
    }

    sal_Bool operator== (const CntHTTPCookie& rOther) const
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
    const String&            m_rURL;
    List&                    m_rCookieList;
    CntHTTPCookieRequestType m_eType;
    sal_uInt16                   m_nRet;

    CntHTTPCookieRequest (
        const String& rURL,
        List& rCookieList,
        CntHTTPCookieRequestType eType)
        : m_rURL (rURL),
          m_rCookieList (rCookieList),
          m_eType(eType),
          m_nRet (CNTHTTP_COOKIE_POLICY_BANNED) {}
};

#endif // SVTOOLS_HTTPCOOK_HXX

