/*************************************************************************
 *
 *  $RCSfile: postx.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:41 $
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
//*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-//
//                                                                            //
// (C) 1997 Star Division GmbH, Hamburg, Germany                              //
//                                                                            //
// $Revision: 1.1.1.1 $  $Author: hr $  $Date: 2000-09-18 17:05:41 $    //
//                                                                            //
// $Workfile:   postx.h  $                                                    //
//  $Modtime:   08 Aug 1997 10:13:36  $                                       //
//                                                                            //
//*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-//

#ifndef _POSTX_H
#define _POSTX_H

#if defined __cplusplus && ! defined LINUX
}
#endif

/* X-Types */
#undef Window
#undef BYTE
#undef INT8
#undef BOOL
#undef Font
#undef Cursor
#undef String
#undef KeyCode
#undef Region
#undef Icon
#undef Time
#undef Boolean

#undef Min
#undef Max
#undef class
#undef new
#undef DestroyAll
#undef Success

#undef Printer
/* #undef FontInfo */
#undef Orientation

#undef GetToken
#undef ReleaseToken
#undef InitializeToken
#undef NextRequest

/* Network Audio System */
#undef Sound

#ifdef KeyPress
#if KeyPress != 2
Error KeyPress must be Equal 2
#endif
#undef KeyPress
#endif
#define XLIB_KeyPress 2

#endif

