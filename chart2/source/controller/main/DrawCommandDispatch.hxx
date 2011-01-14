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
#ifndef CHART2_DRAWCOMMANDDISPATCH_HXX
#define CHART2_DRAWCOMMANDDISPATCH_HXX

#include "FeatureCommandDispatchBase.hxx"

#include <tools/solar.h>

class SfxItemSet;
class SdrObject;

//.............................................................................
namespace chart
{
//.............................................................................

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

    virtual bool isFeatureSupported( const ::rtl::OUString& rCommandURL );

    void setAttributes( SdrObject* pObj );
    void setLineEnds( SfxItemSet& rAttr );

protected:
    // WeakComponentImplHelperBase
    virtual void SAL_CALL disposing();

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    // state of a feature
    virtual FeatureState getState( const ::rtl::OUString& rCommand );

    // execute a feature
    virtual void execute( const ::rtl::OUString& rCommand, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rArgs );

    // all the features which should be handled by this class
    virtual void describeSupportedFeatures();

private:
    void setInsertObj( sal_uInt16 eObj );
    SdrObject* createDefaultObject( const sal_uInt16 nID );

    bool parseCommandURL( const ::rtl::OUString& rCommandURL, sal_uInt16* pnFeatureId, ::rtl::OUString* pBaseCommand, ::rtl::OUString* pCustomShapeType );

    ChartController* m_pChartController;
    ::rtl::OUString m_aCustomShapeType;
};

//.............................................................................
} //  namespace chart
//.............................................................................

// CHART2_DRAWCOMMANDDISPATCH_HXX
#endif
