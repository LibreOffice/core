/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: so_msg.hxx,v $
 * $Revision: 1.4 $
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

#ifndef __PLUGIN_MSG_HXX
#define __PLUGIN_MSG_HXX
enum msg_type
{
    SO_EMPTY,        // 0
    SO_SET_WINDOW,   // 1
    SO_SET_URL,      // 2
    SO_NEW_INSTANCE, // 3
    SO_DESTROY,      // 4
    SO_SHUTDOWN,     // 5
    SO_PRINT         // 6
};

#define plugin_Int32 unsigned long int

typedef struct _PLUGIN_MSG {
    plugin_Int32 msg_id;
    plugin_Int32 instance_id;
    plugin_Int32 wnd_id;
    plugin_Int32 wnd_x;
    plugin_Int32 wnd_y;
    plugin_Int32 wnd_w;
    plugin_Int32 wnd_h;
    char url[484];
} PLUGIN_MSG;

#endif
