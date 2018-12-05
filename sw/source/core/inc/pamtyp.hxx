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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_PAMTYP_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_PAMTYP_HXX

#include <unotools/textsearch.hxx>
#include <node.hxx>

class SwpHints;
struct SwPosition;
class SwPaM;
class SwTextAttr;
class SwFormat;
class SfxPoolItem;
class SwRootFrame;

namespace i18nutil {
    struct SearchOptions2;
}
namespace utl {
    class TextSearch;
}

// function prototypes for the move/find methods of SwPaM

void GoStartDoc( SwPosition*);
void GoEndDoc( SwPosition*);
void GoStartSection( SwPosition*);
void GoEndSection( SwPosition*);
const SwTextAttr* GetFrwrdTextHint( const SwpHints&, size_t&, sal_Int32 );
const SwTextAttr* GetBkwrdTextHint( const SwpHints&, size_t&, sal_Int32 );

bool GoNext(SwNode* pNd, SwIndex * pIdx, sal_uInt16 nMode );
bool GoPrevious(SwNode* pNd, SwIndex * pIdx, sal_uInt16 nMode );
SW_DLLPUBLIC SwContentNode* GoNextNds( SwNodeIndex * pIdx, bool );
SwContentNode* GoPreviousNds( SwNodeIndex * pIdx, bool );

// type definitions of functions
typedef bool (*GoNd)( SwNode*, SwIndex*, sal_uInt16 );
typedef SwContentNode* (*GoNds)( SwNodeIndex*, bool );
typedef void (*GoDoc)( SwPosition* );
typedef void (*GoSection)( SwPosition* );
typedef bool (SwPosition:: *CmpOp)( const SwPosition& ) const;
typedef const SwTextAttr* (*GetHint)( const SwpHints&, size_t&, sal_Int32 );
typedef bool (utl::TextSearch:: *SearchText)( const OUString&, sal_Int32*,
                    sal_Int32*, css::util::SearchResult* );
typedef void (*MvSection)( SwNodeIndex * );

struct SwMoveFnCollection
{
    GoNd      fnNd;
    GoNds     fnNds;
    GoDoc     fnDoc;
    GoSection fnSections;
    CmpOp     fnCmpOp;
    GetHint   fnGetHint;
    SearchText fnSearch;
    MvSection fnSection;
};

// function prototype for searching
SwContentNode* GetNode(SwPaM&, bool&, SwMoveFnCollection const &,
        bool bInReadOnly = false, SwRootFrame const* pLayout = nullptr);

namespace sw {

    std::unique_ptr<SwPaM> MakeRegion(SwMoveFnCollection const & fnMove,
            const SwPaM & rOrigRg);

    /// Search.
    bool FindTextImpl(SwPaM & rSearchPam,
                const i18nutil::SearchOptions2& rSearchOpt,
                bool bSearchInNotes,
                utl::TextSearch& rSText,
                SwMoveFnCollection const & fnMove,
                const SwPaM & rRegion, bool bInReadOnly,
                SwRootFrame const* pLayout);
    bool FindFormatImpl(SwPaM & rSearchPam,
                const SwFormat& rFormat,
                SwMoveFnCollection const & fnMove,
                const SwPaM & rRegion, bool bInReadOnly,
                SwRootFrame const* pLayout);
    bool FindAttrImpl(SwPaM & rSearchPam,
                const SfxPoolItem& rAttr,
                SwMoveFnCollection const & fnMove,
                const SwPaM & rPam, bool bInReadOnly,
                SwRootFrame const* pLayout);

} // namespace sw

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
