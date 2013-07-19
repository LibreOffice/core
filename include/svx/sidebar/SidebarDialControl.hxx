/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef SVX_SIDEBAR_DIAL_CONTROL_HXX
#define SVX_SIDEBAR_DIAL_CONTROL_HXX

#include <svx/dialcontrol.hxx>

namespace svx { namespace sidebar {

/** Provide some improvements over the standard DialControl.
*/
class SVX_DLLPUBLIC SidebarDialControl : public svx::DialControl
{
public:
    SidebarDialControl (Window* pParent, WinBits nBits = 0);
    virtual ~SidebarDialControl (void);

    virtual Size GetOptimalSize() const;
    virtual void MouseButtonDown (const MouseEvent& rMEvt);

protected:
    virtual void HandleMouseEvent (const Point& rPos, bool bInitial);
};

} } // end of namespace svx::sidebar

#endif
