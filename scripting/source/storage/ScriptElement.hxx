/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ScriptElement.hxx,v $
 * $Revision: 1.9 $
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
#ifndef _SCRIPT_FRAMEWORK_STORAGE_SCRIPT_ELEMENT_HXX_
#define _SCRIPT_FRAMEWORK_STORAGE_SCRIPT_ELEMENT_HXX_

#include <osl/mutex.hxx>

#include "ScriptData.hxx"

#include "XMLElement.hxx"

namespace scripting_impl
{
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

class ScriptElement : public ::scripting_impl::XMLElement
{
public:
    /**
    Construct a ScriptElement from a ScriptData  object

    @param sII
    the ScriptData Object
    */
    explicit ScriptElement( ScriptData & sII );
    ~ScriptElement() SAL_THROW (());

private:
    ScriptData m_sII;

};

}

#endif
