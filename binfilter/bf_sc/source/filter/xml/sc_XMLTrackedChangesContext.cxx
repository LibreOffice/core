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

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#include "XMLTrackedChangesContext.hxx"
#include "xmlimprt.hxx"
#include "xmlconti.hxx"
#include "XMLConverter.hxx"
#include "cell.hxx"
#include "textuno.hxx"
#include "editutil.hxx"

#include <bf_xmloff/xmlnmspe.hxx>
#include <bf_xmloff/nmspmap.hxx>
#include <bf_xmloff/xmluconv.hxx>

#include <bf_svtools/zforlist.hxx>

#include <com/sun/star/text/ControlCharacter.hpp>
namespace binfilter {

using namespace ::com::sun::star;
using namespace xmloff::token;

using rtl::OUString;

//-----------------------------------------------------------------------------

class ScXMLChangeInfoContext : public SvXMLImportContext
{
    ScMyActionInfo						aInfo;
    ::rtl::OUStringBuffer					sBuffer;
    ScXMLChangeTrackingImportHelper*	pChangeTrackingImportHelper;
    sal_uInt32							nParagraphCount;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLChangeInfoContext( ScXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLChangeInfoContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

//-----------------------------------------------------------------------------

class ScXMLBigRangeContext : public SvXMLImportContext
{
    ScBigRange&			rBigRange;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLBigRangeContext( ScXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScBigRange& rBigRange);
    virtual ~ScXMLBigRangeContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

//-----------------------------------------------------------------------------

class ScXMLCellContentDeletionContext : public SvXMLImportContext
{
    ::rtl::OUString						sFormulaAddress;
    ::rtl::OUString						sFormula;
    ScBigRange							aBigRange;
    double								fValue;
    ScXMLChangeTrackingImportHelper*	pChangeTrackingImportHelper;
    ScBaseCell*							pCell;
    sal_uInt32							nID;
    sal_Int32							nMatrixCols;
    sal_Int32							nMatrixRows;
    sal_uInt16							nType;
    sal_uInt8							nMatrixFlag;
    sal_Bool							bBigRange : 1;
    sal_Bool							bContainsCell : 1;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLCellContentDeletionContext( ScXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLCellContentDeletionContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

//-----------------------------------------------------------------------------

class ScXMLDependenceContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*	pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLDependenceContext( ScXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLDependenceContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

//-----------------------------------------------------------------------------

class ScXMLDependingsContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*	pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLDependingsContext( ScXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLDependingsContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

//-----------------------------------------------------------------------------

class ScXMLChangeDeletionContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*	pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLChangeDeletionContext( ScXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLChangeDeletionContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

//-----------------------------------------------------------------------------

class ScXMLDeletionsContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*	pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLDeletionsContext( ScXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLDeletionsContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

//-----------------------------------------------------------------------------

class ScXMLChangeCellContext;

class ScXMLChangeTextPContext : public SvXMLImportContext
{
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList> xAttrList;
    ::rtl::OUString				sLName;
    ::rtl::OUStringBuffer			sText;
    ScXMLChangeCellContext*		pChangeCellContext;
    SvXMLImportContext*			pTextPContext;
    USHORT						nPrefix;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLChangeTextPContext( ScXMLImport& rImport, USHORT nPrfx,
                       const ::rtl::OUString& rLName,
                       const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLChangeCellContext* pChangeCellContext);

    virtual ~ScXMLChangeTextPContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void Characters( const ::rtl::OUString& rChars );

    virtual void EndElement();
};

//-----------------------------------------------------------------------------

class ScXMLChangeCellContext : public SvXMLImportContext
{
    ::rtl::OUString			sText;
    ScBaseCell*&			rOldCell;
    ScEditEngineTextObj*	pEditTextObj;
    double&					rDateTimeValue;
    double					fValue;
    sal_uInt16&				rType;
    sal_Bool				bIsMatrix : 1;
    sal_Bool				bIsCoveredMatrix : 1;
    sal_Bool				bEmpty : 1;
    sal_Bool				bFirstParagraph : 1;
    sal_Bool				bString : 1;
    sal_Bool				bFormula : 1;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLChangeCellContext( ScXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScBaseCell*& rOldCell, ::rtl::OUString& sAddress, ::rtl::OUString& sFormula,
                                      double& fValue, sal_uInt16& nType,
                                      sal_uInt8& nMatrixFlag, sal_Int32& nMatrixCols, sal_Int32& nMatrixRows);
    virtual ~ScXMLChangeCellContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    void CreateTextPContext(sal_Bool bIsNewParagraph);
    sal_Bool IsEditCell() { return pEditTextObj != 0; }
    void SetText(const ::rtl::OUString& sTempText) { sText = sTempText; }

    virtual void EndElement();
};

//-----------------------------------------------------------------------------

class ScXMLPreviousContext : public SvXMLImportContext
{
    ::rtl::OUString						sFormulaAddress;
    ::rtl::OUString						sFormula;
    double								fValue;
    ScXMLChangeTrackingImportHelper*	pChangeTrackingImportHelper;
    ScBaseCell*							pOldCell;
    sal_uInt32							nID;
    sal_Int32							nMatrixCols;
    sal_Int32							nMatrixRows;
    sal_uInt16							nType;
    sal_uInt8							nMatrixFlag;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLPreviousContext( ScXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLPreviousContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

//-----------------------------------------------------------------------------

class ScXMLContentChangeContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*	pChangeTrackingImportHelper;
    ScBigRange							aBigRange;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLContentChangeContext( ScXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLContentChangeContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

//-----------------------------------------------------------------------------

class ScXMLInsertionContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*	pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLInsertionContext( ScXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLInsertionContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

//-----------------------------------------------------------------------------

class ScXMLInsertionCutOffContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*	pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLInsertionCutOffContext( ScXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLInsertionCutOffContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

//-----------------------------------------------------------------------------

class ScXMLMovementCutOffContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*	pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLMovementCutOffContext( ScXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLMovementCutOffContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

//-----------------------------------------------------------------------------

class ScXMLCutOffsContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*	pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLCutOffsContext( ScXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLCutOffsContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

//-----------------------------------------------------------------------------

class ScXMLDeletionContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*	pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLDeletionContext( ScXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLDeletionContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

//-----------------------------------------------------------------------------

class ScXMLMovementContext : public SvXMLImportContext
{
    ScBigRange							aSourceRange;
    ScBigRange							aTargetRange;
    ScXMLChangeTrackingImportHelper*	pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLMovementContext( ScXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLMovementContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

//-----------------------------------------------------------------------------

class ScXMLRejectionContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*	pChangeTrackingImportHelper;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:
    ScXMLRejectionContext( ScXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLRejectionContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

//------------------------------------------------------------------

ScXMLTrackedChangesContext::ScXMLTrackedChangesContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                              ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    rImport.LockSolarMutex();
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    pChangeTrackingImportHelper->SetChangeTrack(sal_True);

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );
        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_PROTECTION_KEY))
            {
                if (sValue.getLength())
                {
                    uno::Sequence<sal_Int8> aPass;
                    SvXMLUnitConverter::decodeBase64(aPass, sValue);
                    pChangeTrackingImportHelper->SetProtection(aPass);
                }
            }
        }
    }
}

ScXMLTrackedChangesContext::~ScXMLTrackedChangesContext()
{
    GetScImport().UnlockSolarMutex();
}

SvXMLImportContext *ScXMLTrackedChangesContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLocalName, XML_CELL_CONTENT_CHANGE))
        {
            pContext = new ScXMLContentChangeContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
        else if (IsXMLToken(rLocalName, XML_INSERTION))
        {
            pContext = new ScXMLInsertionContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
        else if (IsXMLToken(rLocalName, XML_DELETION))
        {
            pContext = new ScXMLDeletionContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
        else if (IsXMLToken(rLocalName, XML_MOVEMENT))
        {
            pContext = new ScXMLMovementContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
        else if (IsXMLToken(rLocalName, XML_REJECTION))
        {
            pContext = new ScXMLRejectionContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLTrackedChangesContext::EndElement()
{
}

ScXMLChangeInfoContext::ScXMLChangeInfoContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    aInfo(),
    nParagraphCount(0)
{
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_OFFICE)
        {
            if (IsXMLToken(aLocalName, XML_CHG_AUTHOR))
            {
                aInfo.sUser = sValue;
            }
            else if (IsXMLToken(aLocalName, XML_CHG_DATE_TIME))
            {
                GetScImport().GetMM100UnitConverter().convertDateTime(aInfo.aDateTime, sValue);
            }
        }
    }
}

ScXMLChangeInfoContext::~ScXMLChangeInfoContext()
{
}

SvXMLImportContext *ScXMLChangeInfoContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_TEXT) && (IsXMLToken(rLocalName, XML_P)) )
    {
        if(nParagraphCount)
            sBuffer.append(static_cast<sal_Unicode>('\n'));
        nParagraphCount++;
        pContext = new ScXMLContentContext( GetScImport(), nPrefix, rLocalName, xAttrList, sBuffer);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLChangeInfoContext::EndElement()
{
    aInfo.sComment = sBuffer.makeStringAndClear();
    pChangeTrackingImportHelper->SetActionInfo(aInfo);
}

ScXMLBigRangeContext::ScXMLBigRangeContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScBigRange& rTempBigRange ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rBigRange(rTempBigRange)
{
    sal_Bool bColumn(sal_False);
    sal_Bool bRow(sal_False);
    sal_Bool bTable(sal_False);
    sal_Int32 nColumn;
    sal_Int32 nRow;
    sal_Int32 nTable;
    sal_Int32 nStartColumn;
    sal_Int32 nEndColumn;
    sal_Int32 nStartRow;
    sal_Int32 nEndRow;
    sal_Int32 nStartTable;
    sal_Int32 nEndTable;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_COLUMN))
            {
                SvXMLUnitConverter::convertNumber(nColumn, sValue);
                bColumn = sal_True;
            }
            else if (IsXMLToken(aLocalName, XML_ROW))
            {
                SvXMLUnitConverter::convertNumber(nRow, sValue);
                bRow = sal_True;
            }
            else if (IsXMLToken(aLocalName, XML_TABLE))
            {
                SvXMLUnitConverter::convertNumber(nTable, sValue);
                bTable = sal_True;
            }
            else if (IsXMLToken(aLocalName, XML_START_COLUMN))
                SvXMLUnitConverter::convertNumber(nStartColumn, sValue);
            else if (IsXMLToken(aLocalName, XML_END_COLUMN))
                SvXMLUnitConverter::convertNumber(nEndColumn, sValue);
            else if (IsXMLToken(aLocalName, XML_START_ROW))
                SvXMLUnitConverter::convertNumber(nStartRow, sValue);
            else if (IsXMLToken(aLocalName, XML_END_ROW))
                SvXMLUnitConverter::convertNumber(nEndRow, sValue);
            else if (IsXMLToken(aLocalName, XML_START_TABLE))
                SvXMLUnitConverter::convertNumber(nStartTable, sValue);
            else if (IsXMLToken(aLocalName, XML_END_TABLE))
                SvXMLUnitConverter::convertNumber(nEndTable, sValue);
        }
    }
    if (bColumn)
        nStartColumn = nEndColumn = nColumn;
    if (bRow)
        nStartRow = nEndRow = nRow;
    if (bTable)
        nStartTable = nEndTable = nTable;
    rBigRange.Set(nStartColumn, nStartRow, nStartTable,
        nEndColumn, nEndRow, nEndTable);
}

ScXMLBigRangeContext::~ScXMLBigRangeContext()
{
}

SvXMLImportContext *ScXMLBigRangeContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLBigRangeContext::EndElement()
{
}

ScXMLCellContentDeletionContext::ScXMLCellContentDeletionContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pCell(NULL),
    nMatrixFlag(MM_NONE),
    nID(0),
    bBigRange(sal_False),
    nType(NUMBERFORMAT_ALL),
    bContainsCell(sal_False),
    nMatrixCols(0),
    nMatrixRows(0)
{
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
                nID = pChangeTrackingImportHelper->GetIDFromString(sValue);
        }
    }
}

ScXMLCellContentDeletionContext::~ScXMLCellContentDeletionContext()
{
}

SvXMLImportContext *ScXMLCellContentDeletionContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLocalName, XML_CHANGE_TRACK_TABLE_CELL))
        {
            bContainsCell = sal_True;
            pContext = new ScXMLChangeCellContext(GetScImport(), nPrefix, rLocalName, xAttrList,
                pCell, sFormulaAddress, sFormula, fValue, nType, nMatrixFlag, nMatrixCols, nMatrixRows );
        }
        else if (IsXMLToken(rLocalName, XML_CELL_ADDRESS))
        {
            DBG_ASSERT(!nID, "a action with a ID should not contain a BigRange");
            bBigRange = sal_True;
            pContext = new ScXMLBigRangeContext(GetScImport(), nPrefix, rLocalName, xAttrList, aBigRange);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLCellContentDeletionContext::EndElement()
{
    ScMyCellInfo* pCellInfo = new ScMyCellInfo(pCell, sFormulaAddress, sFormula, fValue, nType,
            nMatrixFlag, nMatrixCols, nMatrixRows);
    if (nID)
        pChangeTrackingImportHelper->AddDeleted(nID, pCellInfo);
    else
        pChangeTrackingImportHelper->AddGenerated(pCellInfo, aBigRange);
}

ScXMLDependenceContext::ScXMLDependenceContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_uInt32 nID;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
                nID = pChangeTrackingImportHelper->GetIDFromString(sValue);
        }
    }
    pChangeTrackingImportHelper->AddDependence(nID);
}

ScXMLDependenceContext::~ScXMLDependenceContext()
{
}

SvXMLImportContext *ScXMLDependenceContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    // here are no elements

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLDependenceContext::EndElement()
{
}

ScXMLDependingsContext::ScXMLDependingsContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    // here are no attributes
}

ScXMLDependingsContext::~ScXMLDependingsContext()
{
}

SvXMLImportContext *ScXMLDependingsContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLocalName, XML_DEPENDENCE))
            pContext = new ScXMLDependenceContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLDependingsContext::EndElement()
{
}

ScXMLChangeDeletionContext::ScXMLChangeDeletionContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_uInt32 nID;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
                nID = pChangeTrackingImportHelper->GetIDFromString(sValue);
        }
    }
    pChangeTrackingImportHelper->AddDeleted(nID);
}

