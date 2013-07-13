/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _CHART2_PROPERTYMAPPER_HXX
#define _CHART2_PROPERTYMAPPER_HXX

#include <comphelper/InlineContainer.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XShape.hpp>

namespace chart
{

/**
*/

typedef ::std::map< OUString, OUString >            tPropertyNameMap;
typedef ::comphelper::MakeMap< OUString, OUString > tMakePropertyNameMap;

typedef ::std::map< OUString, ::com::sun::star::uno::Any >            tPropertyNameValueMap;
typedef ::comphelper::MakeMap< OUString, ::com::sun::star::uno::Any > tMakePropertyNameValueMap;

typedef ::com::sun::star::uno::Sequence< OUString > tNameSequence;
typedef ::comphelper::MakeSequence< OUString >      tMakeNameSequence;

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
                         , const OUString& rPropName );

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

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
