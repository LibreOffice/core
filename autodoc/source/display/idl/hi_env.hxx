/*************************************************************************
 *
 *  $RCSfile: hi_env.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:31:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef ADC_DISPLAY_HI_ENV_HXX
#define ADC_DISPLAY_HI_ENV_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <cosv/ploc.hxx>
#include <cosv/ploc_dir.hxx>
    // PARAMETERS
#include <toolkit/out_position.hxx>

namespace ary
{
namespace idl
{
    class Gate;
    class CodeEntity;
}
}
namespace display
{
    class CorporateFrame;
}
namespace output
{
    class Tree;
}

class AryAccess;
class LinkHelper;

/** @resp
    Provides enviroment information to the HTML factory
    classes.

    @descr
    All information that is not included in the data, especially
    about the layout of the output tree and the access to
    information from the repository are provided here.

    @see HtmlFactory
*/
class HtmlEnvironment_Idl
{
  public:
    // LIFECYCLE
                        HtmlEnvironment_Idl(
                            const csv::ploc::Path &
                                                io_rOutputDir,
                            const ary::idl::Gate &
                                                i_rGate,
                            const display::CorporateFrame &
                                                i_rLayout );
                        ~HtmlEnvironment_Idl();

    // OPERATIONS
    void                Goto_Directory(
                            output::Position    i_pos,
                            bool                i_bCreateDirectoryIfNecessary );
    void                Goto_DirectoryLevelDown(
                            const String &      i_subDirName,
                            bool                i_bCreateDirectoryIfNecessary );
    void                Goto_DirectoryLevelUp();
    void                Set_CurFile(
                            const String &      i_fileName );
    void                Set_CurPageCe(
                            const ary::idl::CodeEntity *
                                                i_ce )
                                                { pCurPageCe = i_ce; }
    // INQUIRY
    const ary::idl::Gate &
                        Gate() const            { return *pGate; }
    const AryAccess &   Data() const            { return *pData; }
    const char *        Link2Manual(
                            const String &      i_link ) const;

    /// This may be reimplemented for removing dead links to members.
    bool                Is_MemberExistenceCheckRequired() const
                                                { return false; }

    /// @return Holds only the current directory, not the current file.
    output::Position &  CurPosition() const     { return aCurPosition; }
    void                Get_CurFilePath(
                            StreamStr &         o_buffer ) const
                                                { o_buffer << aCurPath; }

    const display::CorporateFrame &
                        Layout() const          { return *pLayout; }
    const LinkHelper &  Linker() const          { return *pLinker; }

    void                Get_LinkTo(
                            StreamStr &         o_result,
                            output::Position    i_destination )
                                                { CurPosition().Get_LinkTo(o_result, i_destination); }
    String              CurPageCe_AsText() const;
    const ary::idl::CodeEntity *
                        CurPageCe() const       { return pCurPageCe; }

    // ACCESS
    output::Tree &      OutputTree()            { return *pOutputTree; }

  private:
    // Local
    void                create_Directory(
                            const csv::ploc::Path &
                                                i_path );

    // DATA
    csv::ploc::Directory
                        aOutputRoot;
    csv::ploc::Path     aCurPath;

    Dyn<AryAccess>      pData;          /// @invariant *pData is valid.
    const ary::idl::Gate *
                        pGate;          /// @invariant pGate != 0.
    Dyn<output::Tree>   pOutputTree;    /// @invariant *pOutputTree is valid.
    mutable output::Position
                        aCurPosition;
    const ary::idl::CodeEntity *
                        pCurPageCe;

    const display::CorporateFrame *
                        pLayout;

    Dyn<LinkHelper>     pLinker;
};


#endif


