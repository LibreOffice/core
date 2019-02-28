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

#include <svl/poolitem.hxx>
#include "hintids.hxx"
#include "fmtautofmt.hxx"
#include "fmtinfmt.hxx"
#include "fmtrfmrk.hxx"
#include "fmtruby.hxx"
#include "fmtfld.hxx"
#include "fmtflcnt.hxx"
#include "fmtftn.hxx"
#include "fchrfmt.hxx"
#include "tox.hxx"

class SfxItemPool;

class SAL_DLLPUBLIC_RTTI SwTextAttr
{
private:
    SfxPoolItem * const m_pAttr;
    sal_Int32 m_nStart;
    bool m_bDontExpand          : 1;
    bool m_bLockExpandFlag      : 1;

    bool m_bDontMoveAttr        : 1;    // refmarks, toxmarks
    bool m_bCharFormatAttr         : 1;    // charfmt, inet
    bool m_bOverlapAllowedAttr  : 1;    // refmarks, toxmarks
    bool m_bPriorityAttr        : 1;    // attribute has priority (redlining)
    bool m_bDontExpandStart     : 1;    // don't expand start at paragraph start (ruby)
    bool m_bNesting             : 1;    // SwTextAttrNesting
    bool m_bHasDummyChar        : 1;    // without end + meta
    bool m_bFormatIgnoreStart   : 1;    ///< text formatting should ignore start
    bool m_bFormatIgnoreEnd     : 1;    ///< text formatting should ignore end
    bool m_bHasContent          : 1;    // text attribute with content

    SwTextAttr(SwTextAttr const&) = delete;
    SwTextAttr& operator=(SwTextAttr const&) = delete;

protected:
    SwTextAttr( SfxPoolItem& rAttr, sal_Int32 nStart );
    virtual ~SwTextAttr() COVERITY_NOEXCEPT_FALSE;

    void SetLockExpandFlag( bool bFlag )    { m_bLockExpandFlag = bFlag; }
    void SetDontMoveAttr( bool bFlag )      { m_bDontMoveAttr = bFlag; }
    void SetCharFormatAttr( bool bFlag )       { m_bCharFormatAttr = bFlag; }
    void SetOverlapAllowedAttr( bool bFlag ){ m_bOverlapAllowedAttr = bFlag; }
    void SetDontExpandStartAttr(bool bFlag) { m_bDontExpandStart = bFlag; }
    void SetNesting(const bool bFlag)       { m_bNesting = bFlag; }
    void SetHasDummyChar(const bool bFlag)  { m_bHasDummyChar = bFlag; }
    void SetHasContent( const bool bFlag )  { m_bHasContent = bFlag; }

public:

    /// destroy instance
    static void Destroy( SwTextAttr * pToDestroy, SfxItemPool& rPool );

    /// start position
                  sal_Int32& GetStart()        { return m_nStart; }
            const sal_Int32& GetStart() const  { return m_nStart; }

    /// end position
    virtual      sal_Int32* GetEnd(); // also used to change the end position
    inline const sal_Int32* End() const;
    /// end (if available), else start
    inline const sal_Int32* GetAnyEnd() const;

    inline void SetDontExpand( bool bDontExpand );
    bool DontExpand() const                 { return m_bDontExpand; }
    bool IsLockExpandFlag() const           { return m_bLockExpandFlag; }
    bool IsDontMoveAttr() const             { return m_bDontMoveAttr; }
    bool IsCharFormatAttr() const              { return m_bCharFormatAttr; }
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
    sal_uInt16 Which() const { return GetAttr().Which(); }

    bool operator==( const SwTextAttr& ) const;

    inline const SwFormatCharFormat           &GetCharFormat() const;
    inline const SwFormatAutoFormat           &GetAutoFormat() const;
    inline const SwFormatField               &GetFormatField() const;
    inline const SwFormatFootnote               &GetFootnote() const;
    inline const SwFormatFlyCnt            &GetFlyCnt() const;
    inline const SwTOXMark              &GetTOXMark() const;
    inline const SwFormatRefMark           &GetRefMark() const;
    inline const SwFormatINetFormat           &GetINetFormat() const;
    inline const SwFormatRuby              &GetRuby() const;

    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

class SAL_DLLPUBLIC_RTTI SwTextAttrEnd : public virtual SwTextAttr
{
protected:
    sal_Int32 m_nEnd;

public:
    SwTextAttrEnd( SfxPoolItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );

