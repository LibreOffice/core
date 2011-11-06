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



#ifndef _PREVIEWPAGES_HXX
#define _PREVIEWPAGES_HXX

// classes <Point>, <Size> and <Rectangle>
#include <tools/gen.hxx>

class SwPageFrm;

/** data structure for a preview page in the current preview layout

    OD 12.12.2002 #103492# - struct <PrevwPage>

    @author OD
*/
struct PrevwPage
{
    const SwPageFrm*  pPage;
    bool        bVisible;
    Size        aPageSize;
    Point       aPrevwWinPos;
    Point       aLogicPos;
    Point       aMapOffset;

    inline PrevwPage();
};

inline PrevwPage::PrevwPage()
    : pPage( 0 ),
      bVisible( false ),
      aPageSize( Size(0,0) ),
      aPrevwWinPos( Point(0,0) ),
      aLogicPos( Point(0,0) ),
      aMapOffset( Point(0,0) )
{};

#endif
