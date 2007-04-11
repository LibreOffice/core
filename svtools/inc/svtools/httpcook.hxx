/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: httpcook.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:21:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
#include <svtools/poolitem.hxx>
#endif

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
    const String&            m_rURL;
    List&                    m_rCookieList;
    CntHTTPCookieRequestType m_eType;
    USHORT                   m_nRet;

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

