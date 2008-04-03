/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlHelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:49:01 $
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
#ifndef DBA_XMLHELPER_HXX
#define DBA_XMLHELPER_HXX

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include <xmloff/xmlprmap.hxx>
#endif
#ifndef _XMLOFF_CONTEXTID_HXX_
#include <xmloff/contextid.hxx>
#endif
#ifndef _XMLOFF_FORMS_CONTROLPROPERTYHDL_HXX_
#include <xmloff/controlpropertyhdl.hxx>
#endif

#include <memory>

#define CTF_DB_ROWHEIGHT                            (XML_DB_CTF_START + 1)
#define CTF_DB_ISVISIBLE                            (XML_DB_CTF_START + 2)
#define CTF_DB_MASTERPAGENAME                       (XML_DB_CTF_START + 3)
#define CTF_DB_NUMBERFORMAT                         (XML_DB_CTF_START + 4)
#define CTF_DB_COLUMN_TEXT_ALIGN                    (XML_DB_CTF_START + 5)

#define XML_DB_TYPE_EQUAL                           (XML_DB_TYPES_START + 1)

namespace dbaxml
{
    class OPropertyHandlerFactory : public ::xmloff::OControlPropertyHandlerFactory
    {
    protected:
        mutable ::std::auto_ptr<XMLConstantsPropertyHandler>    m_pDisplayHandler;
        mutable ::std::auto_ptr<XMLPropertyHandler>             m_pTextAlignHandler;
    public:
        OPropertyHandlerFactory();
        virtual ~OPropertyHandlerFactory();

        virtual const XMLPropertyHandler* GetPropertyHandler(sal_Int32 _nType) const;
    };

    class OXMLHelper
    {
    public:
        static UniReference < XMLPropertySetMapper > GetTableStylesPropertySetMapper();
        static UniReference < XMLPropertySetMapper > GetColumnStylesPropertySetMapper();
        static UniReference < XMLPropertySetMapper > GetCellStylesPropertySetMapper();
        static UniReference < XMLPropertySetMapper > GetRowStylesPropertySetMapper();
    };
// -----------------------------------------------------------------------------
} // dbaxml
// -----------------------------------------------------------------------------
#endif // DBA_XMLHELPER_HXX

