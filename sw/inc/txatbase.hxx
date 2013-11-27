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


#ifndef _TXATBASE_HXX
#define _TXATBASE_HXX

#include <tools/solar.h>
#include <svl/poolitem.hxx>
#include <hintids.hxx>
#include <errhdl.hxx>

#include <boost/utility.hpp>


class SfxItemPool;
class SvXMLAttrContainerItem;
class SwFmtRuby;
class SwFmtCharFmt;
class SwFmtAutoFmt;
class SwFmtINetFmt;
class SwFmtFld;
class SwFmtFtn;
class SwFmtFlyCnt;
class SwTOXMark;
class SwFmtRefMark;
class SwFmtMeta;


class SwTxtAttr : private boost::noncopyable
{
private:
    SfxPoolItem * const m_pAttr;
    xub_StrLen m_nStart;
    bool m_bDontExpand          : 1;
    bool m_bLockExpandFlag      : 1;

    bool m_bDontMoveAttr        : 1;    // refmarks, toxmarks
    bool m_bCharFmtAttr         : 1;    // charfmt, inet
    bool m_bOverlapAllowedAttr  : 1;    // refmarks, toxmarks
    bool m_bPriorityAttr        : 1;    // attribute has priority (redlining)
    bool m_bDontExpandStart     : 1;    // don't expand start at paragraph start (ruby)
    bool m_bNesting             : 1;    // SwTxtAttrNesting
    bool m_bHasDummyChar        : 1;    // without end + meta
    bool m_bHasContent          : 1;    // text attribute with content

protected:
    SwTxtAttr( SfxPoolItem& rAttr, xub_StrLen nStart );
    virtual ~SwTxtAttr();

    void SetLockExpandFlag( bool bFlag )    { m_bLockExpandFlag = bFlag; }
    void SetDontMoveAttr( bool bFlag )      { m_bDontMoveAttr = bFlag; }
    void SetCharFmtAttr( bool bFlag )       { m_bCharFmtAttr = bFlag; }
    void SetOverlapAllowedAttr( bool bFlag ){ m_bOverlapAllowedAttr = bFlag; }
    void SetDontExpandStartAttr(bool bFlag) { m_bDontExpandStart = bFlag; }
    void SetNesting(const bool bFlag)       { m_bNesting = bFlag; }
    void SetHasDummyChar(const bool bFlag)  { m_bHasDummyChar = bFlag; }
    void SetHasContent( const bool bFlag )  { m_bHasContent = bFlag; }

public:

    /// destroy instance
    static void Destroy( SwTxtAttr * pToDestroy, SfxItemPool& rPool );

    /// start position
                  xub_StrLen* GetStart()        { return & m_nStart; }
            const xub_StrLen* GetStart() const  { return & m_nStart; }

    /// end position
            virtual xub_StrLen* GetEnd(); // also used to change the end position
            inline const xub_StrLen* End() const;
            /// end (if available), else start
            inline const xub_StrLen* GetAnyEnd() const;

    inline void SetDontExpand( bool bDontExpand );
    bool DontExpand() const                 { return m_bDontExpand; }
    bool IsLockExpandFlag() const           { return m_bLockExpandFlag; }
    bool IsDontMoveAttr() const             { return m_bDontMoveAttr; }
    bool IsCharFmtAttr() const              { return m_bCharFmtAttr; }
    bool IsOverlapAllowedAttr() const       { return m_bOverlapAllowedAttr; }
    bool IsPriorityAttr() const             { return m_bPriorityAttr; }
    void SetPriorityAttr( bool bFlag )      { m_bPriorityAttr = bFlag; }
    bool IsDontExpandStartAttr() const      { return m_bDontExpandStart; }
    bool IsNesting() const                  { return m_bNesting; }
    bool HasDummyChar() const               { return m_bHasDummyChar; }
    bool HasContent() const                 { return m_bHasContent; }

    inline const SfxPoolItem& GetAttr() const;
    inline       SfxPoolItem& GetAttr();
    inline sal_uInt16 Which() const { return GetAttr().Which(); }

    virtual int         operator==( const SwTxtAttr& ) const;

