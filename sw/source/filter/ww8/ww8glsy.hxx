/*************************************************************************
 *
 *  $RCSfile: ww8glsy.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _WW8GLSY_HXX
#define _WW8GLSY_HXX

#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif

#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif

#ifndef _WW8SCAN_HXX
#include <ww8scan.hxx>
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
    BOOL IsGlossaryFib();
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
    BOOL Load( SwTextBlocks &rBlocks, BOOL bSaveRelFile );
    ~WW8Glossary()                  { delete pGlossary; }
    WW8GlossaryFib *GetFib()        { return pGlossary; }
    USHORT GetNoStrings() const     { return nStrings; }

private:
    WW8GlossaryFib *pGlossary;
    SvStorageStreamRef xTableStream;
    SvStorageStreamRef &rStrm;
    SvStorageRef xStg;
    USHORT nStrings;

    BOOL MakeEntries( SwDoc *pD, SwTextBlocks &rBlocks, BOOL bSaveRelFile,
                        SvStrings& rStrings, SvStrings& rExtra);
    BOOL HasBareGraphicEnd(SwDoc *pD,SwNodeIndex &rIdx);
};





#endif
