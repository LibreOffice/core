/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
namespace rtl { class OUString; }

namespace dp_gui {

class DependencyDialog: public ModalDialog {
public:
    DependencyDialog(
        Window * parent, std::vector< rtl::OUString > const & dependencies);

    ~DependencyDialog();

private:
    DependencyDialog(DependencyDialog &); // not defined
    void operator =(DependencyDialog &); // not defined

    virtual void Resize();

    FixedText m_text;
    ListBox m_list;
    OKButton m_ok;
    Size m_listDelta;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
