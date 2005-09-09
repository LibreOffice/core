/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PreviewValueSet.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:42:17 $
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

#ifndef SD_TOOLPANEL_PREVIEW_VALUE_SET_HXX
#define SD_TOOLPANEL_PREVIEW_VALUE_SET_HXX

#include <svtools/valueset.hxx>


namespace sd { namespace toolpanel {
class TreeNode;
} }

namespace sd { namespace toolpanel { namespace controls {


/** Adapt the svtools valueset to the needs of the master page controlls.
*/
class PreviewValueSet
    : public ValueSet
{
public:
    PreviewValueSet (TreeNode* pParent);
    ~PreviewValueSet (void);

    void SetRightMouseClickHandler (const Link& rLink);
    virtual void Paint (const Rectangle& rRect);
    virtual void Resize (void);

    /** When a request for the display of a context menu is made to this
        method then that request is forwarded via the ContextMenuCallback.
        This way the owning class can handle the context menu without having
        to be derived from this class.
        Use SetContextMenuCallback to set or rest the handler.
    */
    virtual void Command (const CommandEvent& rEvent);

    void SetPreviewWidth (int nWidth);

    sal_Int32 GetPreferredWidth (sal_Int32 nHeight);
    sal_Int32 GetPreferredHeight (sal_Int32 nWidth);

    /** Set the number of rows and columns according to the current number
        of items.  Call this method when new items have been inserted.
    */
    void Rearrange (void);

    /** Set the callback function to which requests for context menus are
        forewarded.  Call with an empty Link to reset the callback
        function.
    */
    void SetContextMenuCallback (const Link& rLink);

protected:
    virtual void MouseButtonDown (const MouseEvent& rEvent);

private:
    Link maRightMouseClickHandler;
    Link maContextMenuCallback;
    TreeNode* mpParent;
    int mnPreviewWidth;
    const int mnBorderWidth;
    const int mnBorderHeight;

    USHORT CalculateColumnCount (int nWidth) const;
    USHORT CalculateRowCount (USHORT nColumnCount) const;
};

} } } // end of namespace ::sd::toolpanel::controls

#endif
