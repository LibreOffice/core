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
#ifndef CHART2_DRAWCOMMANDDISPATCH_HXX
#define CHART2_DRAWCOMMANDDISPATCH_HXX

#include "FeatureCommandDispatchBase.hxx"

#include <tools/solar.h>

class SfxItemSet;
class SdrObject;

namespace chart
{

class ChartController;

/** This is a CommandDispatch implementation for drawing objects.
 */
class DrawCommandDispatch: public FeatureCommandDispatchBase
{
public:
    DrawCommandDispatch( const ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >& rxContext, ChartController* pController );
    virtual ~DrawCommandDispatch();

    // late initialisation, especially for adding as listener
    virtual void initialize();

    virtual bool isFeatureSupported( const OUString& rCommandURL );

    void setAttributes( SdrObject* pObj );
    void setLineEnds( SfxItemSet& rAttr );

protected:
    // WeakComponentImplHelperBase
    virtual void SAL_CALL disposing();

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    // state of a feature
    virtual FeatureState getState( const OUString& rCommand );

    // execute a feature
    virtual void execute( const OUString& rCommand, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rArgs );

    // all the features which should be handled by this class
    virtual void describeSupportedFeatures();

private:
    void setInsertObj( sal_uInt16 eObj );
    SdrObject* createDefaultObject( const sal_uInt16 nID );

    bool parseCommandURL( const OUString& rCommandURL, sal_uInt16* pnFeatureId, OUString* pBaseCommand, OUString* pCustomShapeType );

    ChartController* m_pChartController;
    OUString m_aCustomShapeType;
};

} //  namespace chart

// CHART2_DRAWCOMMANDDISPATCH_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
