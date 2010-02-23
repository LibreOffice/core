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
