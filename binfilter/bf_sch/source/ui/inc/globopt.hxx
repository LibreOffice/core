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
#ifndef _SCH_GLOBOPT_HXX
#define _SCH_GLOBOPT_HXX

#ifndef _VCL_FLDUNIT_HXX
#include <vcl/fldunit.hxx>
#endif
namespace binfilter {

namespace sch
{

namespace util
{

/** @descr Retrieve the FieldUnit to be used for the UI.  This unit is retrieved
    from the registry settings of the Calc application.

    If this setting can not be found there is a fallback to cm which is the most
    common setting worldwide (or not?)

    @return the FieldUnit enum.  See <vcl/fldunit.hxx> for definition
 */
FieldUnit GetMeasureUnit();

}  // namespace util
}  // namespace sch


// _SCH_GLOBOPT_HXX
} //namespace binfilter
#endif
