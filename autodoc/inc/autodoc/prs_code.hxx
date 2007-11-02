/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prs_code.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:21:50 $
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

#ifndef AUTODOC_PRS_CODE_HXX
#define AUTODOC_PRS_CODE_HXX



namespace csv
{
    namespace ploc
    {
        class Path;
    }
}

namespace ary
{
    class Repository;
}

namespace autodoc
{
    class FileCollector_Ifc;
    class DocumentationParser_Ifc;




/** Interface for parsing code of a programming language and
    delivering the information into an Autodoc Repository.
**/
class CodeParser_Ifc
{
  public:
    virtual             ~CodeParser_Ifc() {}

    virtual void        Setup(
                            ary::Repository &   o_rRepository,
                            const autodoc::DocumentationParser_Ifc &
                                                i_rDocumentationInterpreter ) = 0;

    virtual void        Run(
                            const autodoc::FileCollector_Ifc &
                                                i_rFiles ) = 0;
};




} // namespace autodoc
#endif
