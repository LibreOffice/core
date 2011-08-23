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

#ifndef ADC_DISPLAY_HTML_PAGEMAKE_HXX
#define ADC_DISPLAY_HTML_PAGEMAKE_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/ary_disp.hxx>
#include <cosv/tpl/processor.hxx>
#include "hdimpl.hxx"
    // COMPONENTS
    // PARAMETERS
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
    namespace loc
    {
        class File;
    }
}


class OuputPage_Environment;
class HtmlDocuFile;



class PageDisplay : public ary::Display,
                    public csv::ConstProcessor<ary::cpp::Class>,
                    public csv::ConstProcessor<ary::cpp::Enum>,
                    public csv::ConstProcessor<ary::cpp::Typedef>,
                    public HtmlDisplay_Impl
{
  public:
                        PageDisplay(
                            OuputPage_Environment &
                                                io_rEnv );
    virtual				~PageDisplay();

    void                Create_OverviewFile();
    void                Create_AllDefsFile();
    void                Create_IndexFiles();
    void                Create_HelpFile();

    void                Create_NamespaceFile();

    void                Setup_OperationsFile_for(
                            const ary::loc::File &
                                                i_rFile );
    void                Setup_OperationsFile_for(
                            const ary::cpp::Class &
                                                i_rClass );
    void                Setup_DataFile_for(
                            const ary::loc::File &
                                                i_rFile );
    void                Setup_DataFile_for(
                            const ary::cpp::Class &
                                                i_rClass );
    /// Used with Setup_OperatonsFile_for().
    void                Create_File();


    // Interface for Children of SpecializedPageMaker:
    void                Write_NameChainWithLinks(
                            const ary::cpp::CodeEntity &
                                                i_rCe );

    // Necessary, to call Process() on this class.
    using csv::ConstProcessor<ary::cpp::Class>::Process;
    using csv::ConstProcessor<ary::cpp::Enum>::Process;
    using csv::ConstProcessor<ary::cpp::Typedef>::Process;

 private:
    // Interface csv::ConstProcessor<>:
    virtual void	  	do_Process(
                            const ary::cpp::Class &
                                                i_rData );
    virtual void	  	do_Process(
                            const ary::cpp::Enum &
                                                i_rData );
    virtual void	  	do_Process(
                            const ary::cpp::Typedef &
                                                i_rData );
    // Interface ary::cpp::Display:
    virtual const ary::cpp::Gate *
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
    Dyn<HtmlDocuFile>	pMyFile;
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
