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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_SHAPECONTROLLER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_SHAPECONTROLLER_HXX

#include "FeatureCommandDispatchBase.hxx"
#include <tools/link.hxx>

class AbstractSvxObjectNameDialog;
class SdrObject;

namespace chart
{

class ChartController;

/** This is a CommandDispatch implementation for shapes.
 */
class ShapeController: public FeatureCommandDispatchBase
{
    friend class ControllerCommandDispatch;

public:
    ShapeController( const css::uno::Reference< css::uno::XComponentContext >& rxContext, ChartController* pController );
    virtual ~ShapeController();

    // late initialisation, especially for adding as listener
    virtual void initialize() override;

protected:
    // WeakComponentImplHelperBase
    virtual void SAL_CALL disposing() override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source )
        throw (css::uno::RuntimeException, std::exception) override;

    // state of a feature
    virtual FeatureState getState( const OUString& rCommand ) override;

    // execute a feature
    virtual void execute( const OUString& rCommand, const css::uno::Sequence< css::beans::PropertyValue>& rArgs ) override;

    // all the features which should be handled by this class
    virtual void describeSupportedFeatures() override;

private:
    DECL_LINK_TYPED( CheckNameHdl, AbstractSvxObjectNameDialog&, bool);

    void executeDispatch_FormatLine();
    void executeDispatch_FormatArea();
    void executeDispatch_TextAttributes();
    void executeDispatch_TransformDialog();
    void executeDispatch_ObjectTitleDescription();
    void executeDispatch_RenameObject();
    void executeDispatch_ChangeZOrder( sal_uInt16 nId );
    void executeDispatch_FontDialog();
    void executeDispatch_ParagraphDialog();

    SdrObject* getFirstAdditionalShape();
    SdrObject* getLastAdditionalShape();
    bool       isBackwardPossible();
    bool       isForwardPossible();

    ChartController* m_pChartController;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_SHAPECONTROLLER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
