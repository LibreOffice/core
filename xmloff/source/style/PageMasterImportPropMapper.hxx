/*************************************************************************
 *
 *  $RCSfile: PageMasterImportPropMapper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sab $ $Date: 2000-10-23 15:30:39 $
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

#ifndef _XMLOFF_PAGEMASTERIMPORTPROPMAPPER_HXX
#define _XMLOFF_PAGEMASTERIMPORTPROPMAPPER_HXX

#ifndef _XMLOFF_XMLIMPPR_HXX
#include <xmloff/xmlimppr.hxx>
#endif

class PageMasterImportPropertyMapper : public SvXMLImportPropertyMapper
{
protected:

public:

    PageMasterImportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper );
    virtual ~PageMasterImportPropertyMapper();

    /** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_IMPORT flag set */
/*  virtual sal_Bool handleSpecialItem(
            XMLPropertyState& rProperty,
            ::std::vector< XMLPropertyState >& rProperties,
            const ::rtl::OUString& rValue,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap ) const;*/

    /** this method is called for every item that has the MID_FLAG_NO_ITEM_IMPORT flag set */
/*  virtual sal_Bool handleNoItem(
            sal_Int32 nIndex,
            ::std::vector< XMLPropertyState >& rProperties,
               const ::rtl::OUString& rValue,
               const SvXMLUnitConverter& rUnitConverter,
               const SvXMLNamespaceMap& rNamespaceMap ) const;*/

    /** This method is called when all attributes have been processed. It may be used to remove items that are incomplete */
    virtual void finished(
            ::std::vector< XMLPropertyState >& rProperties ) const;
};

class PageMasterHeaderImportPropertyMapper : public SvXMLImportPropertyMapper
{
protected:

public:

    PageMasterHeaderImportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper );
    virtual ~PageMasterHeaderImportPropertyMapper();

    /** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_IMPORT flag set */
/*  virtual sal_Bool handleSpecialItem(
            XMLPropertyState& rProperty,
            ::std::vector< XMLPropertyState >& rProperties,
            const ::rtl::OUString& rValue,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap ) const;*/

    /** this method is called for every item that has the MID_FLAG_NO_ITEM_IMPORT flag set */
/*  virtual sal_Bool handleNoItem(
            sal_Int32 nIndex,
            ::std::vector< XMLPropertyState >& rProperties,
               const ::rtl::OUString& rValue,
               const SvXMLUnitConverter& rUnitConverter,
               const SvXMLNamespaceMap& rNamespaceMap ) const;*/

    /** This method is called when all attributes have been processed. It may be used to remove items that are incomplete */
    virtual void finished(
            ::std::vector< XMLPropertyState >& rProperties ) const;
};

class PageMasterFooterImportPropertyMapper : public SvXMLImportPropertyMapper
{
protected:

public:

    PageMasterFooterImportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper );
    virtual ~PageMasterFooterImportPropertyMapper();

    /** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_IMPORT flag set */
/*  virtual sal_Bool handleSpecialItem(
            XMLPropertyState& rProperty,
            ::std::vector< XMLPropertyState >& rProperties,
            const ::rtl::OUString& rValue,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap ) const;*/

    /** this method is called for every item that has the MID_FLAG_NO_ITEM_IMPORT flag set */
/*  virtual sal_Bool handleNoItem(
            sal_Int32 nIndex,
            ::std::vector< XMLPropertyState >& rProperties,
               const ::rtl::OUString& rValue,
               const SvXMLUnitConverter& rUnitConverter,
               const SvXMLNamespaceMap& rNamespaceMap ) const;*/

    /** This method is called when all attributes have been processed. It may be used to remove items that are incomplete */
    virtual void finished(
            ::std::vector< XMLPropertyState >& rProperties ) const;
};
#endif
