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

#ifndef XMLOFF_FORMSTYLES_HXX
#define XMLOFF_FORMSTYLES_HXX

#ifndef _XMLOFF_PRSTYLEI_HXX_ 
#include "prstylei.hxx"
#endif
namespace binfilter {

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OControlStyleContext
    //=====================================================================
    class OControlStyleContext : public XMLPropStyleContext
    {
    protected:
        ::rtl::OUString		m_sNumberStyleName;

    public:
        OControlStyleContext(
            SvXMLImport& _rImport,
            const sal_uInt16 _Prefix,
            const ::rtl::OUString& _rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList,
            SvXMLStylesContext& _rParentStyles,
            const sal_uInt16 _nFamily,
            const sal_Bool _bDefaultStyle
        );

        inline const ::rtl::OUString& getNumberStyleName( ) const { return m_sNumberStyleName; }

    protected:
        virtual void SetAttribute(
            sal_uInt16 _nPrefixKey,
            const ::rtl::OUString& _rLocalName,
            const ::rtl::OUString& _rValue
        );
    };

//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter
#endif // XMLOFF_FORMSTYLES_HXX

