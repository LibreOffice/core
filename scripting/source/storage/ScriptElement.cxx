/*************************************************************************
 *
 *  $RCSfile: ScriptElement.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: lkovacs $ $Date: 2002-09-23 14:08:29 $
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

#ifndef _SCRIPT_FRAMEWORK_STORAGE_SCRIPT_ELEMENT_HXX_
#include "ScriptElement.hxx"
#endif
#include <util/util.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script::framework;

namespace scripting_impl
{

//*************************************************************************
/**
    Construct a ScriptElement from a ScriptImplInfo object

    @param sII
    the ScriptImplInfoObject
*/
ScriptElement::ScriptElement(storage::ScriptImplInfo & sII) :
        m_sII( sII ),
        XMLElement(OUSTR("script"))
{
    OSL_TRACE("ScriptElement ctor called\n");

    addAttribute(OUSTR("language"), sII.scriptLanguage);
    addAttribute(OUSTR("deploymentdir"), sII.scriptRoot);

    {
        XMLElement* xel = new XMLElement(OUSTR("logicalname"));
        xel->addAttribute(OUSTR("value"), sII.logicalName);
        Reference <xml::sax::XAttributeList> xal(xel);
        addSubElement(xal);
    }

    {
        XMLElement* xel = new XMLElement(OUSTR("languagename"));
        xel->addAttribute(OUSTR("value"), sII.functionName);
        xel->addAttribute(OUSTR("location"), sII.scriptLocation);
        Reference <xml::sax::XAttributeList> xal(xel);
        addSubElement(xal);
    }

    storage::ScriptDepFile *pArray = sII.scriptDependencies.getArray();
    sal_Int32 len = sII.scriptDependencies.getLength();

    if (len > 0)
    {
        XMLElement* xel = new XMLElement(OUSTR("dependencies"));

        for(sal_Int32 i = 0; i < len; i++)
        {
            XMLElement* subxel =  new XMLElement(OUSTR("dependfile"));
            subxel->addAttribute(OUSTR("name"), pArray[i].fileName);
            if(pArray[i].isDeliverable)
            {
                subxel->addAttribute(OUSTR("isdeliverable"), OUSTR("yes"));
            }
            else
            {
                subxel->addAttribute(OUSTR("isdeliverable"), OUSTR("no"));
            }
            Reference <xml::sax::XAttributeList> subxal(subxel);
            xel->addSubElement(subxal);
        }

        Reference <xml::sax::XAttributeList> xal(xel);
        addSubElement(xal);
    }

    if(sII.scriptDescription.getLength() > 0)
    {
        XMLElement* xel = new XMLElement(OUSTR("description"),
                                         sII.scriptDescription);
        Reference< xml::sax::XAttributeList > xal(xel);
        addSubElement(xal);
    }

    storage::ScriptDeliverFile *pt_Array = sII.parcelDelivers.getArray();
    sal_Int32 len_d = sII.parcelDelivers.getLength();

    if(len_d > 0)
    {
        XMLElement* xel = new XMLElement(OUSTR("delivery"));

        for(sal_Int32 i = 0; i < len_d; i++)
        {
            XMLElement* subxel = new XMLElement(OUSTR("deliverfile"));
            subxel->addAttribute(OUSTR("name"), pt_Array[i].fileName);
            subxel->addAttribute(OUSTR("type"), pt_Array[i].fileType);
            Reference < xml::sax::XAttributeList > subxal(subxel);
            xel->addSubElement(subxal);
        }

        Reference <xml::sax::XAttributeList> xal(xel);
        addSubElement(xal);
    }
}

//*************************************************************************
ScriptElement::~ScriptElement() SAL_THROW(())
{
}

}
