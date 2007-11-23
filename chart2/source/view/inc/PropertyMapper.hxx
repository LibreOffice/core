/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertyMapper.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 12:11:26 $
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
#ifndef _CHART2_PROPERTYMAPPER_HXX
#define _CHART2_PROPERTYMAPPER_HXX

#ifndef INCLUDED_COMPHELPER_INLINE_CONTAINER_HXX
#include <comphelper/InlineContainer.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

typedef ::std::map< ::rtl::OUString, ::rtl::OUString >            tPropertyNameMap;
typedef ::comphelper::MakeMap< ::rtl::OUString, ::rtl::OUString > tMakePropertyNameMap;

typedef ::std::map< ::rtl::OUString, ::com::sun::star::uno::Any >            tPropertyNameValueMap;
typedef ::comphelper::MakeMap< ::rtl::OUString, ::com::sun::star::uno::Any > tMakePropertyNameValueMap;

typedef ::com::sun::star::uno::Sequence< rtl::OUString > tNameSequence;
typedef ::comphelper::MakeSequence< rtl::OUString >      tMakeNameSequence;

typedef ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > tAnySequence;
typedef ::comphelper::MakeSequence< ::com::sun::star::uno::Any >      tMakeAnySequence;

class PropertyMapper
{
public:
    static void setMappedProperties(
          const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet >& xTarget
        , const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet >& xSource
        , const tPropertyNameMap& rMap
        , tPropertyNameValueMap* pOverwriteMap=0 );

    static void getValueMap(
          tPropertyNameValueMap& rValueMap
        , const tPropertyNameMap& rNameMap
        , const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet >& xSourceProp
        );

    static void getMultiPropertyLists(
                  tNameSequence& rNames
                , tAnySequence&  rValues
                , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xProp
                , const tPropertyNameMap& rMap
                );

    static void getMultiPropertyListsFromValueMap(
                  tNameSequence& rNames
                , tAnySequence&  rValues
                , const tPropertyNameValueMap& rValueMap
                );

    static ::com::sun::star::uno::Any*
                getValuePointer( tAnySequence& rPropValues
                         , const tNameSequence& rPropNames
                         , const rtl::OUString& rPropName );

    static void setMultiProperties(
                  const tNameSequence& rNames
                , const tAnySequence&  rValues
                , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet >& xTarget );

    static const tMakePropertyNameMap& getPropertyNameMapForCharacterProperties();
    static const tMakePropertyNameMap& getPropertyNameMapForParagraphProperties();
    static const tMakePropertyNameMap& getPropertyNameMapForFillProperties();
    static const tMakePropertyNameMap& getPropertyNameMapForLineProperties();
    static const tMakePropertyNameMap& getPropertyNameMapForFillAndLineProperties();
    static const tMakePropertyNameMap& getPropertyNameMapForTextShapeProperties();

    static const tMakePropertyNameMap& getPropertyNameMapForFilledSeriesProperties();
    static const tMakePropertyNameMap& getPropertyNameMapForLineSeriesProperties();

    static void getTextLabelMultiPropertyLists(
                const ::com::sun::star::uno::Reference<
                      ::com::sun::star::beans::XPropertySet >& xSourceProp
                , tNameSequence& rPropNames, tAnySequence& rPropValues
                , bool bName=true
                , sal_Int32 nLimitedSpace=-1
                , bool bLimitedHeight=false );

    /** adds line-, fill- and character properties and sets some suitable
        defaults for auto-grow properties
     */
    static void getPreparedTextShapePropertyLists(
        const ::com::sun::star::uno::Reference<
              ::com::sun::star::beans::XPropertySet >& xSourceProp
        , tNameSequence& rPropNames
        , tAnySequence& rPropValues );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
