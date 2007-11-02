/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: d_types4doc.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:01:20 $
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

#ifndef ARY_DOC_D_TYPES4DOC_HXX
#define ARY_DOC_D_TYPES4DOC_HXX

// USED SERVICES



namespace ary
{
namespace doc
{


/** Type of a documentation: multiple lines or single line.
*/
enum E_BlockType
{
    dbt_none = 0,
    dbt_multiline,
    dbt_singleline
};

/** Type of documentation text: with html or without.
*/
enum E_TextType
{
    dtt_none = 0,
    dtt_plain,
    dtt_html
};

namespace nodetype
{

typedef int id;

}   //  namespace nodetype



}   //  namespace doc
}   //  namespace ary
#endif
