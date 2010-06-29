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

#ifndef ADC_ADC_CMD_HXX
#define ADC_ADC_CMD_HXX



// USED SERVICES
    // BASE CLASSES
#include <cosv/comdline.hxx>
    // COMPONENTS
    // PARAMETERS


namespace autodoc
{
namespace command
{

/** Context for a command, which can be read from the command line.
*/
class Context
{
  public:
    typedef StringVector::const_iterator opt_iter;

    virtual             ~Context() {}

    void                Init(
                            opt_iter &          it,
                            opt_iter            itEnd );
  private:
    virtual void        do_Init(
                            opt_iter &          it,
                            opt_iter            itEnd ) = 0;
};

// IMPLEMENTATION
inline void
Context::Init( opt_iter &          i_nCurArgsBegin,
               opt_iter            i_nEndOfAllArgs )

{ do_Init(i_nCurArgsBegin, i_nEndOfAllArgs); }



/** Interface for commands, autodoc is able to perform.
*/
class Command : public Context
{
  public:
    /** Running ranks of the commands are to be maintained at one location:
        Here!
    */
    enum E_Ranks
    {
        rank_Load       = 10,
        rank_Parse      = 20,
        rank_Save       = 30,
        rank_CreateHtml = 40,
        rank_CreateXml  = 50
    };


    bool                Run() const;
    int                 RunningRank() const;

  private:
    virtual bool        do_Run() const = 0;
    virtual int         inq_RunningRank() const = 0;
};

// IMPLEMENTATION
inline bool
Command::Run() const
{ return do_Run(); }
inline int
Command::RunningRank() const
{ return inq_RunningRank(); }




/** The exception thrown, if the command line is invalid.
*/
class X_CommandLine
{
  public:
                        X_CommandLine(
                            const char *        i_sExplanation )
                            :   sExplanation(i_sExplanation) {}

    void                Report(
                            std::ostream &      o_rOut )
                            { o_rOut << "Error in command line: "
                                     << sExplanation << Endl(); }
  private:
    String              sExplanation;
};




}   // namespace command
}   // namespace autodoc
#endif
