/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _CHART2_PROPERTYMAPPER_HXX
#define _CHART2_PROPERTYMAPPER_HXX

#include <comphelper/InlineContainer.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XShape.hpp>

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

    static ::com::sun::star::uno::Any*
                getValuePointerForLimitedSpace( tAnySequence& rPropValues
                         , const tNameSequence& rPropNames
                         , bool bLimitedHeight );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
