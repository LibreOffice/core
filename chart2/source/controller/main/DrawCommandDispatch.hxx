/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DrawCommandDispatch.hxx,v $
 * $Revision: 1.4 $
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

#include "CommandDispatch.hxx"

#include <com/sun/star/frame/CommandGroup.hpp>
#include <com/sun/star/frame/DispatchInformation.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/util/URL.hpp>

#include <tools/solar.h>

namespace chart
{

struct ControllerFeature: public ::com::sun::star::frame::DispatchInformation
{
    sal_uInt16 nFeatureId;
};

typedef ::std::map< ::rtl::OUString,
                    ControllerFeature,
                    ::std::less< ::rtl::OUString > > SupportedFeatures;

struct FeatureState
{
    bool bEnabled;
    ::com::sun::star::uno::Any aState;

    FeatureState() : bEnabled( false ) { }
};

class ChartController;

/** This is a CommandDispatch implementation for drawing objects.
 */
class DrawCommandDispatch: public CommandDispatch
{
public:
    DrawCommandDispatch( const ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >& rxContext, ChartController* pController );
    virtual ~DrawCommandDispatch();

    // late initialisation, especially for adding as listener
    virtual void initialize();

    bool isFeatureSupported( const ::com::sun::star::util::URL& rURL ) const;

protected:
    // XDispatch
    virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& URL,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments )
        throw (::com::sun::star::uno::RuntimeException);

    // WeakComponentImplHelperBase
    virtual void SAL_CALL disposing();

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void fireStatusEvent( const ::rtl::OUString& rURL,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xSingleListener );

    // state of a feature
    virtual FeatureState getState( sal_uInt16 nId ) const;

    // execute a feature
    virtual void execute( sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rArgs );

    // all the features which should be handled by this class
    virtual void describeSupportedFeatures();

    /** describes a feature supported by the controller

        Must not be called outside <member>describeSupportedFeatures</member>.

        @param pAsciiCommandURL
            the URL of the feature command
        @param nId
            the id of the feature. Later references to this feature usually happen by id, not by
            URL.
        @param nGroup
            the command group of the feature. This is important for configuring the controller UI
            by the user, see also <type scope="com::sun::star::frame">CommandGroup</type>.
    */
    void implDescribeSupportedFeature( const sal_Char* pAsciiCommandURL, sal_uInt16 nId,
        sal_Int16 nGroup = ::com::sun::star::frame::CommandGroup::INTERNAL );


private:
    void fillSupportedFeatures();

    void setInsertObj( USHORT eObj, const ::rtl::OUString& rShapeType = ::rtl::OUString() );

    mutable SupportedFeatures m_aSupportedFeatures;
    ChartController* m_pChartController;
};

} //  namespace chart

// CHART2_DRAWCOMMANDDISPATCH_HXX
#endif
