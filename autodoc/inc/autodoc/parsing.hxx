/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: parsing.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:26:58 $
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