ScXMLChangeDeletionContext::~ScXMLChangeDeletionContext()
{
}

SvXMLImportContext *ScXMLChangeDeletionContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    // here are no elements

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLChangeDeletionContext::EndElement()
{
}

ScXMLDeletionsContext::ScXMLDeletionsContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    // here are no attributes
}

ScXMLDeletionsContext::~ScXMLDeletionsContext()
{
}

SvXMLImportContext *ScXMLDeletionsContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLocalName, XML_CHANGE_DELETION))
            pContext = new ScXMLChangeDeletionContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (IsXMLToken(rLocalName, XML_CELL_CONTENT_DELETION))
            pContext = new ScXMLCellContentDeletionContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLDeletionsContext::EndElement()
{
}

ScXMLChangeTextPContext::ScXMLChangeTextPContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xTempAttrList,
                                      ScXMLChangeCellContext* pTempChangeCellContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pTextPContext(NULL),
    sLName(rLName),
    xAttrList(xTempAttrList),
    nPrefix(nPrfx),
    sText()
{
    pChangeCellContext = pTempChangeCellContext;
    // here are no attributes
}

ScXMLChangeTextPContext::~ScXMLChangeTextPContext()
{
    if (pTextPContext)
        delete pTextPContext;
}

SvXMLImportContext *ScXMLChangeTextPContext::CreateChildContext( USHORT nTempPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xTempAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_TEXT) && (IsXMLToken(rLName, XML_S)) && !pTextPContext)
    {
        sal_Int32 nRepeat(0);
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; i++ )
        {
            ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
            ::rtl::OUString sValue = xAttrList->getValueByIndex( i );
            ::rtl::OUString aLocalName;
            USHORT nPrfx = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                                sAttrName, &aLocalName );
            if ((nPrfx == XML_NAMESPACE_TEXT) && (IsXMLToken(aLocalName, XML_C)))
                nRepeat = sValue.toInt32();
        }
        if (nRepeat)
            for (sal_Int32 j = 0; j < nRepeat; j++)
                sText.append(static_cast<sal_Unicode>(' '));
        else
            sText.append(static_cast<sal_Unicode>(' '));
    }
    else
    {
        if (!pChangeCellContext->IsEditCell())
            pChangeCellContext->CreateTextPContext(sal_False);
        sal_Bool bWasContext (sal_True);
        if (!pTextPContext)
        {
            bWasContext = sal_False;
            pTextPContext = GetScImport().GetTextImport()->CreateTextChildContext(
                                    GetScImport(), nPrefix, sLName, xAttrList);
        }
        if (pTextPContext)
        {
            if (!bWasContext)
                pTextPContext->Characters(sText.makeStringAndClear());
            pContext = pTextPContext->CreateChildContext(nTempPrefix, rLName, xTempAttrList);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLChangeTextPContext::Characters( const ::rtl::OUString& rChars )
{
    if (!pTextPContext)
        sText.append(rChars);
    else
        pTextPContext->Characters(rChars);
}

void ScXMLChangeTextPContext::EndElement()
{
    if (!pTextPContext)
        pChangeCellContext->SetText(sText.makeStringAndClear());
}

ScXMLChangeCellContext::ScXMLChangeCellContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScBaseCell*& rTempOldCell, ::rtl::OUString& rAddress, ::rtl::OUString& rFormula,
                                            double& fDateTimeValue, sal_uInt16& nType,
                                            sal_uInt8& nMatrixFlag, sal_Int32& nMatrixCols, sal_Int32& nMatrixRows ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    bEmpty(sal_True),
    bFirstParagraph(sal_True),
    bFormula(sal_False),
    bString(sal_True),
    pEditTextObj(NULL),
    rOldCell(rTempOldCell),
    rType(nType),
    rDateTimeValue(fDateTimeValue)
{
    sal_Bool bIsMatrix(sal_False);
    sal_Bool bIsCoveredMatrix(sal_False);
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_VALUE_TYPE))
            {
                if (IsXMLToken(sValue, XML_FLOAT))
                    bString = sal_False;
                else if (IsXMLToken(sValue, XML_DATE))
                    rType = NUMBERFORMAT_DATE;
                else if (IsXMLToken(sValue, XML_TIME))
                    rType = NUMBERFORMAT_TIME;
            }
            else if (IsXMLToken(aLocalName, XML_VALUE))
            {
                SvXMLUnitConverter::convertDouble(fValue, sValue);
                bEmpty = sal_False;
            }
            else if (IsXMLToken(aLocalName, XML_DATE_VALUE))
            {
                if (GetScImport().GetMM100UnitConverter().setNullDate(GetScImport().GetModel()))
                    GetScImport().GetMM100UnitConverter().convertDateTime(rDateTimeValue, sValue);
            }
            else if (IsXMLToken(aLocalName, XML_TIME_VALUE))
            {
                GetScImport().GetMM100UnitConverter().convertTime(rDateTimeValue, sValue);
            }
            else if (IsXMLToken(aLocalName, XML_FORMULA))
            {
                bEmpty = sal_False;
                rFormula = sValue;
                ScXMLConverter::ParseFormula(rFormula);
                bFormula = sal_True;
            }
            else if (IsXMLToken(aLocalName, XML_CELL_ADDRESS))
            {
                rAddress = sValue;
            }
            else if (IsXMLToken(aLocalName, XML_MATRIX_COVERED))
            {
                bIsCoveredMatrix = IsXMLToken(sValue, XML_TRUE);
            }
            else if (IsXMLToken(aLocalName, XML_NUMBER_MATRIX_COLUMNS_SPANNED))
            {
                bIsMatrix = sal_True;
                SvXMLUnitConverter::convertNumber(nMatrixCols, sValue);
            }
            else if (IsXMLToken(aLocalName, XML_NUMBER_MATRIX_ROWS_SPANNED))
            {
                bIsMatrix = sal_True;
                SvXMLUnitConverter::convertNumber(nMatrixRows, sValue);
            }
        }
    }
    if (bIsCoveredMatrix)
        nMatrixFlag = MM_REFERENCE;
    else if (bIsMatrix && nMatrixRows && nMatrixCols)
        nMatrixFlag = MM_FORMULA;
}

