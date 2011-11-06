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



#ifndef _STDCTRL_HXX
#define _STDCTRL_HXX

#include "svtools/svtdllapi.h"

#ifndef _EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif

// -------------
// - FixedInfo -
// -------------

class SVT_DLLPUBLIC FixedInfo : public FixedText
{
public:
    FixedInfo( Window* pParent, WinBits nWinStyle = WB_LEFT );
    FixedInfo( Window* pParent, const ResId& rResId );
};

namespace svt
{
    // ----------------------------
    // - svt::SelectableFixedText -
    // ----------------------------

    class SVT_DLLPUBLIC SelectableFixedText : public Edit
    {
    private:
        void    Init();

    public:
                SelectableFixedText( Window* pParent, WinBits nWinStyle );
                SelectableFixedText( Window* pParent, const ResId& rResId );
        virtual ~SelectableFixedText();

        virtual void    LoseFocus();
    };

} // namespace svt

#endif  // _STDCTRL_HXX

