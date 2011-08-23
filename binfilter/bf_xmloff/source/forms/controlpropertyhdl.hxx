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

#ifndef _XMLOFF_FORMS_CONTROLPROPERTYHDL_HXX_
#define _XMLOFF_FORMS_CONTROLPROPERTYHDL_HXX_

#include "prhdlfac.hxx"
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include "xmlexppr.hxx"
#include "ControlBorderHandler.hxx"
namespace binfilter {

struct SvXMLEnumMapEntry;
class XMLConstantsPropertyHandler;
//.........................................................................
namespace xmloff
{
//.........................................................................

    class IFormsExportContext;
    class IFormsImportContext;

    //=====================================================================
    //= ORotationAngleHandler
    //=====================================================================
    class ORotationAngleHandler : public XMLPropertyHandler
    {
    public:
        ORotationAngleHandler();

        virtual sal_Bool importXML( const ::rtl::OUString& _rStrImpValue, ::com::sun::star::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const;
        virtual sal_Bool exportXML( ::rtl::OUString& _rStrExpValue, const ::com::sun::star::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const;
    };

    //=====================================================================
    //= OFontWidthHandler
    //=====================================================================
    class OFontWidthHandler : public XMLPropertyHandler
    {
    public:
        OFontWidthHandler();

        virtual sal_Bool importXML( const ::rtl::OUString& _rStrImpValue, ::com::sun::star::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const;
        virtual sal_Bool exportXML( ::rtl::OUString& _rStrExpValue, const ::com::sun::star::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const;
    };

    //=====================================================================
    //= OControlPropertyHandlerFactory
    //=====================================================================
    class OControlPropertyHandlerFactory : public XMLPropertyHandlerFactory
    {
    protected:
        mutable XMLConstantsPropertyHandler*	m_pTextAlignHandler;
        mutable OControlBorderHandler*			m_pControlBorderHandler;
        mutable ORotationAngleHandler*			m_pRotationAngleHandler;
        mutable OFontWidthHandler*				m_pFontWidthHandler;
        mutable XMLConstantsPropertyHandler*	m_pFontEmphasisHandler;
        mutable XMLConstantsPropertyHandler*	m_pFontReliefHandler;

    public:
        OControlPropertyHandlerFactory();
        virtual ~OControlPropertyHandlerFactory();

        virtual const XMLPropertyHandler* GetPropertyHandler(sal_Int32 _nType) const;
    };

//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter
#endif // _XMLOFF_FORMS_CONTROLPROPERTYHDL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
