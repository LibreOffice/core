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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_DEBUG_COLORPANEL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_DEBUG_COLORPANEL_HXX

#include <svtools/valueset.hxx>
#include <vcl/window.hxx>


namespace svx { namespace sidebar {


/** This demo panel shows the colors that are available from the
    StyleSettings.
*/
class ColorPanel
    : public ValueSet
{
public:
    ColorPanel (vcl::Window* pParent);
    virtual ~ColorPanel (void);

    // From vcl::Window
    virtual void Resize (void) SAL_OVERRIDE;
    virtual void DataChanged (const DataChangedEvent& rEvent) SAL_OVERRIDE;

private:
    /** Depending on the given number of columns and the item size
        calculate the number of rows that are necessary to display all
        items.
    */
    int CalculateRowCount (const Size& rItemSize, int nColumnCount);
    void Fill (void);
};

} } // end of namespace ::svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
