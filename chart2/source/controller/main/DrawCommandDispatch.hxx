/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef CHART2_DRAWCOMMANDDISPATCH_HXX
#define CHART2_DRAWCOMMANDDISPATCH_HXX

#include "FeatureCommandDispatchBase.hxx"

#include <tools/solar.h>

class SfxItemSet;
class SdrObject;
class SdrObjectCreationInfo;

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
    void setInsertObj(const SdrObjectCreationInfo& rSdrObjectCreationInfo);
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
