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

#ifndef _XMLOFF_NAMEDBOOLPROPERTYHANDLER_HXX
#define _XMLOFF_NAMEDBOOLPROPERTYHANDLER_HXX

#include <xmloff/xmlprhdl.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/uno/Type.h>
#include <xmloff/xmlement.hxx>

/**
    PropertyHandler for a named xml bool type:
*/
class XMLNamedBoolPropertyHdl : public XMLPropertyHandler
{
private:
    const OUString maTrueStr;
    const OUString maFalseStr;

public:
    XMLNamedBoolPropertyHdl( const OUString& rTrueStr, const OUString& rFalseStr ) : maTrueStr( rTrueStr ), maFalseStr( rFalseStr ) {}

    XMLNamedBoolPropertyHdl(
            ::xmloff::token::XMLTokenEnum eTrue,
            ::xmloff::token::XMLTokenEnum eFalse
            ) :
        maTrueStr( ::xmloff::token::GetXMLToken( eTrue ) ),
        maFalseStr( ::xmloff::token::GetXMLToken( eFalse ) )
    {}

    virtual ~XMLNamedBoolPropertyHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

#endif      // _XMLOFF_NAMEDBOOLPROPERTYHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
