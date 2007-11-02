/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: a_docuaccess.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:45:08 $
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

#ifndef ARY_ADOC_DOCUACCESS_HXX
#define ARY_ADOC_DOCUACCESS_HXX



// USED SERVICES
    // BASE CLASSES
    // OTHER
#include <ary/doc/d_hypertext.hxx>


namespace ary
{
namespace doc
{
    class Documentation;
    class NamedList;
    class Struct;
}
}


namespace ary
{
namespace adoc
{

enum E_LifeCycle
{
    lc_unkown,
    lc_owned,
    lc_extern,
    lc_refcounted
};


class TypeNameReceiver;



/** Semantic access to a documentation whose content follows the rules of
    autodoc standard documentation.
*/
class DocuAccess
{
  public:
    enum E_TagId
    {
        ti_none = 0,

        ti_attention,
        ti_author,
        ti_base,
        ti_change,
        ti_collab,
        ti_deprecated,
        ti_derive,
        ti_descr,
        ti_docdate,
        ti_exception,
        ti_instance,
        ti_internal,
        ti_invariant,
        ti_key,
        ti_life,
        ti_multi,
        ti_onerror,
        ti_param,
        ti_persistence,
        ti_postcond,
        ti_precond,
        ti_published,
        ti_resp,
        ti_return,
        ti_returns,
        ti_see,
        ti_short,
        ti_since,
        ti_summary,
        ti_throws,
        ti_todo,
        ti_tpl,
        ti_version,
        ti_ATTENTION,
        ti_UnknownAtTag,
        ti_DocuReference,
        ti_NVI,

        ti_extra,               /// Tag that is not known.

        ti_MAX                  /// Keep this the highest value.
    };

    explicit            DocuAccess(
                            ary::doc::Documentation *
                                                io_docu = 0 );
    ary::doc::Documentation *
                        Assign_Container(       /// @return Previously assigned container.
                            ary::doc::Documentation *
                                                io_docu );
    ary::doc::Documentation *
                        Release_Container();

    void                Set_Deprecated();
    void                Set_Internal();
    void                Set_Published();

    ary::doc::HyperText &
                        Add_Description();

    ary::doc::HyperText &
                        Add_StdTag(
                            E_TagId             i_tag );
    ary::doc::HyperText &
                        Add_base(
                            const String &      i_baseTypeName );
    ary::doc::HyperText &
                        Add_collab(
                            const String &      i_partner );
    ary::doc::HyperText &
                        Add_key(
                            const String &      i_keyWord );
    ary::doc::HyperText &
                        Add_param(
                            const String &      i_parameterName,
                            const String &      i_validRange );
    ary::doc::HyperText &
                        Add_see(
                            const String &      i_referencedEntity );
    ary::doc::HyperText &
                        Add_since(
                            const String &      i_versionId,
                            const String &      i_fullVersionText );
    ary::doc::HyperText &
                        Add_throws(
                            const String &      i_exceptionName );
    ary::doc::HyperText &
                        Add_tpl(
                            const String &      i_templateParameter );
    ary::doc::HyperText &
                        Add_ExtraTag(
                            const String &      i_tagName );
    ary::doc::HyperText &
                        Add_TagWithMissingParameter(
                            E_TagId             i_tag );

  private:
    ary::doc::Struct &  Data();
    ary::doc::Struct &  Get_Data();
    ary::doc::NamedList &
                        Get_List(
                            E_TagId             i_tag );
    // DATA
    ary::doc::Documentation *
                        pContainer;
    ary::doc::Struct *  pData;
};




}   // namespace adoc
}   // namespace ary
#endif
