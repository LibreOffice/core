/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hfi_xrefpage.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:50:08 $
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

#ifndef ADC_DISPLAY_HFI_XREFPAGE_HXX
#define ADC_DISPLAY_HFI_XREFPAGE_HXX



// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS


class HF_IdlXrefs : public HtmlFactory_Idl
{
  public:
    typedef void (*F_GET_SUBLIST)(dyn_ce_list&, const client&);

                        HF_IdlXrefs(
                            Environment &       io_rEnv,
                            Xml::Element &      o_rOut,
                            const String &      i_prefix,
                            const client &      i_ce);
    virtual             ~HF_IdlXrefs();

    /** @descr
        Only lists which are tried to be produced by Produce_List() or
        Produce_Tree(), will occur in the content directory of the page.
        They will have links, if the list or tree has at least one element,
        else the list is mentioned in the directory without link.

        @param i_label [*i_label == '#']
    */
    void                Produce_List(
                            const char *        i_title,
                            const char *        i_label,
                            ce_list &           i_iterator ) const;
    void                Write_ManualLinks(
                            const client &      i_ce ) const;
    /** @descr
        Only lists which are tried to be produced by Produce_List() or
        Produce_Tree(), will occur in the content directory of the page.
        They will have links, if the list or tree has at least one element,
        else the list is mentioned in the directory without link.

        @param i_label [*i_label == '#']
    */
    void                Produce_Tree(
                            const char *        i_title,
                            const char *        i_label,
                            const client &      i_ce,
                            F_GET_SUBLIST       i_sublistcreator ) const;

  private:
    // Locals
    void                produce_Main(
                            const String &      i_prefix,
                            const client &      i_ce ) const;
    void                make_Navibar(
                            const client &      i_ce ) const;
    /// @return true if there are any elements in sub lists.
    void                recursive_make_ListInTree(
                            Xml::Element &      o_rDisplay,
                            uintt               i_level,    /// 0 is highest
                            const client &      i_ce,
                            ce_list &           i_iterator,
                            F_GET_SUBLIST       i_sublistcreator ) const;

    // DATA
    Xml::Element &      rContentDirectory;
    const client *      pClient;
};



// IMPLEMENTATION

#endif
