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

#ifndef _SV_GRAPH_H
#define _SV_GRAPH_H

#include <vcl/dllapi.h>
#include <tools/string.hxx>
#include <tools/solar.h>

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

    OUString        maUpperName;
    ReaderData*     mpReaderData;
    sal_Bool            mbIsReading;

                    GraphicReader() :
                        mpReaderData( NULL ),
                        mbIsReading( sal_False ) {}

public:

    virtual         ~GraphicReader();

    const OUString&   GetUpperFilterName() const { return maUpperName; }
    ReaderData*     GetReaderData() const { return mpReaderData; }
    sal_Bool            IsReading() const { return mbIsReading; }

    // TODO: when incompatible changes are possible again
    // the preview size hint should be redone
    void            DisablePreviewMode();
    void            SetPreviewSize( const Size& );
    Size            GetPreviewSize() const;
};

#endif // _SV_GRAPH_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