ScXMLChangeCellContext::~ScXMLChangeCellContext()
{
}

SvXMLImportContext *ScXMLChangeCellContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_TEXT) && (IsXMLToken(rLocalName, XML_P)))
    {
        bEmpty = sal_False;
        if (bFirstParagraph)
        {
            pContext = new ScXMLChangeTextPContext(GetScImport(), nPrefix, rLocalName, xAttrList, this);
            bFirstParagraph = sal_False;
        }
        else
        {
            if (!pEditTextObj)
                CreateTextPContext(sal_True);
            pContext = GetScImport().GetTextImport()->CreateTextChildContext(
                GetScImport(), nPrefix, rLocalName, xAttrList);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLChangeCellContext::CreateTextPContext(sal_Bool bIsNewParagraph)
{
    if (GetScImport().GetDocument())
    {
        pEditTextObj = new ScEditEngineTextObj();
        pEditTextObj->acquire();
        pEditTextObj->GetEditEngine()->SetEditTextObjectPool(GetScImport().GetDocument()->GetEditPool());
        uno::Reference <text::XText> xText = pEditTextObj;
        if (xText.is())
        {
            uno::Reference<text::XTextCursor> xTextCursor = xText->createTextCursor();
            if (bIsNewParagraph)
            {
                xText->setString(sText);
                xTextCursor->gotoEnd(sal_False);
                uno::Reference < text::XTextRange > xTextRange (xTextCursor, uno::UNO_QUERY);
                if (xTextRange.is())
                    xText->insertControlCharacter(xTextRange, text::ControlCharacter::PARAGRAPH_BREAK, sal_False);
            }
            GetScImport().GetTextImport()->SetCursor(xTextCursor);
        }
    }
}

void ScXMLChangeCellContext::EndElement()
{
    if (!bEmpty)
    {
        if (pEditTextObj)
        {
            if (GetImport().GetTextImport()->GetCursor().is())
            {
                //GetImport().GetTextImport()->GetCursor()->gotoEnd(sal_False);
                if( GetImport().GetTextImport()->GetCursor()->goLeft( 1, sal_True ) )
                {
                    OUString sEmpty;
                    GetImport().GetTextImport()->GetText()->insertString(
                        GetImport().GetTextImport()->GetCursorAsRange(), sEmpty,
                        sal_True );
                }
            }
            if (GetScImport().GetDocument())
                rOldCell = new ScEditCell(pEditTextObj->CreateTextObject(), GetScImport().GetDocument(), GetScImport().GetDocument()->GetEditPool());
            GetScImport().GetTextImport()->ResetCursor();
            // delete pEditTextObj;
            pEditTextObj->release();
        }
        else
        {
            if (!bFormula)
            {
                if (sText.getLength() && bString)
                    rOldCell = new ScStringCell(sText);
                else
                    rOldCell = new ScValueCell(fValue);
            }
            else
            {
                // do nothing, this has to do later (on another place)
                /*ScAddress aCellPos;
                rOldCell = new ScFormulaCell(GetScImport().GetDocument(), aCellPos, sFormula);
                if (bString)
                    static_cast<ScFormulaCell*>(rOldCell)->SetString(sValue);
                else
                    static_cast<ScFormulaCell*>(rOldCell)->SetDouble(fValue);
                static_cast<ScFormulaCell*>(rOldCell)->SetInChangeTrack(sal_True);
                if (bIsCoveredMatrix)
                    static_cast<ScFormulaCell*>(rOldCell)->SetMatrixFlag(MM_REFERENCE);
                else if (bIsMatrix && nMatrixRows && nMatrixCols)
                {
                    static_cast<ScFormulaCell*>(rOldCell)->SetMatrixFlag(MM_FORMULA);
                    static_cast<ScFormulaCell*>(rOldCell)->SetMatColsRows(static_cast<sal_uInt16>(nMatrixCols), static_cast<sal_uInt16>(nMatrixRows));
                }*/
            }
        }
    }
    else
        rOldCell = NULL;
}

ScXMLPreviousContext::ScXMLPreviousContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    nID(0),
    sFormulaAddress(),
    sFormula(),
    nMatrixFlag(MM_NONE),
    pOldCell(NULL),
    nType(NUMBERFORMAT_ALL),
    nMatrixCols(0),
    nMatrixRows(0)
{
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
                nID = pChangeTrackingImportHelper->GetIDFromString(sValue);
        }
    }
}

