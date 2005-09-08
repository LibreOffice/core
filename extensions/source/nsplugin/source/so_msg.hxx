/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: so_msg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:42:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
