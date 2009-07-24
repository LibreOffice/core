/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ShapeController.hxx,v $
 * $Revision: 1.0 $
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
#ifndef CHART2_SHAPECONTROLLER_HXX
#define CHART2_SHAPECONTROLLER_HXX

#include "FeatureCommandDispatchBase.hxx"
#include <tools/link.hxx>

class AbstractSvxNameDialog;

//.............................................................................
namespace chart
{
//.............................................................................

class ChartController;

/** This is a CommandDispatch implementation for shapes.
 */
class ShapeController: public FeatureCommandDispatchBase
{
    friend class ControllerCommandDispatch;

public:
    ShapeController( const ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >& rxContext, ChartController* pController );
    virtual ~ShapeController();

    // late initialisation, especially for adding as listener
    virtual void initialize();

protected:
    // WeakComponentImplHelperBase
    virtual void SAL_CALL disposing();

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    // state of a feature
    virtual FeatureState getState( const ::rtl::OUString& rCommand ) const;

    // execute a feature
    virtual void execute( const ::rtl::OUString& rCommand, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rArgs );

    // all the features which should be handled by this class
    virtual void describeSupportedFeatures();

private:
    DECL_LINK( CheckNameHdl, AbstractSvxNameDialog* );

    void executeDispatch_FormatLine();
    void executeDispatch_FormatArea();
    void executeDispatch_TextAttributes();
    void executeDispatch_TransformDialog();
    void executeDispatch_ObjectTitleDescription();
    void executeDispatch_RenameObject();
    void executeDispatch_FontDialog();

    ChartController* m_pChartController;
};

//.............................................................................
} //  namespace chart
//.............................................................................

// CHART2_SHAPECONTROLLER_HXX
#endif
