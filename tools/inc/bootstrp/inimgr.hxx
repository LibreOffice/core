/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inimgr.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:06:49 $
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
#ifndef _INIMGR_HXX
#define _INIMGR_HXX

#include <tools/fsys.hxx>
#include <tools/string.hxx>

/*****************************************************************************
Purpose: Allows to work on a local set of initialisation files
If Update is used, the user must ensure that only one set of
Source and Destination Dir is used. Otherwise ForceUpdate has to be used
*****************************************************************************/

class IniManager
{
private:
    BOOL bUpdate;

    ByteString sGlobalDir;      /// holds the org. ini dir
    ByteString sLocalPath;      /// holds path of local ini dir

public:
    IniManager( ByteString &rDir, ByteString &rLocalDir );
    IniManager( ByteString &rDir );
    IniManager();

    ByteString ToLocal( ByteString &rPath );
    void Update();          /// Call ForceUpdate the First Time called
    void ForceUpdate();

    static ByteString GetLocalIni();
    static ByteString GetGlobalIni();
};

#endif

