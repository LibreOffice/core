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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_H
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_H

#include "dp_gui_updatedata.hxx"
#include "dp_misc.h"
#include "dp_gui.hrc"
#include <rtl/ref.hxx>
#include <rtl/instance.hxx>
#include <osl/thread.hxx>
#include <vcl/svapp.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <svtools/svtabbx.hxx>
#include <svtools/headbar.hxx>
#include <com/sun/star/ucb/XContentEventListener.hpp>
#include <osl/mutex.hxx>

namespace com { namespace sun { namespace star {
    namespace container {
        class XNameAccess;
    }
    namespace frame {
        class XDesktop;
    }
    namespace awt {
        class XWindow;
    }
    namespace uno {
        class XComponentContext;
    }
    namespace deployment {
        class XPackageManagerFactory;
    }
} } }

namespace svt {
    class FixedHyperlink;
}

namespace dp_gui {

enum PackageState { REGISTERED, NOT_REGISTERED, AMBIGUOUS, NOT_AVAILABLE };



} // namespace dp_gui

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
