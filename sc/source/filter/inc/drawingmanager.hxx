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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_DRAWINGMANAGER_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_DRAWINGMANAGER_HXX

#include "drawingbase.hxx"

namespace com { namespace sun { namespace star {
    namespace drawing { class XDrawPage; }
    namespace drawing { class XShape; }
    namespace drawing { class XShapes; }
} } }

namespace oox { namespace drawingml { class ShapePropertyMap; } }

namespace oox {
namespace xls {

const sal_uInt16 BIFF_OBJ_INVALID_ID        = 0;

// BIFF drawing objects
class BiffDrawingBase;

// BIFF drawing page

/** Base class for a container for all objects on a drawing page (in a
    spreadsheet or in an embedded chart object).

    For BIFF import, it is needed to load all drawing objects before converting
    them to UNO shapes. There might be some dummy drawing objects (e.g. the
    dropdown buttons of autofilters) which have to be skipped. The information,
    that a drawing object is a dummy object, may be located after the drawing
    objects themselves.

    The BIFF8 format stores drawing objects in the DFF stream (stored
    fragmented in MSODRAWING records), and in the OBJ records. The DFF stream
    fragments are collected in a single stream, and the complete stream will be
    processed afterwards.
 */
class BiffDrawingBase : public WorksheetHelper
{
public:
    explicit            BiffDrawingBase( const WorksheetHelper& rHelper,
                            const css::uno::Reference< css::drawing::XDrawPage >& rxDrawPage );

    /** Final processing after import of the all drawing objects. */
    void                finalizeImport();

private:
    css::uno::Reference< css::drawing::XDrawPage >
                        mxDrawPage;         /// UNO draw page used to insert the shapes.
};

/** Drawing manager of a single sheet. */
class BiffSheetDrawing : public BiffDrawingBase
{
public:
    explicit            BiffSheetDrawing( const WorksheetHelper& rHelper );

};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
