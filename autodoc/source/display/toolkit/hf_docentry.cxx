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

#include <precomp.h>
#include <toolkit/hf_docentry.hxx>


// NOT FULLY DEFINED SERVICES


HF_DocEntryList::HF_DocEntryList( Xml::Element & o_out )
    :   HtmlMaker( o_out >>* new Html::DefList )
{
}

HF_DocEntryList::~HF_DocEntryList()
{
}

Xml::Element &
HF_DocEntryList::Produce_Term(const char * i_sTerm )
{
    Xml::Element &
        ret = CurOut()
           >> *new Html::DefListTerm
              >> *new Html::Bold;
    if ( NOT csv::no_str(i_sTerm))
        ret
            << i_sTerm;
    return ret;
}

Xml::Element &
HF_DocEntryList::Produce_NormalTerm(const char * i_sTerm)
{
    Xml::Element &
        ret = CurOut()
           >> *new Html::DefListTerm;
    if ( NOT csv::no_str(i_sTerm))
        ret
            << i_sTerm;
    return ret;
}

Xml::Element &
HF_DocEntryList::Produce_Definition()
{
    return CurOut()
           >> *new Html::DefListDefinition;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
