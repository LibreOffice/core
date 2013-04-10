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
#ifndef SFX_SIDEBAR_HXX
#define SFX_SIDEBAR_HXX

#include "sfx2/dllapi.h"
#include <vcl/window.hxx>
#include <com/sun/star/frame/XFrame.hpp>


namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;

namespace sfx2 { namespace sidebar {


/** SFX-less version of a module dependent task pane, filled with tool panels as specified in the respective
    module's configuration
*/
class SFX2_DLLPUBLIC Sidebar
    : public Window
{
public:
        /** creates a new instance
            @param i_rParentWindow
                the parent window
            @param i_rDocumentFrame
                the frame to which the task pane belongs. Will be passed to any custom tool panels created
                via an XUIElementFactory. Also, it is used to determine the module which the task pane is
                responsible for, thus controlling which tool panels are actually available.
        */
    Sidebar(
        Window& rParentWindow,
        const cssu::Reference<css::frame::XFrame>& rxDocumentFrame);


    virtual ~Sidebar (void);

protected:
    virtual void Resize (void);
    virtual void GetFocus (void);

private:
};


} } // end of namespace sfx2::sidebar

#endif
