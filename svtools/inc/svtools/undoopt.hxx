/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: undoopt.hxx,v $
 * $Revision: 1.3 $
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
#ifndef INCLUDED_SVTOOLS_UNDOOPT_HXX
#define INCLUDED_SVTOOLS_UNDOOPT_HXX

#include "svtools/svtdllapi.h"
#include <sal/types.h>
#include <svtools/brdcst.hxx>
#include <svtools/lstner.hxx>
#include <svtools/options.hxx>

class SvtUndoOptions_Impl;

class SVT_DLLPUBLIC SvtUndoOptions:
    public svt::detail::Options, public SfxBroadcaster, public SfxListener
{
    SvtUndoOptions_Impl*    pImp;

public:
                            SvtUndoOptions();
                            virtual ~SvtUndoOptions();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    void                    SetUndoCount( sal_Int32 n );
    sal_Int32               GetUndoCount() const;
};

#endif

