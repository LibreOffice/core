/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: boolexpression.hxx,v $
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

#ifndef _BOOLEXPRESSION_HXX
#define _BOOLEXPRESSION_HXX

// include for parent class
#include "computedexpression.hxx"

namespace xforms
{

/** BoolExpression represents a computed XPath expression that returns
 * a bool value and caches the results.
 *
 * As this class has no virtual methods, it should never be used
 * polymorphically. */
class BoolExpression : public ComputedExpression
{
public:
    BoolExpression();
    ~BoolExpression();

    /// set the expression string
    /// (overridden for new definition of a simple expression)
    void setExpression( const rtl::OUString& rExpression );
};

} // namespace xforms

#endif
