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

#pragma once

#include <ObjectIdentifier.hxx>
#include <TitleHelper.hxx>

namespace chart
{

/** Provides localized ui strings for the userinterface.
*/

class ObjectNameProvider
{
public:
    static OUString getName( ObjectType eObjectType, bool bPlural=false );
    static OUString getAxisName( std::u16string_view rObjectCID
                        , const rtl::Reference<::chart::ChartModel>& xChartModel  );
    static OUString getGridName( std::u16string_view rObjectCID
                        , const rtl::Reference<::chart::ChartModel>& xChartModel );
    static OUString getTitleName( std::u16string_view rObjectCID
                        , const rtl::Reference<::chart::ChartModel>& xChartModel );
    static OUString getTitleNameByType( TitleHelper::eTitleType eType );

    static OUString getNameForCID(
        std::u16string_view rObjectCID,
        const rtl::Reference<::chart::ChartModel>& xChartDocument );

    static OUString getName_ObjectForSeries(
        ObjectType eObjectType,
        std::u16string_view rSeriesCID,
        const rtl::Reference<::chart::ChartModel>& xChartDocument );
    static OUString getName_ObjectForAllSeries( ObjectType eObjectType );

    /** Provides help texts for the various chart elements.
        The parameter rObjectCID has to be a ClassifiedIdentifier - see class ObjectIdentifier.
     */
    static OUString getHelpText( std::u16string_view rObjectCID, const rtl::Reference<::chart::ChartModel>& xChartModel, bool bVerbose=false );

    /** This is used for showing the currently selected object in the status bar
        (command "Context")
     */
    static OUString getSelectedObjectText( std::u16string_view rObjectCID, const rtl::Reference<::chart::ChartModel>& xChartDocument );
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
