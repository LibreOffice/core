/*************************************************************************
 *
 *  $RCSfile: exithelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 14:49:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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