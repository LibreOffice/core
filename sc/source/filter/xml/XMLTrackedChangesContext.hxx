/*************************************************************************
 *
 *  $RCSfile: XMLTrackedChangesContext.hxx,v $
 *
 *  $Revision: 1.6 $
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

#ifndef _SC_XMLTRACKEDCHANGESCONTEXT_HXX
#define _SC_XMLTRACKEDCHANGESCONTEXT_HXX

#ifndef _SC_XMLCHANGETRACKINGIMPORTHELPER_HXX
#include "XMLChangeTrackingImportHelper.hxx"
#endif
#ifndef SC_CHGTRACK_HXX
#include "chgtrack.hxx"
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

class ScXMLImport;
class ScXMLChangeTrackingImportHelper;
class ScEditEngineTextObj;

class ScXMLTrackedChangesContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLTrackedChangesContext( ScXMLImport& rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLTrackedChangesContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const NAMESPACE_RTL(OUString)& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

class ScXMLChangeInfoContext : public SvXMLImportContext
{
    ScMyActionInfo                      aInfo;
    rtl::OUStringBuffer                 sBuffer;
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;
    sal_uInt32                          nParagraphCount;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLChangeInfoContext( ScXMLImport& rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLChangeInfoContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const NAMESPACE_RTL(OUString)& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLBigRangeContext : public SvXMLImportContext
{
    ScBigRange&         rBigRange;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLBigRangeContext( ScXMLImport& rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScBigRange& rBigRange);
    virtual ~ScXMLBigRangeContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const NAMESPACE_RTL(OUString)& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLCellContentDeletionContext : public SvXMLImportContext
{
    rtl::OUString                       sFormulaAddress;
    rtl::OUString                       sFormula;
    ScBigRange                          aBigRange;
    double                              fValue;
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;
    ScBaseCell*                         pCell;
    sal_uInt32                          nID;
    sal_Int32                           nMatrixCols;
    sal_Int32                           nMatrixRows;
    sal_uInt16                          nType;
    sal_uInt8                           nMatrixFlag;
    sal_Bool                            bBigRange : 1;
    sal_Bool                            bContainsCell : 1;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLCellContentDeletionContext( ScXMLImport& rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLCellContentDeletionContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const NAMESPACE_RTL(OUString)& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLDependenceContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLDependenceContext( ScXMLImport& rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLDependenceContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const NAMESPACE_RTL(OUString)& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLDependingsContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLDependingsContext( ScXMLImport& rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLDependingsContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const NAMESPACE_RTL(OUString)& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLChangeDeletionContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLChangeDeletionContext( ScXMLImport& rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLChangeDeletionContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const NAMESPACE_RTL(OUString)& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLDeletionsContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLDeletionsContext( ScXMLImport& rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLDeletionsContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const NAMESPACE_RTL(OUString)& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLChangeCellContext;

class ScXMLChangeTextPContext : public SvXMLImportContext
{
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList> xAttrList;
    rtl::OUString               sLName;
    rtl::OUStringBuffer         sText;
    ScXMLChangeCellContext*     pChangeCellContext;
    SvXMLImportContext*         pTextPContext;
    USHORT                      nPrefix;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLChangeTextPContext( ScXMLImport& rImport, USHORT nPrfx,
                       const NAMESPACE_RTL(OUString)& rLName,
                       const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLChangeCellContext* pChangeCellContext);

    virtual ~ScXMLChangeTextPContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void Characters( const ::rtl::OUString& rChars );

    virtual void EndElement();
};

class ScXMLChangeCellContext : public SvXMLImportContext
{
    rtl::OUString           sText;
    ScBaseCell*&            rOldCell;
    ScEditEngineTextObj*    pEditTextObj;
    double&                 rDateTimeValue;
    double                  fValue;
    sal_uInt16&             rType;
    sal_Bool                bIsMatrix : 1;
    sal_Bool                bIsCoveredMatrix : 1;
    sal_Bool                bEmpty : 1;
    sal_Bool                bFirstParagraph : 1;
    sal_Bool                bString : 1;
    sal_Bool                bFormula : 1;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLChangeCellContext( ScXMLImport& rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScBaseCell*& rOldCell, rtl::OUString& sAddress, rtl::OUString& sFormula,
                                      double& fValue, sal_uInt16& nType,
                                      sal_uInt8& nMatrixFlag, sal_Int32& nMatrixCols, sal_Int32& nMatrixRows);
    virtual ~ScXMLChangeCellContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const NAMESPACE_RTL(OUString)& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    void CreateTextPContext(sal_Bool bIsNewParagraph);
    sal_Bool IsEditCell() { return pEditTextObj != 0; }
    void SetText(const rtl::OUString& sTempText) { sText = sTempText; }

    virtual void EndElement();
};

class ScXMLPreviousContext : public SvXMLImportContext
{
    rtl::OUString                       sFormulaAddress;
    rtl::OUString                       sFormula;
    double                              fValue;
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;
    ScBaseCell*                         pOldCell;
    sal_uInt32                          nID;
    sal_Int32                           nMatrixCols;
    sal_Int32                           nMatrixRows;
    sal_uInt16                          nType;
    sal_uInt8                           nMatrixFlag;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLPreviousContext( ScXMLImport& rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLPreviousContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const NAMESPACE_RTL(OUString)& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLContentChangeContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;
    ScBigRange                          aBigRange;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLContentChangeContext( ScXMLImport& rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLContentChangeContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const NAMESPACE_RTL(OUString)& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

class ScXMLInsertionContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLInsertionContext( ScXMLImport& rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLInsertionContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const NAMESPACE_RTL(OUString)& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

class ScXMLInsertionCutOffContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLInsertionCutOffContext( ScXMLImport& rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLInsertionCutOffContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const NAMESPACE_RTL(OUString)& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

class ScXMLMovementCutOffContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLMovementCutOffContext( ScXMLImport& rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLMovementCutOffContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const NAMESPACE_RTL(OUString)& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

class ScXMLCutOffsContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLCutOffsContext( ScXMLImport& rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLCutOffsContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const NAMESPACE_RTL(OUString)& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

class ScXMLDeletionContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLDeletionContext( ScXMLImport& rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLDeletionContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const NAMESPACE_RTL(OUString)& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

class ScXMLMovementContext : public SvXMLImportContext
{
    ScBigRange                          aSourceRange;
    ScBigRange                          aTargetRange;
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLMovementContext( ScXMLImport& rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLMovementContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const NAMESPACE_RTL(OUString)& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

class ScXMLRejectionContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLRejectionContext( ScXMLImport& rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLRejectionContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const NAMESPACE_RTL(OUString)& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

#endif

