/*************************************************************************
 *
 *  $RCSfile: pagemake.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:24 $
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

#ifndef ADC_DISPLAY_HTML_PAGEMAKE_HXX
#define ADC_DISPLAY_HTML_PAGEMAKE_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/cpp_disp.hxx>
#include "hdimpl.hxx"
    // COMPONENTS
    // PARAMETERS
#include <ary/ids.hxx>
#include <ary/cpp/c_namesp.hxx>

namespace ary
{
    namespace cpp
    {
        class Namespace;
        class Class;
        class Enum;
        class Typedef;
    }
}


class OuputPage_Environment;
class HtmlDocuFile;


class PageDisplay : public ary::cpp::Display,
                    public HtmlDisplay_Impl
{
  public:
                        PageDisplay(
                            OuputPage_Environment &
                                                io_rEnv );
    virtual             ~PageDisplay();

    void                Create_OverviewFile();
    void                Create_AllDefsFile();
    void                Create_IndexFiles();
    void                Create_HelpFile();

    void                Create_NamespaceFile();

    void                Setup_OperationsFile_for(
                            const ary::cpp::FileGroup &
                                                i_rFile );
    void                Setup_OperationsFile_for(
                            const ary::cpp::Class &
                                                i_rClass );
    void                Setup_DataFile_for(
                            const ary::cpp::FileGroup &
                                                i_rFile );
    void                Setup_DataFile_for(
                            const ary::cpp::Class &
                                                i_rClass );
    /// Used with Setup_OperatonsFile_for().
    void                Create_File();

    // Interface ary::cpp::Display
    virtual void        Display_Class(
                            const ary::cpp::Class &
                                                i_rData );
    virtual void        Display_Enum(
                            const ary::cpp::Enum &
                                                i_rData );
    virtual void        Display_Typedef(
                            const ary::cpp::Typedef &
                                                i_rData );

    // Interface for Children of SpecializedPageMaker:
    void                Write_NameChainWithLinks(
                            const ary::CodeEntity &
                                                i_rCe );
 private:
    // Interface ary::cpp::Display:
    virtual const ary::DisplayGate *
                        inq_Get_ReFinder() const;
    // Locals
    HtmlDocuFile &      File()                  { return *pMyFile; }
    void                RecursiveWrite_NamespaceLink(
                            const ary::cpp::Namespace *
                                                i_pNamespace );
    void                RecursiveWrite_ClassLink(
                            const ary::cpp::Class *
                                                i_pClass,
                            uintt               i_nLevelDistance );
    void                SetupFileOnCurEnv(
                            const char *        i_sTitle );
    void                Write_NavBar_Enum(
                            const ary::cpp::Enum &
                                                i_rData );
    void                Write_TopArea_Enum(
                            const ary::cpp::Enum &
                                                i_rData );
    void                Write_DocuArea_Enum(
                            const ary::cpp::Enum &
                                                i_rData );
    void                Write_ChildList_Enum(
                            const ary::cpp::Enum &
                                                i_rData );
    void                Write_NavBar_Typedef(
                            const ary::cpp::Typedef &
                                                i_rData );
    void                Write_TopArea_Typedef(
                            const ary::cpp::Typedef &
                                                i_rData );
    void                Write_DocuArea_Typedef(
                            const ary::cpp::Typedef &
                                                i_rData );
    void                Create_IndexFile(
                            int                 i_nLetter );

    // DATA
    Dyn<HtmlDocuFile>   pMyFile;
};



#endif

