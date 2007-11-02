/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: adc_cmd.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:43:04 $
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
