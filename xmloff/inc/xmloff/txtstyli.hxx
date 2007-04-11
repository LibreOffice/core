/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txtstyli.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:30:27 $
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
#ifndef _XMLOFF_TXTSTYLI_HXX_
#define _XMLOFF_TXTSTYLI_HXX_

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _XMLOFF_PRSTYLEI_HXX_
#include <xmloff/prstylei.hxx>
#endif

class SvXMLTokenMap;
class XMLEventsImportContext;

class XMLOFF_DLLPUBLIC XMLTextStyleContext : public XMLPropStyleContext
{
    ::rtl::OUString             sListStyleName;
    ::rtl::OUString             sCategoryVal;
    ::rtl::OUString             sDropCapTextStyleName;
    ::rtl::OUString             sMasterPageName;
    ::rtl::OUString             sDataStyleName; // for grid columns only
    const ::rtl::OUString       sIsAutoUpdate;
    const ::rtl::OUString       sCategory;
    const ::rtl::OUString       sNumberingStyleName;
public:
    const ::rtl::OUString       sDropCapCharStyleName;
private:
    const ::rtl::OUString       sPageDescName;

    sal_Int8    nOutlineLevel;

    sal_Bool    bAutoUpdate : 1;
    sal_Bool    bHasMasterPageName : 1;

    sal_Bool bHasCombinedCharactersLetter : 1;

    // --> OD 2006-09-21 #i69523#
    // introduce import of empty list style
    sal_Bool mbListStyleSet : 1;
    // <--

    XMLEventsImportContext* pEventContext;

protected:

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const ::rtl::OUString& rLocalName,
                               const ::rtl::OUString& rValue );

public:

    TYPEINFO();

    XMLTextStyleContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            SvXMLStylesContext& rStyles, sal_uInt16 nFamily,
            sal_Bool bDefaultStyle = sal_False );
    virtual ~XMLTextStyleContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    sal_Bool IsAutoUpdate() const { return bAutoUpdate; }

    const ::rtl::OUString& GetListStyle() const { return sListStyleName; }
    // --> OD 2006-10-13 #i69629#
    inline const sal_Bool IsListStyleSet() const
    {
        return mbListStyleSet;
    }
    // <--
    const ::rtl::OUString& GetMasterPageName() const { return sMasterPageName; }
    sal_Bool HasMasterPageName() const { return bHasMasterPageName; }
    const ::rtl::OUString& GetDropCapStyleName() const { return sDropCapTextStyleName; }
    const ::rtl::OUString& GetDataStyleName() const { return sDataStyleName; }

    virtual void CreateAndInsert( sal_Bool bOverwrite );
    virtual void Finish( sal_Bool bOverwrite );
    virtual void SetDefaults();

    // overload FillPropertySet, so we can get at the combined characters
    virtual void FillPropertySet(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & rPropSet );

    inline sal_Bool HasCombinedCharactersLetter()
        { return bHasCombinedCharactersLetter; }

    const ::std::vector< XMLPropertyState > & _GetProperties() { return GetProperties(); }
};

#endif