ScXMLPreviousContext::~ScXMLPreviousContext()
{
}

SvXMLImportContext *ScXMLPreviousContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_TABLE) && (IsXMLToken(rLocalName, XML_CHANGE_TRACK_TABLE_CELL)))
        pContext = new ScXMLChangeCellContext(GetScImport(), nPrefix, rLocalName, xAttrList,
            pOldCell, sFormulaAddress, sFormula, fValue, nType, nMatrixFlag, nMatrixCols, nMatrixRows);

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLPreviousContext::EndElement()
{
    ScMyCellInfo* pCellInfo = new ScMyCellInfo(pOldCell, sFormulaAddress, sFormula, fValue, nType,
        nMatrixFlag, nMatrixCols, nMatrixRows);
    pChangeTrackingImportHelper->SetPreviousChange(nID, pCellInfo);
}

ScXMLContentChangeContext::ScXMLContentChangeContext(  ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);

    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
            {
                nActionNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (IsXMLToken(aLocalName, XML_ACCEPTANCE_STATE))
            {
                if (IsXMLToken(sValue, XML_ACCEPTED))
                    nActionState = SC_CAS_ACCEPTED;
                else if (IsXMLToken(sValue, XML_REJECTED))
                    nActionState = SC_CAS_REJECTED;
            }
            else if (IsXMLToken(aLocalName, XML_REJECTING_CHANGE_ID))
            {
                nRejectingNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(SC_CAT_CONTENT);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
}

ScXMLContentChangeContext::~ScXMLContentChangeContext()
{
}

SvXMLImportContext *ScXMLContentChangeContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_OFFICE) && (IsXMLToken(rLocalName, XML_CHANGE_INFO)))
    {
        pContext = new ScXMLChangeInfoContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }
    else if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLocalName, XML_CELL_ADDRESS))
        {
            pContext = new ScXMLBigRangeContext(GetScImport(), nPrefix, rLocalName, xAttrList, aBigRange);
        }
        else if (IsXMLToken(rLocalName, XML_DEPENDENCES))
        {
            pContext = new ScXMLDependingsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
        else if (IsXMLToken(rLocalName, XML_DELETIONS))
            pContext = new ScXMLDeletionsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (IsXMLToken(rLocalName, XML_PREVIOUS))
        {
            pContext = new ScXMLPreviousContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLContentChangeContext::EndElement()
{
    pChangeTrackingImportHelper->SetBigRange(aBigRange);
    pChangeTrackingImportHelper->EndChangeAction();
}

ScXMLInsertionContext::ScXMLInsertionContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    sal_Int32 nPosition(0);
    sal_Int32 nCount(1);
    sal_Int32 nTable(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);
    ScChangeActionType nActionType(SC_CAT_INSERT_COLS);

    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
            {
                nActionNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (IsXMLToken(aLocalName, XML_ACCEPTANCE_STATE))
            {
                if (IsXMLToken(sValue, XML_ACCEPTED))
                    nActionState = SC_CAS_ACCEPTED;
                else if (IsXMLToken(sValue, XML_REJECTED))
                    nActionState = SC_CAS_REJECTED;
            }
            else if (IsXMLToken(aLocalName, XML_REJECTING_CHANGE_ID))
            {
                nRejectingNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (IsXMLToken(aLocalName, XML_TYPE))
            {
                if (IsXMLToken(sValue, XML_ROW))
                    nActionType = SC_CAT_INSERT_ROWS;
                else if (IsXMLToken(sValue, XML_TABLE))
                    nActionType = SC_CAT_INSERT_TABS;
            }
            else if (IsXMLToken(aLocalName, XML_POSITION))
            {
                SvXMLUnitConverter::convertNumber(nPosition, sValue);
            }
            else if (IsXMLToken(aLocalName, XML_TABLE))
            {
                SvXMLUnitConverter::convertNumber(nTable, sValue);
            }
            else if (IsXMLToken(aLocalName, XML_COUNT))
            {
                SvXMLUnitConverter::convertNumber(nCount, sValue);
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(nActionType);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
    pChangeTrackingImportHelper->SetPosition(nPosition, nCount, nTable);
}

ScXMLInsertionContext::~ScXMLInsertionContext()
{
}

SvXMLImportContext *ScXMLInsertionContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_OFFICE) && (IsXMLToken(rLocalName, XML_CHANGE_INFO)))
    {
        pContext = new ScXMLChangeInfoContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }
    else if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLocalName, XML_DEPENDENCES))
            pContext = new ScXMLDependingsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (IsXMLToken(rLocalName, XML_DELETIONS))
            pContext = new ScXMLDeletionsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLInsertionContext::EndElement()
{
    pChangeTrackingImportHelper->EndChangeAction();
}

