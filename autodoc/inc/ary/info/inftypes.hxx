/*************************************************************************
 *
 *  $RCSfile: inftypes.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:12:13 $
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

#ifndef ARY_INFO_INFTYPES_HXX
#define ARY_INFO_INFTYPES_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace info
{




typedef uintt GlobalTextId;



/** Because this enum is used as index list for displayed
    tag headlines, the items must neither be moved nor deleted.
    Only adding to the end is allowed. atid_MAX always has to exist
    and to be the last used value.
    Also assigning numbers to the values is forbidden.
*/
enum E_AtTagId
{
    atid_ATT = 0,
    atid_author,
    atid_change,
    atid_collab,
    atid_contact,

    atid_copyright,
    atid_deprecated,
    atid_descr,
    atid_docdate,
    atid_derive,

    atid_dyn,
    atid_instance,
    atid_interface,
    atid_invariant,
    atid_life,

    atid_multi,
    atid_onerror,
    atid_persist,
    atid_postcond,
    atid_precond,

    atid_resp,
    atid_return,
    atid_short,
    atid_todo,
    atid_version,

    atid_since,
    atid_MAX
};

/** Because this enum is used as index list for displayed
    tag headlines, the items must neither be moved nor deleted.
    Only adding to the end is allowed. C_eAtTag_NrOfClasses always has to exist
    and to be the last used value.
    Also assigning other numbers to the values, than in this
    existing scheme, is forbidden.
*/
enum E_AtTagClass
{
    atc_std = 0,

    atc_base = atid_MAX,
    atc_exception = atid_MAX + 1,
    atc_implements = atid_MAX + 2,
    atc_keyword = atid_MAX + 3,
    atc_parameter = atid_MAX + 4,

    atc_see = atid_MAX + 5,
    atc_template = atid_MAX + 6,
    atc_label = atid_MAX + 7,
    atc_since = atid_MAX + 8,
    C_eAtTag_NrOfClasses
};



}   // namespace info
}   // namespace ary


#endif

