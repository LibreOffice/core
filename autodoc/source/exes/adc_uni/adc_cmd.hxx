/*************************************************************************
 *
 *  $RCSfile: adc_cmd.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-11-14 18:02:00 $
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
                            ostream &           o_rOut )
                            { o_rOut << "Error in command line: "
                                     << sExplanation << Endl(); }
  private:
    String              sExplanation;
};





}   // namespace command
}   // namespace autodoc


#endif

