/*************************************************************************
 *
 *  $RCSfile: dlg_ObjectProperties.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: iha $ $Date: 2003-11-13 15:17:07 $
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

class ObjectPropertiesDialogParameter
{
public:
    ObjectPropertiesDialogParameter( const rtl::OUString& rObjectCID );
    virtual ~ObjectPropertiesDialogParameter();

    void        init( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel );
    ObjectType  getObjectType() const;

    bool HasGeometryProperties() const;
    bool HasStatisticProperties() const;
    bool ProvidesSecondaryYAxis() const;
    bool HasAreaProperties() const;
    bool HasLineProperties() const;
    bool HasSymbolProperties() const;
    bool HasScaleProperties() const;
    bool CanAxisLabelsBeStaggered() const;

private:
    rtl::OUString   m_aObjectCID;
    ObjectType      m_eObjectType;
    bool m_bAffectsMultipleObjects;//is true if more than one object of the given type will be changed (e.g. all axes or all titles)

    bool m_bHasGeometryProperties;
    bool m_bHasStatisticProperties;
    bool m_bProvidesSecondaryYAxis;
    bool m_bHasAreaProperties;
    bool m_bHasLineProperties;
    bool m_bHasSymbolProperties;
    bool m_bHasScaleProperties;
    bool m_bCanAxisLabelsBeStaggered;
};

/*************************************************************************
|*
|* dialog for properties of different chart object
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

    const ObjectPropertiesDialogParameter * const        m_pParameter;
    const ViewElementListProvider* const                 m_pViewElementListProvider;

    SfxItemSet*     m_pSymbolShapeProperties;
    Graphic*        m_pAutoSymbolGraphic;

    ChangeType      nColorTableState;
    ChangeType      nGradientListState;
    ChangeType      nHatchingListState;
    ChangeType      nBitmapListState;

    static USHORT GetResId(ObjectType eObjectType);
    virtual void PageCreated(USHORT nId, SfxTabPage& rPage);

public:
    SchAttribTabDlg(Window* pParent, const SfxItemSet* pAttr,
                    const ObjectPropertiesDialogParameter* pDialogParameter,
                    const ViewElementListProvider* pViewElementListProvider );
    virtual ~SchAttribTabDlg();

    //pSymbolShapeProperties: Properties to be set on the symbollist shapes
    //pAutoSymbolGraphic: Graphic to be shown if AutoSymbol gets selected
    //this class takes ownership over both parameter
    void setSymbolInformation( SfxItemSet* pSymbolShapeProperties, Graphic* pAutoSymbolGraphic );
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif

