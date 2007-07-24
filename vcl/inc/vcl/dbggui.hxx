/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbggui.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 09:58:29 $
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

#ifndef _SV_DBGGUI_HXX
#define _SV_DBGGUI_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

// ------------
// - DBG_UITL -
// ------------

#ifdef DBG_UTIL

class Window;
class XubString;

void DbgGUIInit();
void DbgGUIDeInit();
void DbgGUIStart();
void DbgDialogTest( Window* pWindow );

/** registers a named user-defined channel for emitting the diagnostic messages
    @return
        a unique number for this channel, which can be used for ->DbgData::nErrorOut,
        ->DbgData::nWarningOut and ->DbgData::nTraceOut
    @see DBG_OUT_USER_CHANNEL_0
*/
USHORT DbgRegisterNamedUserChannel( const XubString& _rChannelUIName, DbgPrintLine pProc );

#define DBGGUI_INIT()           DbgGUIInit()
#define DBGGUI_DEINIT()         DbgGUIDeInit()
#define DBGGUI_START()          DbgGUIStart()

#define DBG_DIALOGTEST( pWindow )                   \
    if ( DbgIsDialog() )                            \
        DbgDialogTest( pWindow );

#else


#define DBGGUI_INIT()
#define DBGGUI_DEINIT()
#define DBGGUI_START()

#define DBG_DIALOGTEST( pWindow )

#endif

#endif  // _SV_DBGGUI_HXX
