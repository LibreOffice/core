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

#ifndef SC_SOLVERUTIL_HXX
#define SC_SOLVERUTIL_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
    namespace sheet { class XSolver; }
} } }


class ScSolverUtil
{
public:
    static void GetImplementations( com::sun::star::uno::Sequence<rtl::OUString>& rImplNames,
                                    com::sun::star::uno::Sequence<rtl::OUString>& rDescriptions );
    static com::sun::star::uno::Reference<com::sun::star::sheet::XSolver> GetSolver( const rtl::OUString& rImplName );
    static com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> GetDefaults( const rtl::OUString& rImplName );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
