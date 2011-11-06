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


#ifndef _REDLNAUT_HXX
#define _REDLNAUT_HXX

#include <tools/string.hxx>
#include <tools/color.hxx>

#include "swtypes.hxx"

class SfxItemSet;
class SwAttrPool;

class SwRedlineAuthor
{
    String sAuthor;
    Color aChgLineColor;
    SfxItemSet *pInsAttrSet, *pDelAttrSet, *pFmtAttrSet;
    short eChgLineOrient;
    sal_uInt8 cDelChar;
public:
    SwRedlineAuthor( SwAttrPool& rPool, const String& );
    SwRedlineAuthor( const SwRedlineAuthor& );
    ~SwRedlineAuthor();

    SwRedlineAuthor& operator=( const SwRedlineAuthor& );
};


#endif
