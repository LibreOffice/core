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

#ifndef SD_DEBUG_DIALOG_HXX
#define SD_DEBUG_DIALOG_HXX

#include <boost/noncopyable.hpp>
#include <vector>

class WorkWindow;
class Window;

namespace sd { namespace slidesorter {

class SlideSorter;
class DebugControl;

class SlideSorterDebugDialog
    : public ::boost::noncopyable
{
public:
    static SlideSorterDebugDialog* CreateDebugDialog (SlideSorter& rSlideSorter);
    ~SlideSorterDebugDialog (void);

private:
    WorkWindow* mpTopLevelWindow;
    ::std::vector<DebugControl*> maControls;

    SlideSorterDebugDialog (SlideSorter& rSlideSorter);
    void DeleteWindow (::Window* pWindow);
};


} } // end of namespace ::sd::slidesorter

#endif
