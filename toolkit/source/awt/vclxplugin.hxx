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



#ifndef LAYOUT_AWT_VCLXPLUGIN_HXX
#define LAYOUT_AWT_VCLXPLUGIN_HXX

#include <toolkit/awt/vclxwindow.hxx>
#include <tools/wintypes.hxx>

class Control;
namespace layoutimpl
{

namespace css = ::com::sun::star;

class VCLXPlugin : public VCLXWindow
{
public:
    Window *mpWindow;
    ::Control *mpPlugin;
    WinBits mStyle;

    VCLXPlugin( Window *p, WinBits b );

    void SetPlugin( ::Control *p );

protected:
    ~VCLXPlugin();

    // XComponent
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Size SAL_CALL getMinimumSize()
        throw(::com::sun::star::uno::RuntimeException);

private:
    VCLXPlugin( VCLXPlugin const & );
    VCLXPlugin& operator=( VCLXPlugin const & );
};

} // namespace layoutimpl

#endif /* LAYOUT_AWT_VCLXPLUGIN_HXX */
