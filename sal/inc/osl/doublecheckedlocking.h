/*************************************************************************
 *
 *  $RCSfile: doublecheckedlocking.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sb $ $Date: 2002-06-24 08:29:42 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#if !defined INCLUDED_OSL_DOUBLECHECKEDLOCKING_H
#define INCLUDED_OSL_DOUBLECHECKEDLOCKING_H

#if defined __cplusplus
extern "C" {
#endif /* __cplusplus */

/** A platform specific macro needed to make double-checked locking work.

    See
    <http://www.cs.umd.edu/~pugh/java/memoryModel/DoubleCheckedLocking.html>
    for a description of double-checked locking, why it is broken, and how it
    can be fixed.  On platforms where it is necessary, this macro will expand
    to some memory barrier instruction.  On many platforms, double-checked
    locking works as it is, though, so on those platforms this macro will be
    empty.  This is a macro instead of a (C++ inline) function to allow for
    maximum performance in both C and C++.

    If possible, use the rtl_Instance template instead of explicitly spelling
    out the double-checked locking pattern.  There are few cases where you
    will have to spell it out explicitly (e.g., the logic of a certain
    instance of the pattern is too complex to be mapped to the template, or
    some compiler refuses to compile a template instantiation due to internal
    compiler errors), though, and you should always call this macro at the
    right places then:

      static T * pInstance = 0;

      T * p = pInstance;
      OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
      if (!p)
      {
          Guard aGuard(aMutex);
          p = pInstance;
          if (!p)
          {
              p = ...;
              OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
              pInstance = p;
          }
      }
      return p;

    One extra advantage of this macro is that it makes it easier to find all
    places where double-checked locking is used.
 */
#define OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER() /* empty */

#if defined __cplusplus
}
#endif /* __cplusplus */

#endif /* INCLUDED_OSL_DOUBLECHECKEDLOCKING_H */
