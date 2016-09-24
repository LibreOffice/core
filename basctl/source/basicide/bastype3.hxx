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
#ifndef INCLUDED_BASCTL_SOURCE_BASICIDE_BASTYPE3_HXX
#define INCLUDED_BASCTL_SOURCE_BASICIDE_BASTYPE3_HXX

#include <svtools/svmedit.hxx>
#include <iderid.hxx>
#include <vcl/accel.hxx>

namespace basctl
{

class ExtendedEdit : public Edit
{
private:
    Accelerator               aAcc;
    Link<Accelerator&,void>   aAccHdl;
    Link<ExtendedEdit*,void>  aLoseFocusHdl;

protected:
    DECL_LINK_TYPED( EditAccHdl, Accelerator&, void );
    DECL_LINK_TYPED( ImplGetFocusHdl, Control&, void );
    DECL_LINK_TYPED( ImplLoseFocusHdl, Control&, void );

public:
                    ExtendedEdit( vcl::Window* pParent, IDEResId nRes );

    void            SetAccHdl( const Link<Accelerator&,void>& rLink )         { aAccHdl = rLink; }
    Accelerator&    GetAccelerator()                                          { return aAcc; }
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_BASICIDE_BASTYPE3_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
