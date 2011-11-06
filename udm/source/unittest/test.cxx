/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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




