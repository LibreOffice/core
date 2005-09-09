/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: test.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:39:56 $
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




