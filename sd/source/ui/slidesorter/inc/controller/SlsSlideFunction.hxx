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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CONTROLLER_SLSSLIDEFUNCTION_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CONTROLLER_SLSSLIDEFUNCTION_HXX

#include "fupoor.hxx"

namespace sd { namespace slidesorter {
class SlideSorter;
} }


namespace sd { namespace slidesorter { namespace controller {

/** Base class for functions of the slide sorter.
*/
class SlideFunction
    : public FuPoor
{
public:
    TYPEINFO_OVERRIDE();

    virtual sal_Bool MouseMove (const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonUp (const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonDown (const MouseEvent& rMEvt);

protected:
    SlideFunction (
        SlideSorter& rSlideSorter,
        SfxRequest& rRequest);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
