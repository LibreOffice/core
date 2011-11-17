/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright 2011 Lionel Elie Mamane <lionel@mamane.lu>
 *
 * LibreOffice - a multi-platform office productivity suite
 *
 * This file is part of LibreOffice.
 *
 * LibreOffice is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * LibreOffice is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with LibreOffice.  If not, see
 * <http://www.gnu.org/licenses/lgpl-3.0.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#ifndef _PQ_COMMON_
#define _PQ_COMMON_

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

#endif /* _PQ_COMMON_ */
