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
#ifndef SFX_SIDEBAR_LAYOUTABLE_WINDOW_INTERFACE_HXX
#define SFX_SIDEBAR_LAYOUTABLE_WINDOW_INTERFACE_HXX

#include "sfx2/dllapi.h"
#include <tools/gen.hxx>
#include <sal/types.h>
#include <com/sun/star/ui/LayoutSize.hpp>

class Window;

namespace sfx2 { namespace sidebar {


class SFX2_DLLPUBLIC ILayoutableWindow
{
public:
    virtual ~ILayoutableWindow();

    /** Return the preferred height with the constraint, that the
        window will be set to the given width.
    */
    virtual ::com::sun::star::ui::LayoutSize GetHeightForWidth (const sal_Int32 nWidth) = 0;
};


} } // end of namespace ::sd::sidebar

#endif