    virtual sal_Int32* GetEnd() override;
};

// attribute that must not overlap others
class SAL_DLLPUBLIC_RTTI SwTextAttrNesting : public SwTextAttrEnd
{
protected:
    SwTextAttrNesting( SfxPoolItem & i_rAttr,
        const sal_Int32 i_nStart, const sal_Int32 i_nEnd );
    virtual ~SwTextAttrNesting() override;
};

inline const sal_Int32* SwTextAttr::End() const
{
    return const_cast<SwTextAttr * >(this)->GetEnd();
}

inline const sal_Int32* SwTextAttr::GetAnyEnd() const
{
    const sal_Int32* pEnd = End();
    return pEnd ? pEnd : &GetStart();
}

inline const SfxPoolItem& SwTextAttr::GetAttr() const
{
    assert( m_pAttr );
    return *m_pAttr;
}

inline SfxPoolItem& SwTextAttr::GetAttr()
{
    return const_cast<SfxPoolItem&>(
            const_cast<const SwTextAttr*>(this)->GetAttr());
}

inline void SwTextAttr::SetDontExpand( bool bDontExpand )
{
    if ( !m_bLockExpandFlag )
    {
        m_bDontExpand = bDontExpand;
    }
}

inline const SwFormatCharFormat& SwTextAttr::GetCharFormat() const
{
    assert( m_pAttr && m_pAttr->Which() == RES_TXTATR_CHARFMT );
    return static_cast<const SwFormatCharFormat&>(*m_pAttr);
}

inline const SwFormatAutoFormat& SwTextAttr::GetAutoFormat() const
{
    assert( m_pAttr && m_pAttr->Which() == RES_TXTATR_AUTOFMT );
    return static_cast<const SwFormatAutoFormat&>(*m_pAttr);
}

inline const SwFormatField& SwTextAttr::GetFormatField() const
{
    assert( m_pAttr
            && ( m_pAttr->Which() == RES_TXTATR_FIELD
                 || m_pAttr->Which() == RES_TXTATR_ANNOTATION
                 || m_pAttr->Which() == RES_TXTATR_INPUTFIELD ));
    return static_cast<const SwFormatField&>(*m_pAttr);
}

inline const SwFormatFootnote& SwTextAttr::GetFootnote() const
{
    assert( m_pAttr && m_pAttr->Which() == RES_TXTATR_FTN );
    return static_cast<const SwFormatFootnote&>(*m_pAttr);
}

inline const SwFormatFlyCnt& SwTextAttr::GetFlyCnt() const
{
    assert( m_pAttr && m_pAttr->Which() == RES_TXTATR_FLYCNT );
    return static_cast<const SwFormatFlyCnt&>(*m_pAttr);
}

inline const SwTOXMark& SwTextAttr::GetTOXMark() const
{
    assert( m_pAttr && m_pAttr->Which() == RES_TXTATR_TOXMARK );
    return static_cast<const SwTOXMark&>(*m_pAttr);
}

inline const SwFormatRefMark& SwTextAttr::GetRefMark() const
{
    assert( m_pAttr && m_pAttr->Which() == RES_TXTATR_REFMARK );
    return static_cast<const SwFormatRefMark&>(*m_pAttr);
}

inline const SwFormatINetFormat& SwTextAttr::GetINetFormat() const
{
    assert( m_pAttr && m_pAttr->Which() == RES_TXTATR_INETFMT );
    return static_cast<const SwFormatINetFormat&>(*m_pAttr);
}

inline const SwFormatRuby& SwTextAttr::GetRuby() const
{
    assert( m_pAttr && m_pAttr->Which() == RES_TXTATR_CJK_RUBY );
    return static_cast<const SwFormatRuby&>(*m_pAttr);
}

// these should be static_casts but with virtual inheritance it's not possible
template<typename T, typename S> inline T static_txtattr_cast(S * s)
{
    return dynamic_cast<T>(s);
}
template<typename T, typename S> inline T static_txtattr_cast(S & s)
{
    return dynamic_cast<T>(s);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
