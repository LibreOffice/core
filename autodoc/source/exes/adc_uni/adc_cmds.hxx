/*************************************************************************
 *
 *  $RCSfile: adc_cmds.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-12 11:06:13 $
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

#ifndef ADC_ADC_CMDS_HXX
#define ADC_ADC_CMDS_HXX



// USED SERVICES
    // BASE CLASSES
#include "adc_cmd.hxx"
    // COMPONENTS
    // PARAMETERS

namespace autodoc
{
namespace command
{


/** A command that produces HTML output from the Autodoc Repository.
*/
class CreateHtml : public Command
{
  public:
                        CreateHtml();
                        ~CreateHtml();

    const String &      OutputDir() const;
    const String &      DevelopersManual_HtmlRoot() const
                                                { return sDevelopersManual_HtmlRoot; }

  private:
    // Interface Context:
    virtual void        do_Init(
                            opt_iter &          i_nCurArgsBegin,
                            opt_iter            i_nEndOfAllArgs );
    // Interface Command:
    virtual bool        do_Run() const;
    virtual int         inq_RunningRank() const;

    // Locals
    void                run_Cpp() const;
    void                run_Idl() const;

    // DATA
    String              sOutputRootDirectory;
    String              sDevelopersManual_HtmlRoot;
    bool                bSimpleLinks;
};

inline const String &
CreateHtml::OutputDir() const
    { return sOutputRootDirectory; }


#if 0 // KORR_FUTUREs
//class CreateXml : public Command
//{
//  public:
//                        CreateXml();
//                        ~CreateXml();
//
//    const String &     OutputDir() const;
//
//  private:
//    // Interface Command:
//    virtual char * *    do_Init(
//                            opt_iter &          i_nCurArgsBegin,
//                            opt_iter            i_nEndOfAllArgs );
//    // DATA
//    String              sOutputRootDirectory;
//};
//
//inline const String &
//CreateXml::OutputDir() const
//    { return sOutputRootDirectory; }
//
//class Load : public Command
//{
//  public:
//                        Load(
//                            const char *        i_sRepositoryDirectory = "" );
//                        ~Load();
//
//    const String &      ReposyDir() const;
//
//  private:
//    // Interface Command:
//    virtual char * *    do_Init(
//                            opt_iter &          i_nCurArgsBegin,
//                            opt_iter            i_nEndOfAllArgs );
//    virtual void        do_Run() const;
//
//    // DATA
//    String              sRepositoryDirectory;
//};
//
//inline const String &
//Load::ReposyDir() const
//    { return sRepositoryDirectory; }
//
//class Save : public Command
//{
//  public:
//                        Save();
//                        ~Save();
//
//    const String &      ReposyDir() const;
//
//  private:
//    // Interface Command:
//    virtual char * *    do_Init(
//                            opt_iter &          i_nCurArgsBegin,
//                            opt_iter            i_nEndOfAllArgs );
//    // DATA
//    String              sRepositoryDirectory;
//};
//
//inline const String &
//Save::ReposyDir() const
//    { return sRepositoryDirectory; }
#endif // 0  - KORR_FUTURE




extern const String C_opt_Verbose;

extern const String C_opt_Parse;
extern const String C_opt_Name;
extern const String C_opt_LangAll;
extern const String C_opt_ExtensionsAll;
extern const String C_opt_DevmanFile;

extern const String C_arg_Cplusplus;
extern const String C_arg_Idl;
extern const String C_arg_Java;

extern const String C_opt_Project;
//extern const String C_opt_Lang;
//extern const String C_opt_Extensions;
extern const String C_opt_SourceTree;
extern const String C_opt_SourceDir;
extern const String C_opt_SourceFile;

extern const String C_opt_CreateHtml;
extern const String C_opt_DevmanRoot;
extern const String C_opt_SimpleLinks;

//extern const String C_opt_CreateXml;
//extern const String C_opt_Load;
//extern const String C_opt_Save;


inline void
CHECKOPT( bool b, const char * miss, const String & opt )
{
    if ( NOT b )
    {
        StreamLock slMsg(100);
        throw X_CommandLine( slMsg() << "Missing " << miss <<" after " << opt << "." << c_str );
    }
}

}   // namespace command
}   // namespace autodoc


#endif

