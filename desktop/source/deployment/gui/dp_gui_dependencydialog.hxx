/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_DEPENDENCYDIALOG_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_DEPENDENCYDIALOG_HXX

#include "sal/config.h"

#include <vector>
#include "tools/gen.hxx"
#include "vcl/button.hxx"
#include "vcl/dialog.hxx"
#include "vcl/fixed.hxx"
#include "vcl/lstbox.hxx"

class Window;

namespace dp_gui {

class DependencyDialog: public ModalDialog {
public:
    DependencyDialog(
        Window * parent, std::vector< OUString > const & dependencies);

    virtual ~DependencyDialog();

private:
    DependencyDialog(DependencyDialog &); // not defined
    void operator =(DependencyDialog &); // not defined

    virtual void Resize() SAL_OVERRIDE;

    FixedText m_text;
    ListBox m_list;
    OKButton m_ok;
    Size m_listDelta;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
