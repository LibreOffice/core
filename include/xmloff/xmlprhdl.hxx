/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _XMLOFF_PROPERTYHANDLERBASE_HXX
#define _XMLOFF_PROPERTYHANDLERBASE_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <sal/types.h>
#include <rtl/ustring.hxx>

namespace com{ namespace sun{ namespace star{ namespace uno{ class Any; } } } }

class SvXMLUnitConverter;
struct XMLPropertyState;

///////////////////////////////////////////////////////////////////////////
//
/** Abstract base-class for different XML-types. Derivations of this class
    knows how to compare, im/export a special XML-type
*/
class XMLOFF_DLLPUBLIC XMLPropertyHandler
{
    // ??? const SvXMLUnitConverter& rUnitConverter         Do we need this ?

public:
    // Just needed for virtual destruction
    virtual ~XMLPropertyHandler();

    /** Compares two Any's in case of the given XML-data-type.
        By default the Any-equal-operator is called. */
    virtual bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;

    /// Imports the given value in cas of the given XML-data-type
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const =0;
    /// Exports the given value in cas of the given XML-data-type
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const =0;
};

#endif      // _XMLOFF_PROPERTYHANDLERBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
