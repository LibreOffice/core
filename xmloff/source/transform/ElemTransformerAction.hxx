/*************************************************************************
 *
 *  $RCSfile: ElemTransformerAction.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:47:58 $
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

#ifndef _XMLOFF_ELEMTRANSFORMERACTION_HXX
#define _XMLOFF_ELEMTRANSFORMERACTION_HXX

#ifndef _XMLOFF_TRANSFORMERACTION_HXX
#include "TransformerAction.hxx"
#endif

enum XMLElemTransformerAction
{
    XML_ETACTION_EOT=XML_TACTION_EOT,   // uses for initialization only
    XML_ETACTION_COPY,                  // copy all (default)
                                        // for persistent elements: keep
                                        // elem content
    XML_ETACTION_COPY_TEXT,             // copy all
                                        // for persistent elements: keep
                                        // text content
    XML_ETACTION_COPY_CONTENT,          // ignore elem, process content
    XML_ETACTION_RENAME_ELEM,           // rename element:
                                        // - param1: namespace +
                                        //           token of local name
    XML_ETACTION_RENAME_ELEM_PROC_ATTRS,// rename element and proc attr:
                                        // - param1: elem namespace +
                                        //           token of local name
                                        // - param2: attr acation map
    XML_ETACTION_RENAME_ELEM_ADD_ATTR,  // rename element and add attr:
                                        // - param1: elem namespace +
                                        //           token of local name
                                        // - param2: attr namespace +
                                        //           token of local name
                                        // - param3: attr value
    XML_ETACTION_RENAME_ELEM_ADD_PROC_ATTR, // rename element and add attr:
                                        // - param1: elem namespace +
                                        //           token of local name
                                        // - param2: attr namespace +
                                        //           token of local name
                                        // - param3: attr value (low)
                                        //           attr action map (high)
    XML_ETACTION_RENAME_ELEM_COND,      // rename element if child of another:
                                        // - param1: namespace +
                                        //           token of new local name
                                        // - param2: namespace +
                                        //           token of parent element
    XML_ETACTION_RENAME_ELEM_PROC_ATTRS_COND,// rename element and proc attr
                                        // if child of another:
                                        // - param1: elem namespace +
                                        //           token of new local name
                                        // - param2: attr acation map
                                        // - param3: namespace +
                                        //           token of parent element
    XML_ETACTION_PROC_ATTRS,            // proc attr:
                                        // - param1: attr acation map
    XML_ETACTION_MOVE_ATTRS_TO_ELEMS,   // turn some attributes into elems
                                        // - param1: action map specifying
                                        //   the attrs that shall be moved
    XML_ETACTION_MOVE_ELEMS_TO_ATTRS,   // turn some elems into attrs
                                        // - param1: action map specifying
                                        //   the elems that shall be moved
    XML_ETACTION_PROC_ATTRS_COND,       // proc attr if child of another:
                                        // - param1: namespace +
                                        //           token of parent element
                                        // - param2: attr acation map
    XML_ETACTION_USER_DEFINED=0x80000000,// user defined actions start here
    XML_ETACTION_END=XML_TACTION_END
};

#endif  //  _XMLOFF_ELEMTRANSFORMERACTION_HXX
