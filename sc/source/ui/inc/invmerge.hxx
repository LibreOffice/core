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



#ifndef SC_INVMERGE_HXX
#define SC_INVMERGE_HXX

#include <tools/gen.hxx>

#include <vector>

class Window;

class ScInvertMerger
{
private:
    Window*     pWin;
    ::std::vector< Rectangle >* pRects;
    Rectangle   aTotalRect;
    Rectangle   aLineRect;

    void        FlushLine();
    void        FlushTotal();

public:
                ScInvertMerger( Window* pWindow );
                ScInvertMerger( ::std::vector< Rectangle >* pRectangles );
                ~ScInvertMerger();

    void        AddRect( const Rectangle& rRect );
    void        Flush();
};



#endif


