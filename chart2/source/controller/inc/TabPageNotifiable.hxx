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


#ifndef CHART2_TABPAGENOTIFIABLE_HXX
#define CHART2_TABPAGENOTIFIABLE_HXX

#include <vcl/tabpage.hxx>
#include <tools/color.hxx>

// color to use as foreground for an invalid range
#define RANGE_SELECTION_INVALID_RANGE_FOREGROUND_COLOR COL_WHITE
// color to use as background for an invalid range
#define RANGE_SELECTION_INVALID_RANGE_BACKGROUND_COLOR 0xff6563

namespace chart
{

class TabPageNotifiable
{
public:
    virtual void setInvalidPage( TabPage * pTabPage ) = 0;
    virtual void setValidPage( TabPage * pTabPage ) = 0;
};

} //  namespace chart

// CHART2_TABPAGENOTIFIABLE_HXX
#endif
