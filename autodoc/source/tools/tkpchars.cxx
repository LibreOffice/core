/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tkpchars.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:20:10 $
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

#include <precomp.h>
#include <tools/tkpchars.hxx>

// NOT FULLY DECLARED SERVICES
#include <cosv/bstream.hxx>
#include <cosv/x.hxx>



CharacterSource::CharacterSource()
    :   dpSource(new char[2]),
        nSourceSize(0),
        nCurPos(0),
        nLastCut(0),
        nLastTokenStart(0),
        cCharAtLastCut(0)
{
    dpSource[nSourceSize] = NULCH;
    dpSource[nSourceSize+1] = NULCH;
}

CharacterSource::~CharacterSource()
{
    delete [] dpSource;
}

void
CharacterSource::LoadText(csv::bstream & io_rSource)
{
    if (dpSource != 0)
        delete [] dpSource;

    io_rSource.seek(0, csv::end);
    nSourceSize = intt(io_rSource.position());
    io_rSource.seek(0);

    dpSource = new char[nSourceSize+1];

    intt nCount = (intt) io_rSource.read(dpSource,nSourceSize);
    if (nCount != nSourceSize)
        throw csv::X_Default("IO-Error: Could not load file completely.");

    dpSource[nSourceSize] = NULCH;

    BeginSource();
}

void
CharacterSource::LoadText( const char * i_sSourceText )
{
    if (dpSource != 0)
        delete [] dpSource;

    nSourceSize = strlen(i_sSourceText);

    dpSource = new char[nSourceSize+1];
    strcpy( dpSource,  i_sSourceText);      // SAFE STRCPY (#100211# - checked)

    BeginSource();
}

///  KORR_FUTURE:  So far, this works only when tokens do not cross inserted text boundaries.
void
CharacterSource::InsertTextAtCurPos( const char * i_sText2Insert )
{
    if ( i_sText2Insert == 0 ? true : strlen(i_sText2Insert) == 0 )
        return;

    aSourcesStack.push( S_SourceState(
                            dpSource,
                            nSourceSize,
                            nCurPos,
                            nLastCut,
                            nLastTokenStart,
                            cCharAtLastCut ) );

    nSourceSize = strlen(i_sText2Insert);
    dpSource = new char[nSourceSize+1];
    strcpy( dpSource,  i_sText2Insert);     // SAFE STRCPY (#100211# - checked)

    BeginSource();
}

const char *
CharacterSource::CutToken()
{
    dpSource[nLastCut] = cCharAtLastCut;
    nLastTokenStart = nLastCut;
    nLastCut = CurPos();
    cCharAtLastCut = dpSource[nLastCut];
    dpSource[nLastCut] = NULCH;

    return &dpSource[nLastTokenStart];
}

void
CharacterSource::BeginSource()
{
    nCurPos = 0;
    nLastCut = 0;
    nLastTokenStart = 0;
    cCharAtLastCut = dpSource[nLastCut];
    dpSource[nLastCut] = NULCH;
}

//  KORR_FUTURE:  So far, this works only when tokens do not cross inserted text boundaries.
char
CharacterSource::MoveOn_OverStack()
{
    while ( aSourcesStack.size() > 0 AND nCurPos >= nSourceSize-1 )
    {
        S_SourceState & aState = aSourcesStack.top();
        delete [] dpSource;

        dpSource = aState.dpSource;
        nSourceSize = aState.nSourceSize;
        nCurPos = aState.nCurPos;
        nLastCut = aState.nLastCut;
        nLastTokenStart = aState.nLastTokenStart;
        cCharAtLastCut = aState.cCharAtLastCut;

        aSourcesStack.pop();
    }

    if ( nLastCut < nCurPos )
        CutToken();

    return CurChar();
}

CharacterSource::
S_SourceState::S_SourceState( DYN char *    dpSource_,
                              intt          nSourceSize_,
                              intt          nCurPos_,
                              intt          nLastCut_,
                              intt          nLastTokenStart_,
                              char          cCharAtLastCut_ )
    :   dpSource(dpSource_),
        nSourceSize(nSourceSize_),
        nCurPos(nCurPos_),
        nLastCut(nLastCut_),
        nLastTokenStart(nLastTokenStart_),
        cCharAtLastCut(cCharAtLastCut_)
{
}

