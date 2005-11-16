/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ww8glsy.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-16 13:54:04 $
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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef _WW8GLSY_HXX
#define _WW8GLSY_HXX

#include <sot/storage.hxx>

#ifndef _WW8SCAN_HXX
#include "ww8scan.hxx"
#endif

class SwTextBlocks;
class SwNodeIndex;
class SwSpzFrmFmts;

/*
 * GlossaryFib takes the document fib and finds the glossary fib which may
 * not exist. The glossary fib has the offsets into the autotext subdocument
 * which is at the end of template .dot's
 */
class WW8GlossaryFib : public WW8Fib
{
public:
    WW8GlossaryFib( SvStream& rStrm, BYTE nWantedVersion ,
        SvStream& rTableStrm, const WW8Fib &rFib) : WW8Fib(rStrm,
        nWantedVersion,FindGlossaryFibOffset(rTableStrm,rStrm,rFib)) {}
    bool IsGlossaryFib();
private:
    UINT32 FindGlossaryFibOffset(SvStream &rTableStrm,SvStream &rStrm,
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
    WW8Glossary( SvStorageStreamRef &refStrm, BYTE nVersion, SvStorage *pStg=0);
    bool Load( SwTextBlocks &rBlocks, bool bSaveRelFile );
    ~WW8Glossary()                  { delete pGlossary; }
    WW8GlossaryFib *GetFib()        { return pGlossary; }
    USHORT GetNoStrings() const     { return nStrings; }

private:
    WW8GlossaryFib *pGlossary;
    SvStorageStreamRef xTableStream;
    SvStorageStreamRef &rStrm;
    SvStorageRef xStg;
    USHORT nStrings;

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
