/*************************************************************************
 *
 *  $RCSfile: XMLDDELinksContext.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: sab $ $Date: 2001-02-28 08:19:33 $
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

#ifndef _SC_XMLDDELINKSCONTEXT_HXX
#define _SC_XMLDDELINKSCONTEXT_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif

#ifndef __SGI_STL_LIST
#include <list>
#endif

class ScXMLImport;

class ScXMLDDELinksContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
public:
    ScXMLDDELinksContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLDDELinksContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

struct ScDDELinkCell
{
    rtl::OUString sValue;
    double fValue;
    sal_Bool bString : 1;
    sal_Bool bEmpty : 1;
};

typedef std::list<ScDDELinkCell> ScDDELinkCells;

class ScXMLDDELinkContext : public SvXMLImportContext
{
    ScDDELinkCells  aDDELinkTable;
    ScDDELinkCells  aDDELinkRow;
    rtl::OUString   sApplication;
    rtl::OUString   sTopic;
    rtl::OUString   sItem;
    sal_Int32       nPosition;
    sal_Int32       nColumns;
    sal_Int32       nRows;
    sal_uInt8       nMode;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
public:
    ScXMLDDELinkContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLDDELinkContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    void SetApplication(const rtl::OUString& sValue) { sApplication = sValue; }
    void SetTopic(const rtl::OUString& sValue) { sTopic = sValue; }
    void SetItem(const rtl::OUString& sValue) { sItem = sValue; }
    void SetMode(const sal_uInt8 nValue) { nMode = nValue; }
    void CreateDDELink();
    void AddColumns(const sal_Int32 nValue) { nColumns += nValue; }
    void AddRows(const sal_Int32 nValue) { nRows += nValue; }
    void AddCellToRow(const ScDDELinkCell& aCell);
    void AddRowsToTable(const sal_Int32 nRows);

    virtual void EndElement();
};

class ScXMLDDESourceContext : public SvXMLImportContext
{
    ScXMLDDELinkContext* pDDELink;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
public:
    ScXMLDDESourceContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDESourceContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLDDETableContext : public SvXMLImportContext
{
    ScXMLDDELinkContext* pDDELink;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
public:
    ScXMLDDETableContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDETableContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLDDEColumnContext : public SvXMLImportContext
{
    ScXMLDDELinkContext* pDDELink;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
public:
    ScXMLDDEColumnContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDEColumnContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLDDERowContext : public SvXMLImportContext
{
    ScXMLDDELinkContext*    pDDELink;
    sal_Int32               nRows;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
public:
    ScXMLDDERowContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDERowContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLDDECellContext : public SvXMLImportContext
{
    rtl::OUString   sValue;
    double          fValue;
    sal_Int32       nCells;
    sal_Bool        bString : 1;
    sal_Bool        bString2 : 1;
    sal_Bool        bEmpty : 1;

    ScXMLDDELinkContext* pDDELink;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
public:
    ScXMLDDECellContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLDDELinkContext* pDDELink);

    virtual ~ScXMLDDECellContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

#endif
