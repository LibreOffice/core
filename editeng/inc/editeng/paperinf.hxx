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


#ifndef _SVX_PAPERINF_HXX
#define _SVX_PAPERINF_HXX

// INCLUDE ---------------------------------------------------------------

#include <vcl/print.hxx>
#include <vcl/mapmod.hxx>
#include <editeng/svxenum.hxx>
#include "editeng/editengdllapi.h"

// forward ---------------------------------------------------------------

class Printer;
class Size;
class String;

// class SvxPaperInfo -----------------------------------------------------

class EDITENG_DLLPUBLIC SvxPaperInfo
{
public:
    static Size     GetDefaultPaperSize( MapUnit eUnit = MAP_TWIP );
    static Size     GetPaperSize( Paper ePaper, MapUnit eUnit = MAP_TWIP );
    static Size     GetPaperSize( const Printer* pPrinter );
    static Paper    GetSvxPaper( const Size &rSize, MapUnit eUnit = MAP_TWIP, bool bSloppy = sal_False );
    static long     GetSloppyPaperDimension( long nSize, MapUnit eUnit = MAP_TWIP );
    static String   GetName( Paper ePaper );
};

// INLINE -----------------------------------------------------------------

inline Size &Swap(Size &rSize)
{
    const long lVal = rSize.Width();
    rSize.Width() = rSize.Height();
    rSize.Height() = lVal;
    return rSize;
}

inline Size &LandscapeSwap(Size &rSize)
{
    if ( rSize.Height() > rSize.Width() )
        Swap( rSize );
    return rSize;
}

#endif
