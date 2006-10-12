/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: exithelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 14:12:40 $
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

#ifndef _DESKTOP_EXITHELPER_HXX_
#define _DESKTOP_EXITHELPER_HXX_

namespace desktop
{

//=============================================================================
/** @short  provide helper functions to handle a abnormal exit
            and contain a list of all "well known" exit codes.
 */
class ExitHelper
{
    //-------------------------------------------------------------------------
    // const
    public:

        //---------------------------------------------------------------------
        /** @short  list of all well known exit codes.

            @descr  Its not allowed to use exit codes hard coded
                    inside office. All places must use these list to
                    be synchron.
         */
        enum EExitCodes
        {
            /// e.g. used to force showing of the command line help
            E_NO_ERROR = 0,
            /// pipe was detected - second office must terminate itself
            E_SECOND_OFFICE = 1,
            /// an uno exception was catched during startup
            E_FATAL_ERROR = 333,    // Only the low 8 bits are significant 333 % 256 = 77
            /// crash during runtime
            E_CRASH = 78,
            /// user force automatic restart after crash
            E_CRASH_WITH_RESTART = 79,
            /// ???
            E_LOCKFILE = 80
        };
};

} // namespace desktop

#endif // #ifndef _DESKTOP_EXITHELPER_HXX_
