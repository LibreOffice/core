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

