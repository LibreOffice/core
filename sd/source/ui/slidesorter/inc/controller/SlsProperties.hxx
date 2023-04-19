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

#pragma once

#include <tools/color.hxx>

namespace sd::slidesorter::controller
{
/** An extensible set of properties used throughout the slide sorter.
*/
class Properties
{
public:
    Properties();

    /** Call this method after receiving a VclEventId::ApplicationDataChanged
        event.
    */
    void HandleDataChangeEvent();

    /** Return the background color.
    */
    const Color& GetBackgroundColor() const { return maBackgroundColor; }
    void SetBackgroundColor(const Color& rColor);

    /** Return the text color.
    */
    void SetTextColor(const Color& rColor);

    /** Return the color in which selections are to be painted.
    */
    const Color& GetSelectionColor() const { return maSelectionColor; }
    void SetSelectionColor(const Color& rColor);

    /** Return the color used for highlighting e.g. the current slide.
    */
    void SetHighlightColor(const Color& rColor);

    /** The UI can be set to be read only independently from the model status.
        Used for instance in the presenter view.
    */
    bool IsUIReadOnly() const { return mbIsUIReadOnly; }

private:
    Color maBackgroundColor;
    Color maTextColor;
    Color maSelectionColor;
    Color maHighlightColor;
    bool mbIsUIReadOnly;
};

} // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
