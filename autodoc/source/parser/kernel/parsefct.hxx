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

#ifndef ADC_PARSER_PARSEFCT_HXX
#define ADC_PARSER_PARSEFCT_HXX


#include <autodoc/parsing.hxx>


/** Interface for parsing code of a programming language and
    delivering the information into an Autodoc Repository.
**/
class ParseToolsFactory : public autodoc::ParseToolsFactory_Ifc
{
  public:
                        ParseToolsFactory();
    virtual				~ParseToolsFactory();

    virtual DYN autodoc::CodeParser_Ifc *
                        Create_Parser_Cplusplus() const;
    virtual DYN autodoc::DocumentationParser_Ifc *
                        Create_DocuParser_AutodocStyle() const;
    virtual DYN autodoc::FileCollector_Ifc *
                        Create_FileCollector(
                            uintt               i_nEstimatedNrOfFiles ) const;
  private:
    static DYN ParseToolsFactory *
                        dpTheInstance_;

    friend class autodoc::ParseToolsFactory_Ifc;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
