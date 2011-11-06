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


#ifndef _EEDATA_HXX
#define _EEDATA_HXX

//////////////////////////////////////////////////////////////////////////////
// extended data definitions for EditEngine portion stripping (StripPortions()
// support). These may be mapped to some primitive definitions from Drawinglayer
// later.
#include <vector>

namespace EEngineData
{
    // spell checking wrong vector containing the redlining data
    class WrongSpellClass
    {
    public:
        sal_uInt32 nStart;
        sal_uInt32 nEnd;

        WrongSpellClass(sal_uInt32 nS, sal_uInt32 nE) : nStart(nS), nEnd(nE) {}
    };

    typedef std::vector< WrongSpellClass > WrongSpellVector;

    // text marking vector containing the EOC, EOW and EOS TEXT definitions
    enum TextMarkingType
    {
        EndOfCaracter = 0,
        EndOfWord = 1,
        EndOfSentence = 2
    };

    class TextMarkingClass
    {
    public:
        TextMarkingType eType;
        sal_uInt32 nIndex;

        TextMarkingClass(TextMarkingType eT, sal_uInt32 nI) : eType(eT), nIndex(nI) {}
    };

    typedef std::vector< TextMarkingClass > TextMarkingVector;

} // end of namespace EditEngine

#endif // _EEDATA_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
