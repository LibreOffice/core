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



#ifndef _PARAGRAPH_DATA_HXX
#define _PARAGRAPH_DATA_HXX

#include <tools/solar.h>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// MT 07/00: Only for internal use, oder some kind like hPara for the few
// functions where you need it outside ( eg. moving paragraphs... )
//
// AW: Unfortunately NOT only local (formally in outliner.hxx), but also
// used in outlobj.hxx. Moved to own header

class ParagraphData
{
    friend class Paragraph;
    friend class OutlinerParaObject;

protected:
    sal_Int16           nDepth;
    sal_Int16           mnNumberingStartValue;
    sal_Bool            mbParaIsNumberingRestart;

public:
    ParagraphData( const ParagraphData& );
    ParagraphData();

    ParagraphData& operator=( const ParagraphData& );

    // compare operator
    bool operator==(const ParagraphData& rCandidate) const;

    // data read access
    sal_Int16 getDepth() const { return nDepth; }
};

//////////////////////////////////////////////////////////////////////////////

typedef ::std::vector< ParagraphData > ParagraphDataVector;

//////////////////////////////////////////////////////////////////////////////

#endif // _PARAGRAPH_DATA_HXX

// eof
