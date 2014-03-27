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
#ifndef INCLUDED_SW_INC_TXATBASE_HXX
#define INCLUDED_SW_INC_TXATBASE_HXX

#include <tools/solar.h>
#include <svl/poolitem.hxx>
#include <hintids.hxx>

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
    sal_Int32 m_nStart;
    bool m_bDontExpand          : 1;
    bool m_bLockExpandFlag      : 1;

    bool m_bDontMoveAttr        : 1;    // refmarks, toxmarks
    bool m_bCharFmtAttr         : 1;    // charfmt, inet
    bool m_bOverlapAllowedAttr  : 1;    // refmarks, toxmarks
    bool m_bPriorityAttr        : 1;    // attribute has priority (redlining)
    bool m_bDontExpandStart     : 1;    // don't expand start at paragraph start (ruby)
    bool m_bNesting             : 1;    // SwTxtAttrNesting
    bool m_bHasDummyChar        : 1;    // without end + meta
    bool m_bFormatIgnoreStart   : 1;    ///< text formatting should ignore start
    bool m_bFormatIgnoreEnd     : 1;    ///< text formatting should ignore end
    bool m_bHasContent          : 1;    // text attribute with content

protected:
    SwTxtAttr( SfxPoolItem& rAttr, sal_Int32 nStart );
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
                  sal_Int32* GetStart()        { return & m_nStart; }
            const sal_Int32* GetStart() const  { return & m_nStart; }

    /// end position
    virtual      sal_Int32* GetEnd(); // also used to change the end position
    inline const sal_Int32* End() const;
    /// end (if available), else start
    inline const sal_Int32* GetAnyEnd() const;

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
    bool IsFormatIgnoreStart() const        { return m_bFormatIgnoreStart; }
    bool IsFormatIgnoreEnd  () const        { return m_bFormatIgnoreEnd  ; }
    void SetFormatIgnoreStart(bool bFlag)   { m_bFormatIgnoreStart = bFlag; }
    void SetFormatIgnoreEnd  (bool bFlag)   { m_bFormatIgnoreEnd   = bFlag; }
    bool HasContent() const                 { return m_bHasContent; }

    inline const SfxPoolItem& GetAttr() const;
    inline       SfxPoolItem& GetAttr();
    inline sal_uInt16 Which() const { return GetAttr().Which(); }

    virtual bool operator==( const SwTxtAttr& ) const;

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
    sal_Int32 m_nEnd;

public:
    SwTxtAttrEnd( SfxPoolItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );

    virtual sal_Int32* GetEnd() SAL_OVERRIDE;
};

// --------------- Inline Implementations ------------------------

inline const sal_Int32* SwTxtAttr::End() const
{
    return const_cast<SwTxtAttr * >(this)->GetEnd();
}

inline const sal_Int32* SwTxtAttr::GetAnyEnd() const
{
    const sal_Int32* pEnd = End();
    return pEnd ? pEnd : GetStart();
}

inline const SfxPoolItem& SwTxtAttr::GetAttr() const
{
    assert( m_pAttr );
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

inline const SwFmtCharFmt& SwTxtAttr::GetCharFmt() const
{
    assert( m_pAttr && m_pAttr->Which() == RES_TXTATR_CHARFMT );
    return (const SwFmtCharFmt&)(*m_pAttr);
}

inline const SwFmtAutoFmt& SwTxtAttr::GetAutoFmt() const
{
    assert( m_pAttr && m_pAttr->Which() == RES_TXTATR_AUTOFMT );
    return (const SwFmtAutoFmt&)(*m_pAttr);
}

inline const SwFmtFld& SwTxtAttr::GetFmtFld() const
{
    assert( m_pAttr
            && ( m_pAttr->Which() == RES_TXTATR_FIELD
                 || m_pAttr->Which() == RES_TXTATR_ANNOTATION
                 || m_pAttr->Which() == RES_TXTATR_INPUTFIELD ));
    return (const SwFmtFld&)(*m_pAttr);
}

inline const SwFmtFtn& SwTxtAttr::GetFtn() const
{
    assert( m_pAttr && m_pAttr->Which() == RES_TXTATR_FTN );
    return (const SwFmtFtn&)(*m_pAttr);
}

inline const SwFmtFlyCnt& SwTxtAttr::GetFlyCnt() const
{
    assert( m_pAttr && m_pAttr->Which() == RES_TXTATR_FLYCNT );
    return (const SwFmtFlyCnt&)(*m_pAttr);
}

inline const SwTOXMark& SwTxtAttr::GetTOXMark() const
{
    assert( m_pAttr && m_pAttr->Which() == RES_TXTATR_TOXMARK );
    return (const SwTOXMark&)(*m_pAttr);
}

inline const SwFmtRefMark& SwTxtAttr::GetRefMark() const
{
    assert( m_pAttr && m_pAttr->Which() == RES_TXTATR_REFMARK );
    return (const SwFmtRefMark&)(*m_pAttr);
}

inline const SwFmtINetFmt& SwTxtAttr::GetINetFmt() const
{
    assert( m_pAttr && m_pAttr->Which() == RES_TXTATR_INETFMT );
    return (const SwFmtINetFmt&)(*m_pAttr);
}

inline const SwFmtRuby& SwTxtAttr::GetRuby() const
{
    assert( m_pAttr && m_pAttr->Which() == RES_TXTATR_CJK_RUBY );
    return (const SwFmtRuby&)(*m_pAttr);
}

inline const SwFmtMeta& SwTxtAttr::GetMeta() const
{
    assert( m_pAttr && (m_pAttr->Which() == RES_TXTATR_META ||
        m_pAttr->Which() == RES_TXTATR_METAFIELD) );
    return (const SwFmtMeta&)(*m_pAttr);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
