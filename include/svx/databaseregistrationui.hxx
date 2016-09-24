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

#ifndef INCLUDED_SVX_DATABASEREGISTRATIONUI_HXX
#define INCLUDED_SVX_DATABASEREGISTRATIONUI_HXX

#include <svx/svxdllapi.h>

namespace vcl { class Window; }

#define SID_SB_POOLING_ENABLED          (RID_OFA_START + 247)
#define SID_SB_DRIVER_TIMEOUTS          (RID_OFA_START + 248)
#define SID_SB_DB_REGISTER              (RID_OFA_START + 249)

namespace svx
{
    /** opens a dialog which allows the user to administrate the database registrations
    */
    sal_uInt16  SVX_DLLPUBLIC  administrateDatabaseRegistration( vcl::Window* _parentWindow );

}

#endif // INCLUDED_SVX_DATABASEREGISTRATIONUI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
