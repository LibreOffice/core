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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_PARAGRAPH_PARALINESPACINGPOPUP_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_PARAGRAPH_PARALINESPACINGPOPUP_HXX

#include "svx/sidebar/Popup.hxx"

#include <boost/function.hpp>
#include <svl/poolitem.hxx>
#include <tools/fldunit.hxx>
#include <editeng/lspcitem.hxx>
#include <sfx2/sidebar/EnumContext.hxx>

namespace svx { namespace sidebar {

class ParaLineSpacingPopup
    : public Popup
{
public :
    ParaLineSpacingPopup (
        Window* pParent,
        const ::boost::function<PopupControl*(PopupContainer*)>& rControlCreator);
    virtual ~ParaLineSpacingPopup (void);

    void Rearrange (SfxItemState currSPState,FieldUnit currMetricUnit,SvxLineSpacingItem* currSPItem ,const ::sfx2::sidebar::EnumContext currentContext);
private:
    void PopupModeEndCallback (void);
};

} } // end of namespace svx::sidebar

#endif


