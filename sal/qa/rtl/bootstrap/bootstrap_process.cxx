/*************************************************************************
 *
 *  $RCSfile: bootstrap_process.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-03 09:06:32 $
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
#include <stdlib.h>
#include <stdio.h>

#ifndef _RTL_BOOTSTRAP_HXX_
#include <rtl/bootstrap.hxx>
#endif

#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

using namespace ::rtl;

// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int argc, char *argv[] )
#else
int _cdecl main( int argc, char* argv[] )
#endif
{
        Bootstrap aBootstrap;
        //custom .ini/rc file
        Bootstrap aBs_custom( OUString::createFromAscii(argv[3]) );
    OUString suValue;
    OUString suDefault( OUString::createFromAscii("mydefault") );
        int flag = atoi( argv[1] );

        switch( flag ) {
       case 1:
        // parameters may be passed by command line arguments
            aBootstrap.getFrom(
            OUString(RTL_CONSTASCII_USTRINGPARAM("UNO_SERVICES")),
            suValue );
        if (suValue.equalsAscii("service.rdb") )
        {
            return 10;
        }
        else
            return 11;
              case 2:
                  // parameters may be passed by ini file
            aBootstrap.getFrom(
            OUString(RTL_CONSTASCII_USTRINGPARAM("EXECUTABLE_RC")),
            suValue );
        if (suValue.equalsAscii("true") )
        {
            return 20;
        }
        else
            return 21;
              case 3:
              // parameters may be passed by command line arguments
            aBootstrap.getFrom(
            OUString(RTL_CONSTASCII_USTRINGPARAM("QADEV_BOOTSTRAP")),
            suValue );
        if (suValue.equalsAscii("sun&ms") )
        {
            return 30;
        }
        else
            return 31;
       case 4:
              // parameters may be passed by custom .ini/rc file
            aBs_custom.getFrom(
            OUString(RTL_CONSTASCII_USTRINGPARAM("RTLVALUE")),
            suValue );
        if (suValue.equalsAscii("qadev17") )
        {
            return 40;
        }
        else
            return 41;
       case 5:
              // parameters may be passed by inheritance
            aBs_custom.getFrom(
            OUString(RTL_CONSTASCII_USTRINGPARAM("EXECUTABLE_RC")),
            suValue );
        if (suValue.equalsAscii("true") )
        {
            return 50;
        }
        else
            return 51;
       default:
             // parameters may be passed by inheritance
            aBs_custom.getFrom(
            OUString(RTL_CONSTASCII_USTRINGPARAM("ABCDE")),
            suValue, suDefault );
        if (suValue.equalsAscii("mydefault") )
        {
            return 60;
        }
        else
            return 61;
       }
}