ScXMLInsertionCutOffContext::ScXMLInsertionCutOffContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_uInt32 nID(0);
    sal_Int32 nPosition(0);
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
            {
                nID = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (IsXMLToken(aLocalName, XML_POSITION))
            {
                SvXMLUnitConverter::convertNumber(nPosition, sValue);
            }
        }
    }
    pChangeTrackingImportHelper->SetInsertionCutOff(nID, nPosition);
}

ScXMLInsertionCutOffContext::~ScXMLInsertionCutOffContext()
{
}

SvXMLImportContext *ScXMLInsertionCutOffContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLInsertionCutOffContext::EndElement()
{
}

ScXMLMovementCutOffContext::ScXMLMovementCutOffContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_uInt32 nID(0);
    sal_Int32 nPosition(0);
    sal_Int32 nStartPosition(0);
    sal_Int32 nEndPosition(0);
    sal_Bool bPosition(sal_False);
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
            {
                nID = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (IsXMLToken(aLocalName, XML_POSITION))
            {
                bPosition = sal_True;
                SvXMLUnitConverter::convertNumber(nPosition, sValue);
            }
            else if (IsXMLToken(aLocalName, XML_START_POSITION))
            {
                SvXMLUnitConverter::convertNumber(nStartPosition, sValue);
            }
            else if (IsXMLToken(aLocalName, XML_END_POSITION))
            {
                SvXMLUnitConverter::convertNumber(nEndPosition, sValue);
            }
        }
    }
    if (bPosition)
        nStartPosition = nEndPosition = nPosition;
    pChangeTrackingImportHelper->AddMoveCutOff(nID, nStartPosition, nEndPosition);
}

