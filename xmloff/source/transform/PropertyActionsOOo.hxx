/*************************************************************************
 *
 *  $RCSfile: PropertyActionsOOo.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 13:34:38 $
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

#ifndef _XMLOFF_PROPERTYACTIONSOOO_HXX
#define _XMLOFF_PROPERTYACTIONSOOO_HXX

#ifndef _XMLOFF_TRANSFORMERACTION_HXX
#include "TransformerAction.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERACTIONINIT_HXX
#include "TransformerActionInit.hxx"
#endif
#ifndef _XMLOFF_ATTRTRANSFORMERACTION_HXX
#include "AttrTransformerAction.hxx"
#endif

enum XMLPropOOOTransformerAction
{
    XML_PTACTION_LINE_MODE=XML_ATACTION_USER_DEFINED,
    XML_PTACTION_UNDERLINE,
    XML_PTACTION_LINETHROUGH,
    XML_PTACTION_KEEP_WITH_NEXT,
    XML_PTACTION_SPLINES,
    XML_ATACTION_INCH2IN_DUPLICATE,
    XML_PTACTION_INTERVAL_MAJOR,
    XML_PTACTION_INTERVAL_MINOR,
    XML_ATACTION_COPY_DUPLICATE,

    // #i25616#
    XML_PTACTION_TRANSPARENCY,

    XML_PTACTION_END=XML_TACTION_END
};

extern XMLTransformerActionInit aGraphicPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aGraphicPropertyOOoElemActionTable[];
extern XMLTransformerActionInit aDrawingPagePropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aPageLayoutPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aHeaderFooterPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aTextPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aTextPropertyOOoElemActionTable[];
extern XMLTransformerActionInit aParagraphPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aParagraphPropertyOOoElemActionTable[];
extern XMLTransformerActionInit aSectionPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aTablePropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aTableColumnPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aTableRowPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aTableCellPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aTableCellPropertyOOoElemActionTable[];
extern XMLTransformerActionInit aListLevelPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aChartPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aChartPropertyOOoElemActionTable[];

#endif  //  _XMLOFF_PROPERTYACTIONSOOO_HXX
