/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: doublecheckedlocking.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:26:53 $
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
      else
          OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
      return p;

    One extra advantage of this macro is that it makes it easier to find all
    places where double-checked locking is used.
 */
#define OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER() /* empty */

#if defined __cplusplus
}
#endif /* __cplusplus */

#endif /* INCLUDED_OSL_DOUBLECHECKEDLOCKING_H */