ScXMLMovementCutOffContext::~ScXMLMovementCutOffContext()
{
}

SvXMLImportContext *ScXMLMovementCutOffContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLMovementCutOffContext::EndElement()
{
}

ScXMLCutOffsContext::ScXMLCutOffsContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    // here are no attributes
}

ScXMLCutOffsContext::~ScXMLCutOffsContext()
{
}

SvXMLImportContext *ScXMLCutOffsContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLocalName, XML_INSERTION_CUT_OFF))
            pContext = new ScXMLInsertionCutOffContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (IsXMLToken(rLocalName, XML_MOVEMENT_CUT_OFF))
            pContext = new ScXMLMovementCutOffContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLCutOffsContext::EndElement()
{
}

ScXMLDeletionContext::ScXMLDeletionContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    sal_Int32 nPosition(0);
    sal_Int32 nMultiSpanned(0);
    sal_Int32 nTable(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);
    ScChangeActionType nActionType(SC_CAT_DELETE_COLS);

    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
            {
                nActionNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (IsXMLToken(aLocalName, XML_ACCEPTANCE_STATE))
            {
                if (IsXMLToken(sValue, XML_ACCEPTED))
                    nActionState = SC_CAS_ACCEPTED;
                else if (IsXMLToken(sValue, XML_REJECTED))
                    nActionState = SC_CAS_REJECTED;
            }
            else if (IsXMLToken(aLocalName, XML_REJECTING_CHANGE_ID))
            {
                nRejectingNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (IsXMLToken(aLocalName, XML_TYPE))
            {
                if (IsXMLToken(sValue, XML_ROW))
                {
                    nActionType = SC_CAT_DELETE_ROWS;
                }
                else if (IsXMLToken(aLocalName, XML_TABLE))
                {
                    nActionType = SC_CAT_DELETE_TABS;
                }
            }
            else if (IsXMLToken(aLocalName, XML_POSITION))
            {
                SvXMLUnitConverter::convertNumber(nPosition, sValue);
            }
            else if (IsXMLToken(aLocalName, XML_TABLE))
            {
                SvXMLUnitConverter::convertNumber(nTable, sValue);
            }
            else if (IsXMLToken(aLocalName, XML_MULTI_DELETION_SPANNED))
            {
                SvXMLUnitConverter::convertNumber(nMultiSpanned, sValue);
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(nActionType);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
    pChangeTrackingImportHelper->SetPosition(nPosition, 1, nTable);
    pChangeTrackingImportHelper->SetMultiSpanned(static_cast<sal_Int16>(nMultiSpanned));
}

ScXMLDeletionContext::~ScXMLDeletionContext()
{
}

SvXMLImportContext *ScXMLDeletionContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_OFFICE) && (IsXMLToken(rLocalName, XML_CHANGE_INFO)))
    {
        pContext = new ScXMLChangeInfoContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }
    else if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLocalName, XML_DEPENDENCES))
            pContext = new ScXMLDependingsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (IsXMLToken(rLocalName, XML_DELETIONS))
            pContext = new ScXMLDeletionsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (IsXMLToken(rLocalName, XML_CUT_OFFS) || rLocalName.equalsAsciiL("cut_offs", 8))
            pContext = new ScXMLCutOffsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else
            DBG_ERROR("don't know this");
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLDeletionContext::EndElement()
{
    pChangeTrackingImportHelper->EndChangeAction();
}

