/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SimpleLayoutWrapper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 18:41:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    virtual int GetPreferredWidth (int nHeight);
    virtual int GetPreferredHeight (int nWidth);
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
