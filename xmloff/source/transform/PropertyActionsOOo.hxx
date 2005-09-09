/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertyActionsOOo.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:54:26 $
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
    XML_PTACTION_SYMBOL,
    XML_PTACTION_SYMBOL_IMAGE_NAME,
    XML_PTACTION_TRANSPARENCY,
    XML_PTACTION_BREAK_INSIDE,
    XML_ATACTION_CAPTION_ESCAPE_OOO,
    XML_ATACTION_MOVE_PROTECT,
    XML_ATACTION_SIZE_PROTECT,
    XML_ATACTION_PROTECT,
    // --> OD 2005-06-10 #i50322#
    XML_ATACTION_WRITER_BACK_GRAPHIC_TRANSPARENCY,
    // <--
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
