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
#ifndef _VIEWFAC_HXX
#define _VIEWFAC_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <tools/string.hxx>
#ifndef _RESID_HXX //autogen
#include <tools/resid.hxx>
#endif

class SfxViewFrame;
class SfxViewShell;
class Window;

typedef SfxViewShell* (*SfxViewCtor)(SfxViewFrame*, SfxViewShell*);
typedef void (*SfxViewInit)();

// CLASS -----------------------------------------------------------------
class SFX2_DLLPUBLIC SfxViewFactory
{
public:
<<<<<<< local
    SfxViewFactory( SfxViewCtor fnC, SfxViewInit fnI, USHORT nOrdinal );
=======
    SfxViewFactory( SfxViewCtor fnC, SfxViewInit fnI,
                    USHORT nOrdinal, const sal_Char* asciiViewName );
>>>>>>> other
    ~SfxViewFactory();

    SfxViewShell *CreateInstance(SfxViewFrame *pViewFrame, SfxViewShell *pOldSh);
    void          InitFactory();
<<<<<<< local
=======

>>>>>>> other
    USHORT        GetOrdinal() const { return nOrd; }

    /// returns a legacy view name. This is "view" with an appended ordinal/ID.
    String        GetLegacyViewName() const;

    /** returns a API-compatible view name.

        For details on which view names are specified, see the XModel2.getAvailableViewControllerNames
        documentation.
    */
    String        GetAPIViewName() const;

private:
    SfxViewCtor fnCreate;
    SfxViewInit fnInit;
    USHORT      nOrd;
<<<<<<< local
=======
    const String    m_sViewName;
>>>>>>> other
};

#endif

