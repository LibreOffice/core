/*************************************************************************
 *
 *  $RCSfile: SdShapeTypes.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: af $ $Date: 2002-02-05 14:34:32 $
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

#include "SdShapeTypes.hxx"
#include "AccessiblePresentationShape.hxx"

namespace accessibility {

::com::sun::star::uno::Reference<
    ::drafts::com::sun::star::accessibility::XAccessible>
    createSdAccessibleShape (const ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible>& rxParent,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape>& rxShape,
        ShapeTypeId nId)
{
    switch (nId)
    {
        case PRESENTATION_TITLE:
        case PRESENTATION_OUTLINER:
        case PRESENTATION_SUBTITLE:
        case PRESENTATION_GRAPHIC_OBJECT:
        case PRESENTATION_PAGE:
        case PRESENTATION_OLE:
        case PRESENTATION_CHART:
        case PRESENTATION_TABLE:
        case PRESENTATION_NOTES:
        case PRESENTATION_HANDOUT:
            return new AccessiblePresentationShape (rxShape, rxParent);
    }
}




ShapeTypeDescriptor aSdShapeTypeList[] = {
    {   PRESENTATION_TITLE,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.TitleTextShape"),
        createSdAccessibleShape },
    {   PRESENTATION_OUTLINER,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.OutlinerShape"),
        createSdAccessibleShape },
    {   PRESENTATION_SUBTITLE,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.SubtitleShape"),
        createSdAccessibleShape },
    {   PRESENTATION_GRAPHIC_OBJECT,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.GraphicObjectShape"),
        createSdAccessibleShape },
    {   PRESENTATION_PAGE,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.PageShape"),
        createSdAccessibleShape },
    {   PRESENTATION_OLE,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.OLE2Shape"),
        createSdAccessibleShape },
    {   PRESENTATION_CHART,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.ChartShape"),
        createSdAccessibleShape },
    {   PRESENTATION_TABLE,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.TableShape"),
        createSdAccessibleShape },
    {   PRESENTATION_NOTES,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.NotesShape"),
        createSdAccessibleShape },
    {   PRESENTATION_HANDOUT,
        ::rtl::OUString::createFromAscii ("com.sun.star.presentation.HandoutShape"),
        createSdAccessibleShape }
};




void RegisterImpressShapeTypes (void)
{
    ShapeTypeHandler::Instance().addShapeTypeList (
        PRESENTATION_HANDOUT - PRESENTATION_TITLE + 1,
        aSdShapeTypeList);
}




} // end of namespace accessibility
