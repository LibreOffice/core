/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SimpleLayoutWrapper.hxx,v $
 * $Revision: 1.7 $
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

#ifndef SD_TASKPANE_SIMPLE_LAYOUT_WRAPPER_HXX
#define SD_TASKPANE_SIMPLE_LAYOUT_WRAPPER_HXX

#include "taskpane/ILayoutableWindow.hxx"

#include <memory>

namespace sd { namespace toolpanel {


/** Simple wrapper arround a regular VCL window that implements the
    methods that return the preferred size of the window.  They are
    given once to the constructor and are not modified by the wrapper.
*/
class SimpleLayoutWrapper
    : public virtual ILayoutableWindow
{
public:
    SimpleLayoutWrapper (
        ::std::auto_ptr< ::Window> pWindow,
        int nPreferredWidth,
        int nPreferredHeight,
        bool bIsResizable);
    virtual ~SimpleLayoutWrapper (void);

    virtual Size GetPreferredSize (void);
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeight);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual bool IsResizable (void);
    virtual ::Window* GetWindow (void);

    using sd::toolpanel::ILayoutableWindow::GetPreferredWidth;
    using sd::toolpanel::ILayoutableWindow::GetPreferredHeight;
private:
    ::std::auto_ptr< ::Window> mpWindow;
    int mnPreferredWidth;
    int mnPreferredHeight;
    bool mbIsResizable;
};

} } // end of namespace ::sd::toolpanel

#endif
