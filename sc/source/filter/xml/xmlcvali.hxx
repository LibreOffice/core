/*************************************************************************
 *
 *  $RCSfile: xmlcvali.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sab $ $Date: 2000-10-12 06:35:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef SC_XMLCVALI_HXX
#define SC_XMLCVALI_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif

class ScXMLImport;

class ScXMLContentValidationsContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLContentValidationsContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLContentValidationsContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLContentValidationContext : public SvXMLImportContext
{
    rtl::OUString   sName;
    rtl::OUString   sHelpTitle;
    rtl::OUString   sHelpMessage;
    rtl::OUString   sErrorTitle;
    rtl::OUString   sErrorMessage;
    rtl::OUString   sErrorMessageType;
    rtl::OUString   sBaseCellAddress;
    rtl::OUString   sCondition;
    sal_Bool        bAllowEmptyCell;
    sal_Bool        bDisplayHelp;
    sal_Bool        bDisplayError;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLContentValidationContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLContentValidationContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

    void SetHelpMessage(const rtl::OUString& sTitle, const rtl::OUString& sMessage, const sal_Bool bDisplay);
    void SetErrorMessage(const rtl::OUString& sTitle, const rtl::OUString& sMessage, const rtl::OUString& sMessageType, const sal_Bool bDisplay);
    void SetErrorMacro(const rtl::OUString& sName, const sal_Bool bExecute);
};

class ScXMLHelpMessageContext : public SvXMLImportContext
{
    rtl::OUString   sTitle;
    rtl::OUString   sMessage;
    sal_Int32       nParagraphCount;
    sal_Bool        bDisplay;

    ScXMLContentValidationContext* pValidationContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLHelpMessageContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLContentValidationContext* pValidationContext);

    virtual ~ScXMLHelpMessageContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLErrorMessageContext : public SvXMLImportContext
{
    rtl::OUString   sTitle;
    rtl::OUString   sMessage;
    rtl::OUString   sMessageType;
    sal_Int32       nParagraphCount;
    sal_Bool        bDisplay;

    ScXMLContentValidationContext* pValidationContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLErrorMessageContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLContentValidationContext* pValidationContext);

    virtual ~ScXMLErrorMessageContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLErrorMacroContext : public SvXMLImportContext
{
    rtl::OUString   sName;
    sal_Bool        bExecute;

    ScXMLContentValidationContext* pValidationContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLErrorMacroContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLContentValidationContext* pValidationContext);

    virtual ~ScXMLErrorMacroContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

#endif
