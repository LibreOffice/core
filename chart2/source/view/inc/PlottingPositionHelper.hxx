/*************************************************************************
 *
 *  $RCSfile: PlottingPositionHelper.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:33 $
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
#ifndef _CHART2_PLOTTINGPOSITIONHELPER_HXX
#define _CHART2_PLOTTINGPOSITIONHELPER_HXX

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_EXPLICITSCALEDATA_HPP_
#include <drafts/com/sun/star/chart2/ExplicitScaleData.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XTRANSFORMATION_HPP_
#include <drafts/com/sun/star/chart2/XTransformation.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX_HPP_
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#endif

#ifndef _B3D_HMATRIX_HXX
#include <goodies/hmatrix.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class PlottingPositionHelper
{
public:
    PlottingPositionHelper();
    virtual ~PlottingPositionHelper();

    void setTransformationSceneToScreen( const ::com::sun::star::drawing::HomogenMatrix& rMatrix);

    void setScales( const ::com::sun::star::uno::Sequence<
            ::drafts::com::sun::star::chart2::ExplicitScaleData >& rScales );

    inline bool   isLogicVisible( double fX, double fY, double fZ ) const;
    inline void   doLogicScaling( double* pX, double* pY, double* pZ ) const;
    inline void   clipLogicValues( double* pX, double* pY, double* pZ ) const;

    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XTransformation >
                  getTransformationLogicToScene() const;

    inline double getLogicMinX() const;
    inline double getLogicMinY() const;
    inline double getLogicMinZ() const;
    inline double getLogicMaxX() const;
    inline double getLogicMaxY() const;
    inline double getLogicMaxZ() const;

    void getLogicMinimum( ::com::sun::star::uno::Sequence< double >& rSeq ) const;
    void getLogicMaximum( ::com::sun::star::uno::Sequence< double >& rSeq ) const;

    void getScreenValuesForMinimum( ::com::sun::star::uno::Sequence< double >& rSeq ) const;
    void getScreenValuesForMaximum( ::com::sun::star::uno::Sequence< double >& rSeq ) const;

protected: //member
    ::com::sun::star::uno::Sequence<
            ::drafts::com::sun::star::chart2::ExplicitScaleData > m_aScales;
    Matrix4D                                                      m_aMatrixScreenToScene;

    //this is calculated based on m_aScales and m_aMatrixScreenToScene
    mutable ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XTransformation >     m_xTransformationLogicToScene;
};

bool PlottingPositionHelper::isLogicVisible(
    double fX, double fY, double fZ ) const
{
    return fX >= m_aScales[0].Minimum && fX <= m_aScales[0].Maximum
        && fY >= m_aScales[1].Minimum && fY <= m_aScales[1].Maximum
        && fZ >= m_aScales[2].Minimum && fZ <= m_aScales[2].Maximum;
}

void PlottingPositionHelper::doLogicScaling( double* pX, double* pY, double* pZ ) const
{
    if(pX && m_aScales[0].Scaling.is())
        *pX = m_aScales[0].Scaling->doScaling(*pX);
    if(pY && m_aScales[1].Scaling.is())
        *pY = m_aScales[1].Scaling->doScaling(*pY);
    if(pZ && m_aScales[2].Scaling.is())
        *pZ = m_aScales[2].Scaling->doScaling(*pZ);
}

void PlottingPositionHelper::clipLogicValues( double* pX, double* pY, double* pZ ) const
{
    if(pX)
    {
        if( *pX < m_aScales[0].Minimum )
            *pX = m_aScales[0].Minimum;
        else if( *pX > m_aScales[0].Maximum )
            *pX = m_aScales[0].Maximum;
    }
    if(pY)
    {
        if( *pY < m_aScales[1].Minimum )
            *pY = m_aScales[1].Minimum;
        else if( *pY > m_aScales[1].Maximum )
            *pY = m_aScales[1].Maximum;
    }
    if(pZ)
    {
        if( *pZ < m_aScales[2].Minimum )
            *pZ = m_aScales[2].Minimum;
        else if( *pZ > m_aScales[2].Maximum )
            *pZ = m_aScales[2].Maximum;
    }
}

inline double PlottingPositionHelper::getLogicMinX() const
{
    return m_aScales[0].Minimum;
}
inline double PlottingPositionHelper::getLogicMinY() const
{
    return m_aScales[1].Minimum;
}
inline double PlottingPositionHelper::getLogicMinZ() const
{
    return m_aScales[2].Minimum;
}

inline double PlottingPositionHelper::getLogicMaxX() const
{
    return m_aScales[0].Maximum;
}
inline double PlottingPositionHelper::getLogicMaxY() const
{
    return m_aScales[1].Maximum;
}
inline double PlottingPositionHelper::getLogicMaxZ() const
{
    return m_aScales[2].Maximum;
}

//.............................................................................
} //namespace chart
//.............................................................................
#endif
