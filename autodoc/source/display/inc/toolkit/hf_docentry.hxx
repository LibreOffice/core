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

#ifndef ADC_DISPLAY_HF_DOCENTRY_HXX
#define ADC_DISPLAY_HF_DOCENTRY_HXX



// USED SERVICES
    // BASE CLASSES
#include "htmlfactory.hxx"
    // COMPONENTS
    // PARAMETERS

/** @resp
    Produces a list of <DT>..</DT> and <DD>.
*/
class HF_DocEntryList : public HtmlMaker
{
  public:

                        HF_DocEntryList(
                            Xml::Element &      o_rOut );
    virtual             ~HF_DocEntryList();

    Xml::Element &      Produce_Term(
                            const char *        i_sTerm = 0 );
    Xml::Element &      Produce_NormalTerm(     /// Font will not be bold.
                            const char *        i_sTerm = 0 );
    Xml::Element &      Produce_Definition();
};



#endif
