/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _XMLOFF_PAGEMASTERPROPHDL_HXX_
#define _XMLOFF_PAGEMASTERPROPHDL_HXX_

#include <xmloff/xmlprhdl.hxx>
#include <rtl/ustring.hxx>
#include <xmloff/xmltoken.hxx>


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

