/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableDesignPanel.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:49:29 $
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

#ifndef SD_TOOLPANEL_CONTROLS_TABLE_DESIGN_PANEL_HXX
#define SD_TOOLPANEL_CONTROLS_TABLE_DESIGN_PANEL_HXX

#include "taskpane/SubToolPanel.hxx"

namespace sd {
class ViewShellBase;
}

namespace sd { namespace toolpanel {
class TreeNode;
class ControlFactory;
} }

namespace sd { namespace toolpanel { namespace controls {

class TableDesignPanel
    : public SubToolPanel
{
public:
    TableDesignPanel (
        TreeNode* pParent,
        ViewShellBase& rBase);
    virtual ~TableDesignPanel (void);

    static std::auto_ptr<ControlFactory> CreateControlFactory (ViewShellBase& rBase);

    virtual Size GetPreferredSize (void);
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeigh);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual ::Window* GetWindow (void);
    virtual bool IsResizable (void);
    virtual bool IsExpandable (void) const;

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > CreateAccessibleObject (
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent);

    using Window::GetWindow;
private:
    Size maPreferredSize;
    ::Window* mpWrappedControl;
};

} } } // end of namespace ::sd::toolpanel::controls

#endif
