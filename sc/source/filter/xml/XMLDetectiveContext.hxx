/*************************************************************************
 *
 *  $RCSfile: XMLDetectiveContext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sab $ $Date: 2000-11-14 18:15:26 $
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

#ifndef _SC_XMLDETECTIVECONTEXT_HXX
#define _SC_XMLDETECTIVECONTEXT_HXX

#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
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

class ScXMLImport;


//___________________________________________________________________

struct ScMyImpDetectiveObj
{
    ScRange                     aSourceRange;
    ScDetectiveObjType          eObjType;
    sal_Bool                    bHasError;

                                ScMyImpDetectiveObj();
};

typedef ::std::vector< ScMyImpDetectiveObj > ScMyImpDetectiveObjVec;


//___________________________________________________________________

struct ScMyImpDetectiveOp
{
    ScAddress                   aPosition;
    ScDetOpType                 eOpType;
    sal_Int32                   nIndex;

    inline                      ScMyImpDetectiveOp() : nIndex( -1 ) {}
};

typedef ::std::vector< ScMyImpDetectiveOp > ScMyImpDetectiveOpVec;

class ScMyImpDetectiveOpArray
{
private:
    ScMyImpDetectiveOpVec       aDetectiveOpVec;

public:
    inline                      ScMyImpDetectiveOpArray() :
                                    aDetectiveOpVec()   {}

    inline void                 AddDetectiveOp( const ScMyImpDetectiveOp& rDetOp )
                                    { aDetectiveOpVec.push_back( rDetOp ); }

    void                        Sort();
    sal_Bool                    GetFirstOp( ScMyImpDetectiveOp& rDetOp );
};


//___________________________________________________________________

class ScXMLDetectiveContext : public SvXMLImportContext
{
private:
    ScMyImpDetectiveObjVec&     rDetectiveObjVec;

    const ScXMLImport&          GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport&                GetScImport()       { return (ScXMLImport&)GetImport(); }

public:
                                ScXMLDetectiveContext(
                                    ScXMLImport& rImport,
                                    USHORT nPrfx,
                                    const ::rtl::OUString& rLName,
                                    ScMyImpDetectiveObjVec& rNewDetectiveObjVec
                                    );
    virtual                     ~ScXMLDetectiveContext();

    virtual SvXMLImportContext* CreateChildContext(
                                    USHORT nPrefix,
                                    const ::rtl::OUString& rLocalName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList
                                    );
    virtual void                EndElement();
};


//___________________________________________________________________

class ScXMLDetectiveHighlightedContext : public SvXMLImportContext
{
private:
    ScMyImpDetectiveObjVec&     rDetectiveObjVec;
    ScMyImpDetectiveObj         aDetectiveObj;
    sal_Bool                    bValid;

    const ScXMLImport&          GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport&                GetScImport()       { return (ScXMLImport&)GetImport(); }

public:
                                ScXMLDetectiveHighlightedContext(
                                    ScXMLImport& rImport,
                                    USHORT nPrfx,
                                    const ::rtl::OUString& rLName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
                                    ScMyImpDetectiveObjVec& rNewDetectiveObjVec
                                    );
    virtual                     ~ScXMLDetectiveHighlightedContext();

    virtual SvXMLImportContext* CreateChildContext(
                                    USHORT nPrefix,
                                    const ::rtl::OUString& rLocalName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList
                                    );
    virtual void                EndElement();
};


//___________________________________________________________________

class ScXMLDetectiveOperationContext : public SvXMLImportContext
{
private:
    ScMyImpDetectiveOp          aDetectiveOp;
    sal_Bool                    bHasType;

    const ScXMLImport&          GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport&                GetScImport()       { return (ScXMLImport&)GetImport(); }

public:
                                ScXMLDetectiveOperationContext(
                                    ScXMLImport& rImport,
                                    USHORT nPrfx,
                                    const ::rtl::OUString& rLName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList
                                    );
    virtual                     ~ScXMLDetectiveOperationContext();

    virtual SvXMLImportContext* CreateChildContext(
                                    USHORT nPrefix,
                                    const ::rtl::OUString& rLocalName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList
                                    );
    virtual void                EndElement();
};


#endif