ScXMLMovementContext::ScXMLMovementContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);

    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
            {
                nActionNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (IsXMLToken(aLocalName, XML_ACCEPTANCE_STATE))
            {
                if (IsXMLToken(sValue, XML_ACCEPTED))
                    nActionState = SC_CAS_ACCEPTED;
                else if (IsXMLToken(sValue, XML_REJECTED))
                    nActionState = SC_CAS_REJECTED;
            }
            else if (IsXMLToken(aLocalName, XML_REJECTING_CHANGE_ID))
            {
                nRejectingNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(SC_CAT_MOVE);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
}

ScXMLMovementContext::~ScXMLMovementContext()
{
}

SvXMLImportContext *ScXMLMovementContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_OFFICE) && (IsXMLToken(rLocalName, XML_CHANGE_INFO)))
    {
        pContext = new ScXMLChangeInfoContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }
    else if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLocalName, XML_DEPENDENCES))
            pContext = new ScXMLDependingsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (IsXMLToken(rLocalName, XML_DELETIONS))
            pContext = new ScXMLDeletionsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (IsXMLToken(rLocalName, XML_SOURCE_RANGE_ADDRESS))
            pContext = new ScXMLBigRangeContext(GetScImport(), nPrefix, rLocalName, xAttrList, aSourceRange);
        else if (IsXMLToken(rLocalName, XML_TARGET_RANGE_ADDRESS))
            pContext = new ScXMLBigRangeContext(GetScImport(), nPrefix, rLocalName, xAttrList, aTargetRange);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLMovementContext::EndElement()
{
    pChangeTrackingImportHelper->SetMoveRanges(aSourceRange, aTargetRange);
    pChangeTrackingImportHelper->EndChangeAction();
}

ScXMLRejectionContext::ScXMLRejectionContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);

    pChangeTrackingImportHelper = pTempChangeTrackingImportHelper;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
            {
                nActionNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (IsXMLToken(aLocalName, XML_ACCEPTANCE_STATE))
            {
                if (IsXMLToken(sValue, XML_ACCEPTED))
                    nActionState = SC_CAS_ACCEPTED;
                else if (IsXMLToken(sValue, XML_REJECTED))
                    nActionState = SC_CAS_REJECTED;
            }
            else if (IsXMLToken(aLocalName, XML_REJECTING_CHANGE_ID))
            {
                nRejectingNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(SC_CAT_MOVE);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
}

ScXMLRejectionContext::~ScXMLRejectionContext()
{
}

SvXMLImportContext *ScXMLRejectionContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_OFFICE) && (IsXMLToken(rLocalName, XML_CHANGE_INFO)))
    {
        pContext = new ScXMLChangeInfoContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }
    else if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLocalName, XML_DEPENDENCES))
            pContext = new ScXMLDependingsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (IsXMLToken(rLocalName, XML_DELETIONS))
            pContext = new ScXMLDeletionsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLRejectionContext::EndElement()
{
    pChangeTrackingImportHelper->EndChangeAction();
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
