/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: general.h,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 13:24:58 $
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

#ifndef __FRAMEWORK_GENERAL_H_
#define __FRAMEWORK_GENERAL_H_

//=============================================================================
// includes

#include <macros/generic.hxx>

/* Normaly XInterface isnt used here ...
   But we need it to be able to define namespace alias css to anything related to ::com::sun::star
   :-) */
#include <com/sun/star/uno/XInterface.hpp>

#include <rtl/ustring.hxx>

//=============================================================================
// namespace

//-----------------------------------------------------------------------------
/** will make our code more readable if we can use such short name css instead
    of typing ::com::sun::star everytimes.

    On the other side we had so many problems with "using namespace" so we dont use
    it here any longer.
 */
namespace css = ::com::sun::star;

namespace framework {

//-----------------------------------------------------------------------------
/** status event mapped from load event of frame loader

    @todo think about me
          should be moved to another more specific place.
 */
static const ::rtl::OUString FEATUREDESCRIPTOR_LOADSTATE = DECLARE_ASCII("loadFinishedOrCancelled");

//-----------------------------------------------------------------------------
/** Those macro is used to make it more clear where a synchronized block will start.
    Because normal documentation code wont be recognized by some developers to be real
    I need something where they are thinking about.

    At least this macro will do nothing ... it's empty.
    But it should make the code more clear .-))
*/
#define SYNCHRONIZED_START

//-----------------------------------------------------------------------------
/** Same then SYNCHRONIZED_START ... but instead it mark the end of such code block.
*/
#define SYNCHRONIZED_END

} // namespace framework

#endif // #ifndef __FRAMEWORK_GENERAL_H_
