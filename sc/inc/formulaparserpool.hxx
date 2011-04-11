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

#ifndef SC_FORMULAPARSERPOOL_HXX
#define SC_FORMULAPARSERPOOL_HXX

#include <boost/unordered_map.hpp>
#include <com/sun/star/sheet/XFormulaParser.hpp>

class ScDocument;

// ============================================================================

/** Stores the used instances of the FilterFormulaParser service
    implementations, mapped by the formula namespace they support. */
class ScFormulaParserPool
{
public:
    explicit            ScFormulaParserPool( const ScDocument& rDoc );
                        ~ScFormulaParserPool();

    /** Returns true, if a formula parser is registered for the passed namespace. */
    bool                hasFormulaParser( const ::rtl::OUString& rNamespace );

    /** Returns the formula parser that is registered for the passed namespace. */
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaParser >
                        getFormulaParser( const ::rtl::OUString& rNamespace );

private:
    typedef ::boost::unordered_map<
        ::rtl::OUString,
        ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaParser >,
        ::rtl::OUStringHash,
        ::std::equal_to< ::rtl::OUString > > ParserMap;

    const ScDocument&   mrDoc;
    ParserMap           maParsers;
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
