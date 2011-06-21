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

#ifndef CSV_COMFUNC_HXX
#define CSV_COMFUNC_HXX

#include <stdlib.h>




namespace csv
{
    class String;


// min, max and range functions
template <class E>
inline E            max(E in1, E in2);
template <class E>
inline E            min(E in1, E in2);
template <class E>
inline bool         in_range(E low, E val, E high);    // return low <= val < high;


// string functions
inline const char * valid_str(const char * str);
inline bool         no_str(const char * str);       // return !str || !strlen(str)


// IMPLEMENTATION
template <class E>
inline E
max(E in1, E in2)   { return in1 < in2 ? in2 : in1; }
template <class E>
inline E
min(E in1, E in2)   { return in1 < in2 ? in1 : in2; }
template <class E>
inline bool
in_range(E low, E val, E high) { return low <= val AND val < high; }

inline const char *
valid_str(const char * str) { return str != 0 ? str : ""; }
inline bool
no_str(const char * str) { return str != 0 ? *str == '\0' : true; }

}   // namespace csv

#define NON_COPYABLE(xy) \
    private: xy(const xy &); xy & operator=(const xy &)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
