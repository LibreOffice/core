/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dlg_ShapeFont.hxx,v $
 * $Revision: 1.0 $
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
#ifndef CHART2_DLG_SHAPEFONT_HXX
#define CHART2_DLG_SHAPEFONT_HXX

#include <sfx2/tabdlg.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

class ViewElementListProvider;

/** dialog for font properties of shapes
 */
class ShapeFontDialog : public SfxTabDialog
{
public:
    ShapeFontDialog( Window* pParent, const SfxItemSet* pAttr,
        const ViewElementListProvider* pViewElementListProvider );
    virtual ~ShapeFontDialog();

private:
    virtual void PageCreated( USHORT nId, SfxTabPage& rPage );

    const ViewElementListProvider* m_pViewElementListProvider;
};

//.............................................................................
} //  namespace chart
//.............................................................................

// CHART2_DLG_SHAPEFONT_HXX
#endif
