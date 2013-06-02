/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _XMLREGRESSIONTYPEPROPERTYHDL_HXX_
#define _XMLREGRESSIONTYPEPROPERTYHDL_HXX_

#include <xmloff/xmlprhdl.hxx>

class XMLRegressionTypePropertyHdl : public XMLPropertyHandler
{
public:
    XMLRegressionTypePropertyHdl();
    virtual ~XMLRegressionTypePropertyHdl();

    virtual sal_Bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

#endif  // _XMLERRORBARSTYLEPROPERTYHDL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
