/*************************************************************************
 *
 *  $RCSfile: diagnose.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:12 $
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


#ifndef _VOS_DIAGNOSE_H_
#define _VOS_DIAGNOSE_H_


#ifndef _OSL_DIAGNOSE_H_
    #include <osl/diagnose.h>
#endif


/*
    Diagnostic support
*/

#define VOS_THIS_FILE       __FILE__

#define VOS_DEBUG_ONLY(s)   _OSL_DEBUG_ONLY(s)
#define VOS_TRACE           _OSL_TRACE
#define VOS_ASSERT(c)       _OSL_ASSERT(c, VOS_THIS_FILE, __LINE__)
#define VOS_VERIFY(c)       _OSL_VERIFY(c, VOS_THIS_FILE, __LINE__)
#define VOS_ENSHURE(c, m)   _OSL_ENSHURE(c, VOS_THIS_FILE, __LINE__, m)
#define VOS_ENSURE(c, m)    _OSL_ENSURE(c, VOS_THIS_FILE, __LINE__, m)

#define VOS_PRECOND(c, m)   VOS_ENSHURE(c, m)
#define VOS_POSTCOND(c, m)  VOS_ENSHURE(c, m)

#endif /* _VOS_DIAGNOSE_H_ */


