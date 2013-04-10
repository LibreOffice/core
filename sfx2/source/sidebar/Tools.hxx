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

#ifndef SFX_SIDEBAR_TOOLS_HXX
#define SFX_SIDEBAR_TOOLS_HXX

#include <vcl/image.hxx>
#include <vcl/gradient.hxx>
#include <tools/svborder.hxx>

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/frame/XFrame.hpp>


#define A2S(s) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s)))
#define S2A(s) rtl::OUStringToOString(s, RTL_TEXTENCODING_ASCII_US).getStr()

namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;


namespace sfx2 { namespace sidebar {

class Tools
{
public:
    static Image GetImage (
        const ::rtl::OUString& rsImageURL,
        const ::rtl::OUString& rsHighContrastImageURL,
        const cssu::Reference<css::frame::XFrame>& rxFrame);

    static Image GetImage (
        const ::rtl::OUString& rsURL,
        const cssu::Reference<css::frame::XFrame>& rxFrame);

    static css::awt::Gradient VclToAwtGradient (const Gradient aGradient);
    static Gradient AwtToVclGradient (const css::awt::Gradient aGradient);

    static SvBorder RectangleToSvBorder (const Rectangle aBox);
};


} } // end of namespace sfx2::sidebar

#endif
