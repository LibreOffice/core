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

#ifndef INCLUDED_SD_SOURCE_UI_SIDEBAR_PREVIEWVALUESET_HXX
#define INCLUDED_SD_SOURCE_UI_SIDEBAR_PREVIEWVALUESET_HXX

#include <svtools/valueset.hxx>

namespace sd { namespace sidebar {

/** Adapt the svtools valueset to the needs of the master page controls.
*/
class PreviewValueSet
    : public ValueSet
{
public:
    explicit PreviewValueSet (vcl::Window* pParent);
    virtual ~PreviewValueSet();

    void SetRightMouseClickHandler (const Link<const MouseEvent&,void>& rLink);
    virtual void Resize() override;

    void SetPreviewSize (const Size& rSize);

    sal_Int32 GetPreferredHeight (sal_Int32 nWidth);

    /** Set the number of rows and columns according to the current number
        of items.  Call this method when new items have been inserted.
    */
    void Rearrange (bool bForceRequestResize = false);

protected:
    virtual void MouseButtonDown (const MouseEvent& rEvent) override;

private:
    Link<const MouseEvent&,void> maRightMouseClickHandler;
    Size maPreviewSize;
    const int mnBorderWidth;
    const int mnBorderHeight;
    const int mnMaxColumnCount;

    sal_uInt16 CalculateColumnCount (int nWidth) const;
    sal_uInt16 CalculateRowCount (sal_uInt16 nColumnCount) const;
};

} } // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
