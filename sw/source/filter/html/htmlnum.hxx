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
#ifndef INCLUDED_SW_SOURCE_FILTER_HTML_HTMLNUM_HXX
#define INCLUDED_SW_SOURCE_FILTER_HTML_HTMLNUM_HXX

#include <swtypes.hxx>
#include <string.h>

#define HTML_NUMBER_BULLET_MARGINLEFT  (o3tl::convertTwip(tools::Long(125), o3tl::Length::mm10))
#define HTML_NUMBER_BULLET_INDENT      (-o3tl::convertTwip(tools::Long(5), o3tl::Length::mm))

class SwTextNode;
class SwNumRule;

// TODO: Unicode: Are these characters the correct ones?
#define HTML_BULLETCHAR_DISC    (0xe008)
#define HTML_BULLETCHAR_CIRCLE  (0xe009)
#define HTML_BULLETCHAR_SQUARE  (0xe00b)

class SwHTMLNumRuleInfo
{
    sal_uInt16      m_aNumStarts[MAXLEVEL];
    SwNumRule   *   m_pNumRule;       // current numbering
    sal_uInt16      m_nDeep;          // current numbering depth (1, 2, 3, ...)
    bool        m_bRestart : 1;   // Export: restart numbering
    bool        m_bNumbered : 1;  // Export: paragraph is numbered

public:

    inline void Set( const SwHTMLNumRuleInfo& rInf );
    void Set( const SwTextNode& rTextNd );

    SwHTMLNumRuleInfo() :
        m_pNumRule( nullptr ), m_nDeep( 0 ),
        m_bRestart( false ), m_bNumbered( false )
    {
        memset( &m_aNumStarts, 0xff, sizeof( m_aNumStarts ) );
    }

    SwHTMLNumRuleInfo( const SwHTMLNumRuleInfo& rInf ) :
        m_pNumRule( rInf.m_pNumRule ), m_nDeep( rInf.m_nDeep ),
        m_bRestart( rInf.m_bRestart ), m_bNumbered( rInf.m_bNumbered )
    {
        memcpy( &m_aNumStarts, &rInf.m_aNumStarts, sizeof( m_aNumStarts ) );
    }

    explicit SwHTMLNumRuleInfo( const SwTextNode& rTextNd ) { Set( rTextNd ); }
    inline SwHTMLNumRuleInfo& operator=( const SwHTMLNumRuleInfo& rInf );

    inline void Clear();

    void SetNumRule( const SwNumRule *pRule ) { m_pNumRule = const_cast<SwNumRule *>(pRule); }
    SwNumRule *GetNumRule() { return m_pNumRule; }
    const SwNumRule *GetNumRule() const { return m_pNumRule; }

    void SetDepth( sal_uInt16 nDepth ) { m_nDeep = nDepth; }
    sal_uInt16 GetDepth() const { return m_nDeep; }
    void IncDepth() { ++m_nDeep; }
    void DecDepth() { if (m_nDeep!=0) --m_nDeep; }
    inline sal_uInt8 GetLevel() const;

    bool IsRestart() const { return m_bRestart; }

    bool IsNumbered() const { return m_bNumbered; }

    inline void SetNodeStartValue( sal_uInt8 nLvl, sal_uInt16 nVal=USHRT_MAX );
    sal_uInt16 GetNodeStartValue( sal_uInt8 nLvl ) const { return m_aNumStarts[nLvl]; }
};

inline SwHTMLNumRuleInfo& SwHTMLNumRuleInfo::operator=(
    const SwHTMLNumRuleInfo& rInf )
{
    Set( rInf );
    return *this;
}

inline void SwHTMLNumRuleInfo::Set( const SwHTMLNumRuleInfo& rInf )
{
    m_pNumRule = rInf.m_pNumRule;
    m_nDeep = rInf.m_nDeep;
    m_bRestart = rInf.m_bRestart;
    m_bNumbered = rInf.m_bNumbered;
    memcpy( &m_aNumStarts, &rInf.m_aNumStarts, sizeof( m_aNumStarts ) );
}

inline void SwHTMLNumRuleInfo::Clear()
{
    m_pNumRule = nullptr;
    m_nDeep = 0;
    m_bRestart = m_bNumbered = false;
    memset( &m_aNumStarts, 0xff, sizeof( m_aNumStarts ) );
}

inline sal_uInt8 SwHTMLNumRuleInfo::GetLevel() const
{
    return
        static_cast<sal_uInt8>( m_pNumRule!=nullptr && m_nDeep != 0
            ? ( m_nDeep<=MAXLEVEL ? m_nDeep-1 : MAXLEVEL - 1 )
            : 0 );
}

inline void SwHTMLNumRuleInfo::SetNodeStartValue( sal_uInt8 nLvl, sal_uInt16 nVal )
{
    m_aNumStarts[nLvl] = nVal;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
