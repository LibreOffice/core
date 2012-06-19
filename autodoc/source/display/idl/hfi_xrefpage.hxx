/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
