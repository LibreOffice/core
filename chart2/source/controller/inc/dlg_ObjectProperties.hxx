/*************************************************************************
 *
 *  $RCSfile: dlg_ObjectProperties.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CHART2_DLG_OBJECTPROPERTIES_HXX
#define _CHART2_DLG_OBJECTPROPERTIES_HXX

#include "chartview/ObjectIdentifier.hxx"

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
// header for typedef ChangeType
#ifndef _SVX_TAB_AREA_HXX
#include <svx/tabarea.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

    /*
enum AttrType
{
    ATTR_TITLE,
    ATTR_LEGEND,
    ATTR_DATA_ROW,
    ATTR_DATA_LINE,
    ATTR_DATA_POINT,
    ATTR_AXIS,
    ATTR_X_AXIS_2D,
    ATTR_X_AXIS_3D,
    ATTR_Y_AXIS_2D,
    ATTR_Y_AXIS_3D,
    ATTR_Z_AXIS,
    ATTR_GRID,
    ATTR_DIAGRAM_AREA,
    ATTR_DIAGRAM_WALL,
    ATTR_DIAGRAM_FLOOR,
    ATTR_LINE,
    ATTR_DIAGRAM_STOCK_LOSS,
    ATTR_DIAGRAM_STOCK_PLUS
};
*/

class OldModelWrapper
{
public:
    OldModelWrapper();
    virtual ~OldModelWrapper();

    //model
    //// BOOL                IsXYChart()         const;
    BOOL                HasScaleProperties() const;
    BOOL                IsBar()             const;
    BOOL                Is3DChart()         const;
    //// BOOL                Is3DPie()           const;
    BOOL                IsStatisticChart()  const;
    BOOL                HasAreaProperties() const;

    /// return TRUE if the current chart type supports a given axis type
    BOOL                ProvidesSecondaryYAxis() const;//was BOOL CanAxis(CHART_AXIS_SECONDARY_Y)
    BOOL                IsCol( long nRow ) const;
};

/*
#define CHATTR_COUNT    (CHATTR_DIAGRAM_FLOOR + 1)

enum OrderMode
{
    CHORDMODE_NONE,
    CHORDMODE_X_AXIS,
    CHORDMODE_Y_AXIS
};

#define CHORDMODE_COUNT (CHORDMODE_Y_AXIS + 1)

#define AxisTypeX 1
#define AxisTypeY 2
#define AxisTypeZ 3
*/

/*************************************************************************
|*
|* Attributs-Tab-Dialog
|*
\************************************************************************/

class ViewElementListProvider;

class SchAttribTabDlg : public SfxTabDialog
{
private:
    ObjectType               eObjectType;
    bool                     bAffectsMultipleObjects;//is true if more than one object of the given type will be changed (e.g. all axes or all titles)
    USHORT                   nDlgType;
    USHORT                   nPageType;

    const OldModelWrapper * const        m_pModelWrapper;
    const ViewElementListProvider* const m_pViewElementListProvider;

    const SfxItemSet*   mpSymbolAttr;
    Graphic             maSymbolGraphic;

    /*
    XColorTable*    pColorTab;
    XGradientList*  pGradientList;
    XHatchList*     pHatchingList;
    XBitmapList*    pBitmapList;
    XDashList*      pDashList;
    XLineEndList*   pLineEndList;
    */

    ChangeType      nColorTableState;
    ChangeType      nGradientListState;
    ChangeType      nHatchingListState;
    ChangeType      nBitmapListState;

    static USHORT GetResId(ObjectType eObjectType);
    ////static USHORT GetResId(AttrType eType);
    virtual void PageCreated(USHORT nId, SfxTabPage& rPage);

public:
    SchAttribTabDlg(Window* pParent, ObjectType eType,
                    const SfxItemSet* pAttr,
                    const ViewElementListProvider* pViewElementListProvider,
                    const OldModelWrapper* pModel,
                    bool  bAffectsMultipleObjects = false,
                    const SfxItemSet* pSymbolAttr=NULL,
                    Graphic aSymbolGraphic=Graphic());
    virtual ~SchAttribTabDlg();
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif

