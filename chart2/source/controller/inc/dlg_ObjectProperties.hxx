/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_ObjectProperties.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:55:10 $
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
#ifndef _CHART2_DLG_OBJECTPROPERTIES_HXX
#define _CHART2_DLG_OBJECTPROPERTIES_HXX

#include "ObjectIdentifier.hxx"

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
// header for typedef ChangeType
#ifndef _SVX_TAB_AREA_HXX
#include <svx/tabarea.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
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

    void            init( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel );
    ObjectType      getObjectType() const;
    rtl::OUString   getLocalizedName() const;

    bool HasGeometryProperties() const;
    bool HasStatisticProperties() const;
    bool HasRegressionProperties() const;
    bool ProvidesSecondaryYAxis() const;
    bool ProvidesOverlapAndGapWidth() const;
    bool HasAreaProperties() const;
    bool HasLineProperties() const;
    bool HasSymbolProperties() const;
    bool HasScaleProperties() const;
    bool CanAxisLabelsBeStaggered() const;
    bool HasNumberProperties() const;
    bool ProvidesStartingAngle() const;

private:
    rtl::OUString   m_aObjectCID;
    ObjectType      m_eObjectType;
    bool m_bAffectsMultipleObjects;//is true if more than one object of the given type will be changed (e.g. all axes or all titles)

    rtl::OUString   m_aLocalizedName;

    bool m_bHasGeometryProperties;
    bool m_bHasStatisticProperties;
    bool m_bHasRegressionProperties;
    bool m_bProvidesSecondaryYAxis;
    bool m_bProvidesOverlapAndGapWidth;
    bool m_bHasAreaProperties;
    bool m_bHasLineProperties;
    bool m_bHasSymbolProperties;
    bool m_bHasScaleProperties;
    bool m_bCanAxisLabelsBeStaggered;
    bool m_bHasNumberProperties;
    bool m_bProvidesStartingAngle;
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
    SvNumberFormatter* m_pNumberFormatter;

    SfxItemSet*     m_pSymbolShapeProperties;
    Graphic*        m_pAutoSymbolGraphic;

    double          m_fAxisMinorStepWidthForErrorBarDecimals;
    bool            m_bOKPressed;

    virtual void PageCreated(USHORT nId, SfxTabPage& rPage);

    Link m_aOriginalOKClickHdl;
    DECL_LINK( OKPressed, void * );

public:
    SchAttribTabDlg(Window* pParent, const SfxItemSet* pAttr,
                    const ObjectPropertiesDialogParameter* pDialogParameter,
                    const ViewElementListProvider* pViewElementListProvider,
                    const ::com::sun::star::uno::Reference<
                            ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );
    virtual ~SchAttribTabDlg();

    //pSymbolShapeProperties: Properties to be set on the symbollist shapes
    //pAutoSymbolGraphic: Graphic to be shown if AutoSymbol gets selected
    //this class takes ownership over both parameter
    void setSymbolInformation( SfxItemSet* pSymbolShapeProperties, Graphic* pAutoSymbolGraphic );

    void SetAxisMinorStepWidthForErrorBarDecimals( double fMinorStepWidth );

    bool DialogWasClosedWithOK() const;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif

