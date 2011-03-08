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

#ifndef _CHART2_OBJECTNAME_PROVIDER_HXX
#define _CHART2_OBJECTNAME_PROVIDER_HXX

#include "ObjectIdentifier.hxx"
#include "TitleHelper.hxx"
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/** Provides localized ui strings for the userinterface.
*/

class ObjectNameProvider
{
public:
    static rtl::OUString getName( ObjectType eObjectType, bool bPlural=false );
    static rtl::OUString getAxisName( const rtl::OUString& rObjectCID
                        , const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XModel >& xChartModel  );
    static rtl::OUString getGridName( const rtl::OUString& rObjectCID
                        , const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XModel >& xChartModel );
    static rtl::OUString getTitleName( const rtl::OUString& rObjectCID
                        , const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XModel >& xChartModel );
    static rtl::OUString getTitleNameByType( TitleHelper::eTitleType eType );

    static rtl::OUString getNameForCID(
        const rtl::OUString& rObjectCID,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument >& xChartDocument );

    static rtl::OUString getName_ObjectForSeries(
        ObjectType eObjectType,
        const rtl::OUString& rSeriesCID,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument >& xChartDocument );
    static rtl::OUString getName_ObjectForAllSeries( ObjectType eObjectType );

    /** Provides help texts for the various chart elements.
        The parameter rObjectCID has to be a ClassifiedIdentifier - see class ObjectIdentifier.
     */
    static rtl::OUString getHelpText( const rtl::OUString& rObjectCID, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel, bool bVerbose=false );
    static rtl::OUString getHelpText( const rtl::OUString& rObjectCID, const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& xChartDocument, bool bVerbose=false );

    /** This is used for showing the currently selected object in the status bar
        (command "Context")
     */
    static rtl::OUString getSelectedObjectText( const rtl::OUString & rObjectCID, const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& xChartDocument );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
