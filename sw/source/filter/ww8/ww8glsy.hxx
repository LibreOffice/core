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

#ifndef _WW8GLSY_HXX
#define _WW8GLSY_HXX

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
    WW8GlossaryFib( SvStream& rStrm, sal_uInt8 nWantedVersion ,
        SvStream& rTableStrm, const WW8Fib &rFib) : WW8Fib(rStrm,
        nWantedVersion,FindGlossaryFibOffset(rTableStrm,rStrm,rFib)) {}
    bool IsGlossaryFib();
private:
    sal_uInt32 FindGlossaryFibOffset(SvStream &rTableStrm,SvStream &rStrm,
        const WW8Fib &rFib);
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
    WW8Glossary( SvStorageStreamRef &refStrm, sal_uInt8 nVersion, SvStorage *pStg=0);
    bool Load( SwTextBlocks &rBlocks, bool bSaveRelFile );
    ~WW8Glossary()                  { delete pGlossary; }
    WW8GlossaryFib *GetFib()        { return pGlossary; }
    sal_uInt16 GetNoStrings() const     { return nStrings; }

private:
    WW8GlossaryFib *pGlossary;
    SvStorageStreamRef xTableStream;
    SvStorageStreamRef &rStrm;
    SvStorageRef xStg;
    sal_uInt16 nStrings;

    bool MakeEntries(SwDoc *pD, SwTextBlocks &rBlocks, bool bSaveRelFile,
        const ::std::vector<OUString>& rStrings,
        const ::std::vector<ww::bytes>& rExtra);
    bool HasBareGraphicEnd(SwDoc *pD,SwNodeIndex &rIdx);

    //No copying
    WW8Glossary(const WW8Glossary&);
    WW8Glossary& operator=(const WW8Glossary&);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
