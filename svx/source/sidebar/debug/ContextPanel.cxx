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

#include "ContextPanel.hxx"

namespace svx { namespace sidebar {

ContextPanel::ContextPanel (::Window* pParent)
    : Window(pParent, 0),
      maApplicationName(this, 0),
      maContextName(this, 0)
{
    maApplicationName.setPosSizePixel(5,5, 250,15);
    maContextName.setPosSizePixel(5,25, 250,15);

    maApplicationName.Show();
    maContextName.Show();
    Show();
}




ContextPanel::~ContextPanel (void)
{
}

void ContextPanel::HandleContextChange (const sfx2::sidebar::EnumContext aContext)
{
    maApplicationName.SetText(aContext.GetApplicationName());
    maContextName.SetText(aContext.GetContextName());
}

} } // end of namespace ::svx::sidebar
