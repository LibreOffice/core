/*************************************************************************
 *
 *  $RCSfile: PropertyActionsOASIS.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:57:43 $
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

#ifndef _XMLOFF_PROPERTYACTIONSOASIS_HXX
#define _XMLOFF_PROPERTYACTIONSOASIS_HXX

#ifndef _XMLOFF_TRANSFORMERACTION_HXX
#include "TransformerAction.hxx"
#endif
#ifndef _XMLOFF_TRANSFORMERACTIONINIT_HXX
#include "TransformerActionInit.hxx"
#endif
#ifndef _XMLOFF_ATTRTRANSFORMERACTION_HXX
#include "AttrTransformerAction.hxx"
#endif

enum XMLPropOASISTransformerAction
{
    XML_OPTACTION_LINE_MODE=XML_ATACTION_USER_DEFINED,
    XML_OPTACTION_UNDERLINE_TYPE,
    XML_OPTACTION_UNDERLINE_STYLE,
    XML_OPTACTION_UNDERLINE_WIDTH,
    XML_OPTACTION_LINETHROUGH_TYPE,
    XML_OPTACTION_LINETHROUGH_STYLE,
    XML_OPTACTION_LINETHROUGH_WIDTH,
    XML_OPTACTION_LINETHROUGH_TEXT,
    XML_OPTACTION_KEEP_WITH_NEXT,
    XML_OPTACTION_INTERPOLATION,
    XML_OPTACTION_INTERVAL_MAJOR,
    XML_OPTACTION_INTERVAL_MINOR_DIVISOR,
    XML_OPTACTION_END=XML_ATACTION_END
};

extern XMLTransformerActionInit aGraphicPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aDrawingPagePropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aPageLayoutPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aHeaderFooterPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aTextPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aParagraphPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aSectionPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aTablePropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aTableColumnPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aTableRowPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aTableCellPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aListLevelPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aChartPropertyOASISAttrActionTable[];

#endif  //  _XMLOFF_PROPERTYACTIONSOASIS_HXX
