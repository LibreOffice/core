/*************************************************************************
 *
 *  $RCSfile: AttrTransformerAction.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:43:29 $
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

#ifndef _XMLOFF_ATTRTRANSFORMERACTION_HXX
#define _XMLOFF_ATTRTRANSFORMERACTION_HXX

#ifndef _XMLOFF_TRANSFORMERACTION_HXX
#include "TransformerAction.hxx"
#endif

enum XMLAttrTransformerAction
{
    XML_ATACTION_EOT=XML_TACTION_EOT,   // uses for initialization only
    XML_ATACTION_COPY,                  // copy attr
    XML_ATACTION_RENAME,                // rename attr:
                                        // - param1: namespace +
                                        //           token of local name
    XML_ATACTION_REMOVE,                // remove attr
    XML_ATACTION_IN2INCH,               // replace "in" with "inch"
    XML_ATACTION_INS2INCHS,             // replace "in" with "inch"
                                        // multiple times
    XML_ATACTION_RENAME_IN2INCH,        // replace "in" with "inch" and rename
                                        // attr:
                                        // - param1: namespace +
                                        //           token of local name
    XML_ATACTION_INCH2IN,               // replace "inch" with "in"
    XML_ATACTION_INCHS2INS,             // replace "inch" with "in"
                                        // multiple times
    XML_ATACTION_RENAME_INCH2IN,        // replace "inch" with "in" and rename
                                        // attr:
                                        // - param1: namespace +
                                        //           token of local name
    XML_ATACTION_STYLE_FAMILY,          // NOP, used for style:family
    XML_ATACTION_DECODE_STYLE_NAME,     // NOP, used for style:name
                                        // - param1: style family
    XML_ATACTION_STYLE_DISPLAY_NAME,    // NOP, used for style:display_name
                                        // - param1: style family
    XML_ATACTION_DECODE_STYLE_NAME_REF, // NOP, used for style:name reference
                                        // - param1: style family
    XML_ATACTION_RENAME_DECODE_STYLE_NAME_REF,  // NOP, used for style:name
                                        // - param1: namespace +
                                        //           token of local name
    XML_ATACTION_ENCODE_STYLE_NAME,     // NOP, used for style:name
    XML_ATACTION_ENCODE_STYLE_NAME_REF, // NOP, used for style:name
    XML_ATACTION_RENAME_ENCODE_STYLE_NAME_REF,  // NOP, used for style:name
                                        // - param1: namespace +
                                        //           token of local name
                                        // - param2: style family
    XML_ATACTION_MOVE_TO_ELEM,          // turn attr into an elem
                                        // - param1: namespace +
                                        //           token of local name
    XML_ATACTION_MOVE_FROM_ELEM,        // turn elem into an attr:
                                        // - param1: namespace +
                                        //           token of local name
    XML_ATACTION_NEG_PERCENT,           // replace % val with 100-%
    XML_ATACTION_RENAME_NEG_PERCENT,    // replace % val with 100-%, rename attr
                                        // - param1: namespace +
                                        //           token of local name
    XML_ATACTION_HREF,                  // xmlink:href
    XML_ATACTION_ADD_NAMESPACE_PREFIX,  // add a namespace prefix
                                        // - param1: prefix
    XML_ATACTION_ADD_APP_NAMESPACE_PREFIX,  // add a namespace prefix
                                        // - param1: default prefix
    XML_ATACTION_RENAME_ADD_NAMESPACE_PREFIX,   // add a namespace prefix
                                        // - param1: namespace +
                                        //           token of local name
                                        // - param2: prefix
    XML_ATACTION_REMOVE_NAMESPACE_PREFIX,// remove a namespace prefix
                                        // - param1: prefix
    XML_ATACTION_REMOVE_ANY_NAMESPACE_PREFIX,// remove any namespace prefix
    XML_ATACTION_RENAME_REMOVE_NAMESPACE_PREFIX,// remove a namespace prefix
                                        // - param1: namespace +
                                        //           token of local name
                                        // - param2: prefix
    XML_ATACTION_EVENT_NAME,
    XML_ATACTION_MACRO_NAME,
    XML_ATACTION_MACRO_LOCATION,
    XML_ATACTION_URI_OOO,                // an URI in OOo notation
                                         // - param1: pacakage URI are supported
    XML_ATACTION_URI_OASIS,              // an URI in OASIS notation
                                         // - param1: pacakage URI are supported
    XML_ATACTION_USER_DEFINED=0x80000000,// user defined actions start here
    XML_ATACTION_END=XML_TACTION_END
};

#endif  //  _XMLOFF_ATTRTRANSFORMERACTION_HXX
