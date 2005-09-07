/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: parsefct.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:42:37 $
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
    virtual             ~ParseToolsFactory();

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

