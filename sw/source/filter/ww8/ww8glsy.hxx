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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_WW8GLSY_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_WW8GLSY_HXX

#include <memory>
#include <sot/storage.hxx>
#include "ww8scan.hxx"

class SwTextBlocks;
class SwNodeIndex;

/*
 * GlossaryFib takes the document fib and finds the glossary fib which may
 * not exist. The glossary fib has the offsets into the autotext subdocument
 * which is at the end of template .dot's
 */
class WW8GlossaryFib : public WW8Fib
{
public:
    WW8GlossaryFib( SvStream& rStrm, sal_uInt8 nWantedVersion, const WW8Fib &rFib)
        : WW8Fib(rStrm, nWantedVersion, FindGlossaryFibOffset(rFib)) {}
    // fGlsy will indicate whether this has AutoText or not
    bool IsGlossaryFib() {
        return m_fGlsy;
    }
private:
    static sal_uInt32 FindGlossaryFibOffset(const WW8Fib &rFib);
};

/*
 * Imports glossaries from word, given the document it gets the usual word
 * doc information, then the glossary fib and uses the usual reader class to
 * wrap the autotext into a star doc. Afterwards taking each section entry and
 * making it a single star autotext entry.
 *
 * ToDo currently all autotext entries become resource hungry star autotext
 * formatted text, need to use a flag in the ww8reader class to determine if
 * an entry is formatted or not.
 */
class WW8Glossary
{
public:
    WW8Glossary( tools::SvRef<SotStorageStream> &refStrm, sal_uInt8 nVersion, SotStorage *pStg);
    bool Load( SwTextBlocks &rBlocks, bool bSaveRelFile );
    std::shared_ptr<WW8GlossaryFib>& GetFib()
    {
        return xGlossary;
    }
    sal_uInt16 GetNoStrings() const
    {
        return nStrings;
    }

private:
    std::shared_ptr<WW8GlossaryFib> xGlossary;
    tools::SvRef<SotStorageStream> xTableStream;
    tools::SvRef<SotStorageStream> &rStrm;
    tools::SvRef<SotStorage> xStg;
    sal_uInt16 nStrings;

    static bool MakeEntries(SwDoc *pD, SwTextBlocks &rBlocks, bool bSaveRelFile,
                            const std::vector<OUString>& rStrings,
                            const std::vector<ww::bytes>& rExtra);
    static bool HasBareGraphicEnd(SwDoc *pD,SwNodeIndex const &rIdx);

    WW8Glossary(const WW8Glossary&) = delete;
    WW8Glossary& operator=(const WW8Glossary&) = delete;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
