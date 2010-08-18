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

