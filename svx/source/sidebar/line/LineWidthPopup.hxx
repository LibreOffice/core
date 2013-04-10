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
#ifndef _SVX_SIDEBAR_LINE_WIDTH_POPUP_HXX_
#define _SVX_SIDEBAR_LINE_WIDTH_POPUP_HXX_

#include "svx/sidebar/Popup.hxx"

#include <svl/poolitem.hxx>

#include <boost/function.hpp>


namespace svx { namespace sidebar {

class LineWidthPopup
    : public Popup
{
public :
    LineWidthPopup (
        Window* pParent,
        const ::boost::function<PopupControl*(PopupContainer*)>& rControlCreator);
    virtual ~LineWidthPopup (void);

    void SetWidthSelect (long lValue, bool bValuable, SfxMapUnit eMapUnit);

private:
    void PopupModeEndCallback (void);
};

} } // end of namespace svx::sidebar

#endif

// eof
