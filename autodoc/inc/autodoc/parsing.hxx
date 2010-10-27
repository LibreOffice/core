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

#ifndef AUTODOC_PARSING_HXX
#define AUTODOC_PARSING_HXX



namespace autodoc
{

class CodeParser_Ifc;
class DocumentationParser_Ifc;
class FileCollector_Ifc;


/** Interface for parsing code of a programming language and
    delivering the information into an Autodoc Repository.
**/
class ParseToolsFactory_Ifc
{
  public:
    virtual             ~ParseToolsFactory_Ifc() {}
    static ParseToolsFactory_Ifc &
                        GetIt_();

    virtual DYN autodoc::CodeParser_Ifc *
                        Create_Parser_Cplusplus() const = 0;

//  virtual DYN autodoc::CodeParser_Ifc *
//                      CreateParser_UnoIDL() const = 0;
//  virtual DYN autodoc::CodeParser_Ifc *
//                      CreateParser_Java() const = 0;
//  virtual DYN autodoc::CodeParser_Ifc *
//                      CreateParser_StoredProcedures() const = 0;

    virtual DYN autodoc::DocumentationParser_Ifc *
                        Create_DocuParser_AutodocStyle() const = 0;

//  virtual DYN autodoc::DocumentationParser_Ifc *
//                      CreateDocuParser_StarOfficeAPIStyle() const = 0;

    virtual DYN autodoc::FileCollector_Ifc *
                        Create_FileCollector(
                            uintt               i_nEstimatedNrOfFiles ) const = 0;  /// Should be somewhat larger than the maximal estimated number of files.
};


} // namespace autodoc



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