    inline const SwFmtCharFmt           &GetCharFmt() const;
    inline const SwFmtAutoFmt           &GetAutoFmt() const;
    inline const SwFmtFld               &GetFmtFld() const;
    inline const SwFmtFtn               &GetFtn() const;
    inline const SwFmtFlyCnt            &GetFlyCnt() const;
    inline const SwTOXMark              &GetTOXMark() const;
    inline const SwFmtRefMark           &GetRefMark() const;
    inline const SwFmtINetFmt           &GetINetFmt() const;
    inline const SwFmtRuby              &GetRuby() const;
    inline const SwFmtMeta              &GetMeta() const;

};

class SwTxtAttrEnd : public SwTxtAttr
{
protected:
    xub_StrLen m_nEnd;

public:
    SwTxtAttrEnd( SfxPoolItem& rAttr, sal_uInt16 nStart, sal_uInt16 nEnd );

    virtual xub_StrLen* GetEnd();
};


// --------------- Inline Implementierungen ------------------------

inline const xub_StrLen* SwTxtAttr::End() const
{
    return const_cast<SwTxtAttr * >(this)->GetEnd();
}

inline const xub_StrLen* SwTxtAttr::GetAnyEnd() const
{
    const xub_StrLen* pEnd = End();
    return pEnd ? pEnd : GetStart();
}

inline const SfxPoolItem& SwTxtAttr::GetAttr() const
{
    ASSERT( m_pAttr, "SwTxtAttr: where is my attribute?" );
    return *m_pAttr;
}

inline SfxPoolItem& SwTxtAttr::GetAttr()
{
    return const_cast<SfxPoolItem&>(
            const_cast<const SwTxtAttr*>(this)->GetAttr());
}

inline void SwTxtAttr::SetDontExpand( bool bDontExpand )
{
    if ( !m_bLockExpandFlag )
    {
        m_bDontExpand = bDontExpand;
    }
}

//------------------------------------------------------------------------

inline const SwFmtCharFmt& SwTxtAttr::GetCharFmt() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_TXTATR_CHARFMT,
        "Wrong attribute" );
    return (const SwFmtCharFmt&)(*m_pAttr);
}

inline const SwFmtAutoFmt& SwTxtAttr::GetAutoFmt() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_TXTATR_AUTOFMT,
        "Wrong attribute" );
    return (const SwFmtAutoFmt&)(*m_pAttr);
}

inline const SwFmtFld& SwTxtAttr::GetFmtFld() const
{
    ASSERT( m_pAttr
            && ( m_pAttr->Which() == RES_TXTATR_FIELD
                 || m_pAttr->Which() == RES_TXTATR_INPUTFIELD ),
            "Wrong attribute" );
    return (const SwFmtFld&)(*m_pAttr);
}

inline const SwFmtFtn& SwTxtAttr::GetFtn() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_TXTATR_FTN, "Wrong attribute" );
    return (const SwFmtFtn&)(*m_pAttr);
}

inline const SwFmtFlyCnt& SwTxtAttr::GetFlyCnt() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_TXTATR_FLYCNT,
        "Wrong attribute" );
    return (const SwFmtFlyCnt&)(*m_pAttr);
}

inline const SwTOXMark& SwTxtAttr::GetTOXMark() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_TXTATR_TOXMARK,
        "Wrong attribute" );
    return (const SwTOXMark&)(*m_pAttr);
}

inline const SwFmtRefMark& SwTxtAttr::GetRefMark() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_TXTATR_REFMARK,
        "Wrong attribute" );
    return (const SwFmtRefMark&)(*m_pAttr);
}

inline const SwFmtINetFmt& SwTxtAttr::GetINetFmt() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_TXTATR_INETFMT,
        "Wrong attribute" );
    return (const SwFmtINetFmt&)(*m_pAttr);
}

inline const SwFmtRuby& SwTxtAttr::GetRuby() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_TXTATR_CJK_RUBY,
        "Wrong attribute" );
    return (const SwFmtRuby&)(*m_pAttr);
}

inline const SwFmtMeta& SwTxtAttr::GetMeta() const
{
    ASSERT( m_pAttr && (m_pAttr->Which() == RES_TXTATR_META ||
                        m_pAttr->Which() == RES_TXTATR_METAFIELD),
        "Wrong attribute" );
    return (const SwFmtMeta&)(*m_pAttr);
}

#endif
