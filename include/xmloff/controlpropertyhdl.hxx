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

#ifndef INCLUDED_XMLOFF_CONTROLPROPERTYHDL_HXX
#define INCLUDED_XMLOFF_CONTROLPROPERTYHDL_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <xmloff/prhdlfac.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <xmloff/xmlexppr.hxx>
#include <xmloff/XMLConstantsPropertyHandler.hxx>

struct SvXMLEnumMapEntry;
class Color;

namespace xmloff
{


    class IFormsExportContext;
    class OFormLayerXMLImport_Impl;


    //= ORotationAngleHandler

    class ORotationAngleHandler : public XMLPropertyHandler
    {
    public:
        ORotationAngleHandler();

        virtual bool importXML( const OUString& _rStrImpValue, css::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const override;
        virtual bool exportXML( OUString& _rStrExpValue, const css::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const override;
    };


    //= OFontWidthHandler

    class OFontWidthHandler : public XMLPropertyHandler
    {
    public:
        OFontWidthHandler();

        virtual bool importXML( const OUString& _rStrImpValue, css::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const override;
        virtual bool exportXML( OUString& _rStrExpValue, const css::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const override;
    };


    //= OControlBorderHandlerBase

    class OControlBorderHandler : public XMLPropertyHandler
    {
    public:
        enum BorderFacet
        {
            STYLE,
            COLOR
        };

        OControlBorderHandler( const BorderFacet _eFacet );

        virtual bool importXML( const OUString& _rStrImpValue, css::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const override;
        virtual bool exportXML( OUString& _rStrExpValue, const css::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const override;

    private:
        BorderFacet m_eFacet;
    };


    //= OControlTextEmphasisHandler

    class OControlTextEmphasisHandler : public XMLPropertyHandler
    {
    public:
        OControlTextEmphasisHandler();

        virtual bool importXML( const OUString& _rStrImpValue, css::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const override;
        virtual bool exportXML( OUString& _rStrExpValue, const css::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const override;
    };


    //= ImageScaleModeHandler

    class XMLOFF_DLLPUBLIC ImageScaleModeHandler : public XMLConstantsPropertyHandler
    {
    public:
        ImageScaleModeHandler();
    };


    //= OControlPropertyHandlerFactory

    class XMLOFF_DLLPUBLIC OControlPropertyHandlerFactory : public XMLPropertyHandlerFactory
    {
    protected:
        mutable XMLConstantsPropertyHandler*    m_pTextAlignHandler;
        mutable OControlBorderHandler*          m_pControlBorderStyleHandler;
        mutable OControlBorderHandler*          m_pControlBorderColorHandler;
        mutable ORotationAngleHandler*          m_pRotationAngleHandler;
        mutable OFontWidthHandler*              m_pFontWidthHandler;
        mutable XMLConstantsPropertyHandler*    m_pFontEmphasisHandler;
        mutable XMLConstantsPropertyHandler*    m_pFontReliefHandler;

    protected:
        virtual ~OControlPropertyHandlerFactory();

    public:
        OControlPropertyHandlerFactory();

        virtual const XMLPropertyHandler* GetPropertyHandler(sal_Int32 _nType) const override;
    };


}   // namespace xmloff


#endif // INCLUDED_XMLOFF_CONTROLPROPERTYHDL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
