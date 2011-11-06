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



#ifndef LAYOUT_AWT_VCLXBUTTON_HXX
#define LAYOUT_AWT_VCLXBUTTON_HXX

#include <toolkit/awt/vclxwindows.hxx>

/* Replacements for broken toolkit/ impls. of ok, cancel, help button, etc. */

namespace layoutimpl
{

class VCLXIconButton : public VCLXButton
{
public:
    VCLXIconButton( Window* p, rtl::OUString aDefaultLabel, const char *pGraphName );
    void Show ();
};

class VCLXOKButton : public VCLXIconButton
{
public:
    VCLXOKButton( Window *p );
};

class VCLXCancelButton : public VCLXIconButton
{
public:
    VCLXCancelButton( Window *p );
};

class VCLXYesButton : public VCLXIconButton
{
public:
    VCLXYesButton( Window *p );
};

class VCLXNoButton : public VCLXIconButton
{
public:
    VCLXNoButton( Window *p );
};

class VCLXRetryButton : public VCLXIconButton
{
public:
    VCLXRetryButton( Window *p );
};

class VCLXIgnoreButton : public VCLXIconButton
{
public:
    VCLXIgnoreButton( Window *p );
};

class VCLXResetButton : public VCLXIconButton
{
public:
    VCLXResetButton( Window *p );
};

class VCLXApplyButton : public VCLXIconButton
{
public:
    VCLXApplyButton( Window *p );
};

class VCLXHelpButton : public VCLXIconButton
{
public:
    VCLXHelpButton( Window *p );
};

// TODO.  Reuse vcl/Morebutton, or make AdvancedButton reuse me?
class VCLXMoreButton : public VCLXIconButton
{
public:
    VCLXMoreButton( Window *p );
};

class VCLXAdvancedButton : public VCLXIconButton
{
public:
    VCLXAdvancedButton( Window *p );
};

} // namespace layoutimpl

#endif // LAYOUT_AWT_VCLXBUTTON_HXX
