/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: adc_cmd_parse.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:04:39 $
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

#ifndef ADC_ADC_CMD_PARSE_HXX
#define ADC_ADC_CMD_PARSE_HXX



// USED SERVICES
    // BASE CLASSES
#include "adc_cmd.hxx"
    // COMPONENTS
#include <cosv/ploc.hxx>
    // PARAMETERS

namespace autodoc
{
namespace command
{

/** A command context which holds the currently parsed programing language
    and its valid file extensions.
*/
struct S_LanguageInfo : public Context
{
    enum E_ProgrammingLanguage
    {
        none,
        cpp,
        idl,
        java
    };
                        S_LanguageInfo()
                            :   eLanguage(none),
                                aExtensions() {}
                        ~S_LanguageInfo();

    void                InitExtensions(
                            opt_iter &          it,
                            opt_iter            itEnd );
    // DATA
    E_ProgrammingLanguage
                        eLanguage;
    StringVector        aExtensions;        // An empty string is possible and means exactly that: files without extension.

  private:
    // Interface Context:
    virtual void        do_Init(
                            opt_iter &          it,
                            opt_iter            itEnd );
};


class S_ProjectData;


/** A command that parses source code into the Autodoc Repository.
*/
class Parse : public Command
{
  public:
    typedef std::vector< DYN S_ProjectData * >  ProjectList;
    typedef ProjectList::const_iterator         ProjectIterator;

                        Parse();
                        ~Parse();

    // INQUIRY
    const String &      ReposyName() const;
    const S_LanguageInfo &
                        GlobalLanguage() const;
    ProjectIterator     ProjectsBegin() const;
    ProjectIterator     ProjectsEnd() const;
    const String &      DevelopersManual_RefFilePath() const
                                                { return sDevelopersManual_RefFilePath; }

  private:
    // Interface Context:
    virtual void        do_Init(
                            opt_iter &          i_nCurArgsBegin,
                            opt_iter            i_nEndOfAllArgs );
    // Interface Command:
    virtual bool        do_Run() const;
    virtual int         inq_RunningRank() const;

    // Locals
    void                do_clName(
                            opt_iter &          it,
                            opt_iter            itEnd );
    void                do_clDevManual(
                            opt_iter &          it,
                            opt_iter            itEnd );
    void                do_clProject(
                            opt_iter &          it,
                            opt_iter            itEnd );
    void                do_clDefaultProject(
                            opt_iter &          it,
                            opt_iter            itEnd );

    // DATA
    String              sRepositoryName;
    S_LanguageInfo      aGlobalLanguage;

    ProjectList         aProjects;

    String              sDevelopersManual_RefFilePath;
};

inline const String &
Parse::ReposyName() const
    { return sRepositoryName; }
inline const S_LanguageInfo &
Parse::GlobalLanguage() const
    { return aGlobalLanguage; }
inline Parse::ProjectIterator
Parse::ProjectsBegin() const
    { return aProjects.begin(); }
inline Parse::ProjectIterator
Parse::ProjectsEnd() const
    { return aProjects.end(); }
//inline const String &
//Parse::DevelopersManual_RefFilePath() const
//    { return sDevelopersManual_RefFilePath; }
//inline const String &
//Parse::DevelopersManual_HtmlRoot() const
//    { return sDevelopersManual_HtmlRoot; }


struct S_Sources : public Context
{
    StringVector        aTrees;
    StringVector        aDirectories;
    StringVector        aFiles;

  private:
    // Interface Context:
    virtual void        do_Init(
                            opt_iter &          it,
                            opt_iter            itEnd );
};

class S_ProjectData : public Context
{
  public:
    enum E_Default { default_prj };

                        S_ProjectData(
                            const S_LanguageInfo &
                                                i_globalLanguage );
                        S_ProjectData(
                            const S_LanguageInfo &
                                                i_globalLanguage,
                            E_Default           unused );
                        ~S_ProjectData();

    bool                IsDefault() const       { return bIsDefault; }
    const String &      Name() const            { return sName; }
    const csv::ploc::Path &
                        RootDirectory() const   { return aRootDirectory; }
    const S_LanguageInfo &
                        Language() const        { return aLanguage; }
    const S_Sources     Sources() const         { return aFiles; }

  private:
    // Interface Context:
    virtual void        do_Init(
                            opt_iter &          it,
                            opt_iter            itEnd );
    // Locals

    // DATA
    String              sName;
    csv::ploc::Path     aRootDirectory;
    S_LanguageInfo      aLanguage;
    S_Sources           aFiles;
    bool                bIsDefault;
};


}   // namespace command
}   // namespace autodoc


#endif
