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

#ifndef SD_GRAPHIC_VIEW_SHELL_BASE_HXX
#define SD_GRAPHIC_VIEW_SHELL_BASE_HXX

#include "ViewShellBase.hxx"


namespace sd {

/** This class exists to be able to register another factory that
    creates the view shell for the Draw application.
*/
class GraphicViewShellBase
    : public ViewShellBase
{
public:
    TYPEINFO();
    SFX_DECL_VIEWFACTORY(GraphicViewShellBase);

    /** This constructor is used by the view factory of the SFX
        macros.
    */
    GraphicViewShellBase (SfxViewFrame *pFrame, SfxViewShell* pOldShell);
    virtual ~GraphicViewShellBase (void);

    /** Callback function for general slot calls.
    */
    virtual void Execute (SfxRequest& rRequest);

protected:
    virtual void InitializeFramework (void);
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
