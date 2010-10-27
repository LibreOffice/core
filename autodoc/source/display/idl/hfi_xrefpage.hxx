/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
