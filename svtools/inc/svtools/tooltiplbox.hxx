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



#ifndef SVTOOLS_TOOLTIPLBOX_HXX
#define SVTOOLS_TOOLTIPLBOX_HXX

#include "svtools/svtdllapi.h"
#include <vcl/lstbox.hxx>

namespace svtools {

// ============================================================================

/** ListBox with tool tips for long entries. */
class ToolTipListBox : public ListBox
{
public:
                                ToolTipListBox( Window* pParent, WinBits nStyle = WB_BORDER );
                                ToolTipListBox( Window* pParent, const ResId& rResId );

protected:
    virtual void                RequestHelp( const HelpEvent& rHEvt );
};

// ----------------------------------------------------------------------------

/** MultiListBox with tool tips for long entries. */
class SVT_DLLPUBLIC ToolTipMultiListBox : public MultiListBox
{
public:
                                ToolTipMultiListBox( Window* pParent, WinBits nStyle = WB_BORDER );
                                ToolTipMultiListBox( Window* pParent, const ResId& rResId );

protected:
    virtual void                RequestHelp( const HelpEvent& rHEvt );
};

// ============================================================================

} // namespace svtools

#endif

