/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: comdline.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:37:43 $
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

#ifndef CSV_COMDLINE_HXX
#define CSV_COMDLINE_HXX
//  KORR_DEPRECATED_3.0
//      Replace by cosv/commandline.hxx.


namespace csv
{

class CommandLine_Ifc
{
  public:
    virtual             ~CommandLine_Ifc() {}

    void                Init(
                            int                 argc,
                            char *              argv[] );
    void                PrintUse() const;
    bool                CheckParameters() const;

  private:
    virtual void        do_Init(
                            int                 argc,
                            char *              argv[] ) = 0;

    virtual void        do_PrintUse() const = 0;
    virtual bool        inq_CheckParameters() const = 0;
};

inline void
CommandLine_Ifc::Init( int                 argc,
                       char *              argv[] )
    { do_Init( argc, argv ); }
inline void
CommandLine_Ifc::PrintUse() const
    { do_PrintUse(); }

}   // namespace csv



#endif

