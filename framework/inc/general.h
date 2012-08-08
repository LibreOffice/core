/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_GENERAL_H_
#define __FRAMEWORK_GENERAL_H_

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
