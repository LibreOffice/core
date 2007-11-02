/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inftypes.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:16:13 $
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

