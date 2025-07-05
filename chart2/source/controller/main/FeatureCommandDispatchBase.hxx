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

#include "CommandDispatch.hxx"

#include <com/sun/star/frame/DispatchInformation.hpp>

enum class ChartCommandID
{
    NONE                         = 0,

    //Draw Command Ids:
    DrawObjectSelect             = 1,
    DrawLine                     = 2,
    DrawLineArrowEnd             = 3,
    DrawRect                     = 4,
    DrawEllipse                  = 5,
    DrawFreelineNoFill           = 6,
    DrawText                     = 7,
    DrawTextVertical             = 8,
    DrawCaption                  = 9,
    DrawCaptionVertical          = 10,
    DrawToolboxCsBasic           = 11,
    DrawToolboxCsSymbol          = 12,
    DrawToolboxCsArrow           = 13,
    DrawToolboxCsFlowchart       = 14,
    DrawToolboxCsCallout         = 15,
    DrawToolboxCsStar            = 16,

    //Shape Controller Command Ids:
    ShapeFormatLine              =  21,
    ShapeFormatArea              =  22,
    ShapeTextAttributes          =  23,
    ShapeTransformDialog         =  24,
    ShapeObjectTitleDescription  =  25,
    ShapeRenameObject            =  26,
    ShapeBringToFront            =  28,
    ShapeForward                 =  29,
    ShapeBackward                = 30,
    ShapeSendToBack              = 31,
    ShapeFontDialog              = 35,
    ShapeParagraphDialog         = 36
};


namespace chart
{

struct ControllerFeature: public css::frame::DispatchInformation
{
    ChartCommandID nFeatureId;
    ControllerFeature() = default;
    ControllerFeature(const OUString& command, sal_Int16 groupId, ChartCommandID featureId)
        : css::frame::DispatchInformation(command, groupId)
        , nFeatureId(featureId)
    {
    }
};

typedef std::map< OUString,
                    ControllerFeature > SupportedFeatures;

struct FeatureState
{
    bool bEnabled;
    css::uno::Any aState;

    FeatureState() : bEnabled( false ) { }
};

/** This is a base class for CommandDispatch implementations with feature support.
 */
class FeatureCommandDispatchBase: public CommandDispatch
{
public:
    explicit FeatureCommandDispatchBase( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~FeatureCommandDispatchBase() override;

    // late initialisation, especially for adding as listener
    virtual void initialize() override;

    virtual bool isFeatureSupported( const OUString& rCommandURL );

protected:
    // XDispatch
    virtual void SAL_CALL dispatch( const css::util::URL& URL,
        const css::uno::Sequence< css::beans::PropertyValue >& Arguments ) override;

    virtual void fireStatusEvent( const OUString& rURL,
        const css::uno::Reference< css::frame::XStatusListener >& xSingleListener ) override;

    // state of a feature
    virtual FeatureState getState( const OUString& rCommand ) = 0;

    // execute a feature
    virtual void execute( const OUString& rCommand, const css::uno::Sequence< css::beans::PropertyValue>& rArgs ) = 0;

    // all the features which should be handled by this class
    virtual void describeSupportedFeatures() = 0;

    /** describes a feature supported by the controller

        Must not be called outside <member>describeSupportedFeatures</member>.

        @param sCommandURL
            the URL of the feature command
        @param nId
            the id of the feature. Later references to this feature usually happen by id, not by
            URL.
        @param nGroup
            the command group of the feature. This is important for configuring the controller UI
            by the user, see also <type scope="css::frame">CommandGroup</type>.
    */
    void implDescribeSupportedFeature( const OUString& sCommandURL, ChartCommandID nId,
        sal_Int16 nGroup );

    mutable SupportedFeatures m_aSupportedFeatures;

    ChartCommandID m_nFeatureId;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
