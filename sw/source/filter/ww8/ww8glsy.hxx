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


/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

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
        const ::std::vector<String>& rStrings,
        const ::std::vector<ww::bytes>& rExtra);
    bool HasBareGraphicEnd(SwDoc *pD,SwNodeIndex &rIdx);

    //No copying
    WW8Glossary(const WW8Glossary&);
    WW8Glossary& operator=(const WW8Glossary&);
};
#endif

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
