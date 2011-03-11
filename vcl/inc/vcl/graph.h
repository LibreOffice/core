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

#ifndef _SV_GRAPH_H
#define _SV_GRAPH_H

#include <vcl/dllapi.h>
#include <tools/string.hxx>
#include <vcl/sv.h>

// ---------------
// - GraphicType -
// ---------------

enum GraphicType
{
    GRAPHIC_NONE,
    GRAPHIC_BITMAP,
    GRAPHIC_GDIMETAFILE,
    GRAPHIC_DEFAULT
};

// -----------------
// - GraphicReader -
// -----------------

class ReaderData;

class VCL_DLLPUBLIC GraphicReader
{
protected:

    String          maUpperName;
    ReaderData*     mpReaderData;
    sal_Bool            mbIsReading;

                    GraphicReader() :
                        mpReaderData( NULL ),
                        mbIsReading( sal_False ) {}

public:

    virtual         ~GraphicReader();

    const String&   GetUpperFilterName() const { return maUpperName; }
    ReaderData*     GetReaderData() const { return mpReaderData; }
    sal_Bool            IsReading() const { return mbIsReading; }

    // TODO: when incompatible changes are possible again
    // the preview size hint should be redone
    sal_Bool            IsPreviewModeEnabled() const;
    void            DisablePreviewMode();
    void            SetPreviewSize( const Size& );
    Size            GetPreviewSize() const;
};

#endif // _SV_GRAPH_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
