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

#ifndef SVX_DATABASE_REGISTRATION_UI_HXX
#define SVX_DATABASE_REGISTRATION_UI_HXX

#include "svx/svxdllapi.h"
#include <tools/solar.h>

class Window;

#define SID_SB_POOLING_ENABLED          (RID_OFA_START + 247)
#define SID_SB_DRIVER_TIMEOUTS          (RID_OFA_START + 248)
#define SID_SB_DB_REGISTER              (RID_OFA_START + 249)

//........................................................................
namespace svx
{
//........................................................................

    /** opens a dialog which allows the user to administrate the database registrations
    */
    sal_uInt16  SVX_DLLPUBLIC  administrateDatabaseRegistration( Window* _parentWindow );

//........................................................................
}   // namespace svx
//........................................................................

#endif // SVX_DATABASE_REGISTRATION_UI_HXX
