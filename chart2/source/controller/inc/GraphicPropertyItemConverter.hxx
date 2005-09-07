/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GraphicPropertyItemConverter.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:22:50 $
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
#ifndef CHART_GRAPHICPROPERTYITEMCONVERTER_HXX
#define CHART_GRAPHICPROPERTYITEMCONVERTER_HXX

#include "ItemConverter.hxx"

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif

class SdrModel;

namespace chart
{
namespace wrapper
{

class GraphicPropertyItemConverter :
        public ::comphelper::ItemConverter
{
public:
    enum eGraphicObjectType
    {
        FILLED_DATA_POINT,
        LINE_DATA_POINT,
        LINE_PROPERTIES,
        FILL_PROPERTIES,
        LINE_AND_FILL_PROPERTIES
    };

    GraphicPropertyItemConverter(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > & rPropertySet,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        eGraphicObjectType eObjectType = FILLED_DATA_POINT );
    virtual ~GraphicPropertyItemConverter();

protected:
    virtual const USHORT * GetWhichPairs() const;
    virtual bool GetItemPropertyName( USHORT nWhichId, ::rtl::OUString & rOutName ) const;

    virtual void FillSpecialItem( USHORT nWhichId, SfxItemSet & rOutItemSet ) const
        throw( ::com::sun::star::uno::Exception );
    virtual bool ApplySpecialItem( USHORT nWhichId, const SfxItemSet & rItemSet )
        throw( ::com::sun::star::uno::Exception );

    void FillBitmapItem( USHORT nWhichId, SfxItemSet & rOutItemSet ) const
        throw( ::com::sun::star::beans::UnknownPropertyException );
    void ApplyBitmapItem( USHORT nWhichId, const SfxItemSet & rItemSet )
        throw( ::com::sun::star::beans::UnknownPropertyException );

private:
    eGraphicObjectType              m_eGraphicObjectType;
    SdrModel &                      m_rDrawModel;
};

} //  namespace wrapper
} //  namespace chart

// CHART_GRAPHICPROPERTYITEMCONVERTER_HXX
#endif
