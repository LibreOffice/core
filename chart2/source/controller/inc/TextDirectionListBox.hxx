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



#ifndef CHART2_TEXTDIRECTIONLISTBOX_HXX
#define CHART2_TEXTDIRECTIONLISTBOX_HXX

#include <svx/frmdirlbox.hxx>

class Window;

namespace chart
{

class TextDirectionListBox : public svx::FrameDirectionListBox
{
public:
    explicit            TextDirectionListBox( Window* pParent, const ResId& rResId,
                            Window* pWindow1 = 0, Window* pWindow2 = 0 );
    virtual             ~TextDirectionListBox();
};

} //namespace chart

#endif

