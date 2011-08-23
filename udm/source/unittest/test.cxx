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

#include <precomp.h>


// NOT FULLY DEFINED SERVICES
#include <udm/integer_i.hxx>
#include <udm/string_i.hxx>

using namespace udm;

// TypeSystem      aTypeSys;


class Function
{
  public:

    intt                nId;
};

class Ctor : public Function
{
  public:

    csv::String         sClassName;
};

class Method : public Function
{
  public:

    csv::String         sFunctionName;
    std::vector< std::pair< intt, String > >
                        aParameters;
};


class PrObj
{
  public:
    intt                nId;
    csv::String         sName;
    std::vector< PrObj* >
                        aMethods;
    std::vector< PrObj* >
                        aData;
};




void fx()
{
    intt            n = 5;
    Dyn<Integer_i>  px = new Integer_i(n);
    Integer &       x = *px;

    csv::String     s;
    Dyn<String_i>   ps = new String_i(s);
    String &        xs = *ps;

    xs = "Hallo";

    int dev01 = 2;

    xs = csv::String("Haha");

    dev01 = 2;

    csv::String s2 = xs;


    dev01 = 2;

//  const char * pc = xs;


    intt a = x;
    x = 15;

    Cout() << a << " " << x << Endl();  // Must be:   5 15
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
