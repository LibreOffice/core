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

#ifndef _SC_XMLDETECTIVECONTEXT_HXX
#define _SC_XMLDETECTIVECONTEXT_HXX

#ifndef _XMLOFF_XMLIMP_HXX
#include <bf_xmloff/xmlimp.hxx>
#endif

#ifndef _COM_SUN_STAR_TABLE_CELLRANGEADDRESS_HPP_
#include <com/sun/star/table/CellRangeAddress.hpp>
#endif

#ifndef SC_DETFUNC_HXX
#include "detfunc.hxx"
#endif
#ifndef SC_DETDATA_HXX
#include "detdata.hxx"
#endif

#ifndef __SGI_STL_LIST
#include <list>
#endif
namespace binfilter {

class ScXMLImport;


//___________________________________________________________________

struct ScMyImpDetectiveObj
{
    ScRange						aSourceRange;
    ScDetectiveObjType			eObjType;
    sal_Bool					bHasError : 1;

                                ScMyImpDetectiveObj();
};

typedef ::std::vector< ScMyImpDetectiveObj > ScMyImpDetectiveObjVec;


//___________________________________________________________________

struct ScMyImpDetectiveOp
{
    ScAddress					aPosition;
    ScDetOpType					eOpType;
    sal_Int32					nIndex;

    inline						ScMyImpDetectiveOp() : nIndex( -1 )	{}
    sal_Bool					operator<(const ScMyImpDetectiveOp& rDetOp) const;
};

typedef ::std::list< ScMyImpDetectiveOp > ScMyImpDetectiveOpList;

class ScMyImpDetectiveOpArray
{
private:
    ScMyImpDetectiveOpList		aDetectiveOpList;

public:
    inline						ScMyImpDetectiveOpArray() :
                                    aDetectiveOpList()	{}

    inline void					AddDetectiveOp( const ScMyImpDetectiveOp& rDetOp )
                                    { aDetectiveOpList.push_back( rDetOp ); }

    void						Sort();
    sal_Bool					GetFirstOp( ScMyImpDetectiveOp& rDetOp );
};


//___________________________________________________________________

class ScXMLDetectiveContext : public SvXMLImportContext
{
private:
    ScMyImpDetectiveObjVec*		pDetectiveObjVec;

    const ScXMLImport&			GetScImport() const	{ return (const ScXMLImport&)GetImport(); }
    ScXMLImport&				GetScImport()		{ return (ScXMLImport&)GetImport(); }

public:
                                ScXMLDetectiveContext(
                                    ScXMLImport& rImport,
                                    USHORT nPrfx,
                                    const ::rtl::OUString& rLName,
                                    ScMyImpDetectiveObjVec* pNewDetectiveObjVec
                                    );
    virtual						~ScXMLDetectiveContext();

    virtual SvXMLImportContext*	CreateChildContext(
                                    USHORT nPrefix,
                                    const ::rtl::OUString& rLocalName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList
                                    );
    virtual void				EndElement();
};


//___________________________________________________________________

class ScXMLDetectiveHighlightedContext : public SvXMLImportContext
{
private:
    ScMyImpDetectiveObjVec*		pDetectiveObjVec;
    ScMyImpDetectiveObj			aDetectiveObj;
    sal_Bool					bValid : 1;

    const ScXMLImport&			GetScImport() const	{ return (const ScXMLImport&)GetImport(); }
    ScXMLImport&				GetScImport()		{ return (ScXMLImport&)GetImport(); }

public:
                                ScXMLDetectiveHighlightedContext(
                                    ScXMLImport& rImport,
                                    USHORT nPrfx,
                                    const ::rtl::OUString& rLName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
                                    ScMyImpDetectiveObjVec* pNewDetectiveObjVec
                                    );
    virtual						~ScXMLDetectiveHighlightedContext();

    virtual SvXMLImportContext*	CreateChildContext(
                                    USHORT nPrefix,
                                    const ::rtl::OUString& rLocalName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList
                                    );
    virtual void				EndElement();
};


//___________________________________________________________________

class ScXMLDetectiveOperationContext : public SvXMLImportContext
{
private:
    ScMyImpDetectiveOp			aDetectiveOp;
    sal_Bool					bHasType : 1;

    const ScXMLImport&			GetScImport() const	{ return (const ScXMLImport&)GetImport(); }
    ScXMLImport&				GetScImport()		{ return (ScXMLImport&)GetImport(); }

public:
                                ScXMLDetectiveOperationContext(
                                    ScXMLImport& rImport,
                                    USHORT nPrfx,
                                    const ::rtl::OUString& rLName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList
                                    );
    virtual						~ScXMLDetectiveOperationContext();

    virtual SvXMLImportContext*	CreateChildContext(
                                    USHORT nPrefix,
                                    const ::rtl::OUString& rLocalName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList
                                    );
    virtual void				EndElement();
};


} //namespace binfilter
#endif

