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

#ifndef _XMLOFF_PAGEMASTERPROPHDL_HXX_
#define _XMLOFF_PAGEMASTERPROPHDL_HXX_

#ifndef _XMLOFF_PROPERTYHANDLERBASE_HXX
#include "xmlprhdl.hxx"
#endif

#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif
namespace binfilter {


//______________________________________________________________________________
// property handler for style:page-usage (style::PageStyleLayout)

class XMLPMPropHdl_PageStyleLayout : public XMLPropertyHandler
{
public:
    virtual					~XMLPMPropHdl_PageStyleLayout();
    virtual sal_Bool		equals(
                                const ::com::sun::star::uno::Any& rAny1,
                                const ::com::sun::star::uno::Any& rAny2
                                ) const;
    virtual sal_Bool		importXML(
                                const ::rtl::OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool		exportXML(
                                ::rtl::OUString& rStrExpValue,
                                const ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
};


//______________________________________________________________________________
// property handler for style:num-format (style::NumberingType)

class XMLPMPropHdl_NumFormat : public XMLPropertyHandler
{
public:
    virtual					~XMLPMPropHdl_NumFormat();
    virtual sal_Bool		importXML(
                                const ::rtl::OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool		exportXML(
                                ::rtl::OUString& rStrExpValue,
                                const ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
};


//______________________________________________________________________________
// property handler for style:num-letter-sync (style::NumberingType)

class XMLPMPropHdl_NumLetterSync : public XMLPropertyHandler
{
public:
    virtual					~XMLPMPropHdl_NumLetterSync();
    virtual sal_Bool		importXML(
                                const ::rtl::OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool		exportXML(
                                ::rtl::OUString& rStrExpValue,
                                const ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
};


//______________________________________________________________________________
// property handler for style:paper-tray-number

class XMLPMPropHdl_PaperTrayNumber : public XMLPropertyHandler
{
public:
    virtual					~XMLPMPropHdl_PaperTrayNumber();
    virtual sal_Bool		importXML(
                                const ::rtl::OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool		exportXML(
                                ::rtl::OUString& rStrExpValue,
                                const ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
};


//______________________________________________________________________________
// property handler for style:print

class XMLPMPropHdl_Print : public XMLPropertyHandler
{
protected:
    ::rtl::OUString			sAttrValue;

public:
                            XMLPMPropHdl_Print( enum ::binfilter::xmloff::token::XMLTokenEnum eValue );
    virtual					~XMLPMPropHdl_Print();

    virtual sal_Bool		importXML(
                                const ::rtl::OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool		exportXML(
                                ::rtl::OUString& rStrExpValue,
                                const ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
};

//______________________________________________________________________________
// property handler for style:table-centering

class XMLPMPropHdl_CenterHorizontal : public XMLPropertyHandler
{
public:
    virtual					~XMLPMPropHdl_CenterHorizontal();
    virtual sal_Bool		importXML(
                                const ::rtl::OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool		exportXML(
                                ::rtl::OUString& rStrExpValue,
                                const ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
};

class XMLPMPropHdl_CenterVertical : public XMLPropertyHandler
{
public:
    virtual					~XMLPMPropHdl_CenterVertical();
    virtual sal_Bool		importXML(
                                const ::rtl::OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool		exportXML(
                                ::rtl::OUString& rStrExpValue,
                                const ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
};

}//end of namespace binfilter
#endif

