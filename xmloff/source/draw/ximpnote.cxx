/*************************************************************************
 *
 *  $RCSfile: ximpnote.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:03 $
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

#pragma hdrstop

#ifndef _XIMPNOTES_HXX
#include "ximpnote.hxx"
#endif

#ifndef _COM_SUN_STAR_PRESENTATION_XPRESENTATIONPAGE_HPP_
#include <com/sun/star/presentation/XPresentationPage.hpp>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////

SdXMLNotesContext::SdXMLNotesContext( SdXMLImport& rImport,
    USHORT nPrfx, const OUString& rLocalName,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLGroupShapeContext( rImport, nPrfx, rLocalName, rShapes ),
    mbNotesMode(FALSE)
{
    if(GetSdImport().IsImpress())
    {
        // get notes page
        uno::Reference< presentation::XPresentationPage > xPresPage(rShapes, uno::UNO_QUERY);
        if(xPresPage.is())
        {
            uno::Reference< drawing::XDrawPage > xNotesDrawPage(xPresPage->getNotesPage(), uno::UNO_QUERY);
            if(xNotesDrawPage.is())
            {
                uno::Reference< drawing::XShapes > xNewShapes(xNotesDrawPage, uno::UNO_QUERY);
                if(xNewShapes.is())
                {
                    // now delete all up-to-now contained shapes from this notes page
                    while(xNewShapes->getCount())
                    {
                        uno::Reference< drawing::XShape > xShape;
                        uno::Any aAny(xNewShapes->getByIndex(0L));

                        aAny >>= xShape;

                        if(xShape.is())
                        {
                            xNewShapes->remove(xShape);
                        }
                    }

                    // set new local shapes context to notes page
                    SetLocalShapesContext(xNewShapes);
                    mbNotesMode = TRUE;
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLNotesContext::~SdXMLNotesContext()
{
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext *SdXMLNotesContext::CreateChildContext( USHORT nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    if(mbNotesMode)
    {
        // OK, notes page is set on base class, objects can be imported on notes page
        SvXMLImportContext *pContext = 0L;

        // some special objects inside presentation:notes context
        // ...







        // call parent when no own context was created
        if(!pContext)
            pContext = SdXMLGroupShapeContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

        return pContext;
    }
    else
    {
        // do not import this content, the notes page could not be accessed
        return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLNotesContext::EndElement()
{
}


