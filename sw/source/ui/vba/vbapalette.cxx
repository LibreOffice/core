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

#include "vbapalette.hxx"
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/word/WdColor.hpp>
#include <ooo/vba/word/WdColorIndex.hpp>

using namespace ::ooo::vba;
using namespace ::ooo::vba::word;
using namespace ::com::sun::star;

static const sal_Int32 ColorTable[] =
{
WdColor::wdColorAutomatic, // 0
WdColor::wdColorBlack,             // 1
WdColor::wdColorBlue,               // 2
WdColor::wdColorTurquoise,     // 3
WdColor::wdColorBrightGreen, // 4
WdColor::wdColorPink,               // 5
WdColor::wdColorRed,                 // 6
WdColor::wdColorYellow,           // 7
WdColor::wdColorWhite,             // 8
WdColor::wdColorDarkBlue,       // 9
WdColor::wdColorTeal,               // 10
WdColor::wdColorGreen,             // 11
WdColor::wdColorViolet,           // 12
WdColor::wdColorDarkRed,         // 13
WdColor::wdColorDarkYellow,   // 14
WdColor::wdColorGray50,           // 15
WdColor::wdColorGray25,           // 16
};

typedef ::cppu::WeakImplHelper1< container::XIndexAccess > XIndexAccess_BASE;

class DefaultPalette : public XIndexAccess_BASE
{
public:
   DefaultPalette(){}

    // Methods XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount() throw (uno::RuntimeException)
    {
        return sizeof(ColorTable) / sizeof(ColorTable[0]);
    }

    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
    {
    if ( Index < 0 || Index >= getCount() )
        throw lang::IndexOutOfBoundsException();
        return uno::makeAny( sal_Int32( ColorTable[ Index ] ) );
    }

    // Methods XElementAcess
    virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException)
    {
        return ::getCppuType( (sal_Int32*)0 );
    }
    virtual ::sal_Bool SAL_CALL hasElements() throw (uno::RuntimeException)
    {
        return sal_True;
    }

};

VbaPalette::VbaPalette()
{
    mxPalette = new DefaultPalette();
}

uno::Reference< container::XIndexAccess >
VbaPalette::getPalette() const
{

    return mxPalette;
}

