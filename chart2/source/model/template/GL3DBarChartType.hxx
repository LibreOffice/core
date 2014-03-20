/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_GL3DBARCHARTTYPE_HXX
#define INCLUDED_CHART2_GL3DBARCHARTTYPE_HXX

#include "ChartType.hxx"
#include "ServiceMacros.hxx"

namespace chart {

/**
 * Chart type that represents 3 dimensional data content in 3D space using
 * OpenGL.
 */
class GL3DBarChartType : public ChartType
{
public:
    GL3DBarChartType( const com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext>& xContext );
    virtual ~GL3DBarChartType();

    APPHELPER_XSERVICEINFO_DECL()

    APPHELPER_SERVICE_FACTORY_HELPER( GL3DBarChartType )

protected:
    GL3DBarChartType( const GL3DBarChartType& rOther );

    virtual OUString SAL_CALL getChartType()
        throw (com::sun::star::uno::RuntimeException, std::exception);

    virtual com::sun::star::uno::Reference<com::sun::star::util::XCloneable>
        SAL_CALL createClone()
            throw (com::sun::star::uno::RuntimeException, std::exception);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
