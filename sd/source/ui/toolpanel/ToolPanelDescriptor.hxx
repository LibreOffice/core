/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ToolPanelDescriptor.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:37:05 $
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

#ifndef SD_TOOL_PANEL_DESCRIPTOR_HXX
#define SD_TOOL_PANEL_DESCRIPTOR_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#include <memory>

class Window;

namespace sd { namespace toolpanel {


/** Collection of information the describes entries of the tool
    panel.  A descriptor owns the control it is associated with.
*/
class ToolPanelDescriptor
{
public:
    /** Create a new descriptor for the given control.
        @param pControl
    */
    ToolPanelDescriptor (::std::auto_ptr< ::Window> pControl,
        const String& rTitle);

    ~ToolPanelDescriptor (void);

    /** Return the height of the title bar.
        @return
            The title bar height is returned in pixels.
    */
    int GetTitleBarHeight (void) const;


    void SetPositionAndSize (const Point& aPosition,
        const Size& aSize);
    void SetPositionAndSize (const Rectangle& aBox);
    void SetWeight (double nWeight);

    Window* GetControl (void) const;
    const String& GetTitle (void) const;
    const Rectangle& GetTitleBox (void) const;
    Rectangle GetPositionAndSize (void) const;
    double GetWeight (void) const;

    int GetTotalHeight (void) const;
    int GetWindowHeight (void) const;

private:
    ::std::auto_ptr< ::Window> mpControl;
    String msTitle;
    Rectangle maTitleBox;
    double mnWeight;
    int mnTotalHeight;

    /// Do not use! Assignment operator is not supported.
    const ToolPanelDescriptor& operator= (
        const ToolPanelDescriptor& aDescriptor);
};

} } // end of namespace ::sd::toolpanel

#endif
