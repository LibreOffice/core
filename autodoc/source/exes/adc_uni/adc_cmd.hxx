/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
