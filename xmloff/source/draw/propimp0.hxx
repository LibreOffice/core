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

#ifndef _PROPIMP0_HXX
#define _PROPIMP0_HXX

#include <xmloff/xmlprhdl.hxx>

// graphic property Stroke

class XMLDurationPropertyHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLDurationPropertyHdl();
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class SvXMLImport;
class XMLOpacityPropertyHdl : public XMLPropertyHandler
{
private:
    SvXMLImport*    mpImport;
public:
    XMLOpacityPropertyHdl( SvXMLImport* pImport );
    virtual ~XMLOpacityPropertyHdl();
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XMLTextAnimationStepPropertyHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLTextAnimationStepPropertyHdl();
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class SvXMLExport;
class XMLDateTimeFormatHdl : public XMLPropertyHandler
{
private:
    SvXMLExport* mpExport;

public:
    XMLDateTimeFormatHdl( SvXMLExport* pExport );
    virtual ~XMLDateTimeFormatHdl();
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

#endif  //  _PROPIMP0_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
