/*************************************************************************
 *
 *  $RCSfile: xmlHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:21:49 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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

#define XML_DB_TYPE_EQUAL                           (XML_DB_TYPES_START + 1)

namespace dbaxml
{
    class OPropertyHandlerFactory : public ::xmloff::OControlPropertyHandlerFactory
    {
    protected:
        mutable ::std::auto_ptr<XMLConstantsPropertyHandler>    m_pDisplayHandler;
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
    };
// -----------------------------------------------------------------------------
} // dbaxml
// -----------------------------------------------------------------------------
#endif // DBA_XMLHELPER_HXX

