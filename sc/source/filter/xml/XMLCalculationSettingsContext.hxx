/*************************************************************************
 *
 *  $RCSfile: XMLCalculationSettingsContext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-01 15:18:17 $
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

#ifndef _SC_XMLCALCULATIONSETTINGSCONTEXT_HXX
#define _SC_XMLCALCULATIONSETTINGSCONTEXT_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif

class ScXMLImport;

class ScXMLCalculationSettingsContext : public SvXMLImportContext
{
    com::sun::star::util::Date aNullDate;
    double fIterationEpsilon;
    sal_Int32 nIterationCount;
    sal_uInt16 nYear2000;
    sal_Bool bIsIterationEnabled : 1;
    sal_Bool bCalcAsShown : 1;
    sal_Bool bIgnoreCase : 1;
    sal_Bool bLookUpLabels : 1;
    sal_Bool bMatchWholeCell : 1;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
public:
    ScXMLCalculationSettingsContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLCalculationSettingsContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    void SetNullDate(const com::sun::star::util::Date& aDate) { aNullDate = aDate; }
    void SetIterationStatus(const sal_Bool bValue) { bIsIterationEnabled = bValue; }
    void SetIterationCount(const sal_Int32 nValue) { nIterationCount = nValue; }
    void SetIterationEpsilon(const double fValue) { fIterationEpsilon = fValue; }
    virtual void EndElement();
};

class ScXMLNullDateContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
public:
    ScXMLNullDateContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList, ScXMLCalculationSettingsContext* pCalcSet);

    virtual ~ScXMLNullDateContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLIterationContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
public:
    ScXMLIterationContext( ScXMLImport& rImport, USHORT nPrfx,
                        const NAMESPACE_RTL(OUString)& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList, ScXMLCalculationSettingsContext* pCalcSet);

    virtual ~ScXMLIterationContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

#endif
