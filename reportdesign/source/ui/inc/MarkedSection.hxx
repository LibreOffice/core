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

#ifndef RPTUI_MARKEDSECTION_HXX
#define RPTUI_MARKEDSECTION_HXX

#include <boost/shared_ptr.hpp>

namespace rptui
{
    class OSectionWindow; // forward declaration

    enum NearSectionAccess
    {
        CURRENT = 0,
        PREVIOUS = -1,
        POST = 1
    };

    class IMarkedSection
    {
    public:
        /** returns the section which is currently marked.
        */
        virtual ::boost::shared_ptr<OSectionWindow> getMarkedSection(NearSectionAccess nsa) const =0;

        /** mark the section on the given position .
        *
        * \param _nPos the position is zero based.
        */
        virtual void markSection(const sal_uInt16 _nPos) = 0;
    };

} // rptui

#endif /* RPTUI_MARKEDSECTION_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
