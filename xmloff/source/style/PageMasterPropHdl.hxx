/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PageMasterPropHdl.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 14:46:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _XMLOFF_PAGEMASTERPROPHDL_HXX_
#define _XMLOFF_PAGEMASTERPROPHDL_HXX_

#ifndef _XMLOFF_PROPERTYHANDLERBASE_HXX
#include "xmlprhdl.hxx"
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif


//______________________________________________________________________________
// property handler for style:page-usage (style::PageStyleLayout)

class XMLPMPropHdl_PageStyleLayout : public XMLPropertyHandler
{
public:
    virtual                 ~XMLPMPropHdl_PageStyleLayout();
    virtual bool            equals(
                                const ::com::sun::star::uno::Any& rAny1,
                                const ::com::sun::star::uno::Any& rAny2
                                ) const;
    virtual sal_Bool        importXML(
                                const ::rtl::OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool        exportXML(
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
    virtual                 ~XMLPMPropHdl_NumFormat();
    virtual sal_Bool        importXML(
                                const ::rtl::OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool        exportXML(
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
    virtual                 ~XMLPMPropHdl_NumLetterSync();
    virtual sal_Bool        importXML(
                                const ::rtl::OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool        exportXML(
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
    virtual                 ~XMLPMPropHdl_PaperTrayNumber();
    virtual sal_Bool        importXML(
                                const ::rtl::OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool        exportXML(
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
    ::rtl::OUString         sAttrValue;

public:
                            XMLPMPropHdl_Print( enum ::xmloff::token::XMLTokenEnum eValue );
    virtual                 ~XMLPMPropHdl_Print();

    virtual sal_Bool        importXML(
                                const ::rtl::OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool        exportXML(
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
    virtual                 ~XMLPMPropHdl_CenterHorizontal();
    virtual sal_Bool        importXML(
                                const ::rtl::OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool        exportXML(
                                ::rtl::OUString& rStrExpValue,
                                const ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
};

class XMLPMPropHdl_CenterVertical : public XMLPropertyHandler
{
public:
    virtual                 ~XMLPMPropHdl_CenterVertical();
    virtual sal_Bool        importXML(
                                const ::rtl::OUString& rStrImpValue,
                                ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
    virtual sal_Bool        exportXML(
                                ::rtl::OUString& rStrExpValue,
                                const ::com::sun::star::uno::Any& rValue,
                                const SvXMLUnitConverter& rUnitConverter
                                ) const;
};

#endif

