/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef AUTODOC_DSP_HTML_STD_HXX
#define AUTODOC_DSP_HTML_STD_HXX



namespace ary
{
    namespace idl
    {
        class Gate;
    }
}

namespace display
{
    class CorporateFrame;
}


namespace autodoc
{

// class HtmlDisplay_Idl_Ifc

class HtmlDisplay_Idl_Ifc
{
  public:
    virtual             ~HtmlDisplay_Idl_Ifc() {}

    void                Run(
                            const char *        i_sOutputDirectory,
                            const ary::idl::Gate &
                                                i_rAryGate,
                            const display::CorporateFrame &
                                                i_rLayout );
  private:
    virtual void        do_Run(
                            const char *        i_sOutputDirectory,
                            const ary::idl::Gate &
                                                i_rAryGate,
                            const display::CorporateFrame &
                                                i_rLayout ) = 0;
};




// IMPLEMENTATION
inline void
HtmlDisplay_Idl_Ifc::Run( const char *                    i_sOutputDirectory,
                          const ary::idl::Gate &          i_rAryGate,
                          const display::CorporateFrame & i_rLayout )
{
    do_Run( i_sOutputDirectory, i_rAryGate, i_rLayout );
}




} // namespace autodoc
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
