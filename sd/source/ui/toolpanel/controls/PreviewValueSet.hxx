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

    void SetPreviewSize (const Size& rSize);

    sal_Int32 GetPreferredWidth (sal_Int32 nHeight);
    sal_Int32 GetPreferredHeight (sal_Int32 nWidth);

    /** Set the number of rows and columns according to the current number
        of items.  Call this method when new items have been inserted.
    */
    void Rearrange (bool bForceRequestResize = false);

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
    Size maPreviewSize;
    const int mnBorderWidth;
    const int mnBorderHeight;
    const int mnMaxColumnCount;

    sal_uInt16 CalculateColumnCount (int nWidth) const;
    sal_uInt16 CalculateRowCount (sal_uInt16 nColumnCount) const;
};

} } } // end of namespace ::sd::toolpanel::controls

#endif
